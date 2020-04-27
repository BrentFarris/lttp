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
#else
	/*--- Set the minimum buffer size --------------------------------------*/
	HANDLE win = cmdwin();
	SMALL_RECT r;
	r.Left = 0;
	r.Right = max(120, GetSystemMetrics(SM_CXMIN));
	r.Top = 0;
	r.Bottom = max(30, GetSystemMetrics(SM_CYMIN));
	COORD xy;
	xy.X = r.Right - r.Left;
	xy.Y = r.Bottom - r.Top;
	SetConsoleScreenBufferSize(win, xy);

	/*--- Get the window border size ---------------------------------------*/
	HWND window = GetConsoleWindow();
	RECT rc, rw;
	POINT ptDiff;
	GetClientRect(window, &rc);
	GetWindowRect(window, &rw);
	int winX = (rw.right - rw.left) - rc.right;
	int winY = (rw.bottom - rw.top) - rc.bottom;

	/*--- Get the font size ------------------------------------------------*/
	CONSOLE_FONT_INFO info;
	GetCurrentConsoleFont(win, FALSE, &info);
	COORD fontSize = GetConsoleFontSize(win, info.nFont);
	int w = xy.X * fontSize.X;
	int h = xy.Y * fontSize.Y;

	MoveWindow(window, rw.left, rw.top, w + winX, h + winY, TRUE);
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
	int x, y;
	int rows, cols;
	Display_get_yx(&y, &x);
	Display_get_rows_cols(&rows, &cols);
	x = 0;
	y += 1;
	SMALL_RECT from = { .Left = x, .Top = y, .Right = cols, .Bottom = rows - y };
	COORD dest = { .X = 0, .Y = y - 1 };
	CHAR_INFO info = { .Char = L' ', .Attributes = 0 };
	ScrollConsoleScreenBuffer(cmdwin(), &from, NULL, dest, &info);
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