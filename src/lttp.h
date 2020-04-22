#ifndef LTTP_H
#define LTTP_H

#include "net.h"
#include <stdint.h>
#include <stdbool.h>
#include "lttp_form.h"

#define LTTP_DEFAULT_PORT			1779

enum lttpErr {
	LTTP_ERR_OUT_OF_MEMORY = 1,
	LTTP_ERR_PORT_CHANGE_WHILE_ACTIVE = 2,
	LTTP_ERR_FAILED_TO_INIT_NETWORK = 3,
	LTTP_ERR_FAILED_TO_CREATE_SERVER = 4,
	LTTP_ERR_FAILED_TO_CREATE_CLIENT = 5,
	LTTP_ERR_FAILED_TO_SEND = 6,
	LTTP_ERR_NO_FORM_DATA_TO_SEND = 7,
};

enum lttpMessageCode {
	LTTP_MESSAGE_CODE_INVALID = 0,
	LTTP_MESSAGE_CODE_TEXT = 1,
	LTTP_MESSAGE_CODE_FORM = 2,
	LTTP_MESSAGE_CODE_FILE = 3,
};

struct lttp;

struct lttp* lttp_new();
void lttp_free(struct lttp* lttp);
int lttp_shutdown(struct lttp* lttp);
void lttp_set_port(struct lttp* lttp, const uint16_t port);
void lttp_set_text_handler(struct lttp* lttp, void* state, int(*handler)(struct lttp* lttp, struct NetHandle* client, void* state, const char* request));
void lttp_set_form_handler(struct lttp* lttp, void* state, int(*handler)(struct lttp* lttp, struct NetHandle* client, void* state, struct lttpForm* form));
void lttp_set_client_connect_handler(struct lttp* lttp, void* state, void(*handler)(struct lttp* lttp, struct NetHandle* client, void* state));
void lttp_set_client_disconnect_handler(struct lttp* lttp, void* state, void(*handler)(struct lttp* lttp, struct NetHandle* client, void* state));
int lttp_update(struct lttp* lttp);
int lttp_serve(struct lttp* lttp);
int lttp_connect(struct lttp* lttp, const char* address);
int lttp_get_last_err(const struct lttp* lttp);
bool lttp_is_waiting(const struct lttp* lttp);
int lttp_send_request(struct lttp* lttp, const char* request);
int lttp_send_response(struct lttp* lttp, struct NetHandle* client, const char* response);
int lttp_send_form_request(struct lttp* lttp, struct NetHandle* client, const struct lttpForm* form);
int lttp_send_form_response(struct lttp* lttp, const struct lttpForm* form);

#endif