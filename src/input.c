#include "input.h"
#include <stdlib.h>
#include <memory.h>

void trim(char* str)
{
	if (str == NULL)
		return;
	char* start = str;
	size_t len = strlen(start);
	char* end = start + (len - 1);
	while (end > start)
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
