#include <stdlib.h>
#include <string.h>
#include "lttp_form.h"

struct lttpFormEntry {
	struct lttpFormEntry* next;
	char* label;
	void* paylodLocation;
	enum lttpFormInputType type;
	int32_t size;
};

struct lttpForm {
	struct lttpFormEntry* head;
	struct lttpFormEntry* tail;
	struct lttpFormEntry* read;
	const uint8_t* payload;
	int32_t payloadSize;
	int32_t size;
	int32_t responseMaxSize;
};

static struct lttpFormEntry* local_create_entry()
{
	struct lttpFormEntry* e = calloc(1, sizeof(struct lttpFormEntry));
	return e;
}

static void local_add_next(struct lttpForm* form, const char* label)
{
	int32_t size = sizeof(int32_t);						// entry->type (can't trust compilers sizes on enum)
	size += sizeof(form->tail->size);
	size += 1;											// NULL for the label

	// TODO:  Review this casting
	const int32_t labelLen = max(0, (int32_t)strlen(label));
	form->size += size + labelLen;						// Labels are not sent back only sent on empty
	form->responseMaxSize += size + form->tail->size;	// Add max data size

	form->tail->label = malloc(labelLen + 1);
	memcpy(form->tail->label, label, labelLen);
	form->tail->label[labelLen] = '\0';
	form->tail->next = local_create_entry();
	form->tail = form->tail->next;
}

struct lttpForm* lttpForm_new()
{
	struct lttpForm* form = calloc(1, sizeof(struct lttpForm));
	form->head = local_create_entry();
	form->tail = form->head;
	form->size = sizeof(int32_t) + sizeof(int32_t);			// Include full size and description size
	form->responseMaxSize = form->size;
	return form;
}

struct lttpForm* lttpForm_from_data(const uint8_t* formData)
{
	struct lttpForm* form = malloc(sizeof(struct lttpForm));
	memset(form, 0, sizeof(struct lttpForm));
	form->size = sizeof(int32_t) + sizeof(int32_t);			// Include full size and description size
	form->responseMaxSize = 0;
	const int32_t fullSize = *(int32_t*)formData;
	int32_t offset = sizeof(int32_t);
	const int32_t descriptionSize = *(int32_t*)(formData + offset);
	offset += sizeof(int32_t);
	uint8_t* payloadLocation = (uint8_t*)formData + descriptionSize;

	while (offset < descriptionSize)
	{
		struct lttpFormEntry* e = local_create_entry();
		e->type = *(enum lttpFormInputType*)(formData + offset);
		offset += sizeof(int32_t);
		e->size = *(int32_t*)(formData + offset);
		offset += sizeof(int32_t);

		// TODO:  Doe we need to set the form->size, or form->responseMaxSize?

		// TODO:  Look for funny business here
		size_t labelLen = strlen((char*)(formData + offset));
		e->label = malloc(labelLen + 1);
		memcpy(e->label, formData + offset, labelLen + 1);
		offset += (int32_t)labelLen + 1;

		e->paylodLocation = payloadLocation;

		if (e->type == LTTP_FORM_INPUT_TYPE_STRING)
			payloadLocation += strlen((char*)payloadLocation) + 1;
		else
			payloadLocation += e->size;

		if (form->head == NULL)
		{
			form->head = e;
			form->tail = e;
		}
		else
		{
			form->tail->next = e;
			form->tail = form->tail->next;
		}
	}

	if (offset == fullSize)
	{
		form->payload = NULL;
		form->payloadSize = 0;
	}
	else
	{
		form->payload = formData + offset;
		form->payloadSize = fullSize - offset;
	}

	form->read = form->head;
	return form;
}

void lttpForm_free(struct lttpForm* form)
{
	struct lttpFormEntry* entry = form->head;
	while (entry != NULL)
	{
		struct lttpFormEntry* next = entry->next;
		free(entry);
		entry = next;
	}
	free(form);
}

void lttpForm_add_string(struct lttpForm* form, const int32_t maxLength, const char* label)
{
	if (maxLength < 0)
		return;				// TODO:  Return error
	form->tail->type = LTTP_FORM_INPUT_TYPE_STRING;
	form->tail->size = maxLength;
	local_add_next(form, label);
}

void lttpForm_add(struct lttpForm* form, const enum lttpFormInputType type, const char* label)
{
	if (type == LTTP_FORM_INPUT_TYPE_STRING)
		lttpForm_add_string(form, DEFAULT_MAX_STRING_LENGTH, label);
	else
	{
		form->tail->type = type;
		switch (type)
		{
			case LTTP_FORM_INPUT_TYPE_CHAR:
				form->tail->size = 1;
				break;
			case LTTP_FORM_INPUT_TYPE_BOOL:
				form->tail->size = 1;
				break;
			case LTTP_FORM_INPUT_TYPE_FLOAT:
				form->tail->size = 4;
				break;
			case LTTP_FORM_INPUT_TYPE_DOUBLE:
				form->tail->size = 8;
				break;
			case LTTP_FORM_INPUT_TYPE_INT_8:
				form->tail->size = 1;
				break;
			case LTTP_FORM_INPUT_TYPE_UINT_8:
				form->tail->size = 1;
				break;
			case LTTP_FORM_INPUT_TYPE_INT_16:
				form->tail->size = 2;
				break;
			case LTTP_FORM_INPUT_TYPE_UINT_16:
				form->tail->size = 2;
				break;
			case LTTP_FORM_INPUT_TYPE_INT_32:
				form->tail->size = 4;
				break;
			case LTTP_FORM_INPUT_TYPE_UINT_32:
				form->tail->size = 4;
				break;
			case LTTP_FORM_INPUT_TYPE_INT_64:
				form->tail->size = 8;
				break;
			case LTTP_FORM_INPUT_TYPE_UINT_64:
				form->tail->size = 8;
				break;
			default:
				form->tail->type = LTTP_FORM_INPUT_TYPE_NULL;
				break;
		}

		if (form->tail->type != LTTP_FORM_INPUT_TYPE_NULL)
			local_add_next(form, label);
		else
			return;		// TODO:  Return error
	}
}

