#include "ui.h"
#include "forms.h"
#include "input.h"
#include <memory.h>
#include "display.h"

int Forms_handler(struct lttp* lttp, struct NetHandle* client, void* state, struct lttpForm* form)
{
	struct InputState* s = (struct InputState*)state;
	const int32_t count = lttpForm_get_field_count(form);
	struct TextInput** buffs = malloc(sizeof(struct TextInput*) * count);
	int32_t* types = malloc(sizeof(int32_t) * count);
	int32_t buffSize = 0;
	Display_clear();
	Display_move(0, 0);

	UI_clear_and_print(s->ui, "The server has sent a form for you to fill out.\n\0");
	for (int32_t i = 0; i < count; ++i)
	{
		struct lttpFormField f = lttpForm_get_next_field(form);
		int32_t size = max(32, f.size);
		buffs[i] = TextInput_new(size);
		UI_print_wrap(s->ui, f.label);
		if (f.type == LTTP_FORM_INPUT_TYPE_BOOL)
			UI_print_wrap(s->ui, " (yes/no)\0");
		UI_print_wrap(s->ui, "?\0");
		UI_print_command_prompt(s->ui, s->command, ">\0", " \0");
		Display_refresh();
		struct InputState r;
		r.command = buffs[i];
		r.ui = s->ui;
		while (!TextInput_read(&r))
		{

		}
		if (f.type == LTTP_FORM_INPUT_TYPE_STRING)
			buffSize += (int32_t)TextInput_get_len(buffs[i]) + 1;
		else
			buffSize += f.size;
		types[i] = f.type;
		UI_print_wrap(s->ui, " => \0");
		UI_print_wrap(s->ui, TextInput_get_buffer(buffs[i]));
		UI_print_wrap(s->ui, "\n\0");
	}

	UI_print_command_prompt(s->ui, s->command, ">\0", " \0");

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
				if (buff[0] == '1' || buff[0] == 'y')
					buffer[writeOffset++] = 1;
				else
					buffer[writeOffset++] = 0;
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
