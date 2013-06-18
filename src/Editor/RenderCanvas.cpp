//@@COPYRIGHT@@

// OpenGL context shared between all canvases
static wxGLContext *sharedContext = NULL;

// wxGLCanvas creation attributes
static int canvasAttribs[] = {
	WX_GL_RGBA,
	WX_GL_DOUBLEBUFFER,
	WX_GL_MIN_RED, 8,
	WX_GL_MIN_GREEN, 8,
	WX_GL_MIN_BLUE, 8,
	WX_GL_MIN_ALPHA, 0,
	WX_GL_DEPTH_SIZE, 0,
	WX_GL_STENCIL_SIZE, 0,
	0
};

RenderCanvas::RenderCanvas(wxWindow *parent, wxWindowID id): wxGLCanvas(parent, id, canvasAttribs)
{
	// Disable background to reduce flickering
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);

	// First make sure the window is actually created
	Show();

	// Now see if we need to create an OpenGL context
	if (!sharedContext)
		sharedContext = new wxGLContext(this);
}

void RenderCanvas::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
	// Very horrible hack that magically fixes flickering when resizing
	if (x == 0)
		x = wxDefaultCoord;
	if (y == 0)
		y = wxDefaultCoord;
	wxGLCanvas::DoSetSize(x, y, width, height, sizeFlags);
}

void RenderCanvas::OnPaint(wxPaintEvent &)
{
	// Set up rendering
	sharedContext->SetCurrent(*this);
	wxPaintDC dc(this);

	// Adjust viewport
	glViewport(0, 0, (GLint)GetSize().x, (GLint)GetSize().y);
	glClearColor(1, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Display stuff
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(1.0f, -1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(-1.0f, -1.0f);
	glEnd();

	// Render
	SwapBuffers();
}

BEGIN_EVENT_TABLE(RenderCanvas, wxGLCanvas)
	EVT_PAINT(RenderCanvas::OnPaint)
END_EVENT_TABLE()
