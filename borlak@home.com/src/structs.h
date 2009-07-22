/*****************************************************************************
* This file is part of a 16kbyte mud created by Michael "Borlak" Morrison,   * 
* as part of a contest held by Erwin S. Andreasen.                           *
*                                                                            *
* You may use this file, and the corresponding files that make up the mud,   *
* in any way you so desire.  The only requirements are to leave the headers, *
* such as this one, intact, unmodified, and at the beginning of the file.    *
* Also, it is not neccessary but I would appreciate it if you include this   * 
* header in any source or header files you add to the mud.                   *
*****************************************************************************/

/*
structs.h:
Purpose of this file:  All structure templates within the code
belong in this file.
*/

struct socket_t {
	SOCKET_T	*next;
	SOCKET_T	*prev;
	SOCKET_T	*reply;

	char		inbuf[MAX_BUFFER];
	char		outbuf[MAX_BUFFER];
	char		*name;
	char		*password;
	char		*host;

	int		desc;
	int		connected;
};
