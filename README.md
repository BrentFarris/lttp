# Lightweight Text Transfer Protocol (LTTP)
The Lightweight Text Transfer Protocol (LTTP) is a TCP/IP connection protocol for human readable text exchange between one or more machines. This specification is designed as a means of transfering human readable UTF-8 text from a server to a client machine in a way that is efficient on both bandwidth and compute resources. Any machine with access to input, output, and the internet protocol (IP) is capable of implementing and using this protocol.

### LTTP client software
A spec complete client is provided [on GitHub](https://github.com/BrentFarris/lttp/tree/master/src/client) and can be used to create any custom client software with additional features. It is worth noting that no information about the specific networked client is sent as part of any packets. We've seen software client information being abused by specific hosts in HTTP. LTTP is not designed in a way that will allow client software favoritism and all servers should comply with the LTTP protocol and function properly with the standard lttp client code provided here.

### LTTP server software
A sample LTTP server can be found [on GitHub](https://github.com/BrentFarris/lttp/tree/master/src/server) which shows how to start the server given the base server code implementation. If you are to use this sample code, then you will also need the rest of the code in the same GitHub repository to get started. The code provided in the [LTTP GitHub repository](https://github.com/BrentFarris/lttp) is by no means the only allowed code to host a LTTP server; it is just meant as the most portible and up to date server code implementation.

## The LTTP specification
The first byte of the payload holds both the message code and the number of bytes to use for the length of the message. The first 3 bits are used to denote the length in bytes of the payload length, we'll call this **N**. The next 5 bits are used to denote the message type code. The next **N** bytes hold the length of the entire message (including the first info byte discussed).

From the first byte of the message you can get the length of **N** bytes by the following:
```c
// Assuming our uint8_t* network message is named "buff"
size_t msgLengthSize = (size_t)1 << (buff[0] & 0b0111);
```

Now you have what you need to read the length of the message to know how many bytes are being sent across the network:
```c
// Assuming our uint8_t* network message is named "buff"
uint64_t msgLen = 0;
memcpy(&msgLen, buff + 1, msgLengthSize);
// NOTE:  MAKE SURE you do proper buffer overflow handling, the above code is meant for simplicity to aid understanding
```

From the first byte of the message you can get the message type code by the following:
```c
// Assuming our uint8_t* network message is named "buff"
uint8_t typeCode = (buff[0] & 0b11111000) >> 3;
```

TBD

### Text (type code 1)
TBD

### Form (type code 2)
TBD

### File (type code 3)
TBD
