#include "ui.h"
#include <stdio.h>
#include "input.h"
#include "forms.h"
#include <signal.h>
#include <string.h>
#include "../net.h"
#include <stdbool.h>
#include "display.h"
#include "../lttp.h"
#include "text_input.h"
#include "../lttp_form.h"

#define INPUT_BUFFER_SIZE	65556
static volatile bool s_active = 1;

static int local_interrupt_handler(int sig)
{
	s_active = false;
}

static int local_text_handler(struct lttp* lttp, struct NetHandle* client, void* state, const char* msg)
{
	struct InputState* s = (struct InputState*)state;
	UI_clear_and_print(s->ui, msg);
	UI_print_command_prompt(s->ui, s->command, ">\0", " \0");
	return 0;
}

int main(int argc, char** argv)
{
	int err = 0;
	signal(SIGINT, local_interrupt_handler);
	s_active = true;

	Display_init();
	Display_move(0, 0);

	struct InputState state;
	state.command = TextInput_new(INPUT_BUFFER_SIZE);
	state.ui = UI_new();
	UI_input_area_adjusted(state.ui, 1);
	struct lttp* lttp = lttp_new();
	lttp_set_text_handler(lttp, &state, local_text_handler);
	lttp_set_form_handler(lttp, &state, Forms_handler);

	UI_clear_and_print(state.ui, "Welcome to the official LTTP client. Start by typing in the server name/address you wish to connect to.");
	UI_print_command_prompt(state.ui, state.command, ">\0", " \0");
	while (s_active)
	{
		if (TextInput_read(&state) && TextInput_get_len(state.command) > 0)
		{
			err = lttp_connect(lttp, TextInput_get_buffer(state.command));
			TextInput_clear(state.command);
			if (err)
			{
				UI_clear_and_print(state.ui, "Failed to connect to specified host, please try again.");
				UI_print_command_prompt(state.ui, state.command, ">\0", " \0");
			}
			else
			{
				UI_clear_and_print(state.ui, "Loading...");
				UI_print_command_prompt(state.ui, state.command, ">\0", " \0");
				break;
			}
		}
	}

	if (err)
		s_active = false;

	while (s_active)
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
		Display_refresh();
	}

	Display_clear();
	Display_print_str("%s", "Cancel has been invoked! Closing connection with server...");
	UI_free(state.ui);
	TextInput_free(state.command);
	lttp_shutdown(lttp);
	lttp_free(lttp);
	Display_quit();
	return err;
}
