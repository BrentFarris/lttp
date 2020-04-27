#ifndef LTTP_INPUT_H
#define LTTP_INPUT_H

#include <wchar.h>
#include <uchar.h>
#include <stdint.h>

struct InputState {
	struct TextInput* command;
	struct ClientUI* ui;
};

void trim(char* str);
int8_t strtoint8(const char* str);
uint8_t strtouint8(const char* str);
int16_t strtoint16(const char* str);
uint16_t strtouint16(const char* str);
int32_t strtoint32(const char* str);
uint32_t strtouint32(const char* str);
int64_t strtoint64(const char* str);
uint64_t strtouint64(const char* str);
int32_t stridxof(const char* haystack, const char* needle, const int32_t offset);
size_t u8strlen(const char* str);
size_t strlen16(const char16_t* str);
void str16_to_utf8(const char16_t* str, char** outStr);
void utf8_to_str16(const char* str, char16_t** outStr);

#endif