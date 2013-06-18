//@@COPYRIGHT@@

// Filesystem interface

namespace Filesystem {

// File offset type. Using 64bit to allow large files.
typedef int64_t fsOffset_t;

// Generic file interface, all operations are unbuffered (direct to OS, not direct to disk)
// Access from multiple threads is safe since there is no current file position
class File: boost::noncopyable {
public:
	// Close the file
	virtual ~File() {}

	// Read or write data to the file at the specified position
	// If the file is opened for append the position argument is ignored
	virtual size_t Read(void* buffer, size_t length, fsOffset_t pos, std::error_code& err) = 0;
	virtual size_t Write(const void* data, size_t length, fsOffset_t pos, std::error_code& err) = 0;

	// Same as the above, but throws exceptions instead of returning an error code
	inline size_t Read(void* buffer, size_t length, fsOffset_t pos)
	{
		std::error_code err;
		size_t result = Read(buffer, length, pos, err);
		if (err)
			throw std::system_error(err);
		return result;
	}
	inline size_t Write(const void* data, size_t length, fsOffset_t pos)
	{
		std::error_code err;
		size_t result = Write(data, length, pos, err);
		if (err)
			throw std::system_error(err);
		return result;
	}

	// Asynchronous versions of the above functions. These act as a child of the current task
	// in the context of the thread pool, and can be waited on the same way as a normal task.
	// For writes, the input data must remain available for the entire duration of the write
	virtual void AsyncRead(void* buffer, size_t length, fsOffset_t pos, std::function<void(std::error_code, size_t)>&& callback) = 0;
	virtual void AsyncWrite(const void* data, size_t length, fsOffset_t pos, std::function<void(std::error_code, size_t)>&& callback) = 0;

	// Same as the above, but accepting any function type for the callback
	template<typename T>
	inline void AsyncRead(void* buffer, size_t length, fsOffset_t pos, T&& obj)
	{
		AsyncRead(buffer, length, pos, std::function<void(std::error_code, size_t)>(std::forward<T>(obj)));
	}
	template<typename T>
	inline void AsyncWrite(const void* data, size_t length, fsOffset_t pos, T&& obj)
	{
		AsyncWrite(data, length, pos, std::function<void(std::error_code, size_t)>(std::forward<T>(obj)));
	}
	template<typename T, typename... Args>
	inline void AsyncRead(void* buffer, size_t length, fsOffset_t pos, T&& obj, Args&&... args)
	{
		AsyncRead(buffer, length, pos, std::bind(std::forward<T>(obj), std::forward<Args>(args)...));
	}
	template<typename T, typename... Args>
	inline void AsyncWrite(const void* data, size_t length, fsOffset_t pos, T&& obj, Args&&... args)
	{
		AsyncWrite(data, length, pos, std::bind(std::forward<T>(obj), std::forward<Args>(args)...));
	}

	// Map a part of the file in memory. Returns NULL on error.
	virtual void* MemMapRead(fsOffset_t offset, size_t length) = 0; // Read-only
	virtual void* MemMapCopy(fsOffset_t offset, size_t length) = 0; // Changes not written back on unmap
	virtual void* MemMapEdit(fsOffset_t offset, size_t length) = 0; // Changes written back on unmap
	virtual void MemUnmap(void* ptr) = 0;

	// Get the length of the file
	virtual fsOffset_t Length() = 0;

	// Get the timestamp (time of last modification) of the file or the pak it is in
	virtual time_t Timestamp() = 0;

protected:
	// Can only be constructed using Filesystem::OpenXXX
	File() {}
};

// File handle type
typedef std::unique_ptr<File> FileHandle;
typedef std::shared_ptr<File> SharedFileHandle;

// Buffered file, not thread-safe
class FileBuffer {
public:
	// Initialize without an open file
	explicit FileBuffer(size_t bufferSize = 4096)
		: buffer(new char[bufferSize]), bufSize(bufferSize)
	{
		bufPos = 0;
		bufFill = 0;
		bufWrite = false;

		file = nullptr;
		fileSize = 0;
		pos = 0;
	}

	// Initialize with an open file
	FileBuffer(FileHandle&& f, size_t bufferSize = 4096)
		: buffer(new char[bufferSize]), bufSize(bufferSize)
	{
		bufPos = 0;
		bufFill = 0;
		bufWrite = false;

		file = std::move(f);
		fileSize = file->Length();
		pos = 0;
	}

	// Movable but not copyable
	FileBuffer(FileBuffer&& other) = default;
	FileBuffer& operator=(FileBuffer&& other) = default;

	// Close on destruction
	~FileBuffer()
	{
		Close();
	}

	// Close the currently open file
	void Close()
	{
		Flush();
		file = nullptr;
	}

