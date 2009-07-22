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
util.c:
Purpose of this file: Utilities.  Anything that is not exactly
mud related, but help keep the mud going, add in here.  The 
Random Number Generator should go in here.  
*/

#include "stdh.h"
#include <time.h>
#include <ctype.h>
#include <sys/time.h>

// loops the mud for # of seconds.....
void mudsleep(int seconds)
{
	struct timeval wait_time;

	wait_time.tv_usec = 0;
	wait_time.tv_sec  = seconds;

	if(select(0, 0, 0, 0, &wait_time) < 0) {
		perror("mudsleep: select is not responding");
		exit(1);
	}
}

int is_number(char *str)
{
	int i;

	if( !ValidString(str) )
		return 0;

	for( i = 0; i < strlen(str); i++ )
	{
		if( !isdigit(str[i]) )
			return 0;
	}

	return 1;
}

