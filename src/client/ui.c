#include "ui.h"
#include "input.h"
#include <string.h>
#include <assert.h>
#include "display.h"

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
	book->head = NULL;
	book->tail = NULL;
	book->currentPage = NULL;
}

static void local_print_page(struct PageBuffer* page)
{
	int fromX, fromY;
	Display_get_yx(&fromY, &fromX);
	Display_move(0, 0);
	Display_print_str("%s", page->buffer);
	Display_move(fromY, fromX);
}

static void local_add_page_to_book(struct PageBook* book, const int32_t size)
{
	book->tail->buffer[book->writeIndex] = '\0';
	book->writeIndex = 0;
	book->tail->next = calloc(1, sizeof(struct PageBuffer));
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
	for (int32_t i = 0, pi = 0; i < printLen; ++i, ++pi)
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
		if (pi == bufferSize - 1 || currentRows == rows)
		{
			local_add_page_to_book(book, rows * cols);
			currentRows = 0;
			if (c != '\n')
				nextSpace = stridxof(text, " \0", i + 1);
			pi = 0;
		}
		else
			book->tail->buffer[book->writeIndex++] = c;
	}
	book->tail->buffer[book->writeIndex] = '\0';
}

static void local_reset_book(struct PageBook* book, const int32_t pageSize)
{
	local_clear_book(book);
	book->pages = 1;
	book->currentPageIndex = 1;
	book->writeIndex = 0;
	book->head = calloc(1, sizeof(struct PageBuffer));
	book->head->buffer = malloc(pageSize);
	book->head->buffer[0] = '\0';
	book->tail = book->head;
	book->currentPage = book->tail;
}

/************************************************************************/
/************************************************************************/
/* Book tests                                                           */
/************************************************************************/
/************************************************************************/
static void local_test_book()
{
	const int32_t rows = 50;
	const int32_t cols = 100;
	const int32_t size = rows * cols;
	struct PageBook* book = calloc(1, sizeof(struct PageBook));
	local_reset_book(book, size);
	assert(book->head != NULL);
	assert(book->tail == book->head);
	assert(book->currentPage == book->head);
	assert(book->currentPageIndex == 1);
	assert(book->writeIndex == 0);
	assert(book->head->next == NULL);
	char msg[] = "This is a test";
	local_add_to_book(book, msg, rows, cols);
	assert(book->writeIndex == sizeof(msg) - 1);
	assert(strcmp(book->currentPage->buffer, msg) == 0);
	assert(book->currentPage == book->currentPage);
	assert(book->tail == book->currentPage);
	assert(book->currentPage->next == NULL);
	assert(book->currentPage->prev == NULL);
	local_add_to_book(book, msg, rows, cols);
	assert(book->writeIndex == (sizeof(msg) - 1) * 2);
	assert(strcmp(book->currentPage->buffer, "This is a testThis is a test\0") == 0);
	local_reset_book(book, size);
	assert(book->writeIndex == 0);
	assert(book->currentPage == book->head);
	assert(book->currentPage == book->tail);
	assert(book->currentPage->next == NULL);
	assert(book->currentPage->prev == NULL);
	local_clear_book(book);
	free(book);
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
	local_test_book();
	struct ClientUI* ui = calloc(1, sizeof(struct ClientUI));
	ui->book = calloc(1, sizeof(struct PageBook));
	local_clear_book(ui->book);
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
	int fromX, fromY;
	Display_get_yx(&fromY, &fromX);
	local_add_to_book(ui->book, text, ui->rows, ui->cols);
	local_print_page(ui->book->currentPage);
	local_print_info_bar(ui);
	Display_move(fromY, fromX);
}

void UI_print_command_prompt(struct ClientUI* ui, struct TextInput* command, const char* prefix, const char* separator)
{
	int rows, cols;
	Display_get_rows_cols(&rows, &cols);
	for (int32_t i = ui->rows + 1; i < rows; ++i)
	{
		Display_move(ui->rows + 1, 0);
		Display_clear_to_line_end();
	}
	Display_move(ui->rows + 1, 0);
	Display_print_str("%s%s%s", prefix, separator, TextInput_get_buffer(command));
	Display_refresh();
}

void UI_page_next(struct ClientUI* ui)
{
	// TODO:  Fix copy/paste
	if (ui->book->currentPage->next == NULL)
		return;
	int fromX, fromY;
	Display_get_yx(&fromY, &fromX);
	local_clear_page_space(ui);
	ui->book->currentPage = ui->book->currentPage->next;
	ui->book->currentPageIndex++;
	local_print_page(ui->book->currentPage);
	local_print_info_bar(ui);
	Display_move(fromY, fromX);
	Display_refresh();
}

void UI_page_prev(struct ClientUI* ui)
{
	// TODO:  Fix copy/paste
	if (ui->book->currentPage->prev == NULL)
		return;
	int fromX, fromY;
	Display_get_yx(&fromY, &fromX);
	local_clear_page_space(ui);
	ui->book->currentPage = ui->book->currentPage->prev;
	ui->book->currentPageIndex--;
	local_print_page(ui->book->currentPage);
	local_print_info_bar(ui);
	Display_move(fromY, fromX);
	Display_refresh();
}

void UI_clear_and_print(struct ClientUI* ui, const char* text)
{
	Display_clear();
	Display_move(0, 0);
	local_reset_book(ui->book, ui->rows * ui->cols);
	UI_print_wrap(ui, text);
}

void UI_input_area_adjusted(struct ClientUI* ui, const size_t inputRows)
{
	int rows, cols;
	Display_get_rows_cols(&rows, &cols);
	ui->rows = (int32_t)(rows - inputRows - 1);	/* -1 for information row */
	ui->cols = cols;

	// TODO:  Re-adjust pages count if needed
	local_reset_book(ui->book, ui->rows * ui->cols);
}

static void local_print_info_bar(const struct ClientUI* ui)
{
	int fromX, fromY;
	Display_get_yx(&fromY, &fromX);
	Display_move(ui->rows, 0);
	Display_clear_to_line_end();
	for (int32_t i = 0; i < ui->cols; ++i)
		Display_add_char('=');
	Display_move(ui->rows, 3);
	Display_print_str(" Page %d of %d (page up/down = navigate) ", ui->book->currentPageIndex, ui->book->pages);
	Display_move(fromY, fromX);
}

static void local_clear_page_space(const struct ClientUI* ui)
{
	for (int32_t i = 0; i < ui->rows; ++i)
	{
		Display_move(i, 0);
		Display_clear_to_line_end();
	}
}