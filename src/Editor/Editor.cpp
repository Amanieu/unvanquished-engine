//@@COPYRIGHT@@

// Editor initialization and shutdown

// wxWidgets application class
class MyApp: public wxApp {
public:
	// Initialization function
	bool OnInit()
	{
		Engine::RunArgs();
		mainWindow = new MainWindow();
		mainWindow->Show(true);
		SetTopWindow(mainWindow);
		return true;
	}

	// Shutdown function
	int OnExit()
	{
		Engine::Quit();
	}
};

// Using these instead of IMPLEMENT_APP
IMPLEMENT_APP_NO_MAIN(MyApp)
IMPLEMENT_WX_THEME_SUPPORT

void Engine::Shutdown()
{
	// GUI thread is shutting down
	if (ThreadPool::GetThreadId() == 0)
		return;

	// Destroy the top-level window
	wxCloseEvent event(wxEVT_CLOSE_WINDOW, mainWindow->GetId());
	event.SetEventObject(mainWindow);
	event.SetCanVeto(false);
	mainWindow->GetEventHandler()->AddPendingEvent(event);

	// Give the GUI thread time to shut down
	System::Sleep(10000);

	// Force shutdown by returning
}

// Quit command
static void Quit_f(CmdArgs *args)
{
	// Command help
	if (!args) {
		Msg("usage: quit");
		Msg("Exits the editor.");
		return;
	}

	// Close the main window
	wxCloseEvent event(wxEVT_CLOSE_WINDOW, mainWindow->GetId());
	event.SetEventObject(mainWindow);
	event.SetCanVeto(true);
	mainWindow->GetEventHandler()->AddPendingEvent(event);
}

void Engine::Init()
{
	// Build a fake set of args
	char *argv = const_cast<char *>(Engine::GetProgramName());
	int argc = 1;

	// Register quit command
	Cmd::Register("quit", Quit_f);

	// Start wxWidgets
	wxEntry(argc, &argv);

	// If we reach this point, we are shutting down
	Engine::Quit();
}
