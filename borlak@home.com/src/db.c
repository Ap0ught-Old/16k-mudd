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
db.c:
Purpose of this file: Database management and also memory
management.  Create the functions to load up the database, 
manipulate it, etc.
*/

#include <string.h>
#include <time.h>
#include "stdh.h"

extern time_t current_time;

// game logging to std error..
void log( const char *str ) {
	char *logstr;

	logstr				= ctime( &current_time );
	logstr[strlen(logstr)-1]	= '\0';	
	fprintf( stderr, "%s :: %s\n", logstr, str );
}

// this allocates memory for a string... it first checks if the location that 
// we are writing to has memory on it already, if it does, it frees it, then
// writes the string to it.  this is to keep memory from building up over time
// accidentally.  no worries!  the <EOS> is overwritten with !!!!! because this
// signifies END OF STRING for pfiles... see more in io.c
void str_dup(char **destination, char *str) {
	if(!ValidString(str)) {
		perror("str_dup: tried to allocate 0 byte string");
		exit(1);
	}

	if(strstr(str,"<EOS>") != 0)
		strcpy(str,"!!!!!");

	if(ValidString(*destination))
		free(*destination);

	*destination = malloc(sizeof(char) * (strlen(str) + 1));
	strcpy(*destination,str);
}
