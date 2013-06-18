//@@COPYRIGHT@@

// Command-line arguments (-* args only)
// The program name can be accessed as argv[-1]
static int argc;
static char **argv;

// Real argc (includes +* commands)
static int real_argc;

static inline void EarlyParseArgs()
{
	// Find the first arg which starts with + and hide all args after it
	real_argc = argc;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '+') {
			argc = i;
			break;
		}
	}
}

bool Engine::GetArg(const char *command, int numArgs, const char **buffer)
{
	for (int i = 1; i < argc - numArgs; i++) {
		if (!argv[i])
			continue;
		if (!stricmp(argv[i], command)) {
			// Validate arguments
			for (int j = i + 1; j < i + numArgs; j++) {
				if (!argv[j] || argv[j][0] == '-') {
					Warning("Invalid parameter usage: %s", command);
					return false;
				}
			}

			// Return arguments
			argv[i] = NULL;
			for (int j = 0; j < numArgs; j++) {
				if (buffer)
					buffer[j] = argv[i + j + 1];
				argv[i + j + 1] = NULL;
			}
			return true;
		}
	}

	return false;
}

const char *Engine::GetProgramName()
{
	return argv[0];
}

void Engine::RunArgs()
{
	// Check for unhandled arguments
	for (int i = 1; i < argc; i++) {
		if (argv[i])
			Warning("Unrecognized parameter: %s", argv[i]);
	}

	// Turn all +XXX arguments into commands and run them
	if (argc != real_argc) {
		int cmdStart = argc;
		argv[argc]++;
		for (int i = argc + 1; i <= real_argc; i++) {
			if (i == real_argc || argv[i][0] == '+') {
				CmdArgs args(i - cmdStart, argv + cmdStart);
				Cmd::RunArgs(&args);

				if (i != real_argc) {
					cmdStart = i;
					argv[i]++;
				}
			}
		}
	}
}

// "Real" main function
static void RealMain()
{
	// Parse arguments
	EarlyParseArgs();

	// Initialize any platform-specific stuff
	System::PlatformInit();

	// Initialize the memory system
	Memory::Init();

	// Initialize the math library
	Math::Init();

	// Initialize the thread system
	ThreadPool::Init();

	// Initialize the command system
	Cmd::Init();

	// Initialize the console system
	Log::EarlyInit();
	Terminal::Init();

	// Initialize the file system
	Filesystem::Init();

	// Initialize the log system
	Log::Init();

	// Call the client/server/editor initialization function, which will also
	// start their main loop.
	Engine::Init();
}

// Shut down everything
void Engine::Quit()
{
	Engine::Shutdown();
	Log::Shutdown();
	Filesystem::Shutdown();
	Terminal::Shutdown();

	// Exit
	exit(0);
}

// Program entry point
int main(int _argc, char *_argv[])
{
	// Save the arguments
	argc = _argc;
	argv = _argv;

	// Set up an exception handler around our main function
	System::SetupExceptionHandler(RealMain);

	// Should never return
	__unreachable();
}

const char *Engine::GetVersion()
{
	return VERSION;
}
