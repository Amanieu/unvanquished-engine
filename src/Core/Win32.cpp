//@@COPYRIGHT@@

// Windows-specific stuff

#ifdef _WIN32
static inline const char *GetExceptionString(DWORD code)
{
	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION:
		return "Access violation";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		return "Array bounds exceeded";
	case EXCEPTION_BREAKPOINT:
		return "Breakpoint was encountered";
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		return "Datatype misalignment";
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		return "Float: Denormal operand";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		return "Float: Divide by zero";
	case EXCEPTION_FLT_INEXACT_RESULT:
		return "Float: Inexact result";
	case EXCEPTION_FLT_INVALID_OPERATION:
		return "Float: Invalid operation";
	case EXCEPTION_FLT_OVERFLOW:
		return "Float: Overflow";
	case EXCEPTION_FLT_STACK_CHECK:
		return "Float: Stack check";
	case EXCEPTION_FLT_UNDERFLOW:
		return "Float: Underflow";
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		return "Illegal instruction";
	case EXCEPTION_IN_PAGE_ERROR:
		return "Page error";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		return "Integer: Divide by zero";
	case EXCEPTION_INT_OVERFLOW:
		return "Integer: Overflow";
	case EXCEPTION_INVALID_DISPOSITION:
		return "Invalid disposition";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		return "Noncontinuable exception";
	case EXCEPTION_PRIV_INSTRUCTION:
		return "Privileged instruction";
	case EXCEPTION_SINGLE_STEP:
		return "Single step";
	case EXCEPTION_STACK_OVERFLOW:
		return "Stack overflow";
	default:
		return "Unknown exception";
	}
}

#ifdef _MSC_VER

void System::SetupExceptionHandler(void (*func)())
{
	__try {
		func();
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		Error("Exception 0x%lx caught: %s", GetExceptionCode(), GetExceptionString(GetExceptionCode()));
	}
}

#elif __GNUC__

// Emulating SEH on MinGW

// Extend EXCEPTION_REGISTRATION to include a saved context
struct EXCEPTION_REGISTRATION_EXT {
	EXCEPTION_REGISTRATION_EXT* prev;
	PEXCEPTION_HANDLER handler;
	jmp_buf savedContext;
};

// Push and pop an exception registration
static inline void PushFrame(EXCEPTION_REGISTRATION_EXT *er)
{
#ifdef __i386__
	__asm__ __volatile__ ("movl %%fs:0, %0" : "=r" (er->prev));
	__asm__ __volatile__ ("movl %0, %%fs:0" : : "r" (er));
#else
	NT_TIB *teb = reinterpret_cast<NT_TIB *>(NtCurrentTeb());
	er->prev = teb->ExceptionList;
	teb->ExceptionList = er;
#endif
}
static inline void PopFrame(EXCEPTION_REGISTRATION_EXT *er)
{
#ifdef __i386__
	__asm__ __volatile__ ("movl %0, %%fs:0" : : "r" (er->prev));
#else
	NT_TIB *teb = reinterpret_cast<NT_TIB *>(NtCurrentTeb());
	teb->ExceptionList = er->prev;
#endif
}

static EXCEPTION_DISPOSITION ExceptionHandler(EXCEPTION_RECORD *ExceptionRecord,
                                              void *EstablisherFrame,
                                              CONTEXT *,
                                              void *)
{
	EXCEPTION_REGISTRATION_EXT *er = reinterpret_cast<EXCEPTION_REGISTRATION_EXT *>(EstablisherFrame);
	PopFrame(er);
	longjmp(er->savedContext, ExceptionRecord->ExceptionCode);
}

void System::SetupExceptionHandler(void func())
{
	EXCEPTION_REGISTRATION_EXT er;
	int exceptionCode;

	// Set up handler
	er.handler = ExceptionHandler;
	exceptionCode = setjmp(er.savedContext);
	if (exceptionCode) {
		// We caught an exception
		PopFrame(&er);
		Error("Exception 0x%x caught: %s", exceptionCode, GetExceptionString(exceptionCode));
	}

	// Run function
	PushFrame(&er);
	func();

	// If we reach this point, no exception has been caught.
	PopFrame(&er);
}

#else
#error "Not implemented for this compiler"
#endif

// Windows uses these events to notify apps of events like system shutdown,
// ctrl+c, closing the console, etc.
static BOOL WINAPI CtrlHandler(DWORD dwCtrlType)
{
	const char *string;

	switch (dwCtrlType) {
	case CTRL_C_EVENT:
		string = "Ctrl+C";
		break;
	case CTRL_BREAK_EVENT:
		string = "Ctrl+Break";
		break;
	case CTRL_CLOSE_EVENT:
		string = "Console closed";
		break;
	case CTRL_LOGOFF_EVENT:
		string = "User logout";
		break;
	case CTRL_SHUTDOWN_EVENT:
		string = "System shutdown";
		break;
	default:
		string = "Unknown";
		break;
	}

	Warning("System event %ld: %s", dwCtrlType, string);
	Cmd::ExecuteString("quit");
	return TRUE;
}

// Catch errors and produce a helpful message
static void ErrorHandler(printLevel_t level, const char *msg)
{
	if (level != PRINT_ERROR)
		return;

	// Show a message box with the error message and some instructions
	char buffer[MAX_PRINTMSG * 2];
	snprintf(buffer, sizeof(buffer), "An error occured:\n%s\n\n"
	         "See crashlog.txt for a copy of the console output.", msg);
	MessageBox(NULL, buffer, "Error", MB_OK | MB_ICONERROR);
}

void System::PlatformInit()
{
	// Set the title of the terminal (ignored if we don't have one)
	SetConsoleTitle("Unvanquished Console");

	// Set a handler for system events
	SetConsoleCtrlHandler(CtrlHandler, TRUE);

	// Set a handler for errors
	RegisterPrintHandler(ErrorHandler);
}

const char *System::Win32StrError(DWORD error)
{
	static char *message = NULL;

	// Free previous message
	if (message)
		LocalFree(message);

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, LANG_USER_DEFAULT, reinterpret_cast<char *>(&message), 0, NULL)) {
		message = static_cast<char *>(LocalAlloc(LMEM_FIXED, sizeof("Unknown error 0x12345678")));
		if (!message)
			Error("Out of memory");
		snprintf(message, sizeof("Unknown error 0x12345678"), "Unknown error 0x%08lx", error);
	}

	return message;
}

#endif
