#include "ui.h"
#include "forms.h"
#include "input.h"
#include <memory.h>
#include <ncurses.h>

int Forms_handler(struct lttp* lttp, struct NetHandle* client, void* state, struct lttpForm* form)
{
	struct TextInput* command = (struct TextInput*)state;
	const int32_t count = lttpForm_get_field_count(form);
	struct TextInput** buffs = malloc(sizeof(struct TextInput*) * count);
	int32_t* types = malloc(sizeof(int32_t) * count);
	int32_t buffSize = 0;
	clear();
	UI_print_wrap("The server has sent a form for you to fill out.\0");
	for (int32_t i = 0; i < count; ++i)
	{
		struct lttpFormField f = lttpForm_get_next_field(form);
		int32_t size = max(32, f.size);
		buffs[i] = TextInput_new(size);
		UI_print_command_prompt(command, f.label, ": \0");
		refresh();
		while (!TextInput_read(buffs[i]))
		{

		}
		if (f.type == LTTP_FORM_INPUT_TYPE_STRING)
			buffSize += (int32_t)TextInput_get_len(buffs[i]) + 1;
		else
			buffSize += f.size;
		types[i] = f.type;
		move(i + 1, 0);
		UI_print_wrap(f.label);
		UI_print_wrap(": \0");
		UI_print_wrap(TextInput_get_buffer(buffs[i]));
	}

	UI_print_command_prompt(command, ">\0", " \0");

	uint8_t* buffer = malloc(buffSize);
	int32_t writeOffset = 0;
	for (int32_t i = 0; i < count; ++i)
	{
		const char* buff = TextInput_get_buffer(buffs[i]);
		switch (types[i])
		{
			case LTTP_FORM_INPUT_TYPE_STRING:
			{
				size_t len = TextInput_get_len(buffs[i]);
				memcpy(buffer + writeOffset, buff, len);
				writeOffset += (int32_t)len;
				buffer[writeOffset++] = '\0';
				break;
			}
			case LTTP_FORM_INPUT_TYPE_CHAR:
				buffer[writeOffset++] = buff[0];
				break;
			case LTTP_FORM_INPUT_TYPE_BOOL:
				buffer[writeOffset++] = buff == 0 ? 0 : 1;
				break;
			case LTTP_FORM_INPUT_TYPE_FLOAT:
			{
				float v = strtof(buff, NULL);
				memcpy(buffer + writeOffset, &v, sizeof(float));
				writeOffset += sizeof(float);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_DOUBLE:
			{
				double v = strtod(buff, NULL);
				memcpy(buffer + writeOffset, &v, sizeof(double));
				writeOffset += sizeof(double);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_8:
			{
				int8_t v = strtoint8(buff);
				memcpy(buffer + writeOffset, &v, sizeof(int8_t));
				writeOffset += sizeof(int8_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_8:
			{
				uint8_t v = strtouint8(buff);
				memcpy(buffer + writeOffset, &v, sizeof(uint8_t));
				writeOffset += sizeof(uint8_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_16:
			{
				int16_t v = strtoint16(buff);
				memcpy(buffer + writeOffset, &v, sizeof(int16_t));
				writeOffset += sizeof(int16_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_16:
			{
				uint16_t v = strtouint16(buff);
				memcpy(buffer + writeOffset, &v, sizeof(uint16_t));
				writeOffset += sizeof(uint16_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_32:
			{
				int32_t v = strtoint32(buff);
				memcpy(buffer + writeOffset, &v, sizeof(int32_t));
				writeOffset += sizeof(int32_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_32:
			{
				uint32_t v = strtouint32(buff);
				memcpy(buffer + writeOffset, &v, sizeof(uint32_t));
				writeOffset += sizeof(uint32_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_64:
			{
				int64_t v = strtoint64(buff);
				memcpy(buffer + writeOffset, &v, sizeof(int64_t));
				writeOffset += sizeof(int64_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_64:
			{
				uint64_t v = strtouint64(buff);
				memcpy(buffer + writeOffset, &v, sizeof(uint64_t));
				writeOffset += sizeof(uint64_t);
				break;
			}
		}
		TextInput_free(buffs[i]);
	}
	free(buffs);
	lttpForm_set_payload(form, buffer, buffSize);
	lttp_send_form_response(lttp, form);
	free(buffer);
	return 0;
}
