//@@COPYRIGHT@@

// Main editor window

MainWindow *mainWindow;

MainWindow::MainWindow(): wxFrame(NULL, wxID_ANY, wxT("Unvanquished Editor"))
{
	// Start maximized
	Maximize();

	// Allow AUI to manage this window
	auiManager.SetManagedWindow(this);
for (int i = 0; i < 20; i++)
auiManager.AddPane(new RenderCanvas(this), wxLEFT, wxT("X"));
	// Update everything
	auiManager.Update();
}

MainWindow::~MainWindow()
{
	auiManager.UnInit();
}

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
END_EVENT_TABLE()
