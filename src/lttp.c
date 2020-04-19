#include "net.h"
#include "lttp.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

struct lttp {
	struct Server* server;
	struct NetHandle* client;
	void* requestHandlerState;
	int(*requestHandler)(struct lttp* lttp, struct NetHandle* client, void* state, const char* request);
	void* connectHandlerState;
	void(*connectHandler)(struct lttp* lttp, struct NetHandle* client, void* state);
	void* disconnectHandlerState;
	void(*disconnectHandler)(struct lttp* lttp, struct NetHandle* client, void* state);
	uint16_t port;
	int err;
	bool waitingForResponse;
};

//static void* local_malloc(struct lttp* lttp, const size_t len)
//{
//	void* mem = malloc(len);
//	if (mem == NULL)
//		lttp->err = LTTP_ERR_OUT_OF_MEMORY;
//	return mem;
//}

static bool local_is_server(const struct lttp* lttp)
{
	return lttp->server != NULL;
}

static bool local_is_client(const struct lttp* lttp)
{
	return lttp->client != NULL;
}

static bool local_is_active(const struct lttp* lttp)
{
	return lttp->server != NULL || lttp->client != NULL;
}

static void local_client_connected(void* state, struct NetHandle* client)
{
	struct lttp* lttp = (struct lttp*)state;
	if (lttp->connectHandler != NULL)
		lttp->connectHandler(lttp, client, lttp->connectHandlerState);
}

static void local_client_disconnected(void* state, struct NetHandle* client)
{
	struct lttp* lttp = (struct lttp*)state;
	if (lttp->disconnectHandler != NULL)
		lttp->disconnectHandler(lttp, client, lttp->disconnectHandlerState);
}

struct lttp* lttp_new()
{
	struct lttp* lttp = malloc(sizeof(struct lttp));
	if (lttp == NULL)
		return NULL;
	memset(lttp, 0, sizeof(struct lttp));
	lttp->port = LTTP_DEFAULT_PORT;
	return lttp;
}

void lttp_free(struct lttp* lttp)
{
	free(lttp);
}

int lttp_shutdown(struct lttp* lttp)
{
	if (local_is_server(lttp))
		Network_close_server(lttp->server);
	else if (local_is_client(lttp))
		Network_close_net_handle(lttp->client);
	return Network_quit();
}

void lttp_set_port(struct lttp* lttp, const uint16_t port)
{
	if (!local_is_active(lttp))
		lttp->port = port;
	else
		lttp->err = LTTP_ERR_PORT_CHANGE_WHILE_ACTIVE;
}

void lttp_set_request_handler(struct lttp* lttp, void* state, int(*handler)(struct lttp* lttp, struct NetHandle* client, void* state, const char* request))
{
	lttp->requestHandlerState = state;
	lttp->requestHandler = handler;
}

void lttp_set_client_connect_handler(struct lttp* lttp, void* state, void(*handler)(struct lttp* lttp, struct NetHandle* client, void* state))
{
	lttp->connectHandlerState = state;
	lttp->connectHandler = handler;
}

void lttp_set_client_disconnect_handler(struct lttp* lttp, void* state, void(*handler)(struct lttp* lttp, struct NetHandle* client, void* state))
{
	lttp->disconnectHandlerState = state;
	lttp->disconnectHandler = handler;
}

int lttp_update(struct lttp* lttp)
{
	if (local_is_server(lttp))
	{
		int32_t msgCount = Network_check(Network_get_server_handle(lttp->server));
		if (msgCount > 0)
		{
			size_t clientCount = Network_server_client_count(lttp->server);
			for (size_t i = 0; i < clientCount; ++i)
			{
				struct NetHandle* client = Network_get_client(lttp->server, i);
				// TODO:  Use request struct to get big requests (don't assume message is 1 shot)
				const char* msg = (char*)Network_get_message(client);

				if (lttp->requestHandler != NULL)
					lttp->requestHandler(lttp, client, lttp->requestHandlerState, (const char*)msg);
			}
		}
	}
	else if (local_is_client(lttp))
	{
		int32_t msgCount = Network_check(lttp->client);
		if (msgCount > 0)
		{
			lttp->waitingForResponse = false;
			const uint8_t* msg = Network_get_message(lttp->client);
			printf("%s\n", (char*)msg);
		}
	}
	return 0;
}

int lttp_serve(struct lttp* lttp)
{
	if (Network_init() != 0)
	{
		lttp->err = LTTP_ERR_FAILED_TO_INIT_NETWORK;
		return lttp->err;
	}

	lttp->server = Network_new_server(LOCAL_HOST, lttp->port, 512);
	if (!local_is_server(lttp))
	{
		lttp->err = LTTP_ERR_FAILED_TO_CREATE_SERVER;
		return lttp->err;
	}

	Network_server_set_connect_handler(lttp->server, lttp, local_client_connected);
	Network_server_set_disconnect_handler(lttp->server, lttp, local_client_disconnected);

	return 0;
}

int lttp_connect(struct lttp* lttp, const char* address)
{
	if (Network_init() != 0)
	{
		lttp->err = LTTP_ERR_FAILED_TO_INIT_NETWORK;
		return lttp->err;
	}

	lttp->client = Network_new_client(address, lttp->port);
	if (local_is_client(lttp))
	{
		lttp->err = LTTP_ERR_FAILED_TO_CREATE_CLIENT;
		return lttp->err;
	}

	return 0;
}

int lttp_get_last_err(const struct lttp* lttp)
{
	return lttp->err;
}

bool lttp_is_waiting(const struct lttp* lttp)
{
	return lttp->waitingForResponse;
}

int lttp_send_request(struct lttp* lttp, const char* request)
{
	if (!local_is_client(lttp)) return 0;
	int sentBytes = Network_send(lttp->client, (uint8_t*)request, (int)(strlen(request) + 1));
	if (sentBytes > 0)
		lttp->waitingForResponse = true;
	return 0;
}

int lttp_send_response(struct lttp* lttp, struct NetHandle* client, const char* response)
{
	if (!local_is_server(lttp)) return 0;
	int sentBytes = Network_send(client, (uint8_t*)response, (int)(strlen(response) + 1));
	if (sentBytes > 0)
		return 0;
	return LTTP_ERR_FAILED_TO_SEND_RESPONSE;
}
