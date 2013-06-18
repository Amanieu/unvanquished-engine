//@@COPYRIGHT@@

// Console input field

// Maximum length of a console line
#define MAX_CONSOLE_INPUT 512

// Amount of characters the console view is scrolled horizontally each time
#define CONSOLE_SCROLL 16

// Console prompt
#define CONSOLE_PROMPT "^3-> ^7"

class EXPORT ConsoleField {
public:
	ConsoleField()
	{
		histIndex = -1;
		Clear();
	}

	// Returns only the visible portion of the text
	const char *GetText() const
	{
		return buffer + scroll;
	}

	// Returns the cursor position relative to the view
	unsigned int GetCursor() const
	{
		return cursor - scroll;
	}

	// Cursor movement
	void CursorLeft()
	{
		if (cursor == 0)
			return;
		cursor--;
		if (cursor < scroll)
			scroll = std::max(scroll - CONSOLE_SCROLL, 0);
	}
	void CursorRight()
	{
		if (cursor == static_cast<int>(strlen(buffer)))
			return;
		cursor++;
		if (cursor >= scroll + width)
			scroll = cursor - width + CONSOLE_SCROLL;
	}
	void CursorHome()
	{
		cursor = 0;
		scroll = 0;
	}
	void CursorEnd()
	{
		cursor = strlen(buffer);
		if (cursor >= scroll + width)
			scroll = cursor - width + CONSOLE_SCROLL;
	}

	// Deletion
	void Delete()
	{
		if (cursor == static_cast<int>(strlen(buffer)))
			return;
		memmove(buffer + cursor, buffer + cursor + 1, strlen(buffer + cursor + 1) + 1);
	}
	void Backspace()
	{
		if (cursor == 0)
			return;
		CursorLeft();
		Delete();
	}

	// Word delete
	void WordDelete()
	{
		int shift = 0;

		// First find a word
		while (cursor) {
			if (buffer[cursor - 1] != ' ')
				break;
			shift++;
			cursor--;
		}

		// Go through the word
		while (cursor) {
			if (buffer[cursor - 1] == ' ')
				break;
			shift++;
			cursor--;
		}

		// Delete
		memmove(buffer + cursor, buffer + cursor + shift, strlen(buffer + cursor + shift) + 1);
		if (cursor < scroll)
			scroll = std::max(cursor - CONSOLE_SCROLL, 0);
	}

	// Normal character
	void AddChar(char chr)
	{
		if (strlen(buffer) + 1 >= sizeof(buffer) / sizeof(*buffer))
			return;
		memmove(buffer + cursor + 1, buffer + cursor, strlen(buffer + cursor) + 1);
		buffer[cursor] = chr;
		cursor++;
		if (cursor >= scroll + width)
			scroll = cursor - width + CONSOLE_SCROLL;
	}

	// Up and down keys (console history)
	void HistoryUp();
	void HistoryDown();

	// Enter key (run the command)
	void RunCommand();

	// Autocomplete the current command
	void Autocomplete();

	// Change the width of the field
	void SetWidth(int newWidth)
	{
		width = newWidth;
		if (scroll <= cursor - width)
			scroll = cursor - width + 1;
	}

	// Clear the field
	void Clear()
	{
		buffer[0] = '\0';
		cursor = 0;
		scroll = 0;
	}

private:
	int width;
	int cursor;
	int scroll;
	int histIndex;
	char buffer[MAX_CONSOLE_INPUT];
};

// Functions to allow overriding the default autocompletion. See Cvar.cpp
// for an example.
EXPORT void PrintMatches(const char *str);
EXPORT void PrintCvarMatches(const char *str);
