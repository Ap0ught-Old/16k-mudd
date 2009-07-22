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
main.c:
Purpose of this file:  Starts the mud!  Holds the loop in which
the mud goes on forever.  Also calls the function to check for
socket connections
*/

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "stdh.h"

time_t current_time;
struct timeval last_time;
extern SOCKET_T *socket_list;
extern int host;

int main(int argc, char **argv) {
	int port = 4000;

	if( argc > 1 )
	{
		if( !is_number(argv[1]) )
		{
			log("Invalid port number!");
			exit(1);
		}
		else if( (port = atoi(argv[1])) <= 1024 )
		{
			log("Port must be above 1024!");
			exit(1);
		}
	}

	gettimeofday( &last_time, 0 );
	current_time = (time_t)last_time.tv_sec;
	create_host(port);

	while(1) {
		// mud loops once a second
		mudsleep(1);

		gettimeofday( &last_time, 0 );
		current_time = (time_t)last_time.tv_sec;

		// connect sockets, read and write to them, process commands...wow, that was easy!
		check_connections();
	}

	return 1;
}

