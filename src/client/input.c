#include "input.h"
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
