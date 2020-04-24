#include "ui.h"
#include "../net.h"
#include <stdio.h>
#include "input.h"
#include "forms.h"
#include <string.h>
#include <stdbool.h>
#include "../lttp.h"
#include <ncurses.h>
#include "text_input.h"
#include "../lttp_form.h"

#define INPUT_BUFFER_SIZE	65556

static int local_text_handler(struct lttp* lttp, struct NetHandle* client, void* state, const char* msg)
{
	struct TextInput* command = (struct TextInput*)state;
	clear();
	move(0, 0);
	UI_print_wrap(msg);

	// TODO:  Pagination

	UI_print_command_prompt(command, ">\0", " \0");
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
	halfdelay(10);	/* 1 second */
	noecho();
	keypad(stdscr, TRUE);
	int rows, cols;
	getmaxyx(stdscr, rows, cols);

	struct TextInput* command = TextInput_new(INPUT_BUFFER_SIZE);

	lttp_set_text_handler(lttp, command, local_text_handler);
	lttp_set_form_handler(lttp, command, Forms_handler);

	UI_print_command_prompt(command, ">\0", " \0");
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
