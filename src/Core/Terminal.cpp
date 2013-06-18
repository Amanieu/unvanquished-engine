//@@COPYRIGHT@@

// NCurses/PDCurses-based terminal

static bool cursesOn = false;

// Basic print function for when not using curses
static void BasicPrint(printLevel_t level, const char *msg)
{
	char buffer[MAX_PRINTMSG];

	// Appropriate prefix for the error level
	switch (level) {
	case PRINT_ERROR:
		strcpy(buffer, "ERROR: ");
		break;
	case PRINT_WARNING:
		strcpy(buffer, "WARNING: ");
		break;
	case PRINT_MESSAGE:
	case PRINT_DEBUG:
		buffer[0] = '\0';
		break;
	NO_DEFAULT
	}

	strlcat(buffer, msg, sizeof(buffer));

	// Add a newline at the end
	unsigned int len = strlen(buffer);
	if (len >= sizeof(buffer) - 1)
		len = sizeof(buffer) - 2;
	buffer[len++] = '\n';
	buffer[len] = '\0';

	// Strip color codes from the message
	StripColor(buffer);

	fputs(buffer, stdout);
}

#ifdef USE_CURSES

#include <curses.h>

#define TITLE "^2[ ^3Unvanquished Console ^2]"
#define INPUT_SCROLL 15
#define LOG_SCROLL 5
#define MAX_LOG_LINES 1024
#define CURSES_REFRESH_RATE 100

static ConsoleField inputField;
static WINDOW *logWin = NULL;
static WINDOW *inputWin = NULL;
static WINDOW *clockWin = NULL;

static Mutex cursesLock;

static int scrollLine = 0;
static int lastLine = 1;
static bool forceRedraw = false;

static short savedColor[3];

#define LOG_LINES (LINES - 3)

static inline void SetColor(WINDOW *win, int color)
{
	// We don't want pure black, use grey instead.
	// Using (void) to shut up warnings about unused return values.
	if (color == 0 && has_colors() && !can_change_color())
		(void)wattrset(win, COLOR_PAIR(color + 1) | A_BOLD);
	else
		(void)wattrset(win, COLOR_PAIR(color + 1) | A_NORMAL);
}

static inline void UpdateCursor()
{
// pdcurses uses a different mechanism to move the cursor than ncurses
#ifdef _WIN32
	move(LINES - 1, ColorStrlen(CONSOLE_PROMPT) + 8 + inputField.GetCursor());
	wnoutrefresh(stdscr);
#else
	wmove(inputWin, 0, inputField.GetCursor());
	wnoutrefresh(inputWin);
#endif
}

static void ColorPrint(WINDOW *win, const char *msg, bool stripcodes)
{
	char buffer[MAX_PRINTMSG];
	int length = 0;

	SetColor(win, 7);
	while (*msg) {
		if (IsColorString(msg) || *msg == '\n') {
			// First empty the buffer
			if (length > 0) {
				buffer[length] = '\0';
				waddstr(win, buffer);
				length = 0;
			}

			if (*msg == '\n') {
				// Reset the color and then print a newline
				SetColor(win, 7);
				waddch(win, '\n');
				msg++;
			} else {
				// Set the color
				SetColor(win, msg[1] - '0');

				if (stripcodes)
					msg += 2;
				else {
					if (length >= MAX_PRINTMSG - 3)
						break;

					buffer[length++] = *msg++;
					buffer[length++] = *msg++;
				}
			}
		} else {
			if (length >= MAX_PRINTMSG - 1)
				break;

			buffer[length++] = *msg++;
		}
	}

	// Empty anything still left in the buffer
	if (length > 0) {
		buffer[length] = '\0';
		waddstr(win, buffer);
	}
}

static void UpdateClock()
{
	char buffer[17];
	time_t t = time(NULL);
	struct tm *realtime = localtime(&t);
	werase(clockWin);
	snprintf(buffer, sizeof(buffer) / sizeof(*buffer), "^0[^3%02d%c%02d^0]^7 ", realtime->tm_hour, (realtime->tm_sec & 1) ? ':' : ' ', realtime->tm_min);
	ColorPrint(clockWin, buffer, true);
	wnoutrefresh(clockWin);
}

