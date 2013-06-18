//@@COPYRIGHT@@

// UNIX-specific stuff (everything except Windows)

#ifndef _WIN32
static __thread sigjmp_buf *savedContext = NULL;

void System::SetupExceptionHandler(void (*func)())
{
	sigjmp_buf ctx;
	int sigNum;

	// Set up the saved context
	savedContext = &ctx;
	sigNum = sigsetjmp(ctx, 1);
	if (sigNum) {
		// Signal caught
		savedContext = NULL;
		Error("Signal %d caught: %s", sigNum, strsignal(sigNum));
	}

	// Run the function
	func();

	// If we reach this point, no signal has been caught.
	savedContext = NULL;
}

void System::AttachSignalHandler(int signal, void (*handler)(int))
{
	sigset_t sigset;
	struct sigaction sa;

	sa.sa_flags = SA_RESTART;
	sa.sa_handler = handler;
	sigfillset(&sa.sa_mask);
	sigaction(signal, &sa, NULL);

	sigemptyset(&sigset);
	sigaddset(&sigset, signal);
	pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
}

static __noreturn void CrashSignalHandler(int sigNum)
{
	if (savedContext)
		siglongjmp(*savedContext, sigNum);
	else
		_exit(255);
}

static void QuitSignalHandler(int sigNum)
{
	static bool firstTime = true;
	if (!firstTime)
		CrashSignalHandler(sigNum);
	firstTime = false;
	Warning("Signal %d caught: %s", sigNum, strsignal(sigNum));
	Cmd::ExecuteString("quit");
}

#define addsignal(sig) \
	do { \
		sigaddset(&sigset, sig); \
		sigaction(sig, &sa, NULL); \
	} while (false)

void System::PlatformInit()
{
	// Set up for unicode terminal
	setlocale(LC_CTYPE, "");

	// Set up signal handlers
	sigset_t sigset;
	struct sigaction sa;
	sigemptyset(&sigset);
	sa.sa_flags = SA_RESTART;
	sigfillset(&sa.sa_mask);

	// Signals that we ignore.
	sa.sa_handler = SIG_IGN;

	// We use SIGUSR2 to get to the debugger if one is present, or else
	// just continue execution.
	addsignal(SIGUSR2);

	// If the process is backgrounded (running non interactively)
	// then SIGTTIN or SIGTOU is emitted, if not caught, turns into a SIGSTP.
	addsignal(SIGTTIN);
	addsignal(SIGTTOU);

	// Signals that can't be recovered from. For these we stop all running
	// threads and run the shutdown code.
	sa.sa_handler = CrashSignalHandler;
	addsignal(SIGILL);
	addsignal(SIGFPE);
	addsignal(SIGSEGV);
	addsignal(SIGABRT);
	addsignal(SIGBUS);
	addsignal(SIGTRAP);
	addsignal(SIGTERM);
	addsignal(SIGQUIT);

	// External signals which are non-fatal. We just run /quit so that we
	// exit on the next main loop iteration.
	sa.sa_handler = QuitSignalHandler;
	addsignal(SIGPIPE);
	addsignal(SIGINT);
	addsignal(SIGHUP);

	// The signal mask is inherited by all child threads
	pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
}

#endif
