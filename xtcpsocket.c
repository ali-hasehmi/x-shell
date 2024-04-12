#include "xtcpsocket.h"

#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

void *recv_msg_worker(void *arg)
{
    xtcpsocket_t *currSocket = arg;
    while (1)
    {
        xmessage_t tmp_msg;
        if (xmessage_recv(currSocket, &tmp_msg))
        {
            return NULL;
        }
        xmessage_queue_enqueue(currSocket->incoming_message_queue, &tmp_msg); // what if queue was full?
    }
}

void *send_msg_worker(void *arg)
{
    xtcpsocket_t *currSocket = arg;
    while (1)
    {
        xmessage_t tmp_msg;
        int res = xmessage_queue_dequeue(currSocket->outgoing_message_queue, &tmp_msg); // what if queue was empty?
        xmessage_send(currSocket, &tmp_msg);
    }
}
int serialize_ip_port_remote(xtcpsocket_t *_socket, const char *_ip, uint16_t _port)
{

    if (_socket->address_family == AF_INET)
    {
        struct sockaddr_in *temp = (struct sockaddr_in *)&(_socket->remote_socket_address);
        _socket->host_socket_address_len = _socket->remote_socket_address_len = sizeof(struct sockaddr_in);
        if (memset(temp, 0, sizeof(struct sockaddr_in)) == NULL)
        {
            fprintf(stderr,
                    "[!] serialize_ip_port():memset() failed: %s\r\n", strerror(errno));
            return -1;
        }
        temp->sin_family = AF_INET;
        temp->sin_port = htons(_port);
        if (_ip == NULL)
        {
            temp->sin_addr.s_addr = INADDR_ANY;
        }
        else
        {
            if (inet_pton(AF_INET, _ip, &temp->sin_addr) != 1)
            {
                fprintf(stderr,
                        "[!] xtcpsocket_connect():inet_pton() failed: %s\r\n", strerror(errno));
                return -1;
            }
        }
    }

    else if (_socket->address_family == AF_INET6)
    {
        struct sockaddr_in6 *temp = (struct sockaddr_in6 *)&(_socket->remote_socket_address);
        _socket->host_socket_address_len = _socket->remote_socket_address_len = sizeof(struct sockaddr_in6);
        if (memset(temp, 0, sizeof(struct sockaddr_in6)) == NULL)
        {
            fprintf(stderr,
                    "[!] xtcpsocket_connect():memset() failed: %s\r\n", strerror(errno));
            return -1;
        }
        temp->sin6_family = AF_INET6;
        temp->sin6_port = htons(_port);
        if (_ip == NULL)
        {
            temp->sin6_addr = in6addr_any;
        }
        else
        {

            if (inet_pton(AF_INET6, _ip, &temp->sin6_addr) != 1)
            {
                fprintf(stderr,
                        "[!] serialize_ip_port():inet_pton() failed: %s\r\n", strerror(errno));
                return -1;
            }
        }
    }
    else
    {
        errno = EINVAL;
        fprintf(stderr, "[!] serialize_ip_port() failed: Invalid address_family\r\n");
        return -1;
    }
    return 0;
}