static void Redraw()
{
	int col;

	// This function is called both for resizes and initialization
	if (logWin) {
		// Update the window size
#ifndef _WIN32
		struct winsize winsz = {0, 0, 0, 0};

		ioctl(fileno(stdout), TIOCGWINSZ, &winsz);
		if (winsz.ws_col < 12 || winsz.ws_row < 5)
			return;
		resizeterm(winsz.ws_row, winsz.ws_col);
#endif

		// Delete all windows
		delwin(logWin);
		delwin(inputWin);
		delwin(clockWin);
		erase();
		wnoutrefresh(stdscr);
	}

	// Create the log window
	logWin = newpad(MAX_LOG_LINES, COLS);
	scrollok(logWin, TRUE);
	idlok(logWin, TRUE);
	ColorPrint(logWin, Log::GetBuffer(), true);
	Log::ReleaseBuffer();
	getyx(logWin, lastLine, col);
	if (col)
		lastLine++;
	scrollLine = lastLine - LOG_LINES;
	if (scrollLine < 0)
		scrollLine = 0;
	pnoutrefresh(logWin, scrollLine, 0, 1, 0, LOG_LINES, COLS);

	// Create the input field
	inputWin = newwin(1, COLS - ColorStrlen(CONSOLE_PROMPT) - 8, LINES - 1, ColorStrlen(CONSOLE_PROMPT) + 8);
	inputField.SetWidth(COLS - ColorStrlen(CONSOLE_PROMPT) - 9);
	UpdateCursor();
	wnoutrefresh(inputWin);

	// Create the clock
	clockWin = newwin(1, 8, LINES - 1, 0);
	UpdateClock();

	// Create the border
	SetColor(stdscr, 2);
	for (int i = 0; i < COLS; i++) {
		mvaddch(0, i, ACS_HLINE);
		mvaddch(LINES - 2, i, ACS_HLINE);
	}

	// Display the title bar
	move(0, (COLS - ColorStrlen(TITLE)) / 2);
	ColorPrint(stdscr, TITLE, true);

	// Display the input prompt
	move(LINES - 1, 8);
	ColorPrint(stdscr, CONSOLE_PROMPT, true);

	wnoutrefresh(stdscr);
	doupdate();
}

static void TTY_Clear_f(CmdArgs *)
{
	// Clear the window
	werase(logWin);
	pnoutrefresh(logWin, scrollLine, 0, 1, 0, LOG_LINES, COLS);

	// Move the cursor back to the input field
	UpdateCursor();
	doupdate();
}

static void Print(printLevel_t level, const char *msg)
{
	int col;
	char buffer[MAX_PRINTMSG];

	if (!cursesOn)
		return;

	if (level == PRINT_ERROR) {
		// Shutdown curses now because the screen is cleared on shutdown, and
		// we don't want the error message to get cleared.
		Terminal::Shutdown();
		BasicPrint(level, msg);
		return;
	}

	// Appropriate prefix for the error level
	switch (level) {
	case PRINT_WARNING:
		strcpy(buffer, "^3WARNING: ");
		break;
	case PRINT_MESSAGE:
		buffer[0] = '\0';
		break;
	case PRINT_DEBUG:
		strcpy(buffer, "^5");
		break;
	NO_DEFAULT
	}

	strlcat(buffer, msg, sizeof(buffer));

	// Add a newline at the end if there isn't one.
	unsigned int len = strlen(buffer);
	if (len == sizeof(buffer) - 1)
		len = sizeof(buffer) - 2;
	buffer[len++] = '\n';
	buffer[len] = '\0';

	AutoLock lock(cursesLock);

	// Print the message in the log window
	ColorPrint(logWin, buffer, true);
	getyx(logWin, lastLine, col);
	if (col)
		lastLine++;
	scrollLine = lastLine - LOG_LINES;
	if (scrollLine < 0)
		scrollLine = 0;
	pnoutrefresh(logWin, scrollLine, 0, 1, 0, LOG_LINES, COLS);

	// Move the cursor back to the input field
	UpdateCursor();
	doupdate();
}

static inline void Update()
{
	int numChars = 0;
	static int lastTime = -1;

	AutoLock lock(cursesLock);

	if (time(NULL) != lastTime) {
		lastTime = time(NULL);
		UpdateClock();
		numChars++;
	}

	while (true) {
		int chr = getch();
		numChars++;

		switch (chr) {
		case ERR:
			if (forceRedraw) {
				forceRedraw = false;
				Redraw();
			} else if (numChars > 1) {
				werase(inputWin);
				ColorPrint(inputWin, inputField.GetText(), false);
#ifdef _WIN32
				// Need this for pdcurses to display the cursor properly
				wrefresh(inputWin);
#else
				wnoutrefresh(inputWin);
#endif
				UpdateCursor();
				doupdate();
			}
			return;

		case KEY_ENTER:
		case '\n':
		case '\r':
			lock.Unlock();
			inputField.RunCommand();
			lock.Lock();
			continue;
		case KEY_STAB:
		case '\t':
			lock.Unlock();
			inputField.Autocomplete();
			lock.Lock();
			continue;
		case KEY_LEFT:
			inputField.CursorLeft();
			continue;
		case KEY_RIGHT:
			inputField.CursorRight();
			continue;
		case KEY_UP:
			inputField.HistoryUp();
			continue;
		case KEY_DOWN:
			inputField.HistoryDown();
			continue;
		case KEY_HOME:
			inputField.CursorHome();
			continue;
		case KEY_END:
			inputField.CursorEnd();
			continue;
		case KEY_NPAGE:
			if (scrollLine < lastLine - LOG_LINES) {
				scrollLine = std::min(scrollLine + LOG_SCROLL, lastLine - LOG_LINES);
				pnoutrefresh(logWin, scrollLine, 0, 1, 0, LOG_LINES, COLS);
			}
			if (scrollLine >= lastLine - LOG_LINES) {
				SetColor(stdscr, 2);
				for (int i = COLS - 7; i < COLS - 1; i++)
					mvaddch(LINES - 2, i, ACS_HLINE);
			}
			continue;
		case KEY_PPAGE:
			if (scrollLine > 0) {
				scrollLine = std::max(scrollLine - LOG_SCROLL, 0);
				pnoutrefresh(logWin, scrollLine, 0, 1, 0, LOG_LINES, COLS);
			}
			if (scrollLine < lastLine - LOG_LINES) {
				SetColor(stdscr, 1);
				mvaddstr(LINES - 2, COLS - 7, "(more)");
			}
			continue;
		case KEY_BACKSPACE:
		case '\b':
		case 127:
			inputField.Backspace();
			continue;
		case KEY_DC:
			inputField.Delete();
			continue;
		case 'w' - 'a' + 1:
			inputField.WordDelete();
			continue;
		default:
			if (isprint(chr))
				inputField.AddChar(chr);
			continue;
		}
	}
}

