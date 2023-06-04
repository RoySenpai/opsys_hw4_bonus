/*
 *  Operation Systems (OSs) Course Assignment 4 Bonus
 *  Reactor Header File
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

#ifndef _REACTOR_H
#define _REACTOR_H

#include "settings.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <poll.h>

/********************/
/* Typedefs Section */
/********************/

/*
 * @brief A handler function for a file descriptor.
 * @param fd The file descriptor.
 * @param react Pointer to the reactor object.
 * @return A pointer to something that the handler may return.
 * @note Returning NULL means something went wrong with the file descriptor, and as a result,
 * 			the reactor will automaticly remove the problamtic file descriptor from the list.
*/
typedef void *(*handler_t_reactor)(int fd, void *react);

/*
 * @brief A node in the reactor linked list.
 */
typedef struct _reactor_node reactor_node, *reactor_node_ptr;

/*
 * @brief A reactor object - a linked list of file descriptors and their handlers.
 */
typedef struct _reactor_t reactor_t, *reactor_t_ptr;

/*
 * @brief A pollfd object, used to poll the file descriptors.
*/
typedef struct pollfd pollfd_t, *pollfd_t_ptr;


/**********************/
/* Structures Section */
/**********************/

/*
 * @brief A node in the reactor linked list.
 */
struct _reactor_node
{
	/*
	 * @brief The file descriptor.
	 * @note The first node is always the listening socket.
	*/
	int fd;

	/*
	 * @brief The file descriptor's handler union.
	 * @note The union is used to allow the handler to be printed as a generic pointer,
	 * 			with the compiler's flag -Wpedantic.
	*/
	union _hdlr_func_union_reactor
	{
		/*
		 * @brief The file descriptor's handler.
		 * @note The first node is always the listening socket, and its handler is always
		 * 			to accept a new connection and add it to the reactor.
		*/
		handler_t_reactor handler;

		/*
		 * @brief A pointer to the handler function.
		 * @note This is a generic pointer, and it's used to print the handler's address.
		*/
		void *handler_ptr;
	} hdlr;

	/*
	 * @brief The next node in the linked list.
	 * @note For the last node, this is NULL.
	*/
	reactor_node_ptr next;
};

/*
 * @brief A reactor object - a linked list of file descriptors and their handlers.
 */
struct _reactor_t
{
	/*
	 * @brief The thread in which the reactor is running.
	 * @note The thread is created in startReactor() and deleted in stopReactor().
	 * 			The thread function is reactorRun().
	*/
	pthread_t thread;

	/*
	 * @brief The first node in the linked list.
	 * @note The first node is always the listening socket.
	*/
	reactor_node_ptr head;

	/*
	 * @brief A pointer to an array of pollfd structures.
	 * @note The array is allocated and freed in reactorRun().
	 * @note The array is used in reactorRun() to call poll().
	*/
	pollfd_t_ptr fds;

	/*
	 * @brief A boolean value indicating whether the reactor is running.
	 * @note The value is set to true in startReactor() and to false in stopReactor().
	*/
	bool running;
};


/********************************/
/* Functions Declartion Section */
/********************************/

/*
 * @brief Create a reactor object - a linked list of file descriptors and their handlers.
 * @return A pointer to the created object, or NULL if failed.
 * @note The returned pointer must be freed.
 */
void *createReactor();

/*
 * @brief Start executing the reactor, in a new thread.
 * @param react A pointer to the reactor object.
 * @return void
 */
void startReactor(void *react);

/*
 * @brief Stop the reactor - delete the thread.
 * @param react A pointer to the reactor object.
 * @return void
 */
void stopReactor(void *react);

/*
 * @brief Add a file descriptor to the reactor.
 * @param react A pointer to the reactor object.
 * @param fd The file descriptor to add.
 * @param handler The handler function to call when the file descriptor is ready.
 * @return void
 */
void addFd(void *react, int fd, handler_t_reactor handler);

/*
 * @brief Wait for the reactor to finish.
 * @param react A pointer to the reactor object.
 * @return void
 */
void WaitFor(void *react);

#endif