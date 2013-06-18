//@@COPYRIGHT@@

// Cvar hash table. We must initialize this first because it is required by
// Cvar static constructors.
__init_early(HashTable<Cvar> cvarTable);

int Cvar::modifiedFlags = 0;

// Cvar for enabling cheats
static Cvar com_cheats("com_cheats", CVAR_ROM, "0", "Enables cheats", 0, 1);

// Check if we can set the cvar to this value
inline bool Cvar::Validate(float value)
{
	if (TestFlag(CVAR_ROM)) {
		Warning("%s is read-only", name);
		return false;
	}
	if (TestFlag(CVAR_CHEAT) && !com_cheats.GetBool()) {
		Warning("%s is cheat-protected", name);
		return false;
	}
	if ((min < max) && (value > max || value < min)) {
		Warning("%s is limited to the range [%f, %f]", name, min, max);
		return false;
	}
	return true;
}

void Cvar::SetInternal(const char *newString, float newFloat, int newInt, bool force)
{
	if (!force && !Validate(newFloat))
		return;

	intVal = newInt;
	floatVal = newFloat;
	if (strcmp(stringVal, newString)) {
		FreeString(stringVal);
		stringVal = CopyString(newString);
	}

	SetModified();

	if (setHook)
		setHook(this);
}

void Cvar::Delete()
{
	if (!realVar->TestFlag(CVAR_USER)) {
		Warning("Only user-created cvars can be deleted.");
		return;
	}

	cvarTable.erase(cvarTable.iterator_to(*realVar));

	FreeString(realVar->name);
	FreeString(realVar->description);
	FreeString(realVar->initialValue);
	FreeString(realVar->stringVal);

	// This is a user created cvar so it must have been dynamically allocated
	delete this;
}

void Cvar::Print() const
{
	if (realVar->TestFlag(CVAR_ROM) || !realVar->initialValue[0])
		Msg("%s is: \"%s^7\"", realVar->name, realVar->stringVal);
	else if (!stricmp(realVar->stringVal, realVar->initialValue))
		Msg("%s is: \"%s^7\", the default", realVar->name, realVar->stringVal);
	else
		Msg("%s is: \"%s^7\", default: \"%s^7\"", realVar->name, realVar->stringVal, realVar->initialValue);

	if (realVar->description[0])
		Msg("description: %s", realVar->description);
}

void Cvar::Init(const char *name, int flags, const char *initialValue,
                const char *description, float min, float max,
                cvarHook_t getHook, cvarHook_t setHook, bool copy)
{
	// Check if a cvar with the same name already exists
	realVar = Cvar::Find(name);
	if (realVar) {
		// Allow redefining a user-created cvar with a system-created one, but
		// not the other way.
		if (!(realVar->flags & CVAR_USER)) {
			Warning("Tried to redefined system cvar %s", name);
			return;
		}

		realVar->flags = flags;
		realVar->min = min;
		realVar->max = max;
		realVar->getHook = getHook;
		realVar->setHook = setHook;
		FreeString(realVar->description);
		realVar->description = CopyString(description);
		FreeString(realVar->initialValue);
		realVar->initialValue = CopyString(initialValue);

		// Force back to the initial value if read-only
		if (flags & CVAR_ROM)
			Reset();

		// Mark the variable as modified
		SetModified();

		return;
	}

	// If the cvar is from a module, we must create a copy of it to make sure it
	// presists after the module is unloaded.
	if (copy) {
		realVar = new Cvar(name, flags, initialValue, description, min, max, getHook, setHook);
		return;
	}

	// Copy info
	realVar = this;
	this->name = CopyString(name);
	this->flags = flags;
	this->initialValue = CopyString(initialValue);
	this->description = CopyString(description);
	this->min = min;
	this->max = max;
	this->getHook = getHook;
	this->setHook = setHook;

	// Set the initial values
	stringVal = "";
	Reset();
	SetModified();

	// Add to hash table
	cvarTable.insert_equal(*this);
}
