/*
 *  Operation Systems (OSs) Course Assignment 4 Bonus
 *  Settings Header File
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

#ifndef _SETTINGS_H
#define _SETTINGS_H

/*
 * Include this file before including system headers.  By default, with
 * C99 support from the compiler, it requests POSIX 2001 support.  With
 * C89 support only, it requests POSIX 1997 support.  Override the
 * default behaviour by setting either _XOPEN_SOURCE or _POSIX_C_SOURCE.
*/
#if !defined(_XOPEN_SOURCE) && !defined(_POSIX_C_SOURCE)
	#if __STDC_VERSION__ >= 199901L
		#define _XOPEN_SOURCE 600   /* SUS v3, POSIX 1003.1 2004 (POSIX 2001 + Corrigenda) */
	#else
		#define _XOPEN_SOURCE 500   /* SUS v2, POSIX 1003.1 1997 */
	#endif /* __STDC_VERSION__ */
#endif /* !_XOPEN_SOURCE && !_POSIX_C_SOURCE */

/********************/
/* Settings Section */
/********************/

/*
 * @brief The port on which the server listens.
 * @note The default port is 9034.
*/
#define SERVER_PORT 		9034

/*
 * @brief The maximum number of clients that can connect to the server.
 * @note The default number is 16384 clients.
 * @note For alot of OSes this is above the hard-limit.
*/
#define MAX_QUEUE 			16384

/*
 * @brief The maximum number of bytes that can be read from a socket.
 * @note The default number is 2048 bytes.
*/
#define MAX_BUFFER 			2048

/*
 * @brief Defines the timeout for the poll() function.
 * @note The default timeout is -1.
 * @note A timeout of 0 means that poll() will return immediately.
 * @note A timeout of -1 means that poll() will wait forever.
*/
#define POLL_TIMEOUT 		-1

/*
 * @brief Defines whether the server prints messages or not.
 * @note The default value is 1.
 * @note A value of 0 means that the server won't print any incoming messages.
 * @note A value of 1 means that the server will print every incoming message.
*/
#define SERVER_PRINT_MSGS	1


/************************/
/* Messages definitions */
/************************/

// Prefixes

// Colored prefix for error messages.
#define C_PREFIX_ERROR 		"\033[0;31m[ERROR]\033[0;37m"

// Colored prefix for warning messages.
#define C_PREFIX_WARNING 	"\033[0;33m[WARNING]\033[0;37m"

// Colored prefix for infomation messages.
#define C_PREFIX_INFO 		"\033[0;35m[INFO]\033[0;37m"

// Colored prefix for client messages.
#define C_PREFIX_MESSAGE 	"\033[0;32m[MESSAGE]\033[0;37m"

// Messages

// Information and license message that's printing in the beggining of the program.
#define C_INFO_LICENSE  	"\t\t\t\033[0;36mProactor Server\n" \
							"\tCopyright (C) 2023  Roy Simanovich and Linor Ronen\n" \
							"\tThis program comes with ABSOLUTELY NO WARRANTY.\n" \
							"\tThis is free software, and you are welcome to redistribute it\n" \
							"\tunder certain conditions; see `LICENSE' for details.\033[0;37m\n\n"

// Macro to cleanup the current line.
#define MACRO_CLEANUP		"\33[2K\r\033"

// Length of the relay message prefix.
// This is a fixed length, and should not be changed.
// The length is 33 bytes and the prefix is:
// "Message from client <clientid>: <message>"
#define SERVER_RLY_MSG_LEN	33


/*
 * @brief A signal handler for SIGINT.
 * @note This function is called when the user presses CTRL+C.
 * 			It stops the reactor, closes all sockets and frees all memory,
 * 			Then, it exits the program.
 * @note This function is registered to SIGINT in main().
*/
void signal_handler();

/*
 * @brief A handler for a client socket.
 * @param fd The client socket file descriptor.
 * @param arg The reactor.
 * @return The reactor on success, NULL otherwise.
 * @note This function is called when a client sends a message to the server,
 * 			and prints the message.
*/
void *client_handler(int fd, void *react);

/*
 * @brief A handler for the server socket.
 * @param fd The server socket file descriptor.
 * @param arg The reactor.
 * @return The reactor on success, NULL otherwise.
 * @note This function is called when a new client connects to the server,
 * 			and adds the client to the reactor.
*/
void *server_handler(int fd, void *react);

/*
 * @brief A handler for the fds of the proactor.
 * @param fd The file descriptor.
 * @return 0 on success, 1 otherwise.
 * @note This function is called when a new message is received from one of the clients.
*/
int fds_handler(int fd);

#endif /* !_SETTINGS_H */