#include "input.h"
#include <string.h>
#include <ncurses.h>
#include "text_input.h"

struct TextInput {
	char* buffer;
	size_t maxLen;
	size_t index;
};

struct TextInput* TextInput_new(const size_t len)
{
	struct TextInput* ti = malloc(sizeof(struct TextInput));
	ti->buffer = malloc(len);
	ti->maxLen = len;
	ti->index = 0;
	ti->buffer[0] = '\0';
	return ti;
}

void TextInput_free(struct TextInput* input)
{
	free(input->buffer);
	free(input);
}

bool TextInput_read(struct TextInput* input)
{
	char c = getch();
	if (c != ERR)
	{
		// A key was pressed

		// Send the message to the server
		if (c == '\n')
		{
			input->buffer[input->index] = '\0';
			input->index = 0;
			trim(input->buffer);
			return true;
		}
		else if (c == '\a')
		{
			if (input->index > 0)
			{
				int x, y;
				getyx(stdscr, y, x);
				mvdelch(y, x - 1);
				if (input->index > 0)
					input->buffer[--input->index] = '\0';
			}
		}
		else if (input->index < input->maxLen - 1)
		{
			input->buffer[input->index++] = c;
			input->buffer[input->index] = '\0';
			printw("%c", c);
		}
		refresh();
	}
	return false;
}

const char* TextInput_get_buffer(const struct TextInput* input)
{
	return input->buffer;
}

size_t TextInput_get_len(const struct TextInput* input)
{
	return strlen(input->buffer);
}

void TextInput_clear(struct TextInput* input)
{
	input->buffer[0] = '\0';
}
