//@@COPYRIGHT@@

// Normal file

// O_BINARY may not be defined
#ifndef O_BINARY
#define O_BINARY 0
#endif

// Base functions to submit an async request
class OSFile;
static void AsyncPrepareFile(OSFile *file);
static void AsyncRequestRead(OSFile *file, void *buffer, int length, fsOffset_t offset, const tr1::function<void(int)> &callback);
static void AsyncRequestWrite(OSFile *file, const void *data, int length, fsOffset_t offset, const tr1::function<void()> &callback);

#ifdef _WIN32
// Event object used for synchronous I/O
static __thread HANDLE overlappedEvent = NULL;

// Prepare an OVERLAPPED structure for use
static inline void PrepareOverlapped(OVERLAPPED *overlapped, uint64_t offset)
{
	// Create event if there isn't one for this thread.
	// Set the low bit of the event handle so that synchronous I/O doesn't
	// interfere with I/O completion ports used in asynchronous I/O.
	if (!overlappedEvent)
		overlappedEvent = reinterpret_cast<HANDLE>(reinterpret_cast<intptr_t>(CreateEvent(NULL, TRUE, FALSE, NULL)) | 1);

	// Initialize OVERLAPPED structure
	memset(overlapped, 0, sizeof(OVERLAPPED));
	overlapped->Offset = offset & 0xFFFFFFFF;
	overlapped->OffsetHigh = offset >> 32;
	overlapped->hEvent = overlappedEvent;
}
#endif

