#include "../net.h"
#include <stdio.h>
#include "input.h"
#include <string.h>
#include <stdbool.h>
#include "../lttp.h"
#include "../lttp_form.h"

#define INPUT_BUFFER_SIZE	65556
#ifndef min
#define min(a, b) (a) < (b) ? (a) : (b)
#endif
#ifndef max
#define max(a, b) (a) > (b) ? (a) : (b)
#endif

static int local_text_handler(struct lttp* lttp, struct NetHandle* client, void* state, const char* msg)
{
	printf("\n%s\n", msg);
	return 0;
}

static int local_form_handler(struct lttp* lttp, struct NetHandle* client, void* state, struct lttpForm* form)
{
	const int32_t count = lttpForm_get_field_count(form);
	char** buffs = malloc(sizeof(char*) * count);
	int32_t* types = malloc(sizeof(int32_t) * count);
	int32_t buffSize = 0;
	for (int32_t i = 0; i < count; ++i)
	{
		struct lttpFormField f = lttpForm_get_next_field(form);
		int32_t size = max(32, f.size);
		buffs[i] = malloc(size);
		printf("%s", f.label);
		if (fgets(buffs[i], size, stdin) != NULL)
		{
			trim(buffs[i]);
			if (f.type == LTTP_FORM_INPUT_TYPE_STRING)
				buffSize += (int32_t)strlen(buffs[i]) + 1;
			else
				buffSize += f.size;
			types[i] = f.type;
		}
		else
			buffs[i][0] = '\0';
	}

	uint8_t* buffer = malloc(buffSize);
	int32_t writeOffset = 0;
	for (int32_t i = 0; i < count; ++i)
	{
		switch (types[i])
		{
			case LTTP_FORM_INPUT_TYPE_STRING:
			{
				size_t len = strlen(buffs[i]);
				memcpy(buffer + writeOffset, buffs[i], len);
				writeOffset += (int32_t)len;
				buffer[writeOffset++] = '\0';
				break;
			}
			case LTTP_FORM_INPUT_TYPE_CHAR:
				buffer[writeOffset++] = buffs[i][0];
				break;
			case LTTP_FORM_INPUT_TYPE_BOOL:
				buffer[writeOffset++] = buffs[i] == 0 ? 0 : 1;
				break;
			case LTTP_FORM_INPUT_TYPE_FLOAT:
			{
				float v = strtof(buffs[i], NULL);
				memcpy(buffer + writeOffset, &v, sizeof(float));
				writeOffset += sizeof(float);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_DOUBLE:
			{
				double v = strtod(buffs[i], NULL);
				memcpy(buffer + writeOffset, &v, sizeof(double));
				writeOffset += sizeof(double);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_8:
			{
				int8_t v = strtoint8(buffs[i]);
				memcpy(buffer + writeOffset, &v, sizeof(int8_t));
				writeOffset += sizeof(int8_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_8:
			{
				uint8_t v = strtouint8(buffs[i]);
				memcpy(buffer + writeOffset, &v, sizeof(uint8_t));
				writeOffset += sizeof(uint8_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_16:
			{
				int16_t v = strtoint16(buffs[i]);
				memcpy(buffer + writeOffset, &v, sizeof(int16_t));
				writeOffset += sizeof(int16_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_16:
			{
				uint16_t v = strtouint16(buffs[i]);
				memcpy(buffer + writeOffset, &v, sizeof(uint16_t));
				writeOffset += sizeof(uint16_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_32:
			{
				int32_t v = strtoint32(buffs[i]);
				memcpy(buffer + writeOffset, &v, sizeof(int32_t));
				writeOffset += sizeof(int32_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_32:
			{
				uint32_t v = strtouint32(buffs[i]);
				memcpy(buffer + writeOffset, &v, sizeof(uint32_t));
				writeOffset += sizeof(uint32_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_INT_64:
			{
				int64_t v = strtoint64(buffs[i]);
				memcpy(buffer + writeOffset, &v, sizeof(int64_t));
				writeOffset += sizeof(int64_t);
				break;
			}
			case LTTP_FORM_INPUT_TYPE_UINT_64:
			{
				uint64_t v = strtouint64(buffs[i]);
				memcpy(buffer + writeOffset, &v, sizeof(uint64_t));
				writeOffset += sizeof(uint64_t);
				break;
			}
		}
		free(buffs[i]);
	}
	free(buffs);
	lttpForm_set_payload(form, buffer, buffSize);
	lttp_send_form_response(lttp, form);
	free(buffer);

	return 0;
}

int main(int argc, char** argv)
{
	char inputBuffer[INPUT_BUFFER_SIZE];

	struct lttp* lttp = lttp_new();
	// TODO:  Assuming only argument is the host address, this will change with command line options being added
	if (argc < 2)
		lttp_connect(lttp, LOCAL_HOST);
	else
		lttp_connect(lttp, argv[1]);
	lttp_set_text_handler(lttp, NULL, local_text_handler);
	lttp_set_form_handler(lttp, NULL, local_form_handler);

	while (true)
	{
		printf("> ");
		if (fgets(inputBuffer, INPUT_BUFFER_SIZE, stdin) != NULL)
		{
			trim(inputBuffer);
			if (strcmp(inputBuffer, "quit\0") == 0 || strcmp(inputBuffer, "exit\0") == 0)
				break;
		}
		lttp_send_request(lttp, inputBuffer);
		// Send the message to the server
		do
		{
			lttp_update(lttp);
		} while (lttp_is_waiting(lttp));
	}

	lttp_shutdown(lttp);
	lttp_free(lttp);
	return 0;
}
