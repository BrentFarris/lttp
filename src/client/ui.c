#include "ui.h"
#include "input.h"
#include <string.h>
#include <ncurses.h>

/************************************************************************/
/************************************************************************/
/* Pagination                                                           */
/************************************************************************/
/************************************************************************/
struct PageBuffer {
	char* buffer;
	struct PageBuffer* next;
	struct PageBuffer* prev;
};

struct PageBook {
	struct PageBuffer* head;
	struct PageBuffer* tail;
	struct PageBuffer* currentPage;
	int32_t currentPageIndex;
	int32_t pages;
	int32_t writeIndex;
};

static void local_clear_book(struct PageBook* book)
{
	struct PageBuffer* pb = book->head;
	while (pb != NULL)
	{
		free(pb->buffer);
		struct PageBuffer* c = pb;
		pb = pb->next;
		free(c);
	}
}

static void local_print_page(struct PageBuffer* page)
{
	volatile int fromX, fromY;
	getyx(stdscr, fromY, fromX);
	move(0, 0);
	printw("%s", page->buffer);
	move(fromY, fromX);
}

static void local_add_page_to_book(struct PageBook* book, const int32_t size)
{
	book->tail->buffer[book->writeIndex] = '\0';
	book->writeIndex = 0;
	book->tail->next = malloc(sizeof(struct PageBuffer));
	book->tail->next->buffer = malloc(size);
	book->tail->next->prev = book->tail;
	book->tail = book->tail->next;
	book->pages++;
}

static void local_add_to_book(struct PageBook* book, const char* text,
	const int32_t rows, const int32_t cols)
{
	int currentRows = 0;
	const int32_t printLen = (int32_t)strlen(text);
	int32_t nextSpace = stridxof(text, " \0", 0);
	int32_t lineOffset = book->writeIndex;
	const int32_t bufferSize = rows * cols;
	for (int32_t i = 0; i < printLen; ++i)
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
			if (nextSpace > 0 && nextSpace - lineOffset > cols)
			{
				lineOffset = lastSpace + 1;
				currentRows++;
				c = '\n';
			}
		}
		if (i == bufferSize - 1 || currentRows == rows)
		{
			local_add_page_to_book(book, rows * cols);
			currentRows = 0;
			if (c != '\n')
				nextSpace = stridxof(text, " \0", i + 1);
		}
		else
			book->tail->buffer[book->writeIndex++] = c;
	}
	book->tail->buffer[book->writeIndex] = '\0';
}

static void local_reset_book(struct PageBook* book, const int32_t pageSize)
{
	book->pages = 1;
	book->head = malloc(sizeof(struct PageBuffer));
	book->head->next = NULL;
	book->head->prev = NULL;
	book->tail = book->head;
	book->currentPage = book->tail;
	book->currentPageIndex = 1;
	book->head->buffer = malloc(pageSize);
	book->head->buffer[0] = '\0';
	book->writeIndex = 0;
}

/************************************************************************/
/************************************************************************/
/* User interface API                                                   */
/************************************************************************/
/************************************************************************/
struct ClientUI {
	struct PageBook* book;
	int32_t rows;
	int32_t cols;
	int writeX;
	int writeY;
};
static void local_print_info_bar(const struct ClientUI* ui);
static void local_clear_page_space(const struct ClientUI* ui);

struct ClientUI* UI_new()
{
	struct ClientUI* ui = calloc(1, sizeof(struct ClientUI));
	ui->book = malloc(sizeof(struct PageBook));
	return ui;
}

void UI_free(struct ClientUI* ui)
{
	local_clear_book(ui->book);
	free(ui->book);
	free(ui);
}

void UI_print_wrap(struct ClientUI* ui, const char* text)
{
	volatile int fromX, fromY;
	getyx(stdscr, fromY, fromX);
	local_add_to_book(ui->book, text, ui->rows, ui->cols);
	local_print_page(ui->book->currentPage);
	local_print_info_bar(ui);
	move(fromY, fromX);
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
	if (ui->book->currentPage->next == NULL)
		return;
	int fromX, fromY;
	getyx(stdscr, fromY, fromX);
	local_clear_page_space(ui);
	ui->book->currentPage = ui->book->currentPage->next;
	ui->book->currentPageIndex++;
	local_print_page(ui->book->currentPage);
	local_print_info_bar(ui);
	move(fromY, fromX);
	refresh();
}

void UI_page_prev(struct ClientUI* ui)
{
	// TODO:  Fix copy/paste
	if (ui->book->currentPage->prev == NULL)
		return;
	int fromX, fromY;
	getyx(stdscr, fromY, fromX);
	local_clear_page_space(ui);
	ui->book->currentPage = ui->book->currentPage->prev;
	ui->book->currentPageIndex--;
	local_print_page(ui->book->currentPage);
	local_print_info_bar(ui);
	move(fromY, fromX);
	refresh();
}

void UI_clear_and_print(struct ClientUI* ui, const char* text)
{
	local_clear_book(ui->book);
	clear();
	move(0, 0);
	local_reset_book(ui->book, ui->rows * ui->cols);
	UI_print_wrap(ui, text);
}

void UI_input_area_adjusted(struct ClientUI* ui, const size_t inputRows)
{
	int rows, cols;
	getmaxyx(stdscr, rows, cols);
	ui->rows = rows - inputRows - 1;	/* -1 for information row */
	ui->cols = cols;

	// TODO:  Re-adjust pages count if needed
	local_reset_book(ui->book, ui->rows * ui->cols);
}

static void local_print_info_bar(const struct ClientUI* ui)
{
	volatile int fromX, fromY;
	getyx(stdscr, fromY, fromX);
	move(ui->rows, 0);
	clrtoeol();
	for (int32_t i = 0; i < ui->cols; ++i)
		addch('=');
	move(ui->rows, 3);
	printw(" Page %d of %d (page up/down = navigate) ", ui->book->currentPageIndex, ui->book->pages);
	move(fromY, fromX);
}

static void local_clear_page_space(const struct ClientUI* ui)
{
	for (int32_t i = 0; i < ui->rows; ++i)
	{
		move(i, 0);
		clrtoeol();
	}
}