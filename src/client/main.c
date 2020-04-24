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
	struct InputState* s = (struct InputState*)state;
	UI_clear_and_print(s->ui, msg);
	UI_print_command_prompt(s->ui, s->command, ">\0", " \0");
	return 0;
}

int main(int argc, char** argv)
{
	initscr();
	halfdelay(10);	/* 1 second */
	noecho();
	keypad(stdscr, TRUE);
	move(0, 0);
	printw("Loading...\0");

	struct lttp* lttp = lttp_new();
	// TODO:  Assuming only argument is the host address, this will change with command line options being added
	int err = 0;
	if (argc < 2)
		err = lttp_connect(lttp, LOCAL_HOST);
	else
		err = lttp_connect(lttp, argv[1]);

	if (err)
	{
		endwin();
		exit(1);
	}

	struct InputState state;
	state.command = TextInput_new(INPUT_BUFFER_SIZE);
	state.ui = UI_new();
	UI_input_area_adjusted(state.ui, 1);

	lttp_set_text_handler(lttp, &state, local_text_handler);
	lttp_set_form_handler(lttp, &state, Forms_handler);

	UI_print_command_prompt(state.ui, state.command, ">\0", " \0");
	while (true)
	{
		if (TextInput_read(&state) && TextInput_get_len(state.command) > 0)
		{
			UI_clear_and_print(state.ui, "Communicating with server...\n\0");
			UI_print_wrap(state.ui, "Sending query \0");
			UI_print_wrap(state.ui, TextInput_get_buffer(state.command));
			UI_print_wrap(state.ui, "...\0");
			lttp_send_request(lttp, TextInput_get_buffer(state.command));
			TextInput_clear(state.command);
			UI_print_command_prompt(state.ui, state.command, ">\0", " \0");
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
