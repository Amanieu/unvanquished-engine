//@@COPYRIGHT@@

// Common filesystem stuff

#include "OSFile.h"
#include "Path.h"
#include "Async.h"

// Test the basepath for write permission
static inline bool TestWritePermission()
{
	// Create a temporary file in the write path and then delete it
	File *file = Filesystem::OpenFile(".test_write_permission", FS_WRITE);
	if (file) {
		file->Close();
		Filesystem::RemoveFile(".test_write_permission");
		return true;
	} else
		return false;
}

void Filesystem::Init()
{
	// If the base path was not specified, get it from the executable directory
	const char *basepath;
	if (!Engine::GetArg("-basepath", 1, &basepath)) {
		char buffer[MAX_PATH];
		strlcpy(buffer, Engine::GetProgramName(), sizeof(buffer));

		// Find last slash
#ifdef _WIN32
		char *lastSlash = NULL;
		for (char *p = buffer; *p; p++) {
			if (*p == '/' || *p == '\\')
				lastSlash = p;
		}
#else
		char *lastSlash =  strrchr(buffer, '/');
#endif

		// If no slashes, just use .
		if (lastSlash) {
			*lastSlash = '\0';
			basepath = buffer;
		} else
			basepath = ".";
	}

	// Add basepath
	Filesystem::AddPath(basepath);
	Msg("Using base path: %s", basepath);

	// If homepath is specified on the commandline, just use it
	const char *homepath = NULL;
	if (!Engine::GetArg("-homepath", 1, &homepath)) {
		// If not, then see if we need a homepath: Check basepath write permission
		if (!TestWritePermission()) {
			// Determine the homepath based on the platform
			char buffer[MAX_PATH];
#ifdef _WIN32
			if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, buffer)))
				Error("Could not find My Documents folder");
			strlcat(buffer, "\\My Games\\Unvanquished", sizeof(buffer));
#else
			const char *home = getenv("HOME");
			if (!home)
				Error("Could not determine home directory (try setting HOME)");
#ifdef __APPLE__
			snprintf(buffer, sizeof(buffer), "%s/Library/Application Support/Unvanquished", home);
#else
			snprintf(buffer, sizeof(buffer), "%s/.unvanquished", home);
#endif
#endif
			homepath = buffer;
		}
	}

	// Create and add the homepath
	if (homepath) {
		Filesystem::CreatePath(homepath, true);
		Filesystem::AddPath(homepath);
		Msg("Using home path: %s", homepath);
	}

	// Start the async system
	AsyncInit();
}

const char *Filesystem::GetError()
{
#ifdef _WIN32
	return System::Win32StrError(GetLastError());
#else
	return strerror(errno);
#endif
}

void Filesystem::Shutdown()
{
	AsyncShutdown();
	pathList.clear_and_dispose(DeleteFunctor<FSPath>());
}

void Filesystem::AddPath(const char *path)
{
	pathList.push_front(*new FSPath(path));
}

File *Filesystem::OpenFile(const char *path, fsMode_t mode, bool fullPath)
{
	if (fullPath) {
		// Just open the given path directly
		return OSFile::Open(path, mode);
	} else if (mode == FS_READ) {
		// Loop through every search path, and try opening the file on each
		foreach (FSPath &p, pathList) {
			File *f = p.OpenFile(path, mode);
			if (f)
				return f;
		}

		return NULL;
	} else {
		// Open it on the write path
		return writePath.OpenFile(path, mode);
	}
}

bool FileExists(const char *path, bool fullPath)
{
	if (fullPath) {
		// Just check the given path directly
		return OSFileExists(path);
	} else {
		// Loop through every search path, and try looking for the file on each
		foreach (FSPath &p, pathList) {
			if (p.FileExists(path))
				return true;
		}

		return false;
	}
}

bool Filesystem::CopyFile(const char *from, const char *to, bool fullPath)
{
	// Transfer buffer
	char buffer[65536];

	// Source and destination files
	File *fromFile = Filesystem::OpenFile(from, FS_READ, fullPath);
	if (!fromFile)
		return false;
	File *toFile = Filesystem::OpenFile(to, FS_WRITE, fullPath);
	if (!toFile) {
		fromFile->Close();
		return false;
	}

	// Loop until we reach the end of the source file
	int numBytes;
	do {
		numBytes = fromFile->Read(buffer, sizeof(buffer));
		if (numBytes > 0)
			toFile->Write(buffer, numBytes);
	} while (numBytes > 0);

	// Close files
	fromFile->Close();
	toFile->Close();

	return true;
}

bool Filesystem::MoveFile(const char *from, const char *to, bool fullPath)
{
	// Get the full paths
	char buffer[MAX_PATH];
	char buffer2[MAX_PATH];
	if (!fullPath) {
		strlcpy(buffer, writePath.path, MAX_PATH);
		strlcat(buffer, from, MAX_PATH);
		from = buffer;

		strcpy(buffer2, buffer);
		strlcat(buffer2, to, MAX_PATH);
		to = buffer2;
	}

	// Call OS-specific function to move file
#ifdef _WIN32
	return MoveFileEx(from, to, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) != 0;
#else
	// rename() doesn't work across filesystems. In that case, do a copy and
	// remove instead.
	int result = rename(from, to);
	if (result == -1 && errno == EXDEV) {
		bool success = Filesystem::CopyFile(from, to, true);
		if (success)
			success = Filesystem::RemoveFile(from, true);
		else
			Filesystem::RemoveFile(to, true);
		return success;
	} else
		return result == 0;
#endif
}

