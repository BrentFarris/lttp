#include "../net.h"
#include "../lttp.h"
#include <stdio.h>
#include "input.h"
#include <string.h>
#include <stdbool.h>

#define INPUT_BUFFER_SIZE	65556

int main(void)
{
	char inputBuffer[INPUT_BUFFER_SIZE];

	struct lttp* lttp = lttp_new();
	lttp_connect(lttp, LOCAL_HOST);

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
