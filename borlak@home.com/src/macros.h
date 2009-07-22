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
macros.h:
Purpose of this file:  Macros!  What else can I say? :)
*/

// DATA TYPE HANDLING MACROS ////////////////////////////
#define ValidString(str)	((str) && (str[0]) != '\0')
#define Upper(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))


// LINKED LIST MACROS ///////////////////////////////////
#define NewObject(freelist,obj)	if( freelist == 0 ) {			\
					obj = malloc(sizeof(*obj));	\
					memset(obj,0,sizeof(*obj));	\
				}					\
				else {					\
					obj = freelist;			\
					freelist = freelist->next;	\
					obj->next = NULL;		\
				}

#define AddToList(list,obj)	if( list != 0 ) {		\
					list->prev = obj;	\
					obj->next = list;	\
					list = obj;		\
				}				\
				else {				\
					list = obj;		\
				}

#define RemoveFromList(list,obj)					\
				if( obj->prev == 0 ) {			\
					if( obj->next == 0 ) { 		\
						list = 0;		\
					}				\
					else {				\
						list = obj->next;	\
						list->prev = 0;		\
						obj->next = 0;		\
					}				\
				}					\
				else {					\
					obj->prev->next = obj->next;	\
					obj->next = obj->prev = 0;	\
				}


