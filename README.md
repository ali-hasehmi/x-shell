# x-shell - Linux Reverse Shell
x-shell is a simple Linux reverse shell program written in C. It allows a user to establish a reverse shell connection from a target machine back to a listening attacker machine, providing remote access to execute commands, interact with the target system, as well as download and upload files.

## Features
- Establish reverse shell connections from target machines to attacker machines.
- Interactive command execution and output retrieval.
- Download files from the attacker machine to the target machine.
Upload files from the target machine to the attacker machine.
- Simple and lightweight implementation.

## How To Compile
I used no external library other than standard C Library (libc) so all You need to have is a C Compiler(e.g. gcc) and make utility then just run `make` in top level directory this will create both Server(attacker) and Client(target) binaries:

`./server/xShellServer` -> attacker binary

`./client/xShellClient` -> target binary

## How To Change Server Listening Port and IP?
there is a `config.h` header in the root of the repository, you can change these and some other parameters then you need to run `make` again

# Why?
I wanted to learn linux programming, specifically:
- filesystem handling
- process management
- networking in pure C
- pty

so it's a learning project and not a real one, therefore it lacks some important features:
- no encryption at all, just good old plain-text tcp
- no well-created interactive shell for attacker(probably [gnu-readline](https://en.wikipedia.org/wiki/GNU_Readline) is all what it needs)
- no server daemon(server needs a deamon to handle connections and a front-end is needed to communicate with deamon)
- no configuration parsing(You probably don't want to recompile the whole Project just to change a server listening port:) )
