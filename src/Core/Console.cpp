//@@COPYRIGHT@@

// Autocompletion data
static __thread struct {
	const char *completionString;
	char shortestMatch[MAX_CMD_STRING];
	int matchCount;
} autoCompleteData;

// Console history, shared between all consoles
#define HISTORY_LENGTH 128
#define HISTORY_MASK (HISTORY_LENGTH - 1)
static char history[HISTORY_LENGTH][MAX_CONSOLE_INPUT] = {""};
static int histNext = 0;
static Mutex histLock;

static inline void HistoryAdd(const char *buffer)
{
	if (strcmp(buffer, history[(histNext - 1) & HISTORY_MASK]))
		strcpy(history[histNext++ & HISTORY_MASK], buffer);
}

void ConsoleField::HistoryUp()
{
	histLock.Lock();
	if (histIndex == -1) {
		if (history[(histNext - 1) & HISTORY_MASK][0]) {
			histIndex = histNext - 1;
			if (buffer[0])
				HistoryAdd(buffer);
			strcpy(buffer, history[histIndex & HISTORY_MASK]);
		}
	} else if (((histIndex - 1) & HISTORY_MASK) != (histNext & HISTORY_MASK) &&
	           history[(histIndex - 1) & HISTORY_MASK][0]) {
		if (strcmp(buffer, history[histIndex & HISTORY_MASK]))
			HistoryAdd(buffer);
		histIndex--;
		strcpy(buffer, history[histIndex & HISTORY_MASK]);
	}
	histLock.Unlock();
	cursor = strlen(buffer);
	if (cursor >= width)
		scroll = cursor - width + CONSOLE_SCROLL;
}

void ConsoleField::HistoryDown()
{
	histLock.Lock();
	if (histIndex == -1) {
		if (buffer[0])
			HistoryAdd(buffer);
		buffer[0] = '\0';
	} else {
		histIndex++;
		if (strcmp(buffer, history[(histIndex - 1) & HISTORY_MASK]))
			HistoryAdd(buffer);
		if (histIndex == histNext) {
			histIndex = -1;
			buffer[0] = '\0';
		} else
			strcpy(buffer, history[histIndex & HISTORY_MASK]);
	}
	histLock.Unlock();
	cursor = strlen(buffer);
	if (cursor >= width)
		scroll = cursor - width + CONSOLE_SCROLL;
}

void ConsoleField::RunCommand()
{
	if (!buffer[0])
		return;

	histLock.Lock();
	HistoryAdd(buffer);
	histIndex = -1;
	histLock.Unlock();

	Msg(CONSOLE_PROMPT "%s", buffer);

	// If it starts with a / then it's a command, if not then it's chat.
	if (buffer[0] == '/')
		ThreadPool::AddJob(tr1::bind(Cmd::ExecuteString, strwrap(buffer + 1)));
	else {
		char buf[MAX_CONSOLE_INPUT];
		strcpy(buf, "say \"");
		EscapeQuotes(buf + 5, buffer, sizeof(buf) - 5);
		ThreadPool::AddJob(tr1::bind(Cmd::ExecuteString, strwrap(buf)));
	}

	Clear();
}

static void FindMatches(const char *str)
{
	// See if the start matches the completion string
	if (strnicmp(str, autoCompleteData.completionString, strlen(autoCompleteData.completionString)))
		return;

	autoCompleteData.matchCount++;

	if (autoCompleteData.matchCount == 1) {
		strlcpy(autoCompleteData.shortestMatch, str, sizeof(autoCompleteData.shortestMatch));
		return;
	}

	// Cut shortestMatch to the amount common with str
	for (int i = 0; autoCompleteData.shortestMatch[i]; i++) {
		if (!str[i] || tolower(autoCompleteData.shortestMatch[i]) != tolower(str[i])) {
			autoCompleteData.shortestMatch[i] = '\0';
			break;
		}
	}
}

void PrintMatches(const char *str)
{
	// See if the start matches the completion string
	if (!strnicmp(str, autoCompleteData.completionString, strlen(autoCompleteData.completionString)))
		Msg("    %s", str);
}

// Special version of PrintMatches for cvars, used by Cvar::Complete.
void PrintCvarMatches(const char *str)
{
	// See if the start matches the completion string
	if (!strnicmp(str, autoCompleteData.completionString, strlen(autoCompleteData.completionString)))
		Msg("    %s = %s", str, Cvar::Find(str)->Get());
}

void ConsoleField::Autocomplete()
{
	CmdArgs args;
	int argNum;

	// Always prepend a /
	if (buffer[0] != '/') {
		if (strlen(buffer) + 1 >= sizeof(buffer) / sizeof(*buffer))
			return;
		memmove(buffer + 1, buffer, strlen(buffer) + 1);
		buffer[0] = '/';
		cursor++;
		if (cursor >= scroll + width)
			scroll = cursor - width + CONSOLE_SCROLL;
	}

	// Get the right set of args
	const char *string = buffer + 1;
	int length = cursor;
	do {
		const char *next = Cmd::SplitCommand(string, length);
		if (!next) {
			args = CmdArgs(string, length - 1, false);
			break;
		}
		length -= next - string;
		string = next;
	} while (string);

	// Determine which argument we are completing
	argNum = args.Argc() - 1;
	if (!args.Argc() || buffer[cursor - 1] == ' ') {
		autoCompleteData.completionString = "";
		argNum++;
	} else
		autoCompleteData.completionString = args.Argv(argNum);
	autoCompleteData.shortestMatch[0] = '\0';
	autoCompleteData.matchCount = 0;

	// Try to find a match
	bool isCvar = autoCompleteData.completionString[0] == '$';
	cmd_t *cmd = NULL;
	if (isCvar) {
		autoCompleteData.completionString++;
		Cvar::Complete(FindMatches);
	} else if (argNum == 0) {
		Cmd::Complete(FindMatches);
		Cvar::Complete(FindMatches);
	} else {
		cmd = Cmd::Find(args.Argv(0));
		if (!cmd || !cmd->complete)
			return;
		cmd->complete(&args, argNum, FindMatches);
	}

	// See if we got a match
	if (!autoCompleteData.matchCount)
		return;
	int offset = strlen(autoCompleteData.shortestMatch) - strlen(autoCompleteData.completionString);
	bool addSpace = autoCompleteData.matchCount == 1 && buffer[cursor] != ' ';
	if (addSpace)
		offset++;
	if (offset) {
		if (strlen(buffer) + offset + 1 > sizeof(buffer))
			return;
		memmove(buffer + cursor + offset, buffer + cursor, strlen(buffer + cursor) + 1);
		memcpy(buffer + cursor, autoCompleteData.shortestMatch + strlen(autoCompleteData.completionString), offset);
		if (addSpace)
			buffer[cursor + offset - 1] = ' ';
		cursor += offset;
	}

	// Print all matches
	if (autoCompleteData.matchCount == 1)
		return;
	Msg(CONSOLE_PROMPT "%s", buffer);
	if (isCvar) {
		Cvar::Complete(PrintMatches);
	} else if (argNum == 0) {
		Msg("Commands:");
		Cmd::Complete(PrintMatches);
		Msg("Cvars:");
		Cvar::Complete(PrintMatches);
	} else
		cmd->complete(&args, argNum, PrintMatches);
}
