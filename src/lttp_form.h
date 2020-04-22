#ifndef LTTP_FORM_H
#define LTTP_FORM_H

#include <stdint.h>
#include <stdbool.h>

#define DEFAULT_MAX_STRING_LENGTH	256

enum lttpFormInputType {
	LTTP_FORM_INPUT_TYPE_NULL = 0,
	LTTP_FORM_INPUT_TYPE_STRING = 1,
	LTTP_FORM_INPUT_TYPE_CHAR = 2,
	LTTP_FORM_INPUT_TYPE_BOOL = 3,
	LTTP_FORM_INPUT_TYPE_FLOAT = 4,
	LTTP_FORM_INPUT_TYPE_DOUBLE = 5,
	LTTP_FORM_INPUT_TYPE_INT_8 = 6,
	LTTP_FORM_INPUT_TYPE_UINT_8 = 7,
	LTTP_FORM_INPUT_TYPE_INT_16 = 8,
	LTTP_FORM_INPUT_TYPE_UINT_16 = 9,
	LTTP_FORM_INPUT_TYPE_INT_32 = 10,
	LTTP_FORM_INPUT_TYPE_UINT_32 = 11,
	LTTP_FORM_INPUT_TYPE_INT_64 = 12,
	LTTP_FORM_INPUT_TYPE_UINT_64 = 13
};

struct lttpForm;

struct lttpFormField {
	const char* label;
	const void* data;
	enum lttpFormInputType type;
	int32_t size;
};

struct lttpForm* lttpForm_new();
struct lttpForm* lttpForm_from_data(const uint8_t* formData);
void lttpForm_free(struct lttpForm* form);
void lttpForm_add_string(struct lttpForm* form, const int32_t maxLength, const char* label);
void lttpForm_add(struct lttpForm* form, const enum lttpFormInputType type, const char* label);
uint8_t* lttpForm_get_raw(const struct lttpForm* form);
uint8_t* lttpForm_get_filled(const struct lttpForm* form, int32_t* outSize);
void lttpForm_free_raw(uint8_t* formData);
void lttpForm_free_filled(uint8_t* formData);
bool lttpForm_match(const struct lttpForm* reference, const struct lttpForm* actual);
void lttpForm_set_payload(struct lttpForm* form, const uint8_t* payload, int32_t len);
int32_t lttpForm_get_size(const struct lttpForm* form);
int32_t lttpForm_get_field_count(const struct lttpForm* form);
struct lttpFormField lttpForm_get_next_field(struct lttpForm* form);
const uint8_t* lttpForm_get_payload(struct lttpForm* form);

#endif