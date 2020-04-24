#include "../net.h"
#include <stdio.h>
#include "input.h"
#include <string.h>
#include <stdbool.h>
#include "../lttp.h"
#include <ncurses.h>
#include "text_input.h"
#include "../lttp_form.h"

#define INPUT_BUFFER_SIZE	65556
#ifndef min
#define min(a, b) (a) < (b) ? (a) : (b)
#endif
#ifndef max
#define max(a, b) (a) > (b) ? (a) : (b)
#endif

static void local_print_wrap(const char* text)
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

static void local_print_command_prompt(struct TextInput* command, const char* prefix, const char* separator)
{
	int rows, cols;
	getmaxyx(stdscr, rows, cols);
	move(rows - 1, 0);
	clrtoeol();
	local_print_wrap(prefix);
	local_print_wrap(separator);
	local_print_wrap(TextInput_get_buffer(command));
	refresh();
}

static int local_text_handler(struct lttp* lttp, struct NetHandle* client, void* state, const char* msg)
{
	struct TextInput* command = (struct TextInput*)state;
	clear();
	move(0, 0);
	local_print_wrap(msg);

	// TODO:  Pagination


	local_print_command_prompt(command, ">\0", " \0");
	return 0;
}

static int local_form_handler(struct lttp* lttp, struct NetHandle* client, void* state, struct lttpForm* form)
{
	struct TextInput* command = (struct TextInput*)state;
	const int32_t count = lttpForm_get_field_count(form);
	struct TextInput** buffs = malloc(sizeof(struct TextInput*) * count);
	int32_t* types = malloc(sizeof(int32_t) * count);
	int32_t buffSize = 0;
	clear();
	local_print_wrap("The server has sent a form for you to fill out.\0");
	for (int32_t i = 0; i < count; ++i)
	{
		struct lttpFormField f = lttpForm_get_next_field(form);
		int32_t size = max(32, f.size);
		buffs[i] = TextInput_new(size);
		local_print_command_prompt(command, f.label, ": \0");
		refresh();
		while (!TextInput_read(buffs[i]))
		{

		}
		if (f.type == LTTP_FORM_INPUT_TYPE_STRING)
			buffSize += (int32_t)TextInput_get_len(buffs[i]) + 1;
		else
			buffSize += f.size;
		types[i] = f.type;
		move(i + 1, 0);
		local_print_wrap(f.label);
		local_print_wrap(": \0");
		local_print_wrap(TextInput_get_buffer(buffs[i]));
	}

	local_print_command_prompt(command, ">\0", " \0");

	uint8_t* buffer = malloc(buffSize);
	int32_t writeOffset = 0;
	for (int32_t i = 0; i < count; ++i)
	{
		const char* buff = TextInput_get_buffer(buffs[i]);
		switch (types[i])
		{
			case LTTP_FORM_INPUT_TYPE_STRING:
			{
				size_t len = TextInput_get_len(buffs[i]);
				memcpy(buffer + writeOffset, buff, len);
				writeOffset += (int32_t)len;
				buffer[writeOffset++] = '\0';
				break;
			}
			case LTTP_FORM_INPUT_TYPE_CHAR:
				buffer[writeOffset++] = buff[0];
				break;
			case LTTP_FORM_INPUT_TYPE_BOOL:
				buffer[writeOffset++] = buff == 0 ? 0 : 1;
				break;
			case LTTP_FORM_INPUT_TYPE_FLOAT:
			{
				float v = strtof(buff, NULL);
				memcpy(buffer + writeOffset, &v, sizeof(float));
				writeOffset += sizeof(float);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_DOUBLE:
			{
				double v = strtod(buff, NULL);
				memcpy(buffer + writeOffset, &v, sizeof(double));
				writeOffset += sizeof(double);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_8:
			{
				int8_t v = strtoint8(buff);
				memcpy(buffer + writeOffset, &v, sizeof(int8_t));
				writeOffset += sizeof(int8_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_8:
			{
				uint8_t v = strtouint8(buff);
				memcpy(buffer + writeOffset, &v, sizeof(uint8_t));
				writeOffset += sizeof(uint8_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_16:
			{
				int16_t v = strtoint16(buff);
				memcpy(buffer + writeOffset, &v, sizeof(int16_t));
				writeOffset += sizeof(int16_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_16:
			{
				uint16_t v = strtouint16(buff);
				memcpy(buffer + writeOffset, &v, sizeof(uint16_t));
				writeOffset += sizeof(uint16_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_32:
			{
				int32_t v = strtoint32(buff);
				memcpy(buffer + writeOffset, &v, sizeof(int32_t));
				writeOffset += sizeof(int32_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_32:
			{
				uint32_t v = strtouint32(buff);
				memcpy(buffer + writeOffset, &v, sizeof(uint32_t));
				writeOffset += sizeof(uint32_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_64:
			{
				int64_t v = strtoint64(buff);
				memcpy(buffer + writeOffset, &v, sizeof(int64_t));
				writeOffset += sizeof(int64_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_64:
			{
				uint64_t v = strtouint64(buff);
				memcpy(buffer + writeOffset, &v, sizeof(uint64_t));
				writeOffset += sizeof(uint64_t);
				break;
			}
		}
		TextInput_free(buffs[i]);
	}
	free(buffs);
	lttpForm_set_payload(form, buffer, buffSize);
	lttp_send_form_response(lttp, form);
	free(buffer);
	return 0;
}

int main(int argc, char** argv)
{
	struct lttp* lttp = lttp_new();
	// TODO:  Assuming only argument is the host address, this will change with command line options being added
	int err = 0;
	if (argc < 2)
		err = lttp_connect(lttp, LOCAL_HOST);
	else
		err = lttp_connect(lttp, argv[1]);

	if (err)
		exit(1);

	initscr();
	//cbreak();
	halfdelay(10);	/* 1 second */
	noecho();
	keypad(stdscr, TRUE);
	int rows, cols;
	getmaxyx(stdscr, rows, cols);

	struct TextInput* command = TextInput_new(INPUT_BUFFER_SIZE);

	lttp_set_text_handler(lttp, command, local_text_handler);
	lttp_set_form_handler(lttp, command, local_form_handler);

	local_print_command_prompt(command, ">\0", " \0");
	while (true)
	{
		if (TextInput_read(command) && TextInput_get_len(command) > 0)
		{
			lttp_send_request(lttp, TextInput_get_buffer(command));
			TextInput_clear(command);
		}
		do
		{
			lttp_update(lttp);
		} while (lttp_is_waiting(lttp));
		refresh();
	}

	lttp_shutdown(lttp);
	lttp_free(lttp);
	endwin();
	return 0;
}
