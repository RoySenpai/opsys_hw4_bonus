/*
 *  Operation Systems (OSs) Course Assignment 4 Bonus
 *  Proactor Header File
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

#ifndef _PROACTOR_H
#define _PROACTOR_H

#include "settings.h"
#include <stdbool.h>
#include <pthread.h>

/********************/
/* Typedefs Section */
/********************/

/*
 * @brief The proactor's handler function.
 * @param fd The file descriptor.
 * @return 0 on success, 1 on failure.
*/
typedef int (*handler_t)(int);


/**********************/
/* Structures Section */
/**********************/

/*
 * @brief A node in the proactor's linked list.
 * @param fd The file descriptor.
 * @param handler The file descriptor's handler.
 * @param next The next node in the linked list.
*/
typedef struct _proactor_t_node {
	/*
	 * @brief The file descriptor.
	*/
	int fd;

	/*
	 * @brief The file descriptor's handler union.
	 * @note The union is used to allow the handler to be printed as a generic pointer,
	 * 			with the compiler's flag -Wpedantic.
	*/
	union _hdlr_func_union_proactor
	{
		/*
		 * @brief The file descriptor's handler.
		*/
		handler_t handler;

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
	struct _proactor_t_node *next;
} ProactorNode, *PProactorNode;

/*
 * @brief The proactor's structure.
 * @param thread The proactor's thread identifier.
 * @param head The proactor's head node, stores a linked list of file descriptors and their handlers.
 * @param isRunning A boolean value indicating whether the proactor is running.
 * @param size The proactor's size, i.e. the number of file descriptors in the proactor.
*/
typedef struct _proactor_t {
	/*
	 * @brief The proactor's thread identifier.
	*/
	pthread_t thread;

	/*
	 * @brief The proactor's head node, stores a linked list of file descriptors and their handlers.
	*/
	PProactorNode head;

	/*
	 * @brief A boolean value indicating whether the proactor is running.
	 * @note The value is set to true in runProactor() and to false in cancelProactor().
	*/
	bool isRunning;

	/*
	 * @brief The proactor's size, i.e. the number of file descriptors in the proactor.
	 * @note The value is incremented in addFD2Proactor() and decremented in removeHandler().
	 * 			The value is used to determine whether the proactor is empty.
	*/
	int size;
} Proactor, *PProactor;


/********************************/
/* Functions Declartion Section */
/********************************/

/*
 * @brief Creates a new proactor.
 * @return A pointer to the new proactor, or NULL on failure.
 * @note The proactor must be freed using the function destroyProactor.
*/
void *createProactor();

/*
 * @brief Runs a proactor.
 * @param this A pointer to the proactor.
 * @return 0 on success, 1 on failure.
*/
int runProactor(void *this);

/*
 * @brief Cancels a proactor.
 * @param this A pointer to the proactor.
 * @return 0 on success, 1 on failure.
*/
int cancelProactor(void *this);

/*
 * @brief Adds a file descriptor to a proactor.
 * @param this A pointer to the proactor.
 * @param fd The file descriptor.
 * @param handler The file descriptor's handler.
 * @return 0 on success, 1 on failure.
*/
int addFD2Proactor(void *this, int fd, handler_t handler);

/*
 * @brief Removes a file descriptor from a proactor.
 * @param this A pointer to the proactor.
 * @param fd The file descriptor.
 * @return 0 on success, 1 on failure.
*/
int removeHandler(void *this, int fd);

/*
 * @brief Destroys a proactor.
 * @param this A pointer to the proactor.
 * @return 0 on success, 1 on failure.
 * @note This function must be called to free the proactor.
*/
int destroyProactor(void *this);

#endif // _PROACTOR_H