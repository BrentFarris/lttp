#ifndef LTTP_CLIENT_UI_H
#define LTTP_CLIENT_UI_H

#include "text_input.h"

void UI_print_wrap(const char* text);
void UI_print_command_prompt(struct TextInput* command, const char* prefix, const char* separator);

#endif