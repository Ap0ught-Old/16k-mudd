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
io.c:
Purpose of this file: Input/Output.  Use this file for reading
and writing with files.  
*/

#include <string.h>
#include <ctype.h>
#include "stdh.h"


// this function strips the first "argument".. which can be in single
// or double quotes, and puts it in arg_first... then it returns a
// string with all but the argument it stripped
char *strip_argument(char *argument, char *arg_first) {
	char deli;

	while( isspace(*argument) )
		argument++;

	deli = ' ';
	if( *argument == '\'' || *argument == '"' )
		deli = *argument++;

	while( *argument != '\0' ) {
		if(*argument == deli) {
			argument++;
			break;
		}
		*arg_first = *argument;
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while(isspace(*argument))
		argument++;

	return argument;
}


// read a number from a file...negative numbers work
int fread_number(FILE *fp) {
	int number=0;
	int sign=0;
	char c;

	do {
		c = getc(fp);
	} while(isspace(c));

	switch(c)
	{
	case '+':
		c = getc(fp);
		break;
	case '-':
		sign = 1;
		c = getc(fp);
		break;
	default: break;
	}

	if(!isdigit(c)) {
		log("fread_number: not a number!");
		exit(1);
	}

	while(isdigit(c)) {
		number = number*10+c-'0';
		c      = getc(fp);
	}

	if(sign)
		number = 0 - number;

	return number;
}


// read a word from a file, which can be in single or double quotes,
// or just a single word...
char *fread_word(FILE *fp) {
	static char word[MAX_BUFFER];
	char *pword;
	char deli;

	do {
		deli = getc(fp);
	} while(isspace(deli));

	if(deli == '\'' || deli == '"') {
		pword = word;
	}
	else {
		word[0] = deli;
		pword = word + 1;
		deli = ' ';
	}

	for(; pword < word + MAX_BUFFER; pword++) {
		*pword = getc(fp);
		if(deli == ' ' ? isspace(*pword) : *pword == deli) {
			if( deli == ' ' )
				ungetc(*pword, fp);
			*pword = '\0';
			return word;
		}
	}

	log("fread_word: word too long.");
	exit(1);
	return 0;
}


// this keeps reading all characters, numbers and letters and anything,
// until it finds <EOS> which signifies END OF STRING.. in the game, no
// information that gives saved should have <EOS> in it, but this is
// handled in the str_dup function.
char *fread_string(FILE *fp) {
	static char string[MAX_BUFFER];
	char k;
	int i;

	do {
		k = getc(fp);
	} while(isspace(k));

	string[0] = k;
	string[1] = '\0';

	for( i = 1; i < MAX_BUFFER-2; i++ ) {
		string[i] = getc(fp);
		string[i+1] = '\0';

		if( string[i] == EOF )
			break;

		if(i > 5 && !strcmp(&string[i-4],"<EOS>")) {
			string[i-4] = '\0';
			return string;
		}

	}

	log("fread_string: string too long or no <EOS> found.");
	exit(1);
	return 0;
}

// write a string to a pfile... just supply its tag, the string, and the file.
void fwrite_string(char *tag, char *str, FILE *fp) {
	if(strlen(tag) >= 16) {
		log("fwrite_string: tag names are limited to 15 characters");
		exit(1);
	}
	fprintf(fp,"%s%c%s%s<EOS>\n",tag,strlen(tag) < 8 ? '\t' : ' ', strlen(tag) < 9 ? "\t" : "", str );
}

// write a number to a pfile... just supply its tag, the number, and the file.
void fwrite_number(char *tag, int number, FILE *fp) {
	if(strlen(tag) >= 16) {
		log("fwrite_number: tag names are limited to 15 characters");
		exit(1);
	}
	fprintf(fp,"%s%c%s%d\n",tag,strlen(tag) < 8 ? '\t' : ' ', strlen(tag) < 9 ? "\t" : "", number );
}


// write the player!
void fwrite_player(SOCKET_T *sock) {
	FILE *fp;
	char buf[256];

       	sprintf(buf,"../player/%s",sock->name);

	if((fp = fopen(buf,"w")) == 0) {
		perror("fwrite_player: could not open player_file for writing!");
		exit(1);
	}

	fwrite_string("Name",sock->name,fp);
	fwrite_string("Password",sock->password,fp);
	fprintf(fp,"\n\rEnd\n\r\n\r");
	fclose(fp);
}


// read the player!
int fread_player(SOCKET_T *sock, char *name) {
	FILE *fp;
	char buf[256];
	char *word;
	int end = 0;

	sprintf(buf,"../player/%s",name);

	if((fp = fopen(buf,"r")) == 0)
		return 0;

	while(!end) {
		word = fread_word(fp);

		switch(Upper(word[0]))
		{
		case 'E':
			if(!strcasecmp(word,"End"))
				end = 1;
			break;
		case 'N': 
			if(!strcasecmp(word,"Name"))
				str_dup(&sock->name,fread_string(fp));
			break;
		case 'P': 
			if(!strcasecmp(word,"Password"))
				str_dup(&sock->password,fread_string(fp));
			break;
		default:
			break;
		}
	}
	fclose(fp);
	return 1;
}
