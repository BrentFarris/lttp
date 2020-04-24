#include "ui.h"
#include "input.h"
#include <string.h>
#include <ncurses.h>

void UI_print_wrap(const char* text)
{
	int rows, cols;
	getmaxyx(stdscr, rows, cols);

	// TODO:  If the row is on top of the input area, then paginate

	int x, y;
	int row = 0;
	const int32_t printLen = (int32_t)strlen(text);
	int32_t nextSpace = stridxof(text, " \0", 0);
	int32_t lineOffset = 0;
	for (int32_t i = 0; i < printLen; ++i)
	{
		if (text[i] == '\n')
			lineOffset = i + 1;
		if (i == nextSpace)
		{
			int32_t lastSpace = nextSpace;
			nextSpace = stridxof(text, " \0", i + 1);
			if (nextSpace > 0 && nextSpace - lineOffset > cols)
			{
				getyx(stdscr, y, x);
				move(y + 1, 0);
				lineOffset = lastSpace + 1;
				continue;
			}
		}
		addch(text[i]);
	}
}

void UI_print_command_prompt(struct TextInput* command, const char* prefix, const char* separator)
{
	int rows, cols;
	getmaxyx(stdscr, rows, cols);
	move(rows - 2, 0);
	for (int i = 0; i < cols; ++i)
		addch('_');
	move(rows - 1, 0);
	clrtoeol();
	UI_print_wrap(prefix);
	UI_print_wrap(separator);
	UI_print_wrap(TextInput_get_buffer(command));
	refresh();
}
