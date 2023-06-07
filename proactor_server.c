/*
 *  Operation Systems (OSs) Course Assignment 4 Bonus
 *  Proactor Server Implementation
 *  Copyright (C) 2023  Roy Simanovich and Linor Ronen
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "reactor.h"
#include "proactor.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// The reactor pointer.
void *reactor = NULL;

// The proactor pointer.
void *proactor = NULL;

// The number of clients connected to the server in its lifetime.
uint32_t client_count = 0;

// The total number of bytes received from clients in the server's lifetime.
uint64_t total_bytes_received = 0;

// The total number of bytes sent to clients in the server's lifetime.
uint64_t total_bytes_sent = 0;

// A message to send to clients via the proactor.
char *message = "This is a message from the server! "
				"A client has sent a message to the server, "
				"and the server is sending this message back "
				"to the client via the proactor.\n";

int main(void) {
	struct sockaddr_in server_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(SERVER_PORT),
		.sin_addr.s_addr = INADDR_ANY
	};

	int server_fd = -1, reuse = 1;

	fprintf(stdout, "%s", C_INFO_LICENSE);

	signal(SIGINT, signal_handler);

	fprintf(stdout, "%s Starting server...\n", C_PREFIX_INFO);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "%s socket() failed: %s\n", C_PREFIX_ERROR, strerror(errno));
		return EXIT_FAILURE;
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
	{
		fprintf(stderr, "%s setsockopt(SO_REUSEADDR) failed: %s\n", C_PREFIX_ERROR, strerror(errno));
		close(server_fd);
		return EXIT_FAILURE;
	}

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		fprintf(stderr, "%s bind() failed: %s\n", C_PREFIX_ERROR, strerror(errno));
		close(server_fd);
		return EXIT_FAILURE;
	}

	if (listen(server_fd, MAX_QUEUE) < 0)
	{
		fprintf(stderr, "%s listen() failed: %s\n", C_PREFIX_ERROR, strerror(errno));
		close(server_fd);
		return EXIT_FAILURE;
	}

	fprintf(stdout, "%s Server started successfully.\n", C_PREFIX_INFO);

	fprintf(stdout, "%s Server configuration:\n", C_PREFIX_INFO);
	fprintf(stdout, "%s Server is set to %s\033[0;37m.\n", C_PREFIX_INFO, (SERVER_PRINT_MSGS ? "\033[0;32mprint messages" : "\033[0;31mnot print messages"));

	fprintf(stdout, "%s Server listening on port \033[0;32m%d\033[0;37m.\n", C_PREFIX_INFO, SERVER_PORT);

	reactor = createReactor();

	if (reactor == NULL)
	{
		fprintf(stderr, "%s createReactor() failed: %s\n", C_PREFIX_ERROR, strerror(ENOSPC));
		close(server_fd);
		return EXIT_FAILURE;
	}

	proactor = createProactor();

	if (proactor == NULL)
	{
		fprintf(stderr, "%s createProactor() failed: %s\n", C_PREFIX_ERROR, strerror(ENOSPC));
		close(server_fd);
		free(reactor);
		return EXIT_FAILURE;
	}

	fprintf(stdout, "%s Adding server socket to reactor...\n", C_PREFIX_INFO);

	addFd(reactor, server_fd, server_handler);

	if (((reactor_t_ptr)reactor)->head == NULL)
	{
		fprintf(stderr, "%s addFd() failed: %s\n", C_PREFIX_ERROR, strerror(errno));
		close(server_fd);
		free(reactor);
		free(proactor);
		return EXIT_FAILURE;
	}

	fprintf(stdout, "%s Server socket added to reactor successfully.\n", C_PREFIX_INFO);

	startReactor(reactor);
	WaitFor(reactor);

	signal_handler();

	return EXIT_SUCCESS;
}

void signal_handler() {
	fprintf(stdout, "%s%s Server shutting down...\n", MACRO_CLEANUP, C_PREFIX_INFO);
	
	if (reactor != NULL)
	{
		if (proactor != NULL)
		{
			fprintf(stdout, "%s Cancelling all proactor operations...\n", C_PREFIX_INFO);

			if (((PProactor)proactor)->isRunning)
				cancelProactor(proactor);

			fprintf(stdout, "%s Proactor operations cancelled successfully.\n", C_PREFIX_INFO);

			destroyProactor(proactor);
		}

		if (((reactor_t_ptr)reactor)->running)
			stopReactor(reactor);

		fprintf(stdout, "%s Closing all sockets and freeing memory...\n", C_PREFIX_INFO);

		reactor_node_ptr curr = ((reactor_t_ptr)reactor)->head;
		reactor_node_ptr prev = NULL;

		while (curr != NULL)
		{
			prev = curr;
			curr = curr->next;

			close(prev->fd);
			free(prev);
		}

		free(reactor);

		fprintf(stdout, "%s Memory cleanup complete, may the force be with you.\n", C_PREFIX_INFO);
		fprintf(stdout, "%s Statistics:\n", C_PREFIX_INFO);
		fprintf(stdout, "%s Client count in this session: %d\n", C_PREFIX_INFO, client_count);
		fprintf(stdout, "%s Total bytes received in this session: %lu bytes (%lu KB / %lu MB).\n", C_PREFIX_INFO, 
						total_bytes_received, total_bytes_received / 1024, (total_bytes_received / 1024) / 1024);
		fprintf(stdout, "%s Total bytes sent in this session: %lu bytes (%lu KB / %lu MB).\n", C_PREFIX_INFO, 
						total_bytes_sent, total_bytes_sent / 1024, (total_bytes_sent / 1024) / 1024);

		if (client_count > 0)
		{
			fprintf(stdout, "%s Average bytes received per client: %lu bytes (%lu KB / %lu MB).\n", C_PREFIX_INFO, 
							total_bytes_received / client_count, (total_bytes_received / client_count) / 1024, 
							((total_bytes_received / client_count) / 1024) / 1024);
			fprintf(stdout, "%s Average bytes sent per client: %lu bytes (%lu KB / %lu MB).\n", C_PREFIX_INFO, 
							total_bytes_sent / client_count, (total_bytes_sent / client_count) / 1024, 
							((total_bytes_sent / client_count) / 1024) / 1024);
		}
	}

	else
		fprintf(stdout, "%s Reactor wasn't created, no memory cleanup needed.\n", C_PREFIX_INFO);

	fprintf(stdout, "%s Server is now offline, goodbye.\n", C_PREFIX_INFO);

	exit(EXIT_SUCCESS);
}

void *client_handler(int fd, void *react) {
	char *buf = (char *)calloc(MAX_BUFFER, sizeof(char));

	if (buf == NULL)
	{
		fprintf(stderr, "%s calloc() failed: %s\n", C_PREFIX_ERROR, strerror(errno));
		close(fd);
		return NULL;
	}

	int bytes_read = recv(fd, buf, MAX_BUFFER, 0);

	if (bytes_read <= 0)
	{
		if (bytes_read < 0)
			fprintf(stderr, "%s recv() failed: %s\n", C_PREFIX_ERROR, strerror(errno));

		else
			fprintf(stdout, "%s Client %d disconnected.\n", C_PREFIX_WARNING, fd);

		// Remove the client from the proactor.
		removeHandler(proactor, fd);
		
		free(buf);
		close(fd);
		return NULL;
	}

	total_bytes_received += bytes_read;

	// Make sure the buffer is null-terminated, so we can print it.
	if (bytes_read < MAX_BUFFER)
		*(buf + bytes_read) = '\0';

	else
		*(buf + MAX_BUFFER - 1) = '\0';

	// Remove the arrow keys from the buffer, as they are not printable and mess up the output,
	// and replace them with spaces, so the rest of the message won't cut off.
	for (int i = 0; i < bytes_read - 3; i++)
	{
		if ((*(buf + i) == 0x1b) && (*(buf + i + 1) == 0x5b) && (*(buf + i + 2) == 0x41 || *(buf + i + 2) == 0x42 || *(buf + i + 2) == 0x43 || *(buf + i + 2)== 0x44))
		{
			*(buf + i) = 0x20;
			*(buf + i + 1) = 0x20;
			*(buf + i + 2) = 0x20;

			i += 2;
		}
	}

	// Print the message to the server.
	// We don't need to print it if the server is not configured to print messages.
	if (SERVER_PRINT_MSGS)
		fprintf(stdout, "%s Client %d: %s\n", C_PREFIX_MESSAGE, fd, buf);

	free(buf);

	// Send a response to all clients using the proactor thread, error checking is done inside the function.
	int ret = runProactor(proactor);

	if (ret == 1)
	{
		fprintf(stderr, "%s Proactor error: %s\n", C_PREFIX_ERROR, strerror(errno));
		return NULL;
	}

	// This prevents memory leaks when we cancel the reactor thread. We know it's stupid, but it works.
	pthread_join(((PProactor)proactor)->thread, NULL);

	return react;
}

void *server_handler(int fd, void *react) {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	reactor_t_ptr reactor = (reactor_t_ptr)react;

	// Sanity check.
	if (reactor == NULL)
	{
		fprintf(stderr, "%s Server handler error: %s\n", C_PREFIX_ERROR, strerror(EINVAL));
		return NULL;
	}

	int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);

	// Sanity check, as accept() can return -1 on error.
	if (client_fd < 0)
	{
		fprintf(stderr, "%s accept() failed: %s\n", C_PREFIX_ERROR, strerror(errno));
		return NULL;
	}

	fprintf(stdout, "%s Client %s:%d connected, Reference ID: %d\n", C_PREFIX_INFO, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);

	// Add the client to the reactor.
	addFd(reactor, client_fd, client_handler);

	// Add FD to the proactor, so we can send messages back to the client.
	addFD2Proactor(proactor, client_fd, fds_handler);

	client_count++;

	return react;
}

int fds_handler(int fd) {
	// Sanity check for the file descriptor itself, as the proactor doesn't check it - it relies on the reactor to do so.
	if (fd < 0)
	{
		fprintf(stderr, "%s Proactor failed: Invalid file descriptor.\n", C_PREFIX_ERROR);
		removeHandler(proactor, fd);
		return 1;
	}

	int bytes_sent = send(fd, message, strlen(message), 0);

	// Fatal error - remove the client from the proactor and stop the proactor immediately.
	if (bytes_sent < 0)
	{
		fprintf(stderr, "%s send() failed: %s\n", C_PREFIX_ERROR, strerror(errno));
		removeHandler(proactor, fd);
		return 1;
	}

	// We don't need to check if the client disconnected, as the reactor will handle that automatically.
	total_bytes_sent += bytes_sent;

	return 0;
}