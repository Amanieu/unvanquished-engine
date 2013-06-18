//@@COPYRIGHT@@

// Console variable system

// Cvar flags
enum {
	CVAR_ARCHIVE = BIT(0), // Saved to configuration file
	CVAR_CHEAT = BIT(1), // Can only be modified when cheats are enabled
	CVAR_ROM = BIT(2), // Read-only and can't be modified by the user
	CVAR_USER = BIT(3), // Was created by the user
	CVAR_MODIFIED = BIT(4), // Was modified since last time this flag was cleared
};

class EXPORT Cvar: public HashTable<Cvar>::Hook, private boost::noncopyable {
public:
	// Hook function which can be called at 2 times:
	// - After a cvar is set
	// - Before a cvar is get
	typedef void (*cvarHook_t)(Cvar *var);

	// Constructors
	Cvar(const char *name, int flags, const char *initialValue,
	     const char *description, float min = 1, float max = -1,
	     cvarHook_t getHook = NULL, cvarHook_t setHook = NULL)
	{
		Init(name, flags, initialValue, description, min, max, getHook, setHook);
	}
	Cvar(const char *name, int flags, const char *initialValue,
	     const char *description, cvarHook_t getHook, cvarHook_t setHook)
	{
		Init(name, flags, initialValue, description, 1, -1, getHook, setHook);
	}

	// General cvar info
	const char *Name() const
	{
		return realVar->name;
	}
	const char *Description() const
	{
		return realVar->description;
	}

	// The force argument will cause the set to ignore the CVAR_LATCH flag and
	// modify the real value directly
	void Set(const char *string, bool force = true)
	{
		float value = atof(string);
		realVar->SetInternal(string, value, value, force);
	}
	void SetInt(int value, bool force = true)
	{
		char string[16];
		snprintf(string, sizeof(string), "%d", value);
		realVar->SetInternal(string, value, value, force);
	}
	void SetFloat(float value, bool force = true)
	{
		char string[16];
		snprintf(string, sizeof(string), "%f", value);
		realVar->SetInternal(string, value, value, force);
	}
	void SetBool(bool value, bool force = true)
	{
		SetInt(value ? 1 : 0, force);
	}

	const char *Get()
	{
		if (realVar->getHook)
			realVar->getHook(this);
		return realVar->stringVal;
	}
	int GetInt()
	{
		if (realVar->getHook)
			realVar->getHook(this);
		return realVar->intVal;
	}
	float GetFloat()
	{
		if (realVar->getHook)
			realVar->getHook(this);
		return realVar->floatVal;
	}
	bool GetBool()
	{
		return GetInt() != 0;
	}

	// Deletes this cvar. This is only allowed for user-created cvars.
	void Delete();

	// Prints the current value of the cvar
	void Print() const;

	// Reset the cvar to its initial value
	void Reset()
	{
		realVar->Set(realVar->initialValue);
	}

	// Flag operations
	int GetFlags() const
	{
		return realVar->flags;
	}
	void SetFlag(int flag)
	{
		realVar->flags |= flag;
	}
	void ClearFlag(int flag)
	{
		realVar->flags &= ~flag;
	}
	bool TestFlag(int flag) const
	{
		return realVar->flags & flag;
	}

	// To test for modifications
	void SetModified()
	{
		SetFlag(CVAR_MODIFIED);
		modifiedFlags |= realVar->flags;
	}
	void ClearModified()
	{
		ClearFlag(CVAR_MODIFIED);
	}
	bool IsModified() const
	{
		return TestFlag(CVAR_MODIFIED);
	}

	// Get flags of cvars that were modified
	static void SetModifiedFlags(int flags)
	{
		modifiedFlags |= flags;
	}
	static int GetModifiedFlags(int flags)
	{
		return modifiedFlags & flags;
	}
	static void ClearModifiedFlags(int flags)
	{
		modifiedFlags &= ~flags;
	}

	// Find a cvar by name, returns NULL if not found.
	static Cvar *Find(const char *name);

	// Find all cvars with a flag set. Set flag to 0 to find all cvars.
	template<typename Func> static void FindFlag(int flag, const Func &func);

	// Complete a cvar name, for autocompletion.
	static void Complete(completionCallback_t callback);

private:
	// Name of the variable
	const char *name;

	// If there is another var with the same name, then this will point to it,
	// and all functions will forward to that var.
	Cvar *realVar;

	int intVal;
	float floatVal;
	const char *stringVal;

	int flags;
	const char *initialValue;
	const char *description;
	float min, max;
	cvarHook_t getHook, setHook;

	// Bitwise OR of all flags of cvars that were modified
	static int modifiedFlags;

	bool Validate(float val);
	void SetInternal(const char *newString, float newFloat, int newInt, bool force);
	void Init(const char *name, int flags, const char *initialValue,
	          const char *description, float min, float max,
	          cvarHook_t getHook, cvarHook_t setHook, bool copy = IS_MODULE);

	// Compare functor used in Cvar::Find
	struct CvarCompare {
		bool operator()(const char *str, const Cvar &cvar)
		{
			return stricmp(str, cvar.Name()) == 0;
		}
	};
};

// Hashtable operations
inline size_t hash_value(const Cvar &cvar)
{
	StringIHash hasher;
	return hasher(cvar.Name());
}
inline bool operator==(const Cvar &a, const Cvar &b)
{
	return !stricmp(a.Name(), b.Name());
}

// Hash table of all cvars
extern EXPORT HashTable<Cvar> cvarTable;

inline Cvar *Cvar::Find(const char *name)
{
	HashTable<Cvar>::iterator iter = cvarTable.find(name, StringIHash(), CvarCompare());
	if (iter == cvarTable.end())
		return NULL;
	else
		return &*iter;
}

template<typename Func> inline void Cvar::FindFlag(int flag, const Func &func)
{
	foreach (Cvar &i, cvarTable) {
		if (!flag || i.TestFlag(flag))
			func(i);
	}
}

inline void Cvar::Complete(completionCallback_t callback)
{
	// Use PrintCvarMatches instead of PrintMatches
	if (callback == PrintMatches)
		callback = PrintCvarMatches;

	foreach (Cvar &i, cvarTable)
		callback(i.name);
}
