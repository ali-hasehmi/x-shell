# x-shell - Linux Reverse Shell
x-shell is a simple Linux reverse shell program written in C. It allows a user to establish a reverse shell connection from a target machine back to a listening attacker machine, providing remote access to execute commands, interact with the target system, as well as download and upload files.

## Features
- Establish reverse shell connections from target machines to attacker machines.
- Interactive command execution and output retrieval.
- Download files from the attacker machine to the target machine.
Upload files from the target machine to the attacker machine.
- Simple and lightweight implementation.

## How To Compile


* server side:

in x-shell-server.c change the value of LISTEN_PORT macro to the port you want server listens.
```bash

gcc -o x-shell-server x-shell-server.c xtcpsocket.c xterminal.c xclient-list.c xclient.c xshell.c xmessage_queue.c xmessage.c sfragment.c xfile.c xfragment.c xrequest.c -lm -lpthread -lutil

```

* client side:

in x-shell-client.c change the value of SERVER_IP and SERVER_PORT macros to your server's ip and port respectively.
```bash

gcc -o x-shell-client x-shell-client.c xmessage.c xmessage_queue.c sfragment.c xshell.c xtcpsocket.c xfile.c xrequest.c xfragment.c -lpthread

```