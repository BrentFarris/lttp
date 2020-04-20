#ifndef LTTP_NETWORK_H
#define LTTP_NETWORK_H

#define LOCAL_HOST					"127.0.0.1\0"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct NetHandle;
struct Server;

int Network_init();
void Network_server_set_connect_handler(struct Server* server, void* state,
	void(*handler)(void* state, struct NetHandle* client));
void Network_server_set_disconnect_handler(struct Server* server, void* state,
	void(*handler)(void* state, struct NetHandle* client));
struct Server* Network_new_server(const char* bindingAddress, const uint16_t bindingPort, const int32_t maxClientCount);
struct NetHandle* Network_get_server_handle(struct Server* server);
struct NetHandle* Network_new_client(const char* hostAddress, const uint16_t port);
int32_t Network_check(struct NetHandle* handle);
bool Network_send(const struct NetHandle* handle, const uint8_t* message, const int length, const uint16_t messageCode);
bool Network_send_many(const struct NetHandle** handles, const int32_t socketCount, uint8_t* message, int length, const uint16_t messageCode);
int Network_close_server(struct Server* server);
int Network_close_net_handle(struct NetHandle* handle);
int32_t Network_message_len(const struct NetHandle* handle);
uint16_t Network_get_message_code(const struct NetHandle* handle);
const uint8_t* Network_get_message(const struct NetHandle* handle);
size_t Network_server_client_count(const struct Server* server);
struct NetHandle* Network_get_client(struct Server* server, size_t clientIndex);
int Network_quit();

#endif