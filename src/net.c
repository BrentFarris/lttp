#include "net.h"
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <string.h>
#include <inttypes.h>

#define NET_READ_BUFFER_SIZE		4096

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#define WINDOWS_NETWORKING
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")	// Need to link with Ws2_32.lib
// #pragma comment (lib, "Mswsock.lib")

#define NO_SOCKET_ERROR	0
#define validsocket(s)	((s) != INVALID_SOCKET)
typedef SOCKET CGSocket;
#else
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#define NO_SOCKET_ERROR	0
#define INVALID_SOCKET	-1
#define validsocket(s)	((s) >= 0)
typedef int32_t CGSocket;
#endif

struct NetHandle {
	uint8_t* messageBuffer;
	uint8_t* readBuffer;
	struct addrinfo* addr;
	CGSocket socket;
	int32_t messageLen;
	int32_t currentMessageLen;
	int32_t readLen;
	fd_set writeSet;
	fd_set readSet;
	fd_set exceptionSet;
	bool isServer;
};

struct Server {
	struct NetHandle handle;
	struct NetHandle* clients;
	void* connectHandlerState;
	void(*connectHandler)(void* state, struct NetHandle* client);
	void* disconnectHandlerState;
	void(*disconnectHandler)(void* state, struct NetHandle* client);
	int32_t clientCount;
	int32_t maxClientCount;
};

struct CGClientBuffer {
	uint8_t* message;
	int32_t len;
};

static struct NetHandle local_new_handle(const bool isServer)
{
	return (struct NetHandle)
	{
		.socket = INVALID_SOCKET,
		.readBuffer = isServer ? NULL : malloc(NET_READ_BUFFER_SIZE),
		.messageBuffer = NULL,
		.readLen = 0,
		.messageLen = 0,
		.isServer = isServer
	};
}

static void local_reset_sets(struct NetHandle* handle)
{
	FD_ZERO(&handle->readSet);
	FD_ZERO(&handle->writeSet);
	FD_ZERO(&handle->exceptionSet);

	FD_SET(handle->socket, &handle->readSet);
	FD_SET(handle->socket, &handle->writeSet);
	FD_SET(handle->socket, &handle->exceptionSet);

	if (handle->isServer)
	{
		struct Server* server = (struct Server*)handle;

		// Go through each client and add them to the FD set
		for (int32_t i = 0; i < server->clientCount; ++i)
		{
			CGSocket clientSocket = (server->clients + i)->socket;
			FD_SET(clientSocket, &server->handle.readSet);
			FD_SET(clientSocket, &server->handle.exceptionSet);
		}
	}
}

static bool local_accept_client(struct Server* server)
{
	if (server->clientCount == server->maxClientCount)
		return false;

	CGSocket clientSocket;
	if ((clientSocket = accept(server->handle.socket, NULL, NULL)) == INVALID_SOCKET)
		return false;
	else
	{
		int32_t clientIndex = server->clientCount;
		server->clientCount++;
		(server->clients + clientIndex)->socket = clientSocket;
		return true;
	}
}

static size_t local_get_byte_len_from_info(const uint8_t* buff)
{
	return (size_t)1 << (buff[0] & 0b0111);
}

//static uint8_t local_get_byte_order_code()
//{
//	int16_t word = 0x0001;
//	return ((&word)[0] ?  1 : 0);
//}

static int local_read_message(struct NetHandle* handle)
{
	if (handle->messageBuffer != NULL && handle->currentMessageLen == handle->messageLen)
	{
		// TODO:  This is a great place to optimize memory usage
		free(handle->messageBuffer);
		handle->messageBuffer = NULL;
		handle->currentMessageLen = 0;
		handle->messageLen = 0;
	}

	if (handle->messageBuffer == NULL)
	{
		size_t bytes = local_get_byte_len_from_info(handle->readBuffer);
		if (handle->readLen > (int32_t)(sizeof(uint8_t) + bytes))
		{
			memcpy(&handle->messageLen, handle->readBuffer + sizeof(uint8_t), bytes);

			if (handle->messageLen < NET_READ_BUFFER_SIZE)
				return 1;
			else
			{
				handle->messageBuffer = malloc(handle->messageLen);
				memcpy(handle->messageBuffer, handle->readBuffer, handle->readLen);
				handle->currentMessageLen = handle->readLen;
			}
		}
		else
			return -1;
	}
	else
	{
		if (handle->currentMessageLen + handle->readLen > handle->messageLen)
		{
			free(handle->messageBuffer);
			return -1;
		}
		else
		{
			memcpy(handle->messageBuffer + handle->currentMessageLen,
				handle->readBuffer, handle->readLen);
			handle->currentMessageLen += handle->readLen;

			if (handle->currentMessageLen == handle->messageLen)
				return 1;
		}
	}
	return 0;
}

int Network_init()
{
#ifdef WINDOWS_NETWORKING
	WSADATA wsa_data;
	return WSAStartup(MAKEWORD(1, 1), &wsa_data);
#else
	return 0;
#endif
}