int serialize_ip_port_host(xtcpsocket_t *_socket, const char *_ip, uint16_t _port)
{
    if (_socket->address_family == AF_INET)
    {
        struct sockaddr_in *temp = (struct sockaddr_in *)&(_socket->host_socket_address);
        _socket->host_socket_address_len = _socket->remote_socket_address_len = sizeof(struct sockaddr_in);
        if (memset(temp, 0, sizeof(struct sockaddr_in)) == NULL)
        {
            fprintf(stderr,
                    "[!] serialize_ip_port():memset() failed: %s\r\n", strerror(errno));
            return -1;
        }
        temp->sin_family = AF_INET;
        temp->sin_port = htons(_port);
        if (_ip == NULL)
        {
            temp->sin_addr.s_addr = INADDR_ANY;
        }
        else
        {
            if (inet_pton(AF_INET, _ip, &temp->sin_addr) != 1)
            {
                fprintf(stderr,
                        "[!] xtcpsocket_connect():inet_pton() failed: %s\r\n", strerror(errno));
                return -1;
            }
        }
    }

    else if (_socket->address_family == AF_INET6)
    {
        struct sockaddr_in6 *temp = (struct sockaddr_in6 *)&(_socket->host_socket_address);
        _socket->host_socket_address_len = _socket->remote_socket_address_len = sizeof(struct sockaddr_in6);
        if (memset(temp, 0, sizeof(struct sockaddr_in6)) == NULL)
        {
            fprintf(stderr,
                    "[!] xtcpsocket_connect():memset() failed: %s\r\n", strerror(errno));
            return -1;
        }
        temp->sin6_family = AF_INET6;
        temp->sin6_port = htons(_port);
        if (_ip == NULL)
        {
            temp->sin6_addr = in6addr_any;
        }
        else
        {

            if (inet_pton(AF_INET6, _ip, &temp->sin6_addr) != 1)
            {
                fprintf(stderr,
                        "[!] serialize_ip_port():inet_pton() failed: %s\r\n", strerror(errno));
                return -1;
            }
        }
    }
    else
    {
        errno = EINVAL;
        fprintf(stderr,
                "[!] serialize_ip_port() failed: Invalid address_family\r\n");
        return -1;
    }
    return 0;
}

