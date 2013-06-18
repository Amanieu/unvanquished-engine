//@@COPYRIGHT@@

// Seed the fast generator with the current time
FastRandom fastRandom(time(NULL));

#ifdef _WIN32
typedef int (__stdcall *RtlGenRandom_t)(void *, unsigned long);

// Get the address of RtlGenRandom
static RtlGenRandom_t GetRtlGenRandom() {
	HMODULE hLib = LoadLibrary("ADVAPI32.DLL");
	if (!hLib)
		return NULL;

	return reinterpret_cast<RtlGenRandom_t>(GetProcAddress(hLib, "SystemFunction036"));
}
#endif

void CryptoRandomBytes(void *buffer, int length)
{
	// Call the operating system's random number generator
#ifdef _WIN32
	static RtlGenRandom_t RtlGenRandom = GetRtlGenRandom();
	if (RtlGenRandom && RtlGenRandom(buffer, length))
		return;
#else
	File *file = Filesystem::OpenFile("/dev/urandom", FS_READ, true);
	if (file) {
		int lenRead = file->Read(buffer, length);
		file->Close();
		if (lenRead == length)
			return;
	}
#endif

	// Fall back to a non-secure random number generator
	WarnOnce("Using insecure random number generator");
	for (int i = 0; i < length; i++)
		static_cast<char *>(buffer)[i] = Random() & 0xff;
}
