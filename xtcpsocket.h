#ifndef X_TCPSOCKET_H
#define X_TCPSOCKET_H

#include <stdint.h>
#include <sys/socket.h>

#include "xmessage_queue.h"

typedef enum
{
    xst_server,
    xst_client
} xsocktype_t;

typedef struct
{

    int file_descriptor;
    uint16_t address_family;
    xsocktype_t socket_type;
    xmessage_queue_t *outgoing_message_queue;
    xmessage_queue_t *incoming_message_queue;
    pthread_t *communication_threads;
    struct sockaddr_storage host_socket_address;
    socklen_t host_socket_address_len;
    struct sockaddr_storage remote_socket_address;
    socklen_t remote_socket_address_len;

} xtcpsocket_t;

static int serialize_ip_port_remote(xtcpsocket_t *_socket, const char *_ip, uint16_t _port);

static int serialize_ip_port_host(xtcpsocket_t *_socket, const char *_ip, uint16_t _port);

// Create A New Socket with given address_family
int xtcpsocket_create(xtcpsocket_t *_socket, int _address_family, xsocktype_t _socket_type);

int xtcpsocket_connect(xtcpsocket_t *_socket, const char *_remote_ip, uint16_t _remote_port);

int xtcpsocket_bind(xtcpsocket_t *_socket, const char *_host_ip, uint16_t _host_port);

int xtcpsocket_listen(xtcpsocket_t *_socket, int _back_log);

int xtcpsocket_accept(const xtcpsocket_t *_server_socket, xtcpsocket_t *_client_socket);

int xtcpsocket_init_communication(xtcpsocket_t *_socket);

/*
    Send exactly _buff_size bytes Data from _buff to _socket
    any return value less than _buff_size indicates an error
    in case of error, error message will print in stderr
 */
ssize_t xtcpsocket_sendall(const xtcpsocket_t *_socket, const void *_buff, size_t _buff_size);

/*
    fill exactly _buff_size bytes into _buff from _socket
    any return value less than _buff_size indicats an error
    in case of error, error message will print in stderr
 */
ssize_t xtcpsocket_recvall(const xtcpsocket_t *_socket, void *_buff, size_t _buff_size);

/*
    tries to send _buff_size bytes from _buf
    return the actual amount of data that has been sent
    return -1 in the case of error
*/
ssize_t xtcpsocket_send(const xtcpsocket_t *_socket, const void *_buff, size_t _buff_size);

/*
    tries to read upto _buff_size bytes from _socket to _buff
    return the actual amount of data that has been read
    return -1 in the case of error
*/
ssize_t xtcpsocket_recv(const xtcpsocket_t *_socket, void *_buff, size_t _buff_size);

/*
    put host ip address and host port in _ip_buff and _port arguments
    return -1 if an error happend
    otherwise return 0
*/
int xtcpsocket_gethost(const xtcpsocket_t *_socket, char *_ip_buff, size_t _ip_buff_len, uint16_t *_port_buff);

/*
    put peer ip address and peer port in _ip_buff and _port arguments
    return -1 if an error happend
    otherwise return 0
*/
int xtcpsocket_getremote(const xtcpsocket_t *_socket, char *_ip_buff, size_t _ip_buff_len, uint16_t *_port_buff);

int xtcpsocket_close(const xtcpsocket_t *_socket);
/*
    send all data in the buffer
    set the acutal Data that sent in _size
    return 0 on seuccess,
    Or -1 on failure
*/
static int sendall(int _fd, const void *_buff, size_t *_size, int _flag);

/*
    recieve all data to the buffer
    set the acutal Data that recieved in _size
    return 0 on seuccess,
    Or -1 on failure
*/
static int recvall(int _fd, void *_buff, size_t *_size, int _flag);

#endif /* X_TCPSOCKET_H*/