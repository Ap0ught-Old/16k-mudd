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
socket.c:
Purpose of this file:  Holds the socket code for the mud
which connects players and finds their addresses.         
*/

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <ctype.h>
#include "stdh.h"

// DECLARATIONS ////////////////////////////////////////
char *crypt(const char *key, const char *salt);
int check_name(char *argument);

// LOCAL VARAIBLES /////////////////////////////////////
SOCKET_T	*socket_list;
SOCKET_T	*socket_free;
int		host = 0;
fd_set		fd_read;
fd_set		fd_write;
fd_set		fd_exc;

char *welcome_message =
"\n\r\n\r16kbyte MiniMud\n\r\n\r"
"borlak@home.com\n\r\n\r";

char *leaving_message =
"\n\r\n\rGood-bye!\n\r";


// FUNCTIONS //////////////////////////////////////////

// initiate a socket
void init_socket(SOCKET_T *sock) {
	sock->inbuf[0]	= '\0';
	sock->outbuf[0]	= '\0';
	sock->connected	= CON_GET_NAME;
}

// remove a socket from the game and add it to the free list for 
// memory recycling
void free_socket(SOCKET_T *sock) {
	sock->connected = CON_EXIT;
	close(sock->desc);
	RemoveFromList(socket_list,sock);
	AddToList(socket_free,sock);
}

// write a string to a socket!
void send_to(SOCKET_T *sock, char *message) {
	if(!ValidString(message))
		return;

	if(strlen(message) + strlen(sock->outbuf) >= sizeof(sock->outbuf))
		return;

	strcat(sock->outbuf,message);
}