void Network_server_set_connect_handler(struct Server* server, void* state, void(*handler)(void* state, struct NetHandle* client))
{
	server->connectHandlerState = state;
	server->connectHandler = handler;
}

void Network_server_set_disconnect_handler(struct Server* server, void* state, void(*handler)(void* state, struct NetHandle* client))
{
	server->disconnectHandlerState = state;
	server->disconnectHandler = handler;
}

struct Server* Network_new_server(const char* bindingAddress, const uint16_t bindingPort, const int32_t maxClientCount)
{
	struct Server* server = calloc(1, sizeof(struct Server));
	server->handle = local_new_handle(true);
	server->maxClientCount = maxClientCount;
	server->clients = calloc(maxClientCount, sizeof(struct NetHandle));

	for (int i = 0; i < server->maxClientCount; ++i)
	{
		struct NetHandle* client = server->clients + i;
		client->readBuffer = malloc(NET_READ_BUFFER_SIZE);
		client->socket = INVALID_SOCKET;
	}

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
#ifdef WINDOWS_NETWORKING
	hints.ai_flags = AI_PASSIVE;
#else
	hints.ai_flags = AF_UNSPEC;
#endif

	char portString[6];
	snprintf(portString, 6, "%" PRIu16, bindingPort);

	if (getaddrinfo(NULL, portString, &hints, &server->handle.addr) != 0)
	{
#ifdef WINDOWS_NETWORKING
		printf("getaddrinfo failed with error: %d\n", WSAGetLastError());
#endif
		return server;
	}

	server->handle.socket = socket(server->handle.addr->ai_family,
		server->handle.addr->ai_socktype, server->handle.addr->ai_protocol);
	if (validsocket(server->handle.socket))
	{
		int result = bind(server->handle.socket, server->handle.addr->ai_addr, (int)server->handle.addr->ai_addrlen);
		if (result == NO_SOCKET_ERROR)
		{
			if (listen(server->handle.socket, SOMAXCONN) == NO_SOCKET_ERROR)
			{
				// Setup to support select()
				FD_ZERO(&server->handle.readSet);
				FD_SET(server->handle.socket, &server->handle.readSet);
			}
		}
		else if (errno == 13)
			printf("You do not have permissions to run a server, please try again with elevated permissions");
	}
	else
		server->handle.socket = INVALID_SOCKET;

	return server;
}

struct NetHandle* Network_get_server_handle(struct Server* server)
{
	return &server->handle;
}

struct NetHandle* Network_new_client(const char* hostAddress, const uint16_t port)
{
	struct NetHandle* handle = calloc(1, sizeof(struct NetHandle));
	*handle = local_new_handle(false);

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
#ifdef WINDOWS_NETWORKING
	hints.ai_flags = AI_PASSIVE;
#else
	hints.ai_flags = AF_UNSPEC;
#endif

	char portString[6];
	snprintf(portString, 6, "%" PRIu16, port);

	if (getaddrinfo(hostAddress, portString, &hints, &handle->addr) != 0)
	{
#ifdef WINDOWS_NETWORKING
		printf("getaddrinfo failed with error: %d\n", WSAGetLastError());
#endif
		free(handle->readBuffer);
		free(handle);
		return NULL;
	}

	bool found = false;
	struct addrinfo* ptr = NULL;
	for (ptr = handle->addr; ptr != NULL; ptr = ptr->ai_next)
	{
		handle->socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (!validsocket(handle->socket))
			break;

		// TODO:  Cross-platform shorter socket timeout. Possibly select/poll
		//			also this should help with ctrl+c getting caught up
		if (connect(handle->socket, ptr->ai_addr, (int)ptr->ai_addrlen) != NO_SOCKET_ERROR)
			continue;
		else
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		printf("Failed to connect to the server, errno: %d\n", errno);
		free(handle->readBuffer);
		free(handle);
		return NULL;
	}

	return handle;
}

