//@@COPYRIGHT@@

// Operating system specific functions

namespace System {

// Sets up an exception handler around the given function and runs it.
// Will print an appropriate error message and shutdown if an exception is caught.
// NOTE: This function is called automatically by Thread::SpawnThread().
void SetupExceptionHandler(void (*func)());

#ifndef _WIN32
// Attach a signal handler to the specified signal. Not available on Windows.
void AttachSignalHandler(int signal, void (*handler)(int));
#endif

// Platform-specific initialization
void PlatformInit();

// Sleep for a few msec. This function is only accurate to about 10ms because of
// operating system limitations.
inline void Sleep(int msec)
{
#ifdef _WIN32
	::Sleep(msec);
#else
	usleep(msec * 1000);
#endif
}

// Get a millisecond count from a unspecified point in time
inline int GetMsec()
{
#ifdef _WIN32
	return timeGetTime();
#else
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec * 1000 + now.tv_nsec / 1000000;
#endif
}

#ifdef _WIN32
// strerror() equivalent for Win32 API functions
EXPORT const char *Win32StrError(DWORD error);
#endif

}

// High-precision performance timer
class PerfTimer {
public:
	// Starts the timer
	void Start()
	{
#ifdef _WIN32
		QueryPerformanceCounter(&start);
#else
		clock_gettime(CLOCK_MONOTONIC, &start);
#endif
	}

	// Stops the timer and returns elapsed time in microseconds
	int Stop()
	{
#ifdef _WIN32
		LARGE_INTEGER end, frequency;
		QueryPerformanceCounter(&end);
		QueryPerformanceFrequency(&frequency);
		return (end.QuadPart - start.QuadPart) * 1000000 / frequency.QuadPart;
#else
		struct timespec end, total;
		clock_gettime(CLOCK_MONOTONIC, &end);
		total.tv_sec = end.tv_sec - start.tv_sec;
		total.tv_nsec = end.tv_nsec - start.tv_nsec;
		return total.tv_sec * 1000000 + total.tv_nsec / 1000;
#endif
	}

private:
#ifdef _WIN32
	LARGE_INTEGER start;
#else
	struct timespec start;
#endif
};
