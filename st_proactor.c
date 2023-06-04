/*
 *  Operation Systems (OSs) Course Assignment 4 Bonus
 *  Proactor Library Implementation
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

#include "proactor.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void *proactorRunFunction(void *args) {
	if (args == NULL)
	{
		errno = EINVAL;
		fprintf(stderr, "%s proactorRun() failed: %s\n", C_PREFIX_ERROR, strerror(EINVAL));
		return NULL;
	}

	PProactor proactor = (PProactor)args;

	if (!proactor->isRunning)
	{
		fprintf(stderr, "%s proactorRun() failed: the proactor is not running\n", C_PREFIX_ERROR);
		return NULL;
	}

	fprintf(stderr, "%s Proactor thread started.\n", C_PREFIX_INFO);

	PProactorNode curr = proactor->head;

	while (curr != NULL)
	{
		if (curr->hdlr.handler != NULL)
		{
			int ret = curr->hdlr.handler(curr->fd);

			if (ret != 0)
			{
				fprintf(stderr, "%s proactorRun() failed: handler returned %d\n", C_PREFIX_ERROR, ret);
				proactor->isRunning = false;
				return NULL;
			}
		}

		curr = curr->next;
	}

	proactor->isRunning = false;

	fprintf(stderr, "%s Proactor finished running, total file descriptors: %d\n", C_PREFIX_INFO, proactor->size);
	return proactor;
}

void *createProactor() {
	fprintf(stderr, "%s Creating proactor...\n", C_PREFIX_INFO);

	PProactor proactor = (PProactor) malloc(sizeof(Proactor));

	if (proactor == NULL)
	{
		fprintf(stderr, "%s createProactor() failed: %s\n", C_PREFIX_ERROR, strerror(errno));
		return NULL;
	}

	proactor->thread = 0;
	proactor->head = NULL;
	proactor->isRunning = false;
	proactor->size = 0;

	fprintf(stderr, "%s Proactor created successfully\n", C_PREFIX_INFO);

	return proactor;
}

int runProactor(void *this) {
	if (this == NULL)
	{
		errno = EINVAL;
		fprintf(stderr, "%s runProactor() failed: %s\n", C_PREFIX_ERROR, strerror(EINVAL));
		return 1;
	}

	PProactor proactor = (PProactor)this;

	if (proactor->head == NULL)
	{
		fprintf(stderr, "%s Tried to start a proactor without registered file descriptors.\n", C_PREFIX_WARNING);
		return 1;
	}

	else if (proactor->isRunning)
	{
		fprintf(stderr, "%s Tried to start a proactor that's already running.\n", C_PREFIX_WARNING);
		return 1;
	}

	proactor->isRunning = true;

	if (pthread_create(&proactor->thread, NULL, proactorRunFunction, proactor) != 0)
	{
		fprintf(stderr, "%s runProactor() failed: %s\n", C_PREFIX_ERROR, strerror(errno));
		return 1;
	}

	return 0;
}

int cancelProactor(void *this) {
	if (this == NULL)
	{
		errno = EINVAL;
		fprintf(stderr, "%s cancelProactor() failed: %s\n", C_PREFIX_ERROR, strerror(EINVAL));
		return 1;
	}

	PProactor proactor = (PProactor)this;

	if (!proactor->isRunning)
	{
		fprintf(stderr, "%s Tried to stop a proactor that's not currently running.\n", C_PREFIX_WARNING);
		return 1;
	}

	fprintf(stdout, "%s Stopping proactor thread gracefully...\n", C_PREFIX_INFO);

	pthread_cancel(proactor->thread);
	pthread_join(proactor->thread, NULL);

	proactor->isRunning = false;

	return 0;
}

int addFD2Proactor(void *this, int fd, handler_t handler) {
	if (this == NULL)
	{
		errno = EINVAL;
		fprintf(stderr, "%s addFD2Proactor() failed: %s\n", C_PREFIX_ERROR, strerror(EINVAL));
		return 1;
	}

	PProactor proactor = (PProactor)this;

	PProactorNode node = (PProactorNode) malloc(sizeof(ProactorNode));

	if (node == NULL)
	{
		fprintf(stderr, "%s addFD2Proactor() failed: %s\n", C_PREFIX_ERROR, strerror(errno));
		return 1;
	}

	node->fd = fd;
	node->hdlr.handler = handler;
	node->next = NULL;
	
	if (proactor->head == NULL)
		proactor->head = node;

	else
	{
		PProactorNode curr = proactor->head;

		while (curr->next != NULL)
			curr = curr->next;

		curr->next = node;
	}

	proactor->size++;

	fprintf(stdout, "%s Successfuly added file descriptor %d to the list of proactor, function handler address: %p.\n", C_PREFIX_INFO, fd, node->hdlr.handler_ptr);

	return 0;
}

int removeHandler(void *this, int fd) {
	if (this == NULL)
	{
		errno = EINVAL;
		fprintf(stderr, "%s removeHandler() failed: %s\n", C_PREFIX_ERROR, strerror(EINVAL));
		return 1;
	}

	PProactor proactor = (PProactor)this;

	if (proactor->head == NULL)
	{
		fprintf(stderr, "%s removeHandler() failed: %s\n", C_PREFIX_ERROR, strerror(ENOENT));
		return 1;
	}

	PProactorNode curr = proactor->head;

	if (curr->fd == fd)
	{
		proactor->head = curr->next;
		free(curr);
		return 0;
	}

	while (curr->next != NULL)
	{
		if (curr->next->fd == fd)
		{
			PProactorNode tmp = curr->next;
			curr->next = curr->next->next;
			free(tmp);
			return 0;
		}

		curr = curr->next;
	}

	proactor->size--;

	fprintf(stderr, "%s removeHandler() failed: %s\n", C_PREFIX_ERROR, strerror(ENOENT));
	return 1;
}

int destroyProactor(void *this) {
	if (this == NULL)
	{
		errno = EINVAL;
		fprintf(stderr, "%s destroyProactor() failed: %s\n", C_PREFIX_ERROR, strerror(EINVAL));
		return 1;
	}

	PProactor proactor = (PProactor)this;

	if (proactor->head != NULL)
	{
		PProactorNode curr = proactor->head;

		while (curr != NULL)
		{
			PProactorNode tmp = curr;
			curr = curr->next;
			free(tmp);
		}
	}

	free(proactor);

	fprintf(stdout, "%s Successfuly destroyed proactor.\n", C_PREFIX_INFO);

	return 0;
}