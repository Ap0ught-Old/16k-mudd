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
comm.c:
Purpose of this file:  Communications throughout the mud.  Use
this file for chat rooms, chat talk, tells, etc.
*/

#include <string.h>
#include "stdh.h"

extern SOCKET_T *socket_list;


// the game chat room
void chatroom(SOCKET_T *sock, char *argument) {
	char buf[MAX_BUFFER];
	char arg[MAX_BUFFER];
	SOCKET_T *slist;

	if(argument[0] == '/') {
		argument = strip_argument(argument,arg);

		if(!strcasecmp(arg+1,"who")) {
			buf[0] = '\0';

			for(slist = socket_list; slist; slist = slist->next) {
				sprintf(buf+strlen(buf),"%-12s\n\r", slist->name );
			}
			send_to(sock,buf);
			return;
		}
		if(!strcasecmp(arg+1,"quit")) {
			send_to(sock,"Okay, press enter.\n\r");
			sock->connected = CON_MENU;
			return;
		}
		if(!strcasecmp(arg+1,"emote")) {
			sprintf(buf,"@%s %s\n\r",sock->name,argument);

			for(slist = socket_list; slist; slist = slist->next) {
				if(slist->connected == CON_CHATROOM)
					send_to(slist,buf);
			}
			return;
		}
		if(!strcasecmp(arg+1,"tell")) {
			argument = strip_argument(argument,arg);

			sprintf(buf,"You tell %s> %s\n\r",sock->name,argument);
			send_to(sock,buf);

			sprintf(buf,"%s tells you> %s\n\r",sock->name,argument);

			for(slist = socket_list; slist; slist = slist->next) {
				if(slist->connected == CON_CHATROOM && !strcasecmp(slist->name,arg) ) {
					send_to(slist,buf);
					slist->reply = sock;
					break;
				}
			}
			return;
		}
		if(!strcasecmp(arg+1,"reply")) {
			if( !sock->reply || sock->reply->connected != CON_CHATROOM ) {
				send_to(sock,"They aren't here.\n\r");
				return;
			}
			sprintf(buf,"You reply to %s> %s\n\r",sock->reply->name,argument);
			send_to(sock,buf);
			sprintf(buf,"%s replies> %s\n\r",sock->name,argument);
			send_to(sock->reply,buf);
			return;
		}

		send_to(sock,"Invalid command.\n\r");
		return;
	}

	sprintf(buf,"%s> %s\n\r",sock->name,argument);

	for(slist = socket_list; slist; slist = slist->next) {
		if(slist->connected == CON_CHATROOM)
			send_to(slist,buf);
	}
	return;
}

