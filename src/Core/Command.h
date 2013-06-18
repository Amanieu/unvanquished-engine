//@@COPYRIGHT@@

// Console command system

// Arguments to a console command
#define MAX_CMD_ARGS 16
#define MAX_CMD_STRING 512
class EXPORT CmdArgs {
public:
	CmdArgs()
	{
		argc = 0;
	}

	// Tokenize the string into args
	CmdArgs(const char *__restrict text, int length, bool expandCvars);

	// Build args directly from strings
	CmdArgs(int _argc, char **_argv)
	{
		argc = _argc;

		char *out = cmd;
		for (int i = 0; i < _argc; i++) {
			argv[i] = out;
			strlcpy(out, _argv[i], cmd - out + sizeof(cmd));
			out += strlen(_argv[i]) + 1;
		}
	}

	// Returns the number of arguments
	int Argc() const
	{
		return argc;
	}

	// Returns one argument
	const char *Argv(int arg) const
	{
		Assert(arg >= 0);
		if (arg < argc)
			return argv[arg];
		else
			return "";
	}

	// Returns a string containing all args starting from start. This string
	// uses a static buffer and is only valid until the next call to Args().
	const char *Args(int start = 1);

private:
	int argc;
	char *argv[MAX_CMD_ARGS];
	char cmd[MAX_CMD_STRING];
	char args[MAX_CMD_STRING];
};

// Function to handle a command.
typedef void (*commandFunc_t)(CmdArgs *args);

// Function to auto-complete a command. Call the callback with every possible
// value for the last argument. Argument 0 is always the command name.
typedef void (*completionCallback_t)(const char *str);
typedef void (*completionFunc_t)(CmdArgs *args, int argNum, completionCallback_t callback);

// Console command descriptor
struct cmd_t: public HashTable<cmd_t>::Hook {
	const char *name;
	commandFunc_t command;
	completionFunc_t complete;
};

namespace Cmd {

// Register all basic commands
void Init();

// Register a command with the command subsystem
EXPORT void Register(const char *name, commandFunc_t command, completionFunc_t complete = NULL);
EXPORT void UnRegister(const char *name);

// Find a command by name
EXPORT cmd_t *Find(const char *name);

// Complete a command name
EXPORT void Complete(completionCallback_t callback);

// Find the next subcommand in a string, separated by ; or \n. Returns NULL if
// this was the last one.
EXPORT const char *SplitCommand(const char *__restrict text, int length);

// Execute a command string. The string may contain multiple commands separated
// by ; or \n.
EXPORT void ExecuteString(const char *__restrict string);

// Execute a set of args directly.
EXPORT void RunArgs(CmdArgs *args);

}
