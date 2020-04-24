#ifndef LTTP_CLIENT_UI_H
#define LTTP_CLIENT_UI_H

#include "text_input.h"

struct ClientUI;

struct ClientUI* UI_new();
void UI_free(struct ClientUI* ui);
void UI_print_wrap(struct ClientUI* ui, const char* text);
void UI_print_command_prompt(struct ClientUI* ui, struct TextInput* command, const char* prefix, const char* separator);
void UI_page_next(struct ClientUI* ui);
void UI_page_prev(struct ClientUI* ui);
void UI_clear_and_print(struct ClientUI* ui, const char* text);
void UI_input_area_adjusted(struct ClientUI* ui, const size_t inputRows);

#endif