#include "input.h"
#include <string.h>
#include <memory.h>
#include <ncurses.h>
#include "text_input.h"

struct TextInput {
	char* buffer;
	size_t maxLen;
	size_t endIndex;
	size_t writeIndex;
	int32_t lines;
};

struct TextInput* TextInput_new(const size_t len)
{
	struct TextInput* ti = malloc(sizeof(struct TextInput));
	ti->buffer = malloc(len);
	ti->maxLen = len;
	ti->endIndex = 0;
	ti->writeIndex = 0;
	ti->buffer[0] = '\0';
	ti->lines = 0;
	return ti;
}

void TextInput_free(struct TextInput* input)
{
	free(input->buffer);
	free(input);
}

bool TextInput_read(struct TextInput* input)
{
	int c = getch();
	if (c != ERR)
	{
		switch (c)
		{
			case 525:		/* ctrl + down arrow */
			case KEY_DOWN:
				// down
				break;
			case 566:		/* ctrl + up arrow */
			case KEY_UP:
				// up
				break;
			case 545:		/* ctrl + left arrow */
				if (input->writeIndex > 0)
				{
					for (int32_t i = input->writeIndex - 1; i >= 0; --i)
					{
						if (i == 0 || input->buffer[i - 1] == ' ')
						{
							volatile int x, y, rows, cols;
							getyx(stdscr, y, x);
							getmaxyx(stdscr, rows, cols);
							x -= input->writeIndex - i;
							while (x < 0)
							{
								x += cols - 1;
								y--;
							}
							move(y, x % cols);
							input->writeIndex = i;
							break;
						}
					}
				}
				break;
			case KEY_LEFT:
				if (input->writeIndex > 0)
				{
					volatile int x, y, rows, cols;
					getyx(stdscr, y, x);
					getmaxyx(stdscr, rows, cols);

					input->writeIndex--;
					if (x == 0)
						move(y - 1, cols - 1);
					else
						move(y, x - 1);
				}
				break;
			case 560:		/* ctrl + right arrow */
				if (input->writeIndex < input->endIndex)
				{
					for (int32_t i = input->writeIndex + 1; i <= input->endIndex; ++i)
					{
						if (i == input->endIndex || input->buffer[i - 1] == ' ')
						{
							volatile int x, y, rows, cols;
							getyx(stdscr, y, x);
							getmaxyx(stdscr, rows, cols);
							x += i - input->writeIndex;
							while (x > cols)
							{
								x -= cols - 1;
								y++;
							}
							move(y, x);
							input->writeIndex = i;
							break;
						}
					}
				}
				break;
			case KEY_RIGHT:
				if (input->writeIndex < input->endIndex)
				{
					volatile int x, y, rows, cols;
					getyx(stdscr, y, x);
					getmaxyx(stdscr, rows, cols);

					input->writeIndex++;
					if (x == cols - 1)
						move(y + 1, 0);
					else
						move(y, x + 1);
				}
				break;
			case 10:	/* return key */
				input->buffer[input->endIndex] = '\0';
				input->endIndex = 0;
				input->writeIndex = 0;
				trim(input->buffer);
				return true;
			case 8:		/* ctrl + delete */
			case KEY_BACKSPACE:
				if (input->writeIndex > 0)
				{
					volatile int x, y;
					getyx(stdscr, y, x);
					if (x > 0)
						mvdelch(y, x - 1);
					else
					{
						volatile int rows, cols;
						getmaxyx(stdscr, rows, cols);
						mvdelch(y - 1, cols - 2);

						move(y - (input->lines + 1), 0);
						input->lines--;
						insertln();
						move(rows - 1, cols - 2);
					}
					if (input->endIndex > 0)
					{
						if (input->writeIndex != input->endIndex)
							memcpy(input->buffer + input->writeIndex - 1, input->buffer + input->writeIndex, input->endIndex - input->writeIndex);
						input->buffer[--input->endIndex] = '\0';
						input->writeIndex--;
					}
				}
				break;
			default:
				if (input->endIndex < input->maxLen - 1)
				{
					const size_t ewDiff = input->endIndex - input->writeIndex;
					if (input->writeIndex != input->endIndex)
						memcpy(input->buffer + input->writeIndex + 1, input->buffer + input->writeIndex, ewDiff);
					input->buffer[input->writeIndex] = c;
					input->buffer[++input->endIndex] = '\0';
					printw("%s", input->buffer + input->writeIndex);
					input->writeIndex++;

					volatile int x, y, rows, cols;
					getyx(stdscr, y, x);
					getmaxyx(stdscr, rows, cols);
					if (ewDiff > 0)
						move(y - (ewDiff / cols), x - (ewDiff % cols));
					
					if (y == rows - 1 && x == cols - 1)
					{
						input->lines++;
						move(rows - (input->lines + 2), 0);
						deleteln();
						move(rows - 1, 0);
					}
				}
				break;
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
