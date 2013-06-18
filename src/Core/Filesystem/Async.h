//@@COPYRIGHT@@

// Asynchronous file operations

#ifdef _WIN32
#define USE_WIN32_AIO
#else
#define USE_NO_AIO
#endif

// Generic asynchronous operation descriptor
#ifdef USE_WIN32_AIO
struct fsAsync_t: public OVERLAPPED {
	// Whether this is a read or write operation
	bool write;
};
#else
struct fsAsync_t: public LFQueue<fsAsync_t>::Hook {
	// Whether this is a read or write operation
	bool write;

	// Number of bytes to read/write
	int length;

	// Offset from which to read/write
	fsOffset_t offset;

	// Source/destination buffer
	void *buffer;

	// File to read/write to
	OSFile *file;
};
#endif

// Read operation descriptor
struct fsAsyncRead_t: public fsAsync_t, UseMemPool<fsAsyncRead_t> {
	// Callback function that will be called when the operation is completed.
	tr1::function<void(int)> callback;
};

// Write operation descriptor
struct fsAsyncWrite_t: public fsAsync_t, UseMemPool<fsAsyncWrite_t> {
	// Callback function that will be called when the operation is completed.
	tr1::function<void()> callback;
};

// Queue of operations for the async thread
#ifdef USE_WIN32_AIO
static HANDLE completionPort = CreateIoCompletionPort(NULL, NULL, 0, 0);
#else
static LFQueue<fsAsync_t> asyncQueue;
static Mutex asyncLock;
static Semaphore asyncQueueSem;
static bool stopAsync = false;
#endif

// Semaphore to wait for async thread exit
static Semaphore asyncThreadExit;

// Initialize the async system
static void AsyncThread();
static inline void AsyncInit()
{
	Thread::SpawnThread(AsyncThread);

	// Use a second thread if we don't have real aio, so we can issue 2
	// operations at once, to allow the kernel to reorder operations.
#ifdef USE_NO_AIO
	Thread::SpawnThread(AsyncThread);
#endif
}

#ifdef USE_WIN32_AIO

// Thread which recieves all asynchronous I/O completion notices, and executes
// the callbacks.
static void AsyncThread()
{
	while (true) {
		// Get a completion event
		DWORD result;
		ULONG_PTR completionKey;
		OVERLAPPED *overlapped;
		bool success = GetQueuedCompletionStatus(completionPort, &result, &completionKey, &overlapped, INFINITE);

		// NULL overlapped means exit
		if (!overlapped) {
			asyncThreadExit.Post();
			return;
		}

		// Get the async structure
		fsAsync_t *async = static_cast<fsAsync_t *>(overlapped);

		// Handle errors
		if (!success) {
			if (async->write)
				Warning("Error writing to file: %s", System::Win32StrError(GetLastError()));
			else if (GetLastError() != ERROR_HANDLE_EOF)
				result = 0;
		}

		// Run the callback
		if (async->write) {
			fsAsyncWrite_t *realAsync = static_cast<fsAsyncWrite_t *>(async);
			if (realAsync->callback)
				realAsync->callback();
		} else {
			fsAsyncRead_t *realAsync = static_cast<fsAsyncRead_t *>(async);
			if (realAsync->callback)
				realAsync->callback(result);
		}

		// Free the structure
		delete async;
	}
}

// Shut down the async thread
static inline void AsyncShutdown()
{
	// Wait for the async thread to finish working
	PostQueuedCompletionStatus(completionPort, 0, 0, NULL);
	asyncThreadExit.Wait();
}

static inline void AsyncPrepareFile(OSFile *file)
{
	// Associate the file with the completion port
	CreateIoCompletionPort(file->fd, completionPort, 0, 0);
}

static inline void AsyncRequestRead(OSFile *file, void *buffer, int length, fsOffset_t offset, const tr1::function<void(int)> &callback)
{
	// Build the request
	fsAsyncRead_t *async = new fsAsyncRead_t;
	async->write = false;
	async->callback.swap(const_cast<tr1::function<void(int)> &>(callback));
	OVERLAPPED *overlapped = static_cast<OVERLAPPED *>(async);
	memset(overlapped, 0, sizeof(OVERLAPPED));
	overlapped->Offset = offset & 0xFFFFFFFF;
	overlapped->OffsetHigh = offset >> 32;

	// Run the callback directly if the request was completed immediately
	DWORD bytesRead;
	if (ReadFile(file->fd, buffer, length, &bytesRead, overlapped)) {
		if (async->callback)
			async->callback(bytesRead);
		delete async;
	} else if (GetLastError() == ERROR_HANDLE_EOF) {
		if (async->callback)
			async->callback(bytesRead);
		delete async;
	} else if (GetLastError() != ERROR_IO_PENDING) {
		if (async->callback)
			async->callback(0);
		delete async;
	}
}

