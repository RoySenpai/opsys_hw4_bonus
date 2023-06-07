# Operation Systems (OSs) Course Assignment 4 (Bonus)

### For Computer Science B.S.c Ariel University

**By Roy Simanovich and Linor Ronen**

## Description
In this assignment we created a Reactor (Reactor Design Pattern) that handles multiple client file
descriptors (TCP socket) and handles them accordingly, using an handler function for each file
descriptor. Also, in this specific bonus assignment, we also added a proactor (Proactor Design Pattern)
that handles multiple client file descriptors (TCP socket), and handles them accordingly, using an
handler function for each file descriptor. The proactor gets the file descriptors from the reactor,
and whenever a client file descriptor is read from, the proactor handles it accordingly by sending
a message to all the other clients, and then sending the message to the client that sent the message.

### Reactor Library
The Reactor library supports the following functions:
* `void *createReactor()` – Create a reactor object - a linked list of file descriptors and their handlers.
* `void startReactor(void *react)` – Start executing the reactor, in a new thread. 
* `void stopReactor(void *react)` – Stop the reactor - stop the reactor thread and free all the memory it allocated.
* `void addFd(void *react, int fd, handler_t_reactor handler)` – Add a file descriptor to the reactor.
* `void WaitFor(void *react)` – Joins the reactor thread to the calling thread and wait for the reactor to finish.

The handler function is a function that receives a file descriptor and a reactor object. It's called by the reactor when the file descriptor
is ready to be read from, and the handler function is responsible for reading from the file descriptor and handling the data. It should
return back the reactor object, so the reactor can continue to execute, or NULL if the file descriptor should be removed from the
reactor due to an error, or some other reason.

The signature of the handler function for reactors is: ```void *handler_t_reactor(int fd, void *react);```

**_NOTE_:** Please note that the first file descriptor that is added to the reactor is the main file descriptor (listening socket),
and the reactor never removes it from the list, even if it throws an error. Please also note that all the memory allocations
(mainly big arrays) goes through the heap, and not the stack.

The Reactor library is implemented using the following design patterns:
* **Observer** – The reactor is an observer, and the file descriptors are the subjects.
* **Command** – The handlers are commands that are executed by the reactor.
* **Reactor** – The reactor is a reactor, and the handlers are reactors.

The Reactor library is implemented using a linked list of file descriptors and their handlers.

### Proactor Library
The Proactor library supports the following functions:
* `void *createProactor()` – Create a proactor object - a linked list of file descriptors and their handlers.
* `int runProactor(void *this)` – Start executing the proactor, in a new thread.
* `int cancelProactor(void *this)` – Gracefully stop the proactor - stop the proactor thread.
* `int addFD2Proactor(void *this, int fd, handler_t handler)` – Add a file descriptor to the proactor.
* `int removeHandler(void *this, int fd)` – Remove a file descriptor from the proactor.
* `int destroyProactor(void *this)` – Destroy the proactor - stop the proactor thread and free all the memory it allocated.

The signature of the handler function for proactors is: ```int handler_t(int fd);```. The function should return 0 on success, or 1 on failure.

**_NOTE_:** You should never add the listening socket to the proactor, only the client sockets. Trying to add the listening socket to
the proactor will result in a crash (as sending messages to the listening socket is not supported). The proactor will
automatically remove the file descriptor from the proactor when it encounters an error, so you don't need to remove it
yourself.

The Proactor library is implemented using the following design patterns:
* **Command** – The handlers are commands that are executed by the proactor.
* **Proactor** – The proactor is a proactor, and the handlers are proactors.

### The Assignment in General
The whole assignment was written in C, and supports the following features:
* **Thread Safety** – The reactor library is thread safe, and can be used by multiple threads at the same time.
* **Error Handling** – The reactor library handles errors and returns the appropriate error code.
* **Memory Management** – The reactor library frees all the memory it allocates - no memory leaks are possible when
using the library, and no memory is freed twice.
* **Performance** – The reactor library is very efficient, and uses the minimum amount of memory possible.
* **Documentation** – The reactor library functions are documented using Doxygen style comments, in the header files
`reactor.h` and `proactor.h`.
* **Portability** – The reactor library is portable, and can be used on any Linux machine, with any GNU C Compiler,
and any Make version, as long as the machine supports POSIX threads.
* **Simple API** – The reactor library API is very simple and easy to use.
* **Logging** – The reactor library logs all the errors it encounters to the standard error stream, using the
`fprintf` function, and the `stderr` file descriptor.


## Requirements
* Linux machine (Ubuntu 22.04 LTS preferable)
* GNU C Compiler
* Make

## Building
```
# Cloning the repo to local machine.
git clone https://github.com/RoySenpai/opsys_hw4_bonus.git

# Building all the necessary files & the main programs.
make all

# Export shared libraries.
export LD_LIBRARY_PATH="."
```

## Running
```
# Run the reactor server
./proactor_server
```