// find a sockets internet address or IP number.
// if you find your mud mysteriuosly lagging when someone tries to log on,
// remove the portion where gethostbyaddr looks up the characterized address.
// there is a known bug that will cause the mud to intermittently lag when
// trying to find a true address
void get_address(SOCKET_T *sock, struct sockaddr_in *sockad) {
	char buf[MAX_BUFFER];
	struct hostent *from;
	int addr;
	int size = sizeof(struct sockaddr_in);

	getpeername(sock->desc, (struct sockaddr*)sockad, &size);

	addr = ntohl(sockad->sin_addr.s_addr);
	sprintf( buf, "%d.%d.%d.%d",
		(addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
		(addr >> 8) & 0xFF, (addr) & 0xFF );
	from = gethostbyaddr((char*)&sockad->sin_addr,sizeof(sockad->sin_addr), AF_INET);

	if(from)
		str_dup(&sock->host,from->h_name);
	else
		str_dup(&sock->host,buf);

	sprintf(buf,"Socket connected from %s",sock->host);
	log(buf);
}

// the traditional "nanny" takes care of initial input from a socket,
// logs them in...
void nanny(SOCKET_T *sock, char *argument) {
	char buf[MAX_BUFFER];

	if(!ValidString(argument) && sock->connected != CON_MENU)
		return;

	switch(sock->connected)
	{
	case CON_CONNECTING:
		break;
	case CON_GET_NAME:
		argument[0] = Upper(argument[0]);

		if( !check_name(argument) ) {
			send_to(sock,"Illegal name, try another: ");
			break;
		}

		if( fread_player(sock,argument) ) {
			send_to(sock,"Welcome back, what is your password? ");
			sock->connected = CON_CHECK_PASSWORD;
			break;
		}
		str_dup(&sock->name, argument);
		sprintf(buf,"You want your name to be '%s'? ",argument);
		send_to(sock,buf);
		sock->connected = CON_CONFIRM_NAME;
		break;
	case CON_CONFIRM_NAME:
		switch(argument[0]) {
		case 'y': case 'Y':
			send_to(sock,"What password? ");
			sock->connected = CON_GET_PASSWORD;
			break;
		default:
			send_to(sock,"What is your name, then? ");
			sock->connected = CON_GET_NAME;
			break;
		}
		break;
	case CON_GET_PASSWORD:
		str_dup(&sock->password, crypt(argument,sock->name));

		send_to(sock,"Retype password: ");
		sock->connected = CON_CONFIRM_PASSWORD;
		break;
	case CON_CONFIRM_PASSWORD:
		if(strcmp(crypt(argument,sock->password), sock->password)) {
			send_to(sock, "Passwords don't match, try again: ");
			sock->connected = CON_GET_PASSWORD;
		}
		fwrite_player(sock);
		send_to(sock,"\n\rOkay, hit enter.");
		sock->connected = CON_MENU;
		break;
	case CON_CHECK_PASSWORD:
		if(strcmp(crypt(argument,sock->password), sock->password)) {
			free_socket(sock);
			return;
		}
		send_to(sock,"\n\rOkay, hit enter.");
		sock->connected = CON_MENU;
		break;
	case CON_MENU:
		switch(argument[0]) {
		case '1':
			send_to(sock,"\n\r\n\rWelcome to 16kbyte MiniMud!\n\r");
			send_to(sock,"Just start typing to talk to the people in the game.\n\r");
			send_to(sock,"----------------------------------------------------\n\r");
			send_to(sock,"Commands: /who, /emote, /tell, /reply, /quit\n\r");
			send_to(sock,"----------------------------------------------------\n\r");
			sock->connected = CON_CHATROOM;
			break;
		case '2':
			send_to(sock,leaving_message);
			sock->connected = CON_EXIT;
			break;
		default:
		sprintf(buf,			"\n\r\n\r-= Welcome to 16kbyte MiniMud =-\n\r");
		sprintf(buf+strlen(buf),	"1) Enter the chatroom.\n\r");
		sprintf(buf+strlen(buf),	"2) Quit\n\r");
		sprintf(buf+strlen(buf),	"\n\r:: ");
		send_to(sock,buf);
		break;
		}
		break;
	case CON_CHATROOM:
		chatroom(sock,argument);
		break;
	default:
		break;
	}
}

// create the listening socket for people to connect to
int create_host(int port) {
	char buf[256];
	struct sockaddr_in	sa;

	if((host = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("create_host: failed to create host socket");
		exit(1);
	}

	sa.sin_family		= AF_INET;
	sa.sin_port		= htons(port);
	sa.sin_addr.s_addr	= htonl(INADDR_ANY);

	if(bind(host, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
		perror("create_host: bind failed");
		exit(1);
	}

	if(listen(host, 5) < 0) {
		perror("create_host: listen failed");
		exit(1);
	}

	sprintf(buf,"Host socket listening on port %d, file descriptor %d.\n",port,host);
	log(buf);
	return 1;
}

// connect a new socket!  note the use of NewObject
int new_socket(void) {
	SOCKET_T	*sock = 0;
	int		desc = sizeof(struct sockaddr_in);
	unsigned int	len;
	struct sockaddr_in sockad;

	getsockname(host, (struct sockaddr*)&sockad, &desc);

	if((desc = accept(host, &sockad, &len)) < 0)
		return 0;

	if(fcntl(desc, F_SETFL, FNDELAY) == -1)
		return 0;

	NewObject(socket_free,sock);
	init_socket(sock);
	sock->desc	= desc;
	AddToList(socket_list,sock);

	send_to(sock, welcome_message);
	send_to(sock, "What is your name? ");

	get_address(sock,&sockad);
	return 1;
}

// this beauty connects new sockets and calls the functions to process all commands,
// as well as kick out sockets with errors or that are quitting the game.
int check_connections(void) {
	SOCKET_T	*sock;
	SOCKET_T	*sock_next;
	int 		high = host;

	FD_ZERO(&fd_read);
	FD_ZERO(&fd_write);
	FD_ZERO(&fd_exc);
	FD_SET(host, &fd_read);

	for(sock = socket_list; sock; sock = sock->next) {
		if(sock->desc > high)
			high = sock->desc;

		FD_SET(sock->desc, &fd_read);
		FD_SET(sock->desc, &fd_write);
		FD_SET(sock->desc, &fd_exc);
	}

	if(select(high+1, &fd_read, &fd_write, &fd_exc, 0) < 0)
		return 0;

	if(FD_ISSET(host, &fd_read))
		new_socket();

	for(sock = socket_list; sock; sock = sock_next) {
		sock_next = sock->next;

		if(FD_ISSET(sock->desc, &fd_exc)) {
			free_socket(sock);
			continue;
		}

		if(FD_ISSET(sock->desc, &fd_read)) {
			if(!process_input(sock))
				free_socket(sock);
		}

		if(FD_ISSET(sock->desc, &fd_write)) {
			if(!process_output(sock))
				free_socket(sock);
		}

		if(sock->connected == CON_EXIT) {
			free_socket(sock);
			continue;
		}
	}
	return 1;
}

// write data to a socket
int process_output(SOCKET_T *sock) {
	int i = 0;
	unsigned int total = 0;
	int block;

	if(sock->outbuf[0] == '\0')
		return 1;

	while(total < strlen(sock->outbuf))
	{
		block = strlen(sock->outbuf) - total;

		if((i = write(sock->desc, &sock->outbuf[total], block)) < 0)
			return 0;

		total += i;
	}

	sock->outbuf[0] = '\0';
	return 1;
}

// read data from a socket
int process_input(SOCKET_T *sock) {
	unsigned int len = 0;
	unsigned int result;

	for(;;) {
		len += result = read(sock->desc, sock->inbuf + len, sizeof(sock->inbuf) - len - 10);

		if(sock->inbuf[len-1] == '\n' || sock->inbuf[len-1] == '\r') {
			sock->inbuf[len-1] = '\0';

			for( result = 0; result < strlen(sock->inbuf); result++ ) {
				if(sock->inbuf[result] == '\n' || sock->inbuf[result] == '\r')
					sock->inbuf[result] = '\0';
			}
			break;
		}
		else if(result <= 0) {
			return 0;
		}
	}

	nanny(sock,sock->inbuf);
	sock->inbuf[0] = '\0';

	return 1;
}

// make sure a name is legal for the mud!
int check_name(char *argument)
{
	int i = 0;

	if(!ValidString(argument))
		return 0;

	if( strlen(argument) > 12 )
		return 0;

	for( i = 0; i < strlen(argument); i++ ) {
		if( !isalpha(argument[i]) )
			return 0;
	}

	return 1;
}

