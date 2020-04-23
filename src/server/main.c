#include <stdio.h>
#include <memory.h>
#include "../lttp.h"

#define TWIST	" twist!\0"

static void local_client_connected(struct lttp* lttp, struct NetHandle* client, void* state)
{
	printf("New client %d connected!", Network_handle_get_id(client));
}

static void local_client_disconnected(struct lttp* lttp, struct NetHandle* client, void* state)
{
	printf("Client %d has left the building!", Network_handle_get_id(client));
}

static int local_text_handler(struct lttp* lttp, struct NetHandle* client, void* state, const char* text)
{
	printf("Client sent the message %s. Let's send it back with a twist!", text);

	const size_t inLen = strlen(text);
	const size_t twistLen = strlen(TWIST);
	char* r = malloc(inLen + twistLen + 1);
	memcpy(r, text, inLen);
	memcpy(r + inLen, TWIST, twistLen);
	r[inLen + twistLen] = '\0';
	lttp_send_response(lttp, client, r);
	return 0;
}

static int local_form_handler(struct lttp* lttp, struct NetHandle* client, void* state, struct lttpForm* form)
{
	// TODO:  Show example of sending the client a form
	return 0;
}

int main(void)
{
	struct lttp* lttp = lttp_new();
	lttp_set_text_handler(lttp, NULL, local_text_handler);
	lttp_set_form_handler(lttp, NULL, local_form_handler);
	lttp_set_client_connect_handler(lttp, NULL, local_client_connected);
	lttp_set_client_disconnect_handler(lttp, NULL, local_client_disconnected);

	lttp_serve(lttp);
	while (true)
	{
		lttp_update(lttp);
	}
	lttp_shutdown(lttp);
	lttp_free(lttp);
	return 0;
}