	// Open a different file
	void Open(FileHandle&& f)
	{
		Flush();

		file = std::move(f);
		fileSize = file->Length();
		pos = 0;

		bufPos = 0;
		bufFill = 0;
		bufWrite = false;
	}

	// Get the length and timestamp of the file
	fsOffset_t Length()
	{
		return fileSize;
	}
	time_t Timestamp()
	{
		return file->Timestamp();
	}

	// Manipulate the read/write position
	// These do not work for files opened in append mode
	void SeekCur(fsOffset_t off)
	{
		pos = std::max(pos + off, (fsOffset_t)0);
	}
	void SeekEnd(fsOffset_t off)
	{
		pos = std::max(fileSize + off, (fsOffset_t)0);
	}
	void SeekSet(fsOffset_t off)
	{
		pos = std::max(off, (fsOffset_t)0);
	}
	fsOffset_t Tell()
	{
		return pos;
	}
	bool Eof()
	{
		return pos == fileSize;
	}

	// Get the underlying file handle
	const FileHandle& Handle()
	{
		return file;
	}

	// Flush all unwritten data to the OS
	EXPORT void Flush(std::error_code& err);
	void Flush()
	{
		std::error_code err;
		Flush(err);
		if (err)
			throw std::system_error(err);
	}

	// Read or write data to the file
	EXPORT size_t Read(void* buffer, size_t length, fsOffset_t pos, std::error_code& err);
	EXPORT size_t Write(const void* data, size_t length, fsOffset_t pos, std::error_code& err);
	size_t Read(void* buffer, size_t length, fsOffset_t pos)
	{
		std::error_code err;
		size_t result = Read(buffer, length, pos, err);
		if (err)
			throw std::system_error(err);
		return result;
	}
	size_t Write(const void* data, size_t length, fsOffset_t pos)
	{
		std::error_code err;
		size_t result = Write(data, length, pos, err);
		if (err)
			throw std::system_error(err);
		return result;
	}

	// Same as above using the current file position
	size_t Read(void* buffer, size_t length, std::error_code& err)
	{
		return Read(buffer, length, pos, err);
	}
	size_t Write(const void* data, size_t length, std::error_code& err)
	{
		return Write(data, length, pos, err);
	}
	size_t Read(void* buffer, size_t length)
	{
		std::error_code err;
		size_t result = Read(buffer, length, err);
		if (err)
			throw std::system_error(err);
		return result;
	}
	size_t Write(const void* data, size_t length)
	{
		std::error_code err;
		size_t result = Write(data, length, err);
		if (err)
			throw std::system_error(err);
		return result;
	}

	// Read a line from the file
	EXPORT std::string ReadLine(std::error_code& err);
	std::string ReadLine()
	{
		std::error_code err;
		std::string result = ReadLine(err);
		if (err)
			throw std::system_error(err);
		return result;
	}

	// Write a string to the file
	size_t Write(const std::string& str, std::error_code& err)
	{
		return Write(str.data(), str.size(), err);
	}
	size_t Write(const char* str, std::error_code& err)
	{
		return Write(str, strlen(str), err);
	}
	size_t Write(const std::string& str)
	{
		std::error_code err;
		size_t result = Write(str, err);
		if (err)
			throw std::system_error(err);
		return result;
	}
	size_t Write(const char* str)
	{
		std::error_code err;
		size_t result = Write(str, err);
		if (err)
			throw std::system_error(err);
		return result;
	}

	// Format and write a string to the file
	template<typename... Args>
	size_t Printf(std::error_code& err, const char* fmt, Args&&... args)
	{
		return Write(va(fmt, std::forward<Args>(args)...), err);
	}
	template<typename... Args>
	size_t Printf(const char* fmt, Args&&... args)
	{
		std::error_code err;
		size_t result = Printf(err, fmt, std::forward<Args>(args)...);
		if (err)
			throw std::system_error(err);
		return result;
	}

private:
	// Buffer containing file data
	std::unique_ptr<char[]> buffer;
	size_t bufSize;

	fsOffset_t bufPos; // Position of the start of the buffer in the file
	size_t bufFill; // How much of the buffer contains valid data
	bool bufWrite; // Whether the data in the buffer must be written to the file