bool Filesystem::RemoveFile(const char *path, bool fullPath)
{
	// Get the full path
	char buffer[MAX_PATH];
	if (!fullPath) {
		strlcpy(buffer, writePath.path, MAX_PATH);
		strlcat(buffer, path, MAX_PATH);
		path = buffer;
	}

	// Call OS-specific function to remove file
#ifdef _WIN32
	return DeleteFile(path) != 0;
#else
	return unlink(path) == 0;
#endif
}

void Filesystem::CreatePath(const char *path, bool fullPath)
{
	char buffer[MAX_PATH];

	// Get the full path
	if (!fullPath) {
		strlcpy(buffer, writePath.path, sizeof(buffer));
		strlcat(buffer, path, sizeof(buffer));
	} else
		strlcpy(buffer, path, sizeof(buffer));

	// Iterate through every path element and create it
	for (char *p = buffer; true; p++) {
		if (*p == '/' || *p == '\\') {
			*p = '\0';
#ifdef _WIN32
			_mkdir(buffer);
#else
			mkdir(buffer, 0777);
#endif
			*p = '/';
		} else if (!*p) {
#ifdef _WIN32
			_mkdir(buffer);
#else
			mkdir(buffer, 0777);
#endif
			break;
		}
	}
}

void Filesystem::ListFiles(StringList &fileList, const char *path, const char *extension, fsSearch_t searchType, bool fullPath)
{
	// Allow NULL extension
	if (!extension)
		extension = "";

	// Loop through every search path, and list files in each
	if (!fullPath) {
		foreach (FSPath &p, pathList)
			p.ListFiles(path, extension, searchType, fileList);
	} else
		ListOSFiles(path, extension, searchType, fileList);
}

int File::VPrintf(const char *fmt, va_list ap)
{
	char buffer[MAX_PRINTMSG];
	unsigned int len = vsnprintf(buffer, sizeof(buffer), fmt, ap);

	// See if we need a bigger buffer
	if (len >= sizeof(buffer)) {
		char buffer[len + 1];
		vsnprintf(buffer, len + 1, fmt, ap);
		Write(buffer, len);
	} else
		Write(buffer, len);

	return len;
}
int File::Printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	int ret = VPrintf(fmt, ap);
	va_end(ap);

	return ret;
}

static void AsyncReadFileCallback(File *file, const tr1::function<void(void *, int)> &callback, void *buffer, int result)
{
	file->Close();
	if (callback)
		callback(buffer, result);
}

bool Filesystem::AsyncReadFile(const char *path, bool nulTerminate, const tr1::function<void(void *, int)> &callback, bool fullPath)
{
	File *file = Filesystem::OpenFile(path, FS_READ, fullPath);
	if (!file)
		return false;

	int length = file->Length();
	char *buffer = new char[length + (nulTerminate ? 1 : 0)];
	if (nulTerminate)
		buffer[length] = '\0';

	file->AsyncReadEx(buffer, length, 0, tr1::bind(AsyncReadFileCallback, file, callback, buffer, _1));
	return true;
}

static void AsyncWriteFileCallback(File *file, const tr1::function<void()> &callback)
{
	file->Close();
	if (callback)
		callback();
}

bool Filesystem::AsyncWriteFile(const char *path, const void *data, int length, const tr1::function<void()> &callback, bool fullPath)
{
	File *file = Filesystem::OpenFile(path, FS_WRITE, fullPath);
	if (!file)
		return false;

	file->AsyncWriteEx(data, length, 0, tr1::bind(AsyncWriteFileCallback, file, callback));
	return true;
}

#if 0
// Test stuff
static void Cat_f(CmdArgs *args)
{
	if (!args)
		return;

	char *buf = static_cast<char *>(Filesystem::ReadFile(args->Argv(1), NULL, true));
	Msg("%s", buf);
	MemFree(buf);
}
static Cmd Cat_cmd("cat", Cat_f);

static void Write_f(CmdArgs *args)
{
	if (!args)
		return;

	Filesystem::WriteFile(args->Argv(1), args->Args(2), strlen(args->Args(2)), false);
}
static Cmd Write_cmd("write", Write_f);

static void Copy_f(CmdArgs *args)
{
	if (!args)
		return;

	Filesystem::CopyFile(args->Argv(1), args->Argv(2), false);
}
static Cmd Copy_cmd("copy", Copy_f);

static void Move_f(CmdArgs *args)
{
	if (!args)
		return;

	Filesystem::MoveFile(args->Argv(1), args->Argv(2), false);
}
static Cmd Move_cmd("move", Move_f);

static void Del_f(CmdArgs *args)
{
	if (!args)
		return;

	Filesystem::RemoveFile(args->Argv(1), false);
}
static Cmd Del_cmd("del", Del_f);

static void Mkdir_f(CmdArgs *args)
{
	if (!args)
		return;

	Filesystem::CreatePath(args->Argv(1), false);
}
static Cmd Mkdir_cmd("mkdir", Mkdir_f);

static void Ls_f(CmdArgs *args)
{
	if (!args)
		return;

	StringList files;
	Filesystem::ListFiles(files, args->Argv(1), args->Argv(2), FS_SEARCH_BOTH);
	foreach (const char *i, files)
		Msg("%s", i);
}
static Cmd Ls_cmd("ls", Ls_f);
#endif
