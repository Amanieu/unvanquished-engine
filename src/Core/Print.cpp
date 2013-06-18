//@@COPYRIGHT@@

static std::mutex printLock;
static std::vector<printHandler_t> printHandlers;
// FIXME: Cvar
//static Cvar com_debug("com_debug", 0, "0", "Enables debug messages");

// Exception class thrown by Error()
class ErrorException: public std::runtime_error {
public:
	explicit ErrorException(const std::string& msg)
		: std::runtime_error(msg) {}
};

static void PrintDispatch(const std::string& msg)
{
	std::lock_guard<std::mutex> locked(printLock);
	for (printHandler_t i: printHandlers)
		i(msg);
}

void RegisterPrintHandler(printHandler_t handler)
{
	std::lock_guard<std::mutex> locked(printLock);
	printHandlers.push_back(handler);
}

void RemovePrintHandler(printHandler_t handler)
{
	std::lock_guard<std::mutex> locked(printLock);
	printHandlers.erase(std::remove(printHandlers.begin(), printHandlers.end(), handler), printHandlers.end());
}

void FatalError(const std::string& msg)
{
	// Check for recursive errors
	static bool errorEntered = false;
	if (errorEntered) {
		// Print the error to stderr and exit
		fprintf(stderr, "Recursive error: %s\n", msg.c_str());
		quick_exit(EXIT_FAILURE);
	}
	errorEntered = true;

	// Add a prefix and print in red
	PrintDispatch("^3ERROR: " + msg);

	// Shut down and quit
	// FIXME: Engine
	//Engine::Quit();
}

void Error(const std::string& msg)
{
	// Check for recursive errors
	// FIXME: more handling
	static bool errorEntered = false;
	if (errorEntered) {
		// Make error fatal if recursive
		FatalError(msg);
	}
	errorEntered = true;

	// Add a prefix and print in red
	PrintDispatch("^3ERROR: " + msg);

	// Throw exception
	throw ErrorException(msg);
}

void Warning(const std::string& msg)
{
	// Add a prefix and print in yellow
	PrintDispatch("^3WARNING: " + msg);
}

void Printf(const std::string& msg)
{
	// Print message normally
	PrintDispatch(msg);
}

void Debug(const std::string& msg)
{
	// Print if com_debug is enabled
	// FIXME: Cvar
	//if (com_debug.GetBool())
		Printf(msg);
}

void DWarning(const std::string& msg)
{
	// Print if com_debug is enabled
	// FIXME: Cvar
	//if (com_debug.Get())
		Warning(msg);
}