int xtcpsocket_create(xtcpsocket_t *_socket, int _address_family, xsocktype_t _socket_type)
{
    _socket->communication_threads = NULL;
    _socket->incoming_message_queue = NULL;
    _socket->outgoing_message_queue = NULL;
    _socket->address_family = _address_family;
    _socket->socket_type = _socket_type;
    if ((_socket->file_descriptor = socket(_address_family, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_create():socket() failed: %s\r\n", strerror(errno));
        return -1;
    }
    return 0;
}

int xtcpsocket_connect(xtcpsocket_t *_socket, const char *_remote_ip, uint16_t _remote_port)
{
    if (_socket->socket_type == xst_server)
    {
        fprintf(stderr,
                "[!] xtcpsocket_connect() failed: called on server socket!\r\n");
        return -1;
    }
    if (serialize_ip_port_remote(_socket, _remote_ip, _remote_port))
    {
        fprintf(stderr,
                "[!] xtcpsocket_connect():serialize_ip_port() failed: %s\r\n", strerror(errno));
        return -1;
    }

    if (connect(_socket->file_descriptor,
                (struct sockaddr *)&(_socket->remote_socket_address),
                _socket->remote_socket_address_len))
    {
        fprintf(stderr,
                "[!] xtcpsocket_connect():connect() failed: %s\r\n", strerror(errno));
        return -1;
    }

    if (getsockname(_socket->file_descriptor,
                    (struct sockaddr *)(&_socket->host_socket_address),
                    &_socket->host_socket_address_len) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_connect():getsockname() failed: %s\r\n", strerror(errno));
        return -1;
    }
    return 0;
}

int xtcpsocket_bind(xtcpsocket_t *_socket, const char *_host_ip, uint16_t _host_port)
{
    // if (_socket->socket_type == xst_client)
    // {
    //     fprintf(stderr,
    //             "[!] xtcpsocket_bind() failed: called on client socket!\r\n");
    //     return -1;
    // }

    if (serialize_ip_port_host(_socket, _host_ip, _host_port))
    {
        fprintf(stderr,
                "[!] xtcpsocket_bind():serialize_ip_port() failed: %s\r\n", strerror(errno));
        return -1;
    }
    if (bind(_socket->file_descriptor,
             (struct sockaddr *)(&_socket->host_socket_address),
             _socket->host_socket_address_len) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_bind():bind() failed: %s\r\n", strerror(errno));
        return -1;
    }

    if (getsockname(_socket->file_descriptor,
                    (struct sockaddr *)(&_socket->host_socket_address),
                    &_socket->host_socket_address_len) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_bind():getsockname() failed: %s\r\n", strerror(errno));
        return -1;
    }
    return 0;
}

int xtcpsocket_listen(xtcpsocket_t *_socket, int _back_log)
{
    if (_socket->socket_type == xst_client)
    {
        fprintf(stderr,
                "[!] xtcpsocket_listen():listen() failed: called on client socket\r\n");
        return -1;
    }
    if (listen(_socket->file_descriptor, _back_log) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_listen():listen() failed: %s\r\n", strerror(errno));
        return -1;
    }
    return 0;
}

int xtcpsocket_accept(const xtcpsocket_t *_server_socket, xtcpsocket_t *_client_socket)
{

    memcpy(_client_socket, _server_socket, sizeof(xtcpsocket_t));

    if (
        (_client_socket->file_descriptor = accept(_server_socket->file_descriptor,
                                                  (struct sockaddr *)&(_client_socket->remote_socket_address),
                                                  &(_client_socket->remote_socket_address_len))) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_accept():accept() failed: %s\r\n", strerror(errno));
        return -1;
    }

    return 0;
}

int xtcpsocket_init_communication(xtcpsocket_t *_socket)
{
    // allocating memory for incoming and outgoing message queues
    _socket->incoming_message_queue = malloc(sizeof(xmessage_queue_t));
    if (_socket->incoming_message_queue == NULL)
    {
        fprintf(stderr,
                "[!] xtcpsocket_create():malloc() failed: %s\r\n", strerror(errno));
        return -1;
    }

    _socket->outgoing_message_queue = malloc(sizeof(xmessage_queue_t));
    if (_socket->outgoing_message_queue == NULL)
    {
        fprintf(stderr,
                "[!] xtcpsocket_create():malloc() failed: %s\r\n", strerror(errno));
        return -1;
    }

    // creating queues for incoming and outgoing messages
    if (xmessage_queue_create(_socket->incoming_message_queue))
    {
        fprintf(stderr,
                "[!] xtcpsocket_create():xmessage_queue_create() failed\r\n");
        return -1;
    }
    if (xmessage_queue_create(_socket->outgoing_message_queue))
    {
        fprintf(stderr,
                "[!] xtcpsocket_create():xmessage_queue_create() failed\r\n");
        return -1;
    }

    // allocate memory for threads
    _socket->communication_threads = malloc(2 * sizeof(pthread_t));
    if (pthread_create(&_socket->communication_threads[0],
                       NULL,
                       &recv_msg_worker,
                       (void *)_socket))
    {
        fprintf(stderr,
                "[!] xtcpsocket_create():pthread_create() failed: %s\r\n", strerror(errno));
        return -1;
    }
    if (pthread_create(&_socket->communication_threads[1],
                       NULL,
                       &send_msg_worker,
                       (void *)_socket))
    {
        fprintf(stderr,
                "[!] xtcpsocket_create():pthread_create() failed: %s\r\n", strerror(errno));
        return -1;
    }
}

ssize_t xtcpsocket_sendall(const xtcpsocket_t *_socket, const void *_buff, size_t _buff_size)
{

    if (sendall(_socket->file_descriptor,
                _buff,
                &_buff_size, 0))
    {
        fprintf(stderr,
                "[!] xtcpsocket_sendall():sendall() failed: %s\r\n", strerror(errno));
    }
    return _buff_size;
}

ssize_t xtcpsocket_recvall(const xtcpsocket_t *_socket, void *_buff, size_t _buff_size)
{
    if (recvall(_socket->file_descriptor,
                _buff,
                &_buff_size,
                0))
    {
        fprintf(stderr,
                "[!] xtcpsocket_recv():recvall() failed: %s\r\n", strerror(errno));
    }
    return _buff_size;
}

ssize_t xtcpsocket_send(const xtcpsocket_t *_socket, const void *_buff, size_t _buff_size)
{
    return send(_socket->file_descriptor, _buff, _buff_size, 0);
}

ssize_t xtcpsocket_recv(const xtcpsocket_t *_socket, void *_buff, size_t _buff_size)
{
    return recv(_socket->file_descriptor, _buff, _buff_size, 0);
}

int xtcpsocket_gethost(const xtcpsocket_t *_socket, char *_ip_buff, size_t _ip_buff_len, uint16_t *_port_buff)
{

    if (_socket->address_family == AF_INET)
    {
        struct sockaddr_in *tmp = (struct sockaddr_in *)&(_socket->host_socket_address);
        if (inet_ntop(_socket->address_family,
                      &(tmp->sin_addr.s_addr),
                      _ip_buff,
                      _ip_buff_len) == NULL)
        {
            fprintf(stderr,
                    "[!] xtcpsocket_gethost():inet_ntop() failed: %s\r\n", strerror(errno));
            return -1;
        }
        *_port_buff = ntohs(tmp->sin_port);
    }

    else if (_socket->address_family == AF_INET6)
    {
        struct sockaddr_in6 *tmp = (struct sockaddr_in6 *)&(_socket->host_socket_address);
        if (inet_ntop(_socket->address_family,
                      &(tmp->sin6_addr.__in6_u.__u6_addr8),
                      _ip_buff,
                      _ip_buff_len) == NULL)
        {
            fprintf(stderr,
                    "[!] xtcpsocket_gethost():inet_ntop() failed: %s\r\n", strerror(errno));
            return -1;
        }
        *_port_buff = ntohs(tmp->sin6_port);
    }

    else
    {
        fprintf(stderr,
                "[!] xtcpsocket_gethost() failed: invalid address family in socket\r\n");
        return -1;
    }
    return 0;
}

int xtcpsocket_getremote(const xtcpsocket_t *_socket, char *_ip_buff, size_t _ip_buff_len, uint16_t *_port_buff)
{
    if (_socket->address_family == AF_INET)
    {
        struct sockaddr_in *tmp = (struct sockaddr_in *)&(_socket->remote_socket_address);
        if (inet_ntop(_socket->address_family,
                      &(tmp->sin_addr.s_addr),
                      _ip_buff,
                      _ip_buff_len) == NULL)
        {
            fprintf(stderr,
                    "[!] xtcpsocket_getremote():inet_ntop() failed: %s\r\n", strerror(errno));
            return -1;
        }
        *_port_buff = ntohs(tmp->sin_port);
    }

    else if (_socket->address_family == AF_INET6)
    {
        struct sockaddr_in6 *tmp = (struct sockaddr_in6 *)&(_socket->remote_socket_address);
        if (inet_ntop(_socket->address_family,
                      &(tmp->sin6_addr.__in6_u.__u6_addr8),
                      _ip_buff,
                      _ip_buff_len) == NULL)
        {
            fprintf(stderr,
                    "[!] xtcpsocket_getremote():inet_ntop() failed: %s\r\n", strerror(errno));
            return -1;
        }
        *_port_buff = ntohs(tmp->sin6_port);
    }

    else
    {
        fprintf(stderr,
                "[!] xtcpsocket_getremote() failed: invalid address family in socket\r\n");
        return -1;
    }
    return 0;
}

int xtcpsocket_close(const xtcpsocket_t *_socket)
{
    if (_socket->communication_threads)
    {
        pthread_cancel(_socket->communication_threads[0]);
        pthread_cancel(_socket->communication_threads[1]);
        free(_socket->communication_threads);
    }
    if (_socket->incoming_message_queue)
    {
        if (xmessage_queue_destroy(_socket->incoming_message_queue))
        {
            return -1;
        }
        free(_socket->incoming_message_queue);
    }
    if (_socket->outgoing_message_queue)
    {

        if (xmessage_queue_destroy(_socket->outgoing_message_queue))
        {
            return -1;
        }
        free(_socket->outgoing_message_queue);
    }
    if (close(_socket->file_descriptor))
    {
        return -1;
    }

    return 0;
}

int sendall(int _fd, const void *_buff, size_t *_size, int _flag)
{
    size_t total = 0,
           bytesleft = *_size;
    ssize_t n;
    while (total < *_size)
    {
        n = send(_fd, _buff + total, bytesleft, _flag);
        if (n == -1)
        {
            break;
        }
        total += n;
        bytesleft -= n;
    }
    *_size = total; // return number actually sent here
    return n == -1 ? -1 : 0;
}

int recvall(int _fd, void *_buff, size_t *_size, int _flag)
{
    size_t total = 0,
           bytesleft = *_size;
    ssize_t n;
    while (total < *_size)
    {
        n = recv(_fd, _buff + total, bytesleft, _flag);
        if (n == -1)
        {
            break;
        }
        total += n;
        bytesleft -= n;
    }
    *_size = total; // return number actually recieve here
    return n == -1 ? -1 : 0;
}
