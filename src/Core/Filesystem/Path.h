//@@COPYRIGHT@@

// Path that will be searched when looking for a file

class FSPath: public SList<FSPath>::Hook {
public:
	// Constructor
	FSPath(const char *pathName)
	{
		strlcpy(path, pathName, sizeof(path));
	}

	// Open a file under this path
	File *OpenFile(const char *file, fsMode_t mode)
	{
		char fullPath[MAX_PATH];

		snprintf(fullPath, sizeof(fullPath), "%s/%s", path, file);

		return OSFile::Open(fullPath, mode);
	}

	// Check if a file exists
	bool FileExists(const char *file)
	{
		char fullPath[MAX_PATH];

		snprintf(fullPath, sizeof(fullPath), "%s/%s", path, file);

		return OSFileExists(fullPath);
	}

	// Add matching files to the list list
	void ListFiles(const char *subPath, const char *extension, fsSearch_t searchType, StringList &fileList)
	{
		char fullPath[MAX_PATH];

		snprintf(fullPath, sizeof(fullPath), "%s/%s", path, subPath);

		ListOSFiles(fullPath, extension, searchType, fileList);
	}

	// Actual search path, without trailing slash
	char path[MAX_PATH];
};

// Linked list of search paths for reading.
static SList<FSPath> pathList;

// Path to which all writes are done. This is always the last path added.
#define writePath pathList.front()
