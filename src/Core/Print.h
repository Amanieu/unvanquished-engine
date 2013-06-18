//@@COPYRIGHT@@

// Printing and error handling

// All of the print functions automatically add a newline at the end of the
// message.

// Format a string sprintf-style
template<typename... Args> inline std::string va(const char* fmt, Args&&... args)
{
	return tfm::format(fmt, std::forward<Args>(args)...);
}

// All prints are routed through a print handler, which can be a console, log, etc.
typedef void (*printHandler_t)(const std::string& msg);
EXPORT void RegisterPrintHandler(printHandler_t handler);
EXPORT void RemovePrintHandler(printHandler_t handler);

// Error functions. These do not return. Error() will throw an ErrorException
// and is intended to return to the main menu when in a game. FatalError() will
// abort execution and exit with minimal cleanup.
EXPORT __noreturn __cold void FatalError(const std::string& msg);
EXPORT __noreturn __cold void Error(const std::string& msg);

// Informational messages
EXPORT __cold void Warning(const std::string& msg);
EXPORT void Printf(const std::string& msg);

// Debug messages are only printed if com_debug is enabled
EXPORT void Debug(const std::string& msg);
EXPORT __cold void DWarning(const std::string& msg);

// Variadic wrappers for print functions
template<typename... Args> inline __noreturn __cold void FatalError(const char* fmt, Args&&... args)
{
	FatalError(va(fmt, std::forward<Args>(args)...));
}
template<typename... Args> inline __noreturn __cold void Error(const char* fmt, Args&&... args)
{
	Error(va(fmt, std::forward<Args>(args)...));
}
template<typename... Args> inline __cold void Warning(const char* fmt, Args&&... args)
{
	Warning(va(fmt, std::forward<Args>(args)...));
}
template<typename... Args> inline void Printf(const char* fmt, Args&&... args)
{
	Printf(va(fmt, std::forward<Args>(args)...));
}
template<typename... Args> inline void Debug(const char* fmt, Args&&... args)
{
	Debug(va(fmt, std::forward<Args>(args)...));
}
template<typename... Args> inline __cold void DWarning(const char* fmt, Args&&... args)
{
	DWarning(va(fmt, std::forward<Args>(args)...));
}

// Helper macro to print a warning only once
#define WarnOnce(...) \
	do { \
		static bool warned = false; \
		if (!warned) { \
			warned = true; \
			Warning(__VA_ARGS__); \
		} \
	} while (false)

// Runtime assertions, causes a fatal error when triggered
#ifdef DEBUG
#define AssertMsg(expr, msg)  \
	do { \
		if (!expr) \
			FatalError("Assertion failed in %s() at %s:%d: %s", BOOST_CURRENT_FUNCTION, __FILE__, __LINE__, msg); \
	} while (false)
#else
#define AssertMsg(expr, msg) __assume(expr)
#endif
#define Assert(expr) AssertMsg(expr, #expr)

// Weak assertions, only warns when triggered
#ifdef DEBUG
#define WeakAssertMsg(expr, msg)  \
	do { \
		if (!expr) \
			Warning("Assertion failed in %s() at %s:%d: %s", BOOST_CURRENT_FUNCTION, __FILE__, __LINE__, msg); \
	} while (false)
#else
#define WeakAssertMsg(expr, msg) do {} while (false)
#endif
#define WeakAssert(expr) WeakAssertMsg(expr, #expr)
