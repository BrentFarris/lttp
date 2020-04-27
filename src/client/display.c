#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_DEPRECATE
#include <conio.h>
//https://docs.microsoft.com/en-us/windows/console/console-functions
#endif

#include <uchar.h>
#include "input.h"
#include "display.h"

void Display_init()
{
#ifdef NCURSES
	initscr();
	halfdelay(10);	/* 1 second */
	noecho();
	keypad(stdscr, TRUE);
#endif
}

void Display_quit()
{
#ifdef NCURSES
	endwin();
#endif
}

void Display_clear()
{
#ifdef NCURSES
	clear();
#else
	int rows, cols;
	Display_move(0, 0);
	Display_get_rows_cols(&rows, &cols);
	int len = rows * cols;
	char16_t* blank = malloc(sizeof(char16_t) * len);
	for (int i = 0; i < len; ++i)
		blank[i] = L' ';
	WriteConsole(cmdwin(), blank, len, NULL, NULL);
#endif
}

void Display_move(int y, int x)
{
#ifdef NCURSES
	move(y, x);
#else
	COORD xy;
	xy.X = x;
	xy.Y = y;
	SetConsoleCursorPosition(cmdwin(), xy);
#endif
}

void Display_refresh()
{
#ifdef NCURSES
	refresh();
#endif
}

int Display_get_char()
{
#ifdef NCURSES
	return getch();
#else
	int res = 0;
	while (_kbhit())
	{
		mbstate_t state;
		memset(&state, 0, sizeof state);
		wchar_t wChar = (wchar_t)_getch();
		wchar_t* pwChar = &wChar;
		unsigned char utf8Char = 0;
		wcsrtombs(&utf8Char, &pwChar, 1, &state);
		if (utf8Char == '\0')
			break;
		res <<= 1;
		res |= utf8Char;
	}
	return res > 0 ? res : ERR;
#endif
}

void Display_get_yx(int* outY, int* outX)
{
#ifdef NCURSES
	getyx(stdscr, *outY, *outX);
#else
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(cmdwin(), &info);
	*outY = info.dwCursorPosition.Y;
	*outX = info.dwCursorPosition.X;
#endif
}

void Display_get_rows_cols(int* outRows, int* outCols)
{
#ifdef NCURSES
	getmaxyx(stdscr, *outRows, *outCols);
#else
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(cmdwin(), &info);
	*outCols = info.srWindow.Right - info.srWindow.Left;
	*outRows = info.srWindow.Bottom - info.srWindow.Top + 1;
#endif
}

void Display_clear_to_line_end()
{
#ifdef NCURSES
	clrtoeol();
#else
	int x, y, rows, cols;
	Display_get_yx(&y, &x);
	Display_get_rows_cols(&rows, &cols);
	int len = (cols - x) + 1;
	if (len == 0)
		return;
	char16_t* str = malloc(sizeof(char16_t) * len);
	for (int i = 0; i < len; ++i)
		str[i] = L' ';
	WriteConsole(cmdwin(), str, len, NULL, NULL);
	free(str);
	Display_move(y, x);
#endif
}

void Display_add_char(uint16_t utf8Letter)
{
#ifdef NCURSES
	addch(utf8Letter);
#else
	wchar_t wLetter[1];
	mbstowcs(wLetter, &utf8Letter, 1);
	WriteConsole(cmdwin(), &wLetter, 1, NULL, NULL);
#endif
}

void Display_insert_line()
{
#ifdef NCURSES
	insertln();
#else
	COORD line;
	int rows, cols;
	Display_get_yx(&line.Y, &line.X);
	Display_get_rows_cols(&rows, &cols);
	line.X = 0;
	const DWORD txtLen = cols * (rows - line.Y);
	DWORD readLen = 0;
	wchar_t* scrTxt = malloc(sizeof(wchar_t) * txtLen);
	ReadConsoleOutputCharacter(cmdwin(), scrTxt, txtLen, line, &readLen);
	Display_clear_to_line_end();
	line.Y += 1;
	Display_move(line.Y, line.X);
	WriteConsole(cmdwin(), scrTxt, readLen, NULL, NULL);
	free(scrTxt);
#endif
}

void Display_delete_line()
{
#ifdef NCURSES
	deleteln();
#else
	COORD line;
	int rows, cols;
	Display_get_yx(&line.Y, &line.X);
	Display_get_rows_cols(&rows, &cols);
	line.X = 0;
	line.Y += 1;
	const DWORD txtLen = cols * (rows - line.Y);
	DWORD readLen = 0;
	wchar_t* scrTxt = malloc(sizeof(wchar_t) * txtLen);
	ReadConsoleOutputCharacter(cmdwin(), scrTxt, txtLen, line, &readLen);
	Display_move(line.Y - 1, 0);
	WriteConsole(cmdwin(), scrTxt, readLen, NULL, NULL);

	// TODO:  Will need to clear the bottom line as it will not move

	free(scrTxt);
#endif
}

void Display_delete_char()
{
#ifdef NCURSES
	delch();
#else
	int x, y;
	Display_get_yx(&y, &x);
	WriteConsole(cmdwin(), L" ", 1, NULL, NULL);

	// TODO:  Move the entire buffer over
	int rows, cols;
	Display_get_rows_cols(&rows, &cols);
	const DWORD txtLen = (cols * (rows - y)) - x;
	DWORD readLen = 0;
	wchar_t* scrTxt = malloc(sizeof(wchar_t) * txtLen);
	COORD xy;
	xy.X = x + 1;
	xy.Y = y;
	ReadConsoleOutputCharacter(cmdwin(), scrTxt, txtLen, xy, &readLen);
	Display_move(y, x);
	WriteConsole(cmdwin(), scrTxt, readLen, NULL, NULL);
	Display_move(y, x);
	free(scrTxt);
#endif
}

void Display_delete_char_at(const int y, const int x)
{
#ifdef NCURSES
	mvdelch(y, x);
#else
	Display_move(y, x);
	Display_delete_char();
#endif
}

#ifndef NCURSES
void Display_print_str(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int len = vsnprintf(NULL, 0, format, args);
	char* str = malloc(len + 1);
	vsnprintf(str, len + 1, format, args);
	va_end(args);
	char16_t* wStr;
	utf8_to_str16(str, &wStr);
	free(str);
	WriteConsole(cmdwin(), wStr, len, NULL, NULL);
	free(wStr);
}
#endif