static void UpdateThread()
{
	while (true) {
		// Sleep until we get input from stdin, or 100ms have passed
#ifdef _WIN32
		WaitForSingleObject(GetStdHandle(STD_INPUT_HANDLE), CURSES_REFRESH_RATE);
#else
		struct timeval timeout;
		fd_set fdset;

		FD_ZERO(&fdset);
		FD_SET(STDIN_FILENO, &fdset);
		timeout.tv_sec = CURSES_REFRESH_RATE / 1000;
		timeout.tv_usec = CURSES_REFRESH_RATE * 1000;
		select(STDIN_FILENO + 1, &fdset, NULL, NULL, &timeout);
#endif

		Update();
	}
}

#ifndef _WIN32
static void Resize(int)
{
	// Don't call Redraw() directly, because it is slow, and we might miss more
	// resize signals while redrawing.
	forceRedraw = true;
}
#endif

static inline bool CursesInit()
{
	// Make sure we're on a real terminal
	if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
		return false;
#ifndef _WIN32
	const char* term = getenv("TERM");
	if (term && (!strcmp(term, "raw") || !strcmp(term, "dumb")))
		return false;

	// Enable more colors
	if (!strncmp(term, "xterm", 5) || !strncmp(term, "screen", 6))
		setenv("TERM", "xterm-256color", 1);
#endif

	// Initialize curses and set up the root window
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	meta(stdscr, TRUE);
	wnoutrefresh(stdscr);

	// Set up colors
	if (has_colors()) {
		use_default_colors();
		start_color();
		init_pair(1, COLOR_BLACK, -1);
		init_pair(2, COLOR_RED, -1);
		init_pair(3, COLOR_GREEN, -1);
		init_pair(4, COLOR_YELLOW, -1);
		init_pair(5, COLOR_BLUE, -1);
		init_pair(6, COLOR_CYAN, -1);
		init_pair(7, COLOR_MAGENTA, -1);
		init_pair(8, -1, -1);
		init_pair(9, -1, -1);
		init_pair(10, -1, -1);

		// Use dark grey instead of black
		if (can_change_color()) {
			color_content(COLOR_BLACK, &savedColor[0], &savedColor[1], &savedColor[2]);
			init_color(COLOR_BLACK, 250, 250, 250);
		}
	}

#ifndef _WIN32
	// Catch window resizes
	System::AttachSignalHandler(SIGWINCH, Resize);
#endif

	// Some libraries (such as OpenAL) insist on printing to stderr, which
	// messes up ncurses. So we just shut them up.
	fclose(stderr);

	// Create everything
	Redraw();

	RegisterPrintHandler(Print);

	Cmd::Register("tty_clear", TTY_Clear_f);

	// Set up a thread to handle input and redrawing
	Thread::SpawnThread(UpdateThread);

	return true;
}

static inline void CursesShutdown()
{
	RemovePrintHandler(Print);

	// Restore colors
	if (can_change_color())
		init_color(COLOR_BLACK, savedColor[0], savedColor[1], savedColor[2]);

	erase();
	refresh();
	endwin();
	RegisterPrintHandler(BasicPrint);
	Cmd::UnRegister("tty_clear");
}

#else

static inline bool CursesInit()
{
	return false;
}

static inline void CursesShutdown()
{
}

#endif

void Terminal::Init()
{
#ifdef BUILD_SERVER
	bool useCurses = true;
#else
	bool useCurses = false;
#endif

	if (Engine::GetArg("-curses", 0, NULL))
		useCurses = true;
	else if (Engine::GetArg("-nocurses", 0, NULL))
		useCurses = false;

	if (useCurses)
		cursesOn = CursesInit();

	// Fall back to a basic print handler
	if (!cursesOn)
		RegisterPrintHandler(BasicPrint);
}

void Terminal::Shutdown()
{
	if (cursesOn) {
		cursesOn = false;
		CursesShutdown();
	}
}