uint8_t* lttpForm_get_raw(const struct lttpForm* form)
{
	if (form->size == sizeof(int32_t))
		return NULL;

	uint8_t* raw = malloc(form->size);
	memcpy(raw, &form->size, sizeof(int32_t));				// The full size is the description size
	size_t offset = sizeof(int32_t);
	memcpy(raw + offset, &form->size, sizeof(int32_t));		// The description size is the full size
	offset += sizeof(int32_t);
	struct lttpFormEntry* entry = form->head;
	while (entry != NULL && entry->type != LTTP_FORM_INPUT_TYPE_NULL)
	{
		const int32_t t = (int32_t)entry->type;
		const size_t labelLen = strlen(entry->label) + 1;
		memcpy(raw + offset, &t, sizeof(int32_t));
		offset += sizeof(int32_t);
		memcpy(raw + offset, &entry->size, sizeof(entry->size));		// TODO:  This will need to be explicit for the protocol
		offset += sizeof(entry->size);
		memcpy(raw + offset, entry->label, labelLen);
		offset += labelLen;
		entry = entry->next;
	}

	return raw;
}

uint8_t* lttpForm_get_filled(const struct lttpForm* form, int32_t* outSize)
{
	if (form->payload == NULL || form->payloadSize == 0)
		return NULL;

	int32_t size = sizeof(int32_t) + sizeof(int32_t);	// Total size + description size
	struct lttpFormEntry* entry = form->head;
	while (entry != NULL && entry->type != LTTP_FORM_INPUT_TYPE_NULL)
	{
		size += sizeof(int32_t);				// entry->type (can't trust compilers sizes on enum)
		size += sizeof(form->tail->size);
		size += 1;								// NULL for label
		entry = entry->next;
	}

	int32_t fullSize = size + form->payloadSize;
	*outSize = fullSize;

	uint8_t* filled = malloc(fullSize);
	size_t offset = 0;
	memcpy(filled + offset, &fullSize, sizeof(int32_t));		// Write the full size of the payload
	offset += sizeof(int32_t);
	memcpy(filled + offset, &size, sizeof(int32_t));			// Write the description size of the payload
	offset += sizeof(int32_t);
	entry = form->head;
	while (entry != NULL && entry->type != LTTP_FORM_INPUT_TYPE_NULL)
	{
		const int32_t t = (int32_t)entry->type;
		memcpy(filled + offset, &t, sizeof(int32_t));
		offset += sizeof(int32_t);
		memcpy(filled + offset, &entry->size, sizeof(entry->size));		// TODO:  This will need to be explicit for the protocol
		offset += sizeof(entry->size);
		filled[offset++] = 0;											// NULL for label
		entry = entry->next;
	}

	memcpy(filled + offset, form->payload, form->payloadSize);
	return filled;
}

void lttpForm_free_raw(uint8_t* formData)
{
	free(formData);
}

void lttpForm_free_filled(uint8_t* formData)
{
	free(formData);
}

bool lttpForm_match(const struct lttpForm* reference, const struct lttpForm* actual)
{
	if (reference == NULL || actual == NULL)
		return false;
	if (actual->size > reference->responseMaxSize)
		return false;
	struct lttpFormEntry* a = reference->head;
	struct lttpFormEntry* b = actual->head;
	if (a == NULL || b == NULL)
		return false;
	while (a != NULL && b != NULL)
	{
		if (a->type != b->type)
			return false;
		if (a->size != b->size)
			return false;
		a = a->next;
		b = b->next;
	}
	return true;
}

void lttpForm_set_payload(struct lttpForm* form, const uint8_t* payload, int32_t len)
{
	form->payload = payload;
	form->payloadSize = len;
}

int32_t lttpForm_get_size(const struct lttpForm* form)
{
	return form->size;
}

int32_t lttpForm_get_field_count(const struct lttpForm* form)
{
	int32_t count = 0;
	struct lttpFormEntry* e = form->head;
	for (; e != NULL && e->type != LTTP_FORM_INPUT_TYPE_NULL; ++count)
		e = e->next;
	return count;
}

struct lttpFormField lttpForm_get_next_field(struct lttpForm* form)
{
	struct lttpFormField field;
	field.size = form->read->size;
	field.label = form->read->label;
	field.data = form->read->paylodLocation;
	field.type = form->read->type;
	form->read = form->read->next;
	return field;
}

const uint8_t* lttpForm_get_payload(struct lttpForm* form)
{
	return form->payload;
}