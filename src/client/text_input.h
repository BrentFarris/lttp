#ifndef LTTP_CLIENT_TEXT_INPUT_H
#define LTTP_CLIENT_TEXT_INPUT_H

#include "input.h"
#include <stdlib.h>
#include <stdbool.h>

struct TextInput;

struct TextInput* TextInput_new(const size_t len);
void TextInput_free(struct TextInput* input);
bool TextInput_read(struct InputState* state);
const char* TextInput_get_buffer(const struct TextInput* input);
size_t TextInput_get_len(const struct TextInput* input);
void TextInput_clear(struct TextInput* input);

#endif