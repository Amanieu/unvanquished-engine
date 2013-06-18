//@@COPYRIGHT@@

// Main editor window

class MainWindow: public wxFrame {
public:
	MainWindow();
	~MainWindow();

protected:
	DECLARE_EVENT_TABLE()

private:
	wxAuiManager auiManager;
};

extern MainWindow *mainWindow;
