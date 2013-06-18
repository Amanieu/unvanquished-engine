//@@COPYRIGHT@@

// Server initialization, shutdown and main loop

// Flag to stop the main loop
static bool stopLoop = false;

// Quit command
static void Quit_f(CmdArgs *args)
{
	// Command help
	if (!args) {
		Msg("usage: quit");
		Msg("Exits the game.");
		return;
	}

	// Set shutdown flag
	stopLoop = true;
}

void Engine::Init()
{
	Cmd::Register("quit", Quit_f);
	Engine::RunArgs();

	while (!stopLoop)
		System::Sleep(100);

	Engine::Quit();
}

void Engine::Shutdown()
{
}