static inline void AsyncRequestWrite(OSFile *file, const void *data, int length, fsOffset_t offset, const tr1::function<void()> &callback)
{
	// Build the request
	fsAsyncWrite_t *async = new fsAsyncWrite_t;
	async->write = true;
	async->callback.swap(const_cast<tr1::function<void()> &>(callback));
	OVERLAPPED *overlapped = static_cast<OVERLAPPED *>(async);
	memset(overlapped, 0, sizeof(OVERLAPPED));
	overlapped->Offset = offset & 0xFFFFFFFF;
	overlapped->OffsetHigh = offset >> 32;

	// Run the callback directly if the request was completed immediately
	if (WriteFile(file->fd, data, length, NULL, overlapped)) {
		if (async->callback)
			async->callback();
		delete async;
	} else if (GetLastError() != ERROR_IO_PENDING) {
		Warning("Error writing to file: %s", System::Win32StrError(GetLastError()));

		// We also run the callback in the error case, because other code might
		// be waiting for it.
		if (async->callback)
			async->callback();
		delete async;
	}
}

#else

// Thread which recieves all asynchronous I/O requests, and executes them using
// blocking I/O. 2 threads are used to allow more I/O to be processed at once.
static void AsyncThread()
{
	while (true) {
		// Wait for a request
		asyncQueueSem.Wait();

		// Pop request off the queue
		asyncLock.Lock();
		fsAsync_t *async = asyncQueue.Pop();
		asyncLock.Unlock();

		// NULL request means exit
		if (!async) {
			asyncThreadExit.Post();
			return;
		}

		// Ignore all reads if we are shutting down
		if (stopAsync && !async->write) {
			delete async;
			continue;
		}

		// Execute the request and run the callback
		if (async->write) {
			fsAsyncWrite_t *realAsync = static_cast<fsAsyncWrite_t *>(async);
			if (async->file->mode == FS_APPEND)
				async->file->OSFile::Write(async->buffer, async->length);
			else
				async->file->OSFile::WriteEx(async->buffer, async->length, async->offset);
			if (realAsync->callback)
				realAsync->callback();
		} else {
			fsAsyncRead_t *realAsync = static_cast<fsAsyncRead_t *>(async);
			int result = async->file->OSFile::ReadEx(async->buffer, async->length, async->offset);
			if (realAsync->callback)
				realAsync->callback(result);
		}

		// Free the structure
		delete async;
	}
}

// Shut down the async thread
static inline void AsyncShutdown()
{
	// Wait for the async threads to finish working
	stopAsync = true;
	asyncQueueSem.Post(2);
	asyncThreadExit.Wait();
	asyncThreadExit.Wait();
}

static inline void AsyncPrepareFile(OSFile *)
{
}

static inline void AsyncRequestRead(OSFile *file, void *buffer, int length, fsOffset_t offset, const tr1::function<void(int)> &callback)
{
	// Build request
	fsAsyncRead_t *async = new fsAsyncRead_t;
	async->write = false;
	async->file = file;
	async->buffer = buffer;
	async->length = length;
	async->offset = offset;
	async->callback.swap(const_cast<tr1::function<void(int)> &>(callback));

	// Add request to the end of the queue
	asyncQueue.Push(*async);

	// Signal the async thread that there is a new request
	asyncQueueSem.Post();
}

static inline void AsyncRequestWrite(OSFile *file, const void *data, int length, fsOffset_t offset, const tr1::function<void()> &callback)
{
	// Build request
	fsAsyncWrite_t *async = new fsAsyncWrite_t;
	async->write = true;
	async->file = file;
	async->buffer = const_cast<void *>(data);
	async->length = length;
	async->offset = offset;
	async->callback.swap(const_cast<tr1::function<void()> &>(callback));

	// Add request to the end of the queue
	asyncQueue.Push(*async);

	// Signal the async thread that there is a new request
	asyncQueueSem.Post();
}

#endif