class OSFile: public File, UseMemPool<OSFile> {
public:
	// Open the file named by path, which should be an absolute path.
	static OSFile *Open(const char *path, fsMode_t mode)
	{
#ifdef _WIN32
		DWORD fileFlags = 0;
		DWORD openMode = 0;

		switch (mode) {
		case FS_READ:
			fileFlags = GENERIC_READ;
			openMode = OPEN_EXISTING;
			break;
		case FS_WRITE:
			fileFlags = GENERIC_WRITE;
			openMode = CREATE_ALWAYS;
			break;
		case FS_EDIT:
			fileFlags = GENERIC_READ | GENERIC_WRITE;
			openMode = OPEN_ALWAYS;
			break;
		case FS_APPEND:
			fileFlags = GENERIC_WRITE;
			openMode = OPEN_ALWAYS;
			break;
		NO_DEFAULT
		}

		HANDLE fd = CreateFile(path, fileFlags, FILE_SHARE_READ, NULL, openMode, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
		if (fd == INVALID_HANDLE_VALUE)
			return NULL;
#else
		int fd = -1;

		switch (mode) {
		case FS_READ:
			fd = open(path, O_RDONLY | O_BINARY);
			break;
		case FS_WRITE:
			fd = open(path, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0777);
			break;
		case FS_EDIT:
			fd = open(path, O_RDWR | O_CREAT | O_BINARY, 0777);
			break;
		case FS_APPEND:
			fd = open(path, O_WRONLY | O_CREAT | O_APPEND | O_BINARY, 0777);
			break;
		NO_DEFAULT
		}

		if (fd == -1)
			return NULL;
#endif

		OSFile *file = new OSFile;
		file->fd = fd;
		file->mode = mode;

		// Technically this shouldn't be 0 for the append case, but you
		// shouldn't be seeking in append mode anyways.
		file->position = 0;

		// Get the length of the file
#ifdef _WIN32
		LARGE_INTEGER size;
		GetFileSizeEx(fd, &size);
		file->length = size.QuadPart;
#else
		struct stat buf;
		fstat(fd, &buf);
		file->length = buf.st_size;
#endif

		// Prepare the file for asynchronous I/O. Only used on Win32 to
		// associate the file handle with the completion port.
		AsyncPrepareFile(file);

		return file;
	}

	void Close()
	{
#ifdef _WIN32
		if (!CloseHandle(fd))
			Warning("Error closing file: %s", System::Win32StrError(GetLastError()));
#else
		if (close(fd) == EOF)
			Warning("Error closing file: %s", strerror(errno));
#endif
		delete this;
	}

	int Read(void *buffer, int len)
	{
		// Update position
		fsOffset_t current = position;
		position += len;

		return OSFile::ReadEx(buffer, len, current);
	}

	void Write(const void *data, int len)
	{
		// For appending we need to make sure we are really writing to the end
		// of the file.
		if (mode == FS_APPEND) {
#ifdef _WIN32
			OSFile::WriteEx(data, len, 0xFFFFFFFFFFFFFFFFULL);
#else
			if (write(fd, data, len) == -1)
				Warning("Error writing to file: %s", strerror(errno));
#endif
		} else {
			// Update position and length
			fsOffset_t current = position;
			position += len;
			if (position > length)
				length = position;

			OSFile::WriteEx(data, len, current);
		}
	}

	int ReadEx(void *buffer, int len, fsOffset_t offset)
	{
#ifdef _WIN32
		OVERLAPPED overlapped;
		PrepareOverlapped(&overlapped, offset);

		DWORD bytesRead;
		if (ReadFile(fd, buffer, len, &bytesRead, &overlapped))
			return bytesRead;
		else if (GetLastError() == ERROR_HANDLE_EOF)
			return bytesRead;

		if (GetLastError() == ERROR_IO_PENDING) {
			// Avoid race condition described here:
			// http://msdn.microsoft.com/en-us/library/dd371711.aspx
			WaitForSingleObject(overlapped.hEvent, INFINITE);
			if (GetOverlappedResult(fd, &overlapped, &bytesRead, FALSE))
				return bytesRead;
			else if (GetLastError() == ERROR_HANDLE_EOF)
				return bytesRead;
		}

		return 0;
#else
		int result = pread(fd, buffer, len, offset);
		if (result < 0)
			return 0;
		else
			return result;
#endif
	}

	void WriteEx(const void *data, int len, fsOffset_t offset)
	{
		// Update length
		if (offset + len > length)
			length = offset + len;

#if _WIN32
		OVERLAPPED overlapped;
		PrepareOverlapped(&overlapped, offset);

		if (WriteFile(fd, data, len, NULL, &overlapped))
			return;

		if (GetLastError() == ERROR_IO_PENDING) {
			DWORD bytesWritten;
			// Avoid race condition described here:
			// http://msdn.microsoft.com/en-us/library/dd371711.aspx
			WaitForSingleObject(overlapped.hEvent, INFINITE);
			if (GetOverlappedResult(fd, &overlapped, &bytesWritten, FALSE))
				return;
		}

		Warning("Error writing to file: %s", System::Win32StrError(GetLastError()));
#else
		if (pwrite(fd, data, len, offset) == -1)
			Warning("Error writing to file: %s", strerror(errno));
#endif
	}

	void AsyncRead(void *buffer, int len, const tr1::function<void(int)> &callback)
	{
		// Update position
		fsOffset_t current = position;
		position += len;

		return OSFile::AsyncReadEx(buffer, len, current, callback);
	}

	void AsyncWrite(const void *data, int len, const tr1::function<void()> &callback)
	{
		// For appending we need to make sure we are really writing to the end
		// of the file. Offset is necessary for Win32 async I/O.
		if (mode == FS_APPEND)
			OSFile::AsyncWriteEx(data, len, 0xFFFFFFFFFFFFFFFFULL, callback);
		else {
			// Update position and length
			fsOffset_t current = position;
			position += len;
			if (position > length)
				length = position;

			OSFile::AsyncWriteEx(data, len, current, callback);
		}
	}

	void AsyncReadEx(void *buffer, int len, fsOffset_t offset, const tr1::function<void(int)> &callback)
	{
		AsyncRequestRead(this, buffer, len, offset, callback);
	}

	void AsyncWriteEx(const void *data, int len, fsOffset_t offset, const tr1::function<void()> &callback)
	{
		// Update length
		if (offset + len > length)
			length = offset + len;

		AsyncRequestWrite(this, data, len, offset, callback);
	}

	// Unfortunately there is no Win32 equivalent of fadvise
	void Precache()
	{
#ifndef _WIN32
		posix_fadvise(fd, 0, length, POSIX_FADV_WILLNEED);
#endif
	}

	void *MemMap(fsOffset_t offset, int length, fsMemMap_t mode)
	{
#ifdef _WIN32
		DWORD prot = 0, access = 0;
		switch (mode) {
		case FS_MAP_READ:
			prot = PAGE_READONLY;
			access = FILE_MAP_READ;
			break;
		case FS_MAP_COPY:
			prot = PAGE_WRITECOPY;
			access = FILE_MAP_READ | FILE_MAP_COPY;
			break;
		case FS_MAP_EDIT:
			prot = PAGE_READWRITE;
			access = FILE_MAP_ALL_ACCESS;
			break;
		NO_DEFAULT
		}
		HANDLE fileMappingObject = CreateFileMapping(fd, NULL, prot, 0, 0, NULL);
		if (!fileMappingObject)
			return NULL;
		void *ptr = MapViewOfFile(fileMappingObject, access, offset >> 32, offset, length);
		CloseHandle(fileMappingObject);
		return ptr;
#else
		int flags = 0, prot = 0;
		switch (mode) {
		case FS_MAP_READ:
			prot = PROT_READ;
			flags = MAP_PRIVATE;
			break;
		case FS_MAP_COPY:
			prot = PROT_READ | PROT_WRITE;
			flags = MAP_PRIVATE;
			break;
		case FS_MAP_EDIT:
			prot = PROT_READ | PROT_WRITE;
			flags = MAP_SHARED;
			break;
		NO_DEFAULT
		}
		void *ptr = mmap(NULL, length, prot, flags, fd, offset);
		if (ptr == MAP_FAILED)
			return NULL;
		MemVirtual::RegisterMmap(ptr, length);
		return ptr;
#endif
	}

	void MemUnmap(void *ptr)
	{
#ifdef _WIN32
		UnmapViewOfFile(ptr);
#else
		munmap(ptr, MemVirtual::ReleaseMmap(ptr));
#endif
	}

	// File handle
#ifdef _WIN32
	HANDLE fd;
#else
	int fd;
#endif

	// Make fields from File public for use in Async.h
	using File::position;
	using File::length;
	using File::mode;
};

// List all files in a full OS path
inline void ListOSFiles(const char *path, const char *extension, fsSearch_t searchType, StringList &list)
{
#ifdef _WIN32
	char buffer[256];
	HANDLE findHandle;
	WIN32_FIND_DATA findData;

	snprintf(buffer, sizeof(buffer), "%s\\*%s", path, extension);

	if (searchType == FS_SEARCH_DIRS)
		findHandle = FindFirstFileEx(buffer, FindExInfoStandard, &findData, FindExSearchLimitToDirectories, NULL, 0);
	else
		findHandle = FindFirstFileEx(buffer, FindExInfoStandard, &findData, FindExSearchNameMatch, NULL, 0);

	if (findHandle == INVALID_HANDLE_VALUE)
		return;

	do {
		if (findData.cFileName[0] == '.')
			continue;

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (searchType != FS_SEARCH_FILES)
				list.push_back(list.CopyString(findData.cFileName));
		} else
			list.push_back(list.CopyString(findData.cFileName));
	} while (FindNextFile(findHandle, &findData));
#else
	DIR *dir = opendir(path);
	if (!dir)
		return;

	int extLen = strlen(extension);

	struct dirent *entry;
	while ((entry = readdir(dir))) {
		if (entry->d_name[0] == '.')
			continue;

		// Check extension
		if (extLen) {
			int nameLen = strlen(entry->d_name);
			if (nameLen < extLen)
				continue;
			if (strcmp(entry->d_name + nameLen - extLen, extension))
				continue;
		}

		if (entry->d_type == DT_DIR) {
			if (searchType != FS_SEARCH_FILES)
				list.push_back(list.CopyString(entry->d_name));
		} else if (entry->d_type == DT_REG) {
			if (searchType != FS_SEARCH_DIRS)
				list.push_back(list.CopyString(entry->d_name));
		}
	}

	closedir(dir);
#endif
}

// Check if an OS file exists
inline bool OSFileExists(const char *path)
{
#ifdef _WIN32
	return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
#else
	struct stat buf;
	return stat(path, &buf) == 0;
#endif
}
