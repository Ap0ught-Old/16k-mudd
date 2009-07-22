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
stdh.h:
Purpose of this file:  Standard Header File.  This includes
all the other header files, and system header files that are
used often.  It also contains global functions for easy finding.
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>
#include "macros.h"
#include "typedefs.h"
#include "variables.h"
#include "structs.h"

void chatroom		(SOCKET_T *sock, char *argument);

void log		(const char *str);
void str_dup		(char **destination, char *str);

char *strip_argument	(char *argument, char *arg_first);
int fread_number	(FILE *fp);
char *fread_word	(FILE *fp);
char *fread_string	(FILE *fp);
void fwrite_string	(char *tag, char *str, FILE *fp);
void fwrite_number	(char *tag, int number, FILE *fp);
void fwrite_player	(SOCKET_T *sock);
int fread_player	(SOCKET_T *sock, char *name);

void send_to	(SOCKET_T *sock, char *argument);
void free_socket	(SOCKET_T *sock);
void nanny		(SOCKET_T *sock, char *argument);
int create_host		(int port);
int new_socket		(void);
int check_connections	(void);
int process_input	(SOCKET_T *sock);
int process_output	(SOCKET_T *sock);

void mudsleep		(int sleep);
int is_number		(char *str);