	fsOffset_t pos; // File position
	fsOffset_t fileSize; // File size
	FileHandle file; // File handle
};

// Initialize the filesystem and set up the read and write paths
void Init();

// Restart the filesystem and unload all loaded paks
void Restart();

// Shutdown the filesystem and close all open files
void Shutdown();

// Structure to indentify a package
struct PakInfo {
	std::string name;
	std::string version;
	std::string checksum;
};

// Load a package file, a specific version or checksum can be requested.
// Returns the details of the loaded package.
EXPORT boost::optional<PakInfo> LoadPak(const char* name, const char* version = NULL, const char* checksum = NULL);

// Load a package file, but only allow loading it and its dependancies from the given list
EXPORT boost::optional<PakInfo> LoadPakRestrict(const std::vector<PakInfo>& restrict, const char* name, const char* version = NULL, const char* checksum = NULL);

// Get a list of all loaded packages
EXPORT std::vector<PakInfo> GetLoadedPaks();

// Get a list of all available packages
EXPORT std::vector<PakInfo> GetAvailablePaks();

// Unload a package file
EXPORT void UnloadPak(const char* name);

// Open a file for reading/writing/appending/editing
EXPORT FileHandle OpenRead(const char* path, std::error_code& err);
EXPORT FileHandle OpenReadFromPak(const char* path, std::error_code& err);
EXPORT FileHandle OpenWrite(const char* path, std::error_code& err);
EXPORT FileHandle OpenAppend(const char* path, std::error_code& err);
EXPORT FileHandle OpenEdit(const char* path, std::error_code& err);

// Same as above but throws exceptions on error
inline FileHandle OpenRead(const char* path)
{
	std::error_code err;
	FileHandle result = OpenRead(path, err);
	if (err)
		throw std::system_error(err);
	return result;
}
inline FileHandle OpenReadFromPak(const char* path)
{
	std::error_code err;
	FileHandle result = OpenReadFromPak(path, err);
	if (err)
		throw std::system_error(err);
	return result;
}
inline FileHandle OpenWrite(const char* path)
{
	std::error_code err;
	FileHandle result = OpenWrite(path, err);
	if (err)
		throw std::system_error(err);
	return result;
}
inline FileHandle OpenAppend(const char* path)
{
	std::error_code err;
	FileHandle result = OpenAppend(path, err);
	if (err)
		throw std::system_error(err);
	return result;
}
inline FileHandle OpenEdit(const char* path)
{
	std::error_code err;
	FileHandle result = OpenEdit(path, err);
	if (err)
		throw std::system_error(err);
	return result;
}

// Check if a file exists
EXPORT bool FileExists(const char* path);
EXPORT bool FileExistsInPak(const char* path);

// Move/copy/delete a file
EXPORT void CopyFile(const char* dest, const char* src, std::error_code& err);
EXPORT void CopyFileFromPak(const char* dest, const char* src, std::error_code& err);
EXPORT void MoveFile(const char* dest, const char* src, std::error_code& err);
EXPORT void RemoveFile(const char* path, std::error_code& err);

// Same as above but throws exceptions on error
inline void CopyFile(const char* dest, const char* src)
{
	std::error_code err;
	CopyFile(dest, src, err);
	if (err)
		throw std::system_error(err);
}
inline void CopyFileFromPak(const char* dest, const char* src)
{
	std::error_code err;
	CopyFileFromPak(dest, src, err);
	if (err)
		throw std::system_error(err);
}
inline void MoveFile(const char* dest, const char* src)
{
	std::error_code err;
	MoveFile(dest, src, err);
	if (err)
		throw std::system_error(err);
}
inline void RemoveFile(const char* path)
{
	std::error_code err;
	RemoveFile(path, err);
	if (err)
		throw std::system_error(err);
}

// Get the timestamp (time of last modification) of a file or the pak it is in
EXPORT time_t FileTimestamp(const char* path, std::error_code& err);
EXPORT time_t PakFileTimestamp(const char* path, std::error_code& err);

// Same as above but throws exceptions on error
inline time_t FileTimestamp(const char* path)
{
	std::error_code err;
	time_t result = FileTimestamp(path, err);
	if (err)
		throw std::system_error(err);
	return result;
}
inline time_t PakFileTimestamp(const char* path)
{
	std::error_code err;
	time_t result = PakFileTimestamp(path, err);
	if (err)
		throw std::system_error(err);
	return result;
}

// List all files in a directory matching the given extensions
// If the extension list is empty then all files are returned
// The returned list is not sorted
EXPORT std::vector<std::string> ListFiles(const char* path, std::initializer_list<const char*> ext, std::error_code& err);
EXPORT std::vector<std::string> ListPakFiles(const char* path, std::initializer_list<const char*> ext, std::error_code& err);

// Same as above but searches recursively from the given directory
EXPORT std::vector<std::string> ListFilesRecursive(const char* path, std::initializer_list<const char*> ext, std::error_code& err);
EXPORT std::vector<std::string> ListPakFilesRecursive(const char* path, std::initializer_list<const char*> ext, std::error_code& err);

// Same as above but throws exceptions on error
inline std::vector<std::string> ListFiles(const char* path, std::initializer_list<const char*> ext)
{
	std::error_code err;
	std::vector<std::string> result = ListFiles(path, ext, err);
	if (err)
		throw std::system_error(err);
	return result;
}
inline std::vector<std::string> ListPakFiles(const char* path, std::initializer_list<const char*> ext)
{
	std::error_code err;
	std::vector<std::string> result = ListPakFiles(path, ext, err);
	if (err)
		throw std::system_error(err);
	return result;
}
inline std::vector<std::string> ListFilesRecursive(const char* path, std::initializer_list<const char*> ext)
{
	std::error_code err;
	std::vector<std::string> result = ListFilesRecursive(path, ext, err);
	if (err)
		throw std::system_error(err);
	return result;
}
inline std::vector<std::string> ListPakFilesRecursive(const char* path, std::initializer_list<const char*> ext)
{
	std::error_code err;
	std::vector<std::string> result = ListPakFilesRecursive(path, ext, err);
	if (err)
		throw std::system_error(err);
	return result;
}

// Read an entire file into a string
inline std::string ReadFile(const char* path, std::error_code& err)
{
	std::string result;

	FileHandle file = Filesystem::OpenRead(path, err);
	if (err)
		return result;

	size_t length = file->Length();
	result.resize(length);
	size_t read_length = file->Read(&result[0], length, 0, err);

	// Adjust string length in case of a short read (EOF)
	result.resize(read_length);

	return result;
}

// Write data to a file
inline size_t WriteFile(const char *path, const void *data, size_t length, std::error_code& err)
{
	FileHandle file = Filesystem::OpenWrite(path, err);
	if (err)
		return 0;

	return file->Write(data, length, 0, err);
}
inline size_t WriteFile(const char *path, const std::string& data, std::error_code& err)
{
	return WriteFile(path, data.data(), data.size(), err);
}
inline size_t WriteFile(const char *path, const char* data, std::error_code& err)
{
	return WriteFile(path, data, strlen(data), err);
}

// Same as above but throws exceptions on error
inline std::string ReadFile(const char* path)
{
	std::error_code err;
	std::string result = ReadFile(path, err);
	if (err)
		throw std::system_error(err);
	return result;
}
inline size_t WriteFile(const char *path, const void *data, size_t length)
{
	std::error_code err;
	size_t result = WriteFile(path, data, length, err);
	if (err)
		throw std::system_error(err);
	return result;
}
inline size_t WriteFile(const char *path, const std::string& data)
{
	std::error_code err;
	size_t result = WriteFile(path, data, err);
	if (err)
		throw std::system_error(err);
	return result;
}
inline size_t WriteFile(const char *path, const char* data)
{
	std::error_code err;
	size_t result = WriteFile(path, data, err);
	if (err)
		throw std::system_error(err);
	return result;
}

// Asynchronously write an entire file
// The input data must remain available for the entire duration of the write
template<typename T, typename... Args>
inline void AsyncWriteFile(const char *path, const void *data, size_t length, T&& callback, Args&&... args)
{
	std::error_code err;
	FileHandle file = Filesystem::OpenWrite(path, err);
	if (err) {
		std::bind(std::forward<T>(callback), std::forward<Args>(args)...)(err, 0);
		return;
	}

	file->AsyncWrite(data, length, 0, std::forward<T>(callback), std::forward<Args>(args)...);
}

// Asynchronously read an entire file
// FIXME: replace lambda with a proper class
template<typename T, typename... Args>
inline void AsyncReadFile(const char *path, T&& callback, Args&&... args)
{
	std::error_code err;
	FileHandle file = Filesystem::OpenRead(path, err);
	if (err) {
		std::bind(std::forward<T>(callback), std::forward<Args>(args)...)(err, 0, std::string());
		return;
	}

	// String buffer containing result
	std::shared_ptr<std::string> result = std::make_shared<std::string>();
	size_t length = file->Length();
	result->resize(length);

	// Wrap the given function object in a lambda to return the string result
	auto functor = std::bind(std::forward<T>(callback), std::forward<Args>(args)...);
	file->AsyncRead(&(*result)[0], length, 0,
		[=](std::error_code err, size_t bytes_transferred) {
			result->resize(bytes_transferred);
			functor(err, bytes_transferred, std::move(*result));
		}
	);
}

// std::bind placeholders which can be used in async callbacks
struct error_code {};
struct bytes_transferred {};
struct read_result {}; // std::string result of Filesystem::AsyncReadFile operation

}

// Register the std::bind placeholders
namespace std {

template<> struct is_placeholder<Filesystem::error_code>: public integral_constant<int, 1> {};
template<> struct is_placeholder<Filesystem::bytes_transferred>: public integral_constant<int, 2> {};
template<> struct is_placeholder<Filesystem::read_result>: public integral_constant<int, 3> {};

}
