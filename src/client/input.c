#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include "input.h"
#include <wchar.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

void trim(char* str)
{
	if (str == NULL)
		return;
	char* start = str;
	size_t len = strlen(start);
	char* end = start + (len - 1);
	while (end >= start)
	{
		char c = *end;
		if (c == ' ' || c == '\n' || c == '\t' || c == '\r')
			end--;
		else
		{
			*(++end) = '\0';
			break;
		}
	}
	if (end < start)
		return;
	while (start < end)
	{
		char c = *start;
		if (c == ' ' || c == '\n' || c == '\t' || c == '\r')
			start++;
		else
			break;
	}
	len = end - start;
	if (len == 0)
		str[0] = '\0';
	else if (str != start)
	{
		memmove(str, start, len);
		str[len] = '\0';
	}
}

int8_t strtoint8(const char* str)
{
	int8_t value;

	if (*str == '0' && *(str + 1) != '\0')
	{
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (int8_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (int8_t)strtol((const char*)str, NULL, 2);
		else
			value = (int8_t)strtol((const char*)str, NULL, 8);
	}
	else
		value = atoi((const char*)str);

	return value;
}

uint8_t strtouint8(const char* str)
{
	uint8_t value;

	if (*str == '0' && *(str + 1) != '\0')
	{
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (uint8_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (uint8_t)strtol((const char*)str, NULL, 2);
		else
			value = (uint8_t)strtol((const char*)str, NULL, 8);
	}
	else
		value = atoi((const char*)str);

	return value;
}

int16_t strtoint16(const char* str)
{
	int16_t value;

	if (*str == '0' && *(str + 1) != '\0')
	{
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (int16_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (int16_t)strtol((const char*)str, NULL, 2);
		else
			value = (int16_t)strtol((const char*)str, NULL, 8);
	}
	else
		value = atoi((const char*)str);

	return value;
}

uint16_t strtouint16(const char* str)
{
	uint16_t value;

	if (*str == '0' && *(str + 1) != '\0')
	{
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (uint16_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (uint16_t)strtol((const char*)str, NULL, 2);
		else
			value = (uint16_t)strtol((const char*)str, NULL, 8);
	}
	else
		value = (uint16_t)atoi((const char*)str);

	return value;
}

int32_t strtoint32(const char* str)
{
	int32_t value;

	if (*str == '0' && *(str + 1) != '\0')
	{
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (int32_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (int32_t)strtol((const char*)str, NULL, 2);
		else
			value = (int32_t)strtol((const char*)str, NULL, 8);
	}
	else
		value = (int32_t)atoi((const char*)str);

	return value;
}

uint32_t strtouint32(const char* str)
{
	uint32_t value;

	if (*str == '0' && *(str + 1) != '\0')
	{
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (uint32_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (uint32_t)strtol((const char*)str, NULL, 2);
		else
			value = (uint32_t)strtol((const char*)str, NULL, 8);
	}
	else
		value = (uint32_t)atoi((const char*)str);

	return value;
}

int64_t strtoint64(const char* str)
{
	int64_t value;

	if (*str == '0' && *(str + 1) != '\0')
	{
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (int64_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (int64_t)strtol((const char*)str, NULL, 2);
		else
			value = (int64_t)strtol((const char*)str, NULL, 8);
	}
	else
		value = (int64_t)strtol((const char*)str, NULL, 10);

	return value;
}

uint64_t strtouint64(const char* str)
{
	return strtoull(str, NULL, 10);
}

int32_t stridxof(const char* haystack, const char* needle, const int32_t offset)
{
	int32_t h, n = -1, hLen = (int32_t)strlen((const char*)haystack);
	if (hLen == 0) return -1;
	int32_t nLen = (int32_t)strlen((const char*)needle);

	if (nLen > hLen || !hLen || !nLen)
		return -1;

	for (h = offset; h < hLen; ++h)
	{
		if (n == nLen)
			break;

		for (n = 0; n < nLen; ++n)
		{
			if (haystack[h + n] != needle[n])
				break;
		}
	}

	if (n == nLen)
		return h - 1;

	return -1;
}

size_t u8strlen(const char* str)
{
	size_t len = 0;
	unsigned char c = str[0];
	for (size_t i = 1; c != 0; len++)
	{
		if (c <= 127) i += 1;
		else if ((c & 0xE0) == 0xC0) i += 1;
		else if ((c & 0xF0) == 0xE0) i += 2;
		else if ((c & 0xF8) == 0xF0) i += 3;
		else	// Invalid string
		{
			len = 0;
			break;
		}
		c = str[i];
	}
	return len;
}

size_t strlen16(const char16_t* str)
{
	if (str == NULL) return 0;
	char16_t* s = (char16_t*)str;
	for (; *s; ++s);
	return s - str;
}

void str16_to_utf8(const char16_t* str, char** outStr)
{
	mbstate_t state;
	memset(&state, 0, sizeof state);
	const wchar_t* wStr = (wchar_t*)str;
	size_t len = 1 + wcsrtombs(NULL, &wStr, 0, &state);
	*outStr = malloc(len + 1);
	wcsrtombs(*outStr, &wStr, len, &state);
}

void utf8_to_str16(const char* str, char16_t** outStr)
{
	size_t len = mbstowcs(NULL, str, 0);
	wchar_t* wStr = malloc((len + 1) * sizeof(wchar_t));
	mbstowcs(wStr, str, len);
	*(wStr + len) = '\0';
	*outStr = (char16_t*)wStr;
}