//@@COPYRIGHT@@

// Console log buffer

namespace Log {

// Early initialization for the console to work.
void EarlyInit();

// Initializes the logging system.
void Init();

// Closes the log file.
void Shutdown();

// Get and lock the log buffer
EXPORT const char *GetBuffer();

// Release the log buffer
EXPORT void ReleaseBuffer();

}
