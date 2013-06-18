//@@COPYRIGHT@@

// Engine initialization and shutdown

namespace Engine {

// Turn all +XXX command line into commands and run them, and print warnings for
// any unrecognized arguments. Run this after all subsystems have been
// initialized.
void RunArgs();

// Search the command-line arguments for a specific command: -cmd arg1 arg2
// Arguments are returned in buffer, which should have size numArgs. You can
// pass NULL instead if you do not require the arguments.
bool GetArg(const char *command, int numArgs, const char **buffer);

// Get the name of the program (argv[0])
const char *GetProgramName();

// Shut down the engine and exit
__noreturn __cold void Quit();

// This function will be called after core initialization is complete. It should
// be implemented by the current target (editor, client, server).
void Init();

// This function will be called during engine shutdown. It should be implemented
// by the current target (editor, client, server).
void Shutdown();

// Returns a string representing the current version of the engine
EXPORT const char *GetVersion();

}
