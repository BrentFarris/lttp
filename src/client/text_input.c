#include "ui.h"
#include <string.h>
#include <memory.h>
#include "display.h"
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
	struct TextInput* ti = calloc(1, sizeof(struct TextInput));
	ti->buffer = malloc(len);
	ti->buffer[0] = '\0';
	ti->maxLen = len;
	return ti;
}

void TextInput_free(struct TextInput* input)
{
	free(input->buffer);
	free(input);
}

// TODO:  Swap this back to struct TextInput* input when proper key handling is done
bool TextInput_read(struct InputState* state)
{
	struct TextInput* input = state->command;
	int c = Display_get_char();
	if (c != ERR)
	{
		switch (c)
		{
			case DKEY_PAGE_UP:
				UI_page_prev(state->ui);
				break;
			case DKEY_PAGE_DOWN:
				UI_page_next(state->ui);
				break;
			case DKEY_CTRL_DOWN_ARROW:
			case DKEY_DOWN_ARROW:
				break;
			case DKEY_CTRL_UP_ARROW:
			case DKEY_UP_ARROW:
				break;
			case DKEY_CTRL_LEFT_ARROW:
				if (input->writeIndex > 0)
				{
					for (size_t i = input->writeIndex - 1; i >= 0; --i)
					{
						if (i == 0 || input->buffer[i - 1] == ' ')
						{
							int x, y, rows, cols;
							Display_get_yx(&y, &x);
							Display_get_rows_cols(&rows, &cols);
							x -= (int)(input->writeIndex - i);
							while (x < 0)
							{
								x += cols - 1;
								y--;
							}
							Display_move(y, x % cols);
							input->writeIndex = i;
							break;
						}
					}
				}
				break;
			case DKEY_LEFT_ARROW:
				if (input->writeIndex > 0)
				{
					int x, y, rows, cols;
					Display_get_yx(&y, &x);
					Display_get_rows_cols(&rows, &cols);

					input->writeIndex--;
					if (x == 0)
						Display_move(y - 1, cols - 1);
					else
						Display_move(y, x - 1);
				}
				break;
			case DKEY_CTRL_RIGHT_ARROW:
				if (input->writeIndex < input->endIndex)
				{
					for (size_t i = input->writeIndex + 1; i <= input->endIndex; ++i)
					{
						if (i == input->endIndex || input->buffer[i - 1] == ' ')
						{
							int x, y, rows, cols;
							Display_get_yx(&y, &x);
							Display_get_rows_cols(&rows, &cols);
							x += (int)(i - input->writeIndex);
							while (x > cols)
							{
								x -= cols - 1;
								y++;
							}
							Display_move(y, x);
							input->writeIndex = i;
							break;
						}
					}
				}
				break;
			case DKEY_RIGHT_ARROW:
				if (input->writeIndex < input->endIndex)
				{
					int x, y, rows, cols;
					Display_get_yx(&y, &x);
					Display_get_rows_cols(&rows, &cols);

					input->writeIndex++;
					if (x == cols - 1)
						Display_move(y + 1, 0);
					else
						Display_move(y, x + 1);
				}
				break;
			case DKEY_RETURN:
				input->buffer[input->endIndex] = '\0';
				input->endIndex = 0;
				input->writeIndex = 0;
				trim(input->buffer);
				return true;
			case DKEY_DELETE:
			case DKEY_BACKSPACE:
				if (input->writeIndex > 0)
				{
					int x, y;
					Display_get_yx(&y, &x);
					if (x > 0)
						Display_delete_char_at(y, x - 1);
					else
					{
						int rows, cols;
						Display_get_rows_cols(&rows, &cols);
						Display_delete_char_at(y - 1, cols - 2);

						Display_move(y - (input->lines + 1), 0);
						input->lines--;
						Display_insert_line();
						Display_move(rows - 1, cols - 2);
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
					Display_print_str("%s", input->buffer + input->writeIndex);
					input->writeIndex++;

					int x, y, rows, cols;
					Display_get_yx(&y, &x);
					Display_get_rows_cols(&rows, &cols);
					if (ewDiff > 0)
						Display_move(y - ((int)ewDiff / cols), x - ((int)ewDiff % cols));
					
					if (y == rows - 1 && x == cols - 1)
					{
						input->lines++;
						Display_move(rows - (input->lines + 2), 0);
						Display_delete_line();
						Display_move(rows - 1, 0);
					}
				}
				break;
		}
		Display_refresh();
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