int32_t Network_check(struct NetHandle* handle)
{
	int32_t messageCount = 0;
	int32_t selectTotal;
	local_reset_sets(handle);
	struct timeval t = { 0, 10000 };

	selectTotal = select((int)handle->socket + 1, &handle->readSet, &handle->writeSet, &handle->exceptionSet, &t);
	if (selectTotal < 0)
		return messageCount;

	if (FD_ISSET(handle->socket, &handle->exceptionSet))
		return messageCount;

	if (selectTotal == 0)
		return messageCount;

	if (handle->isServer)
	{
		struct Server* server = (struct Server*)handle;
		if (FD_ISSET(handle->socket, &handle->readSet))
		{

			if (local_accept_client(server))
			{
				if (server->connectHandler != NULL)
					server->connectHandler(server->connectHandlerState, (server->clients + server->clientCount - 1));
			}
		}

		for (int32_t i = 0; i < server->clientCount; ++i)
		{
			CGSocket clientSocket = (server->clients + i)->socket;
			if (FD_ISSET(clientSocket, &handle->readSet))
			{
				struct NetHandle* clientHandle = (server->clients + i);
				if (FD_ISSET(clientHandle->socket, &handle->readSet))
				{
					// MSG_PEEK, MSG_OOB, MSG_WAITALL
					clientHandle->readLen = recv(clientHandle->socket, (char*)clientHandle->readBuffer, NET_READ_BUFFER_SIZE, 0);
					if (clientHandle->readLen <= 0)
					{
						// There wasn't a message to be read
						if (server->disconnectHandler != NULL)
							server->disconnectHandler(server->disconnectHandlerState, clientHandle);
						Network_close_net_handle(clientHandle);
						uint8_t* memLoc = (uint8_t*)(server->clients + i);
						size_t hLen = sizeof(struct NetHandle);
						memmove(memLoc, memLoc + hLen, ((server->clientCount - i) * hLen) - hLen);
						server->clientCount--;
						messageCount--;
						i--;
					}
					else
					{
						switch (local_read_message(clientHandle))
						{
							case 1:
								messageCount++;
								break;
							case -1:
								// TODO:  Disconnect the client for lying
								break;
						}
					}
				}
				else
					clientHandle->readLen = 0;
			}
		}
	}
	else if (FD_ISSET(handle->socket, &handle->readSet))
	{
		handle->readLen = recv(handle->socket, (char*)handle->readBuffer, NET_READ_BUFFER_SIZE, 0);
		switch (local_read_message(handle))
		{
			case 1:
				messageCount++;
				break;
			case -1:
				// TODO:  Disconnect because server has gone mad
				break;
		}
	}
	else
		handle->readLen = 0;

	return messageCount;
}

bool Network_send(const struct NetHandle* handle, const uint8_t* message, const int length, const uint8_t messageCode)
{
	// TODO:  Support splitting of messages (can't send over 32-bit atm)
	size_t len = length + sizeof(uint8_t);

	// TODO:  Test big-endian and little-endian (local_get_byte_order_code)
	uint8_t info = messageCode;
	info <<= 3;
	if (len + sizeof(uint8_t) <= UINT8_MAX)
		info |= 0x00;
	else if (len + sizeof(uint16_t) <= UINT16_MAX)
		info |= 0x01;
	else if (len + sizeof(uint32_t) <= UINT32_MAX)
		info |= 0x02;
	else
		info |= 0x03;

	size_t lenLen = (size_t)1 << (info & 0b0111);
	len += lenLen;

	uint8_t* msg = malloc(len);
	memcpy(msg, &info, sizeof(uint8_t));
	memcpy(msg + sizeof(uint8_t), &len, lenLen);
	memcpy(msg + sizeof(uint8_t) + lenLen, message, length);
	int res = send(handle->socket, (const char*)msg, (int)len, 0);
	return res >= 0;
}

bool Network_send_many(const struct NetHandle** handles, const int32_t socketCount, uint8_t* message, int length, const uint8_t messageCode)
{
	bool success = false;
	for (int32_t i = 0; i < socketCount; ++i)
		success = Network_send(*(handles + i), message, length, messageCode);
	return success;
}

int Network_close_server(struct Server* server)
{
	if (server == NULL) return 0;
	int res = Network_close_net_handle(&server->handle);
	free(server);
	return res;
}

int Network_close_net_handle(struct NetHandle* handle)
{
	int status = 0;
	if (handle == NULL) return status;
#ifdef WINDOWS_NETWORKING
	freeaddrinfo(handle->addr);

	status = shutdown(handle->socket, SD_BOTH);
	if (status == 0)
		status = closesocket(handle->socket);
#else
	free(handle->addr);
	status = shutdown(handle->socket, SHUT_RDWR);
	if (status == 0)
		status = close(handle->socket);
#endif
	return status;
}

int32_t Network_message_len(const struct NetHandle* handle)
{
	if (handle->messageBuffer == NULL)
		return handle->readLen;
	else if (handle->messageLen == handle->currentMessageLen)
		return handle->messageLen;
	return 0;
}

static uint8_t local_get_message_code(const uint8_t* buff)
{
	return (buff[0] & 0b11111000) >> 3;
}

uint8_t Network_get_message_code(const struct NetHandle* handle)
{
	if (handle->messageBuffer == NULL)
		return local_get_message_code(handle->readBuffer);
	return local_get_message_code(handle->messageBuffer);
}

const uint8_t* Network_get_message(const struct NetHandle* handle)
{
	if (handle->messageBuffer == NULL)
		return handle->readBuffer + sizeof(uint8_t) + local_get_byte_len_from_info(handle->readBuffer);
	return handle->messageBuffer + sizeof(uint8_t) + local_get_byte_len_from_info(handle->messageBuffer);
}

size_t Network_server_client_count(const struct Server* server)
{
	return server->clientCount;
}

struct NetHandle* Network_get_client(struct Server* server, size_t clientIndex)
{
	return server->clients + clientIndex;
}

int Network_handle_get_id(const struct NetHandle* handle)
{
	return (int)handle->socket;
}

int Network_quit()
{
#ifdef WINDOWS_NETWORKING
	return WSACleanup();
#else
	return 0;
#endif
}