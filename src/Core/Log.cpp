//@@COPYRIGHT@@

// Log buffer and buffer insertion point
#define LOGBUF_SIZE 65536
static Mutex logMutex;
static char logBuf[LOGBUF_SIZE] = "";
static char *insert = logBuf;

// Log files
static File *logFile = NULL;
static File *crashLogFile = NULL;

// Append all prints to the log buffer
static void LogHandler(printLevel_t level, const char *msg)
{
	char buffer[MAX_PRINTMSG];

	// Appropriate prefix for the error level
	switch (level) {
	case PRINT_ERROR:
		strcpy(buffer, "ERROR: ");
		break;
	case PRINT_WARNING:
		strcpy(buffer, "WARNING: ");
		break;
	case PRINT_MESSAGE:
		buffer[0] = '\0';
		break;
	case PRINT_DEBUG:
		strcpy(buffer, "DEBUG: ");
		break;
	NO_DEFAULT
	}

	strlcat(buffer, msg, sizeof(buffer));

	// Add a newline at the end
	unsigned int len = strlen(buffer);
	if (len == sizeof(buffer) - 1)
		len = sizeof(buffer) - 2;
	buffer[len++] = '\n';
	buffer[len] = '\0';

	// Add the message to the log buffer
	logMutex.Lock();
	if (insert + len >= logBuf + sizeof(logBuf)) {
		memmove(logBuf, logBuf + sizeof(logBuf) / 2, sizeof(logBuf) / 2);
		memset(logBuf + sizeof(logBuf) / 2, 0, sizeof(logBuf) / 2);
		insert -= sizeof(logBuf) / 2;
	}
	strcpy(insert, buffer);
	insert += len;

	// Save buffer to log file
	if (logFile) {
		StripColor(buffer);
		logFile->Write(buffer, len);
	}

	// If it was an error, create a crash log containing the whole log buffer
	if (crashLogFile)
		crashLogFile->Write(buffer, len);
	else if (level == PRINT_ERROR) {
		logMutex.Unlock();
		crashLogFile = Filesystem::OpenFile("crashlog.txt", FS_WRITE);
		logMutex.Lock();
		if (crashLogFile)
			crashLogFile->Write(logBuf, insert - logBuf);
	}
	logMutex.Unlock();
}

void Log::EarlyInit()
{
	// Just register the print handler
	RegisterPrintHandler(LogHandler);
}

void Log::Init()
{
	const char *filename;

	// Open the log file and write anything that was printed before
	if (Engine::GetArg("-log", 1, &filename)) {
		logFile = Filesystem::OpenFile(filename, FS_APPEND);
		if (!logFile) {
			Warning("Couldn't open log file %s for writing", filename);
			return;
		}
		logMutex.Lock();
		logFile->Write(logBuf, insert - logBuf);
		logMutex.Unlock();
	}
}

void Log::Shutdown()
{
	// Close the log files
	if (logFile)
		logFile->Close();
	if (crashLogFile)
		crashLogFile->Close();
	logFile = crashLogFile = NULL;
}

const char *Log::GetBuffer()
{
	logMutex.Lock();
	return logBuf;
}

void Log::ReleaseBuffer()
{
	logMutex.Unlock();
}
