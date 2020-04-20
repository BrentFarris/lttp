#ifndef LTTP_INPUT_H
#define LTTP_INPUT_H

#include <stdint.h>

void trim(char* str);
int8_t strtoint8(const char* str);
uint8_t strtouint8(const char* str);
int16_t strtoint16(const char* str);
uint16_t strtouint16(const char* str);
int32_t strtoint32(const char* str);
uint32_t strtouint32(const char* str);
int64_t strtoint64(const char* str);
uint64_t strtouint64(const char* str);

#endif