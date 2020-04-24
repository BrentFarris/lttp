#include "ui.h"
#include "input.h"
#include <string.h>
#include <ncurses.h>

struct PageBuffer {
	char* buffer;
	struct PageBuffer* next;
	struct PageBuffer* prev;
};

struct ClientUI {
	struct PageBuffer* head;
	struct PageBuffer* tail;
	struct PageBuffer* currentPage;
	int32_t currentPageIndex;
	int32_t pages;
	int32_t rows;
	int32_t cols;
	int writeX;
	int writeY;
};

static void local_reset_ui(struct ClientUI* ui)
{
	ui->writeX = 0;
	ui->writeY = 0;
	ui->pages = 1;
	ui->head = malloc(sizeof(struct PageBuffer));
	ui->head->next = NULL;
	ui->head->prev = NULL;
	ui->tail = ui->head;
	ui->currentPage = ui->tail;
	ui->currentPageIndex = 1;
	ui->head->buffer = malloc(ui->rows * ui->cols);
	ui->head->buffer[0] = '\0';
}

static void local_free_pages(struct PageBuffer* head)
{
	struct PageBuffer* pb = head;
	while (pb != NULL)
	{
		free(pb->buffer);
		struct PageBuffer* c = pb;
		pb = pb->next;
		free(c);
	}
}

struct ClientUI* UI_new()
{
	struct ClientUI* ui = calloc(1, sizeof(struct ClientUI));
	local_reset_ui(ui);
	return ui;
}

void UI_free(struct ClientUI* ui)
{
	local_free_pages(ui->head);
	free(ui);
}

void UI_print_wrap(struct ClientUI* ui, const char* text)
{
	volatile int fromX, fromY;
	getyx(stdscr, fromY, fromX);

	// TODO:  If the row is on top of the input area, then paginate
	move(ui->writeY, ui->writeX);
	int currentRows = 0;
	const int32_t printLen = (int32_t)strlen(text);
	int32_t nextSpace = stridxof(text, " \0", 0);
	int32_t lineOffset = ui->writeY * ui->cols + ui->writeX;
	int32_t writeOffset = strlen(ui->tail->buffer);
	const int32_t bufferSize = ui->rows * ui->cols;
	int32_t i;
	for (i = 0; i < printLen; ++i)
	{
		char c = text[i];
		if (c == '\n')
		{
			lineOffset = i + 1;
			currentRows++;
		}
		if (i == nextSpace)
		{
			int32_t lastSpace = nextSpace;
			nextSpace = stridxof(text, " \0", i + 1);
			if (nextSpace > 0 && nextSpace - lineOffset > ui->cols)
			{
				lineOffset = lastSpace + 1;
				currentRows++;
				c = '\n';
			}
		}

		if (i == bufferSize - 1 || currentRows == ui->rows)
		{
			ui->tail->buffer[writeOffset] = '\0';
			writeOffset = 0;
			ui->tail->next = malloc(sizeof(struct PageBuffer));
			ui->tail->next->buffer = malloc(bufferSize);
			ui->tail->next->prev = ui->tail;
			ui->tail = ui->tail->next;
			ui->pages++;
			currentRows = 0;
			if (c != '\n')
				nextSpace = stridxof(text, " \0", i + 1);
		}
		else
			ui->tail->buffer[writeOffset++] = c;
	}
	ui->tail->buffer[writeOffset] = '\0';

	move(0, 0);
	printw("%s", ui->currentPage->buffer);
	getyx(stdscr, ui->writeY, ui->writeX);

	move(ui->rows, 0);
	clrtoeol();
	for (int32_t i = 0; i < ui->cols; ++i)
		addch('=');
	move(ui->rows, 3);
	printw(" Page %d of %d (page up/down = navigate) ", ui->currentPageIndex, ui->pages);
	move(fromY, fromX);
	refresh();
}

void UI_print_command_prompt(struct ClientUI* ui, struct TextInput* command, const char* prefix, const char* separator)
{
	int rows, cols;
	getmaxyx(stdscr, rows, cols);
	for (int32_t i = ui->rows + 1; i < rows; ++i)
	{
		move(ui->rows + 1, 0);
		clrtoeol();
	}

	move(ui->rows + 1, 0);
	printw("%s%s%s", prefix, separator, TextInput_get_buffer(command));
	refresh();
}

void UI_page_next(struct ClientUI* ui)
{
	// TODO:  Fix copy/paste
	if (ui->currentPage->next == NULL)
		return;
	int fromX, fromY;
	getyx(stdscr, fromY, fromX);
	for (int32_t i = 0; i < ui->rows; ++i)
	{
		move(i, 0);
		clrtoeol();
	}
	move(0, 0);
	ui->currentPage = ui->currentPage->next;
	printw("%s", ui->currentPage->buffer);
	ui->currentPageIndex++;

	move(ui->rows, 0);
	clrtoeol();
	for (int32_t i = 0; i < ui->cols; ++i)
		addch('=');
	getyx(stdscr, ui->writeY, ui->writeX);
	move(ui->rows, 3);
	printw(" Page %d of %d (page up/down = navigate) ", ui->currentPageIndex, ui->pages);
	move(fromY, fromX);
	refresh();
}

void UI_page_prev(struct ClientUI* ui)
{
	// TODO:  Fix copy/paste
	if (ui->currentPage->prev == NULL)
		return;
	int fromX, fromY;
	getyx(stdscr, fromY, fromX);
	for (int32_t i = 0; i < ui->rows; ++i)
	{
		move(i, 0);
		clrtoeol();
	}
	move(0, 0);
	ui->currentPage = ui->currentPage->prev;
	printw("%s", ui->currentPage->buffer);
	ui->currentPageIndex--;

	move(ui->rows, 0);
	clrtoeol();
	for (int32_t i = 0; i < ui->cols; ++i)
		addch('=');
	getyx(stdscr, ui->writeY, ui->writeX);
	move(ui->rows, 3);
	printw(" Page %d of %d (page up/down = navigate) ", ui->currentPageIndex, ui->pages);
	move(fromY, fromX);
	refresh();
}

void UI_clear_and_print(struct ClientUI* ui, const char* text)
{
	local_free_pages(ui->head);
	clear();
	move(0, 0);
	local_reset_ui(ui);
	UI_print_wrap(ui, text);
}

void UI_input_area_adjusted(struct ClientUI* ui, const size_t inputRows)
{
	int rows, cols;
	getmaxyx(stdscr, rows, cols);
	ui->rows = rows - inputRows - 1;	/* -1 for information row */
	ui->cols = cols;

	// TODO:  Re-adjust pages count if needed
}
