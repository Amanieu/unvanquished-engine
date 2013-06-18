//@@COPYRIGHT@@

const char *CmdArgs::Args(int start)
{
	char *p = args;

	Assert(start >= 0);

	if (start >= argc)
		return "";

	for (int i = start; i < argc; i++) {
		if (p + strlen(argv[i]) >= args + sizeof(args))
			break;
		strcpy(p, argv[i]);
		p += strlen(argv[i]);
		if (i != argc - 1)
			*p++ = ' ';
	}

	*p = '\0';

	return args;
}

CmdArgs::CmdArgs(const char *text, int length, bool expandCvars)
{
	const char *in;
	char *out;

	argc = 0;

	// Split the text
	in = text;
	out = cmd;
	while (in - text < length) {
		if (argc == MAX_CMD_ARGS || out >= cmd + sizeof(cmd))
			return;

		// Skip whitespace
		while (in - text < length && isspace(*in))
			in++;
		if (in - text == length)
			return;

		// Skip C++-style // comments
		if (in - text < length - 1 && in[0] == '/' && in[1] == '/')
			return;

		// Found a token
		argv[argc++] = out;
		bool foundCvar = *in == '$';

		// Copy token
		while (in - text < length && !isspace(*in)) {
			if (out >= cmd + sizeof(cmd) - 1)
				break;

			// Handle quoted strings
			if (in[0] == '"') {
				in++;
				while (in - text < length && *in != '"') {
					if (out >= cmd + sizeof(cmd) - 1)
						break;
					if (in - text < length - 1 && in[0] == '\\' && in[1] == '"') {
						*out++ = '"';
						in += 2;
					} else
						*out++ = *in++;
				}
				in++;
				continue;
			}

			// Quote escaping
			if (in - text < length - 1 && in[0] == '\\' && text[1] == '"') {
				*out++ = '"';
				in += 2;
				continue;
			}

			// Comment
			if (in - text < length - 1 && in[0] == '/' && in[1] == '/')
				break;

			*out++ = *in++;
		}

		// End token
		*out++ = '\0';

		// Check for cvar substitution
		if (expandCvars && foundCvar) {
			Cvar *var = Cvar::Find(argv[argc - 1] + 1);
			const char *str = var ? var->Get() : "<invalid cvar>";
			strlcpy(argv[argc - 1], str, argv[argc - 1] - cmd + sizeof(cmd));
			out = argv[argc - 1] + strlen(str) + 1;
		}
	}
}

// Hashtable operations
static inline size_t hash_value(const cmd_t &cmd)
{
	StringIHash hasher;
	return hasher(cmd.name);
}
static inline bool operator==(const cmd_t &a, const cmd_t &b)
{
	return !stricmp(a.name, b.name);
}

// Hash table of all commands
static HashTable<cmd_t> cmdTable;

// Compare functor used in Cmd::Find
struct CmdCompare {
	bool operator()(const char *str, const cmd_t &cmd)
	{
		return stricmp(str, cmd.name) == 0;
	}
};

cmd_t *Cmd::Find(const char *name)
{
	HashTable<cmd_t>::iterator iter = cmdTable.find(name, StringIHash(), CmdCompare());
	if (iter == cmdTable.end())
		return NULL;
	else
		return &*iter;
}

void Cmd::Complete(completionCallback_t callback)
{
	foreach (cmd_t &i, cmdTable)
		callback(i.name);
}

void Cmd::Register(const char *name, commandFunc_t command, completionFunc_t complete)
{
	cmd_t *cmd = new cmd_t;

	cmd->name = CopyString(name);
	cmd->command = command;
	cmd->complete = complete;

	bool success = cmdTable.insert_unique(*cmd).second;

	if (!success)
		Warning("Command %s already exists", name);
}

void Cmd::UnRegister(const char *name)
{
	cmd_t *cmd = Cmd::Find(name);
	if (!cmd)
		return;

	cmdTable.erase(cmdTable.iterator_to(*cmd));
	FreeString(cmd->name);
}

void Cmd::RunArgs(CmdArgs *args)
{
	if (!args->Argc())
		return;

	const char *name = args->Argv(0);

	// Check for a command with that name
	cmd_t *cmd = Cmd::Find(name);
	if (cmd) {
		cmd->command(args);
		return;
	}

	// Check for a cvar with that name
	Cvar *var = Cvar::Find(name);
	if (var) {
		if (args->Argc() == 1)
			var->Print();
		else
			var->Set(args->Args(), false);
		return;
	}

	// Print an error message
	Warning("Unknown command %s", name);
}

const char *Cmd::SplitCommand(const char *text, int length)
{
	bool inQuote = false;
	bool inComment = false;

	while (length) {
		// Comment
		if (length > 1 && text[0] == '/' && text[1] == '/') {
			inComment = true;
			length -= 2;
			text += 2;
			continue;
		}

		// Escaped quote
		if (length > 1 && text[0] == '\\' && text[1] == '"') {
			length -= 2;
			text += 2;
			continue;
		}

		// Quote
		if (text[0] == '"') {
			inQuote = !inQuote;
			length--;
			text++;
			continue;
		}

		// Semicolon
		if (text[0] == ';' && !inQuote && !inComment)
			return text + 1;

		// Newline, extra check for \n at end of file
		if (text[0] == '\n') {
			if (length == 1)
				return NULL;
			else
				return text + 1;
		}

		// Normal character
		length--;
		text++;
	}

	return NULL;
}

void Cmd::ExecuteString(const char *string)
{
	int length = strlen(string);

	do {
		const char *next = Cmd::SplitCommand(string, length);
		CmdArgs args(string, next ? next - string - 1 : length, true);
		Cmd::RunArgs(&args);
		length -= next - string;
		string = next;
	} while (string);
}

// Echo command
static void Echo_f(CmdArgs *args)
{
	// Command help
	if (!args) {
		Msg("usage: echo <text>");
		Msg("Echos <text> to the console.");
		return;
	}

	// Print the text
	Msg("%s", args->Args());
}

// Help command
static void Help_c(CmdArgs *, int argNum, completionCallback_t callback)
{
	if (argNum == 1) {
		Cmd::Complete(callback);
		Cvar::Complete(callback);
	}
}
static void Help_f(CmdArgs *args)
{
	// Command help
	if (!args || args->Argc() != 2) {
		Msg("usage: help <name>");
		Msg("Shows help for <name>, which can be a cvar or a command.");
		return;
	}

	// Look for a command
	cmd_t *cmd = Cmd::Find(args->Argv(1));
	if (cmd) {
		cmd->command(NULL);
		return;
	}

	// Look for a cvar
	Cvar *var = Cvar::Find(args->Argv(1));
	if (var) {
		if (var->Description()[0])
			Msg("%s: %s", var->Name(), var->Description());
		else
			Msg("%s has no help available", var->Name());
		return;
	}

	// Nothing found
	Msg("%s is not a command or cvar name", args->Argv(1));
}

// Cmdlist command
static void CmdList_f(CmdArgs *args)
{
	// Command help
	if (!args) {
		Msg("usage: cmdlist [filter]");
		Msg("Lists all commands which match [filter].");
		return;
	}

	// Loop through all commands
	int numCmd = 0;
	foreach (cmd_t &i, cmdTable) {
		if (args->Argc() >= 2 && !GlobMatch(args->Argv(1), i.name))
			continue;
		numCmd++;
		Msg("    %s", i.name);
	}

	// Print command count
	Msg("%d commands", numCmd);
}

// Exec command
static void Exec_c(CmdArgs *, int argNum, completionCallback_t callback)
{
	if (argNum == 1) {
		// Search for all script files
		StringList list;
		Filesystem::ListFiles(list, "", ".cfg", FS_SEARCH_FILES);
		std::sort(list.begin(), list.end(), StringICompare());
		foreach (const char *i, list)
			callback(i);
	}
}
static void Exec_f(CmdArgs *args)
{
	// Command help
	if (!args || args->Argc() != 2) {
		Msg("usage: exec <file>");
		Msg("Executes the contents of <file> as console commands.");
		return;
	}

	// Read the target file
	char *buffer = static_cast<char *>(Filesystem::ReadFile(args->Argv(1), NULL, true));
	if (!buffer) {
		Warning("Couldn't exec %s", args->Argv(1));
		return;
	}

	// Execute the buffer and free it
	Cmd::ExecuteString(buffer);
	MemFree(buffer);
}

// Set command
static void Set_c(CmdArgs *args, int argNum, completionCallback_t callback)
{
	if (argNum == 1)
		Cvar::Complete(callback);
	else if (argNum == 2) {
		Cvar *var = Cvar::Find(args->Argv(1));
		if (var)
			callback(var->Get());
	}
}
static void Set_f(CmdArgs *args)
{
	// Command help
	if (!args || args->Argc() < 2) {
		Msg("usage: set <variable> <value>");
		Msg("Sets the cvar <variable> to <value>. If <variable> doesn't exist, then it is created.");
		return;
	}

	Cvar *var = Cvar::Find(args->Argv(1));

	// If only one argument, print the cvar.
	if (args->Argc() == 2) {
		if (var)
			var->Print();
		else
			Msg("Cvar %s does not exist", args->Argv(1));
		return;
	}

	// Set the cvar
	if (!var)
		var = new Cvar(args->Argv(1), CVAR_USER, "", "");
	var->Set(args->Args(2), false);
}

// Unset command
static void Unset_c(CmdArgs *, int argNum, completionCallback_t callback)
{
	if (argNum == 1)
		Cvar::Complete(callback);
}
static void Unset_f(CmdArgs *args)
{
	// Command help
	if (!args || args->Argc() != 2) {
		Msg("usage: unset <variable>");
		Msg("Deletes the cvar <variable>.");
		return;
	}

	Cvar *var = Cvar::Find(args->Argv(1));

	if (var)
		var->Delete();
	else
		Msg("Cvar %s does not exist", args->Argv(1));
}

// Cvarlist command
static void CvarList_f(CmdArgs *args)
{
	// Command help
	if (!args) {
		Msg("usage: cvarlist [filter]");
		Msg("Lists all cvars which match [filter].");
		return;
	}

	// Loop through all cvars
	int numCvar = 0;
	foreach (Cvar &i, cvarTable) {
		if (args->Argc() >= 2 && !GlobMatch(args->Argv(1), i.Name()))
			continue;
		numCvar++;
		Msg("    %s = %s", i.Name(), i.Get());
	}

	// Print command count
	Msg("%d cvars", numCvar);
}

void Cmd::Init()
{
	Cmd::Register("echo", Echo_f);
	Cmd::Register("help", Help_f, Help_c);
	Cmd::Register("cmdlist", CmdList_f);
	Cmd::Register("exec", Exec_f, Exec_c);
	Cmd::Register("set", Set_f, Set_c);
	Cmd::Register("unset", Unset_f, Unset_c);
	Cmd::Register("cvarlist", CvarList_f);
}
