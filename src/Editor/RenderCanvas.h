//@@COPYRIGHT@@

// Widget for rendering a renderer view

class RenderCanvas: public wxGLCanvas {
public:
	RenderCanvas(wxWindow *parent, wxWindowID id = wxID_ANY);

protected:
	DECLARE_EVENT_TABLE()

private:
	void OnPaint(wxPaintEvent &);
	void DoSetSize(int x, int y, int width, int height, int sizeFlags);
};
