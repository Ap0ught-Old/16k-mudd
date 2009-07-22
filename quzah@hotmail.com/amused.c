/*-------------------------------------------------------------------------*\
]|  Quzah's Poorly Orchestrated Small MUD. Enjoy the acronym. ;)           |[
\*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*\
]|  Filename:   'amused.c'                                                 |[
]|  Version:    000.16k                                                    |[
]|  Author:     Jeremiah Myers                                             |[
]|  Created:    April.2000                                                 |[
]|  Copyright:  This file is copyright (c) 2000, Jeremiah Myers.           |[
]|  Email:      quzah@quzah.com                                            |[
]|                                                                         |[
]|  Description:                                                           |[
]|      This is the main .c file (Because it contains the 'main' function. |[
]|      Pretty clever how that works, hun?) for qPOSmud. I've renamed this |[
]|      about as many times as I've rewritten it. I'll settle on this for  |[
]|      now. Enjoy the comment blocks; they're patent-pending ;)           |[
]|                                                                         |[
]|  Comments:                                                              |[
]|      I started this entirely wrong. Instead of designing for a working  |[
]|      model, I started a compact model which I spent forever and a day   |[
]|      trying to get working. It works and it's ugly as sin, and it was a |[
]|      major head ache. (I think this would have been way better in C++.  |[
]|      Less redundancy in all of the list creation/destruction functions. |[
]|      I could have done a one-function-fits-all approach for them. Too   |[
]|      bad I don't know enough C++ do do sockets right now... I guess I   |[
]|      could have unionized everything, but that's a pain in the ass.)    |[
]|                                                                         |[
]|      This mud is extremely basic, yet it has the tools (most of them)   |[
]|      needed to be a decent server I suppose. I spent far too much time  |[
]|      rewriting this, so rather than abandon the contest, I decided to   |[
]|      just toss out what I had. Thus, you get a P.O.S. MUD. ;)           |[
]|                                                                         |[
]|  Special thanks:                                                        |[
]|      Extended thanks to Cynbe ru Taren for the many answers to all of   |[
]|      my questions many months ago. Without such help, I probably would  |[
]|      have never written my first set of sockets. Thanks again.          |[
]|                                                                         |[
]|  License:                                                               |[
]|      This is mine. I wrote it. You can use it. Give me credit. The end. |[
]|                                                                         |[
\*-------------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
/*-------------------------------------------------------------------------*\
]|  Macros...             ...because a mud just isn't a mud with out them. |[
\*-------------------------------------------------------------------------*/
#define A case/* savings on 5th call */
#define B break/* savings on 4th call */
#define C char/* savings on 5th call */
#define D(v) oodoo ## v
#define E else/* savings on 4th call */
#define F double/* savings on 4th call */
#define I int/* savings on 7th call */
#define M malloc/* savings on 4th call */
#define N '\0'/* savings on 4th call */
#define R return/* savings on 4th call */
#define T struct t/* text */
#define V void	/* savings on 5th call */
#define X(t) if((t)<0)exit(0)
#define Z struct zock
I dam=0;/*dam the flow of mud*/
/*-------------------------------------------------------------------------*\
]|  Enumeration                                                            |[
]|                                                                         |[
]|      NL is the name length. I use 20 characters, excluding the NULL.    |[
]|      C enumerations are combat style enumerations. Used for fighting.   |[
]|      R enumerations are 'rip' enumerations. Used for ripping text.      |[
]|      S enumerations are state enumerations. Used for Zocket states.     |[
\*-------------------------------------------------------------------------*/
enum{NL=20, RL=0,RW,RN,RS, SNew=0,SGetID,SPlay,SClose};
/*-------------------------------------------------------------------------*\
]|  Z structure                                                            |[
]|                                                                         |[
]|      The Z structure houses all of the socket information. The details  |[
]|      are as follows:                                                    |[
]|                                                                         |[
]|      p = the previous Z instance in the 'allZ' list.                    |[
]|      n = the next Z instance in the 'allZ' list.                        |[
]|      zo = the number representing the actual socket number this Z uses. |[
]|      id = this is used to identify the S instance which uses this S.    |[
]|      st = the state of this Z instance. (SNew,SGetID,SPlay...)          |[
]|      as = contains the address size for this socket.                    |[
]|      inT = a list of unprocessed input.                                 |[
]|      ouT = a list of unsent output.                                     |[
]|      ad = the address structure for this socket.                        |[
\*-------------------------------------------------------------------------*/
Z{Z*p,*n;I zo,id,st,as;T*inT,*ouT;struct sockaddr_in ad;}*allZ=N;
/*-------------------------------------------------------------------------*\
]|  T structure                                                            |[
]|                                                                         |[
]|      There's a mutch better way to do this. That would be to include a  |[
]|      top/end pointer pair inside this funciton. This would eliminate    |[
]|      said pairs in all of the other structures/uses and would clean all |[
]|      of this up a great deal. Since it's the 26th though, I'll skip it  |[
]|      for now. I'll fix it after the contest in an update. (Or if I get  |[
]|      the urge to rewrite this yet again in the next three days!)        |[
]|                                                                         |[
]|      p = the previous T instance in the list.                           |[
]|      n = the next T instance in the list.                               |[
]|      text = the actual text string for this T instance.                 |[
]|                                                                         |[
\*-------------------------------------------------------------------------*/
T{T*p,*n;C*text;};
/*-------------------------------------------------------------------------*\
]|  String Functions                                                       |[
\*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*\
]|  Skip Preceeding White Space                                            |[
]|                                                                         |[
]|      This doesn't strip the whitespace. It leave the original string    |[
]|      untouched. In fact, all it does is return a pointer further down   |[
]|      the string to where the real text begins. (Naturally, this only    |[
]|      happens if there is whitespace. If there is no whitespace then it  |[
]|      will return a pointer to the start of the string, and if there is  |[
]|      only white space it will return a NULL.                            |[
\*-------------------------------------------------------------------------*/
C*spws(C*s){C*c;if(s==N)R s;c=s;while(*c&&isspace((int)*c))c++;R c;}
/*-------------------------------------------------------------------------*\
]|  String Insensitive Comaprison                                          |[
]|                                                                         |[
]|      Since there is no ANSI strcmpi, I wrote my own. Returns -1 if the  |[
]|      first string comes before the second, returns 1 if the second one  |[
]|      comes before the first, and returns 0 if they are the same. Damn   |[
]|      ANSI anyway, this is cutting into my 16k!                          |[
\*-------------------------------------------------------------------------*/
I strIcmp(C*s1,C*s2){/*
	*/I x,y;/*
	*/while(*s1&&*s2){/*
		*/x=tolower(*s1++);/*
		*/y=tolower(*s2++);/*
		*/if(x<y)R-1;/*
		*/if(x>y)R 1;/*
	*/}/*
	*/R (*s2?-1:*s1?1:0);/*
*/}
/*-------------------------------------------------------------------------*\
]|  String Prefix Comparison                                               |[
]|                                                                         |[
]|      Is the first string a prefix of the second string? We return 0 for |[
]|      yes, and -1 for no. Again, we ignore case. There goes more of my   |[
]|      sixteen K.                                                         |[
\*-------------------------------------------------------------------------*/
I strPcmp(C*s1,C*s2){/*
	*/if(!*s1||!*s2||strlen(s1)>strlen(s2))R-1;/*
	*/while(*s1)/*
		*/if(tolower((I)*s1++)!=tolower((I)*s2++))R-1;/*
	*/R 0;/*
*/}
/*-------------------------------------------------------------------------*\
]|  T instance functions                                                   |[
\*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*\
]|  cnewT                                                                  |[
]|                                                                         |[
]|      Allocate a new T instance, malloc and fill the text field. Return  |[
]|      a pointer to the T instance.                                       |[
\*-------------------------------------------------------------------------*/
T*cnewT(C*s){/*
	*/T*t;/*
	*/t=(T*)M(sizeof(T));/*
	*/if(s&&*s){/*
		*/t->text=s==N?N:M(strlen(s)+1);/*
		*/memset(t->text,N,strlen(s)+1);/*
		*/strcpy(t->text,s);/*
	*/} E t->text=N;/*
	*/t->n=t->p=N;/*
	*/R t;/*
*/}
/*-------------------------------------------------------------------------*\
]|  freeT                                                                  |[
]|                                                                         |[
]|      Recursivly destroy a T list. If you want a single T destroyed, you |[
]|      had better make damn sure you cut it free first.                   |[
\*-------------------------------------------------------------------------*/
V freeT(T*t){/*
	*/if(t==N)R;/*
	*/if(t->n)/*
		*/freeT(t->n);/*
	*/if(t->text){/*
		*/memset(t->text,N,strlen(t->text));/*
		*/free(t->text);/*
	*/}/*
	*/free(t);/*
*/}
/*-------------------------------------------------------------------------*\
]|  tfF                                                                    |[
]|                                                                         |[
]|      Text from a File. Given a file pointer, fetch data from it based   |[
]|      on r type. R is defined as follows:                                |[
]|          r==RW: strip one word from the instance t and return it.       |[
]|          r==RL: rip a line (ending with \n) from the instance t.        |[
]|          r==RN: rip a series of digits from the t instance.             |[
]|          r==RS: rip a string (ending with ~) from the instance t.       |[
]|                                                                         |[
]|      All ripped data is returned in its own T instance. God I love the  |[
]|      simplicity of this.                                                |[
\*-------------------------------------------------------------------------*/
T*tfF(FILE*f,I r){/*
	*/C b[1024]={0};/*
	*/I x=0;/*

	*/if(!f)R N;/*

	*//* skip preceeding whitespace *//*
	*/do x=fgetc(f); while(!feof(f)&&isspace(x));/*
	*/if(feof(f))R N;/*
	*//* rip a word, line, number, or string *//*
	*/do b[x++]=fgetc(f); while(!feof(f)&&x<1023&&r==RW?!isspace((I)b[x]):r==RL/*
	*/?b[x]!='\n':r==RN?isalpha((I)b[x]):b[x]!='~');/*
	*/b[x]=N;/*

	*//* stick our text into a new T instance *//*
	*/R cnewT(b);/*
*/}
/*-------------------------------------------------------------------------*\
]|  Zocket Functions                                                       |[
\*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*\
]|  Zin                                                                    |[
]|                                                                         |[
]|      Return the top T instance from Z instance's input list.            |[
\*-------------------------------------------------------------------------*/
T*Zin(Z*z){/*
	*/T*t=N;/*
	*/if(z->inT){/*
		*/t=z->inT;/*
		*/z->inT=z->inT->n;/*
		*/if(z->inT)/*
			*/z->inT->p=N;/*
		*/t->n=N;/*
	*/}/*
	*/R t;/*
*/}
/*-------------------------------------------------------------------------*\
]|  Zout                                                                   |[
]|                                                                         |[
]|      Put the string S into a T instance and stick it on the end of the  |[
]|      Z instance's output list.                                          |[
\*-------------------------------------------------------------------------*/
V Zout(C*s,Z*z){/*
	*/T*t,*u;/*

	*/if(s==N||z==N)R;/*
	*/t=cnewT(s);/*

	*/if(z->ouT==N){/*
		*/z->ouT=t;/*
		*/t->p=t->n=N;/*
	*/}/*
	*/E {/*
		*/for(u=z->ouT;u->n;u=u->n);/*
		*/u->n=t;/*
		*/t->p=u;/*
		*/t->n=N;/*
	*/}/*
*/}
/*-------------------------------------------------------------------------*\
]|  linkZ                                                                  |[
]|                                                                         |[
]|      Link a Z connection to the allZ list. (I may omit this entirely.)  |[
\*-------------------------------------------------------------------------*/
V linkZ(Z*z){z->n=allZ;if(allZ)allZ->p=z;z->p=N;allZ=z;}
/*-------------------------------------------------------------------------*\
]|  pullZ                                                                  |[
]|                                                                         |[
]|      Pull a Z connection from the allZ list. (IE: Unlink it.)           |[
\*-------------------------------------------------------------------------*/
V pullZ(Z*z){/*
	*/if(z->p)z->p->n=z->n;/*
	*/if(z->n)z->n->p=z->p;/*
	*/if(z==allZ)allZ=z->n;/*
	*/z->p=z->n=N;/*
*/}
/*-------------------------------------------------------------------------*\
]|  newZ                                                                   |[
]|                                                                         |[
]|      Very simple stuff. Allocate a Z instance, set zo to -1 and return  |[
]|      the new instance. Normally I'd simply pull it off a list of unused |[
]|      Z instances, but for the sake of size, I'll just malloc and free.  |[
]|                                                                         |[
]|      On that note, I removed freeZ() for the same reason. freeZ() would |[
]|      normally just unlink the Z instance and dump it on the free list,  |[
]|      instead it calls close() on it, pulls it, and frees it. Shrug.     |[
\*-------------------------------------------------------------------------*/
Z*newZ(V){/*
	*/Z*z=M(sizeof(Z));/*
	*/z->p=z->n=N;/*
	*/z->zo=-1;/*
	*/z->id=0;/*
	*/z->st=SNew;/*
	*/z->inT=z->ouT=N;/*
	*/R z;/*
*/}
/*-------------------------------------------------------------------------*\
]|  freeZ                                                                  |[
]|                                                                         |[
]|      Given a Z instance, close it, pull it, and free it.                |[
\*-------------------------------------------------------------------------*/
I freeZ(Z*z){/*
	*/I close(I fd);/*
	*/close(z->zo);/*
	*/pullZ(z);/*
	*/freeT(z->inT);/*
	*/freeT(z->ouT);/*
	*/free(z);/*
	*/R-1;/*
*/}
/*-------------------------------------------------------------------------*\
]|  readZ                                                                  |[
]|                                                                         |[
]|      Read pending input from a socket and stick it on the end of the Z  |[
]|      instance's iEndT list.                                             |[
\*-------------------------------------------------------------------------*/
I readZ(Z*z){/*
	*/static C y[1024],*w=N;/*
	*/I x=-1;/*
	*/T*t,*u;/*
	*/I read(I fd,C*buf,I nbyte);/*
	*/if(z->zo<0)R-1;/*
	*/x=read(z->zo, y, 1023);/*
	*/if(x<1)R freeZ(z);/* freeZ returns -1 always *//*
	*/w=spws(y);/*
	*/if(*w){/*
		*/t=cnewT(w);/*
		*//* If there are no entries. *//*
		*/if(z->inT==N)/*
			*/z->inT=t;/*
		*/E{/*
			*/for(u=z->inT;u->n;u=u->n);/*
			*/u->n=t;/*
			*/t->p=u;/*
		*/}/*
	*/}/*
	*/memset(y,N,1024);/*
	*/R x;/*
*/}
/*-------------------------------------------------------------------------*\
]|  writeZ                                                                 |[
]|                                                                         |[
]|      Write pending output in a Z instance to its socket. You get one    |[
]|      shot here. If you can't take what we're writing, tough shit.       |[
\*-------------------------------------------------------------------------*/
I writeZ(Z*z){/*
	*/T*t;/*
	*/I write(I fd,C*buf,I nbyte);/*

	*/if(z->zo<0)R freeZ(z);/*

	*/for(t=z->ouT;t;t=t->n){/*
		*/if(t->text)/*
			*/if(write(z->zo,t->text,strlen(t->text))<0){/*
				*/freeT(t);/*
				*/R freeZ(z);/*
			*/}/*
	*/}/*
	*/freeT(t);/* recursively trash the output *//*
	*/z->ouT=N;/*
	*/R 0;/*
*/}
/*-------------------------------------------------------------------------*\
]|  initZ                                                                  |[
]|                                                                         |[
]|      Given a port number, bind a the first usable socket to the port,   |[
]|      set it non-blocking, and make it listen.                           |[
\*-------------------------------------------------------------------------*/
I initZ(I p){/* return the first socket *//*
	*/struct sockaddr_in z;/*
	*/I s=-1,t=-1;/*
	*/X(s=socket(AF_INET,SOCK_STREAM,0));/*
	*/X(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char*)&t,sizeof(t)));/*
	*/z.sin_family=AF_INET;/*
	*/z.sin_addr.s_addr=htonl(INADDR_ANY);/*
	*/z.sin_port=htons(p);/*
	*/X(bind(s,(struct sockaddr*)&z,sizeof(z)));/*
	*/X(fcntl(s,F_SETFL,FNDELAY));/*
	*/X(listen(s,SOMAXCONN));/*
	*/R s;/*
*/}
/*-------------------------------------------------------------------------*\
]|  listenZ                                                                |[
]|                                                                         |[
]|      Given the first usable socket, listen on this socket for incomming |[
]|      connections. If some one is waiting, accept them and stick them in |[
]|      the allZ list.                                                     |[
\*-------------------------------------------------------------------------*/
I listenZ(I oZ){/* pass it the first socket *//*
	*/Z*z=N;/*
	*/struct timeval t;/*
	*/fd_set r,w,e;/*
	*/if(oZ<0)R-1;/*
	*/t.tv_sec=0;/*
	*/t.tv_usec=0;/*
	*/FD_ZERO(&r);/*
	*/FD_ZERO(&w);/*
	*/FD_ZERO(&e);/*
	*/FD_SET(oZ,&r);/*
	*/FD_SET(oZ,&w);/*
	*/FD_SET(oZ,&e);/*
	*/X(select(oZ+1,&r,&w,&e,&t));/*
	*/if( FD_ISSET(oZ,&r) || FD_ISSET(oZ,&w) ){/*
		*/if((z=newZ())==N)/*
			*/exit(0);/*
		*/z->as = sizeof( struct sockaddr_in );/*
		*/z->zo = accept( oZ, (struct sockaddr*) &z->ad, &z->as );/*
		*/X(z->zo);/*
		*/X(fcntl(z->zo, F_SETFL, FNDELAY));/*
		*/linkZ(z);/*
		*/R z->zo;/*
    */}/*
    */R 0;/* nothing eventful happened *//*
*/}
#if NAPZ
/*-------------------------------------------------------------------------*\
]|  napZ                                                                   |[
]|                                                                         |[
]|      Take a nap. Given the first socket, our listening socket, take a   |[
]|      rest for some-odd milliseconds.                                    |[
\*-------------------------------------------------------------------------*/
V napZ(I oZ,I u){/*
	*/struct timeval t;/*
	*/X(oZ);/*
	*/t.tv_sec=0;/*
	*/t.tv_usec=u<0?125000:u;/*
	*/X(select(oZ+1,N,N,N,&t));/*
*/}
#endif
/*-------------------------------------------------------------------------*\
]|  VooDoo Functions                                                       |[
\*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*\
]|  Dam                                                                    |[
]|                                                                         |[
]|      Shutdown the mud.                                                  |[
\*-------------------------------------------------------------------------*/
I D(Dam)(C*s,Z*z){/*
	*/Zout("Damming the mud.\n",z);/*
	*/dam=1;/*
	*/R s?SPlay:z?SPlay:SPlay;/*
*/}
/*-------------------------------------------------------------------------*\
]|  Say                                                                    |[
]|                                                                         |[
]|      Echo output to every Z instance in the same location.              |[
\*-------------------------------------------------------------------------*/
I D(Say)(C*s,Z*z){/*
	*/C buf[1024]={0};/*
	*/Z*y;/*
	*/sprintf(buf,"Zocket %d says \"%s\".\n",z->zo,s);/*
	*/for(y=allZ;y;y=y->n)/*
		*/if(y->zo!=z->zo)Zout(buf,y);/*
	*/sprintf(buf,"You say \"%s\".\n",s);/*
	*/Zout(buf,z);/*
	*/R SPlay;/*
*/}
/*-------------------------------------------------------------------------*\
]|  Quit                                                                   |[
]|                                                                         |[
]|      Quit from the mud.                                                 |[
\*-------------------------------------------------------------------------*/
I D(Quit)(C*s,Z*z){/*
	*/Zout("Goodbye.\n",z);/*
	*/R s?SClose:SClose;/*simply used to avoid compiler warnings*//*
*/}
/*-------------------------------------------------------------------------*\
]|  Who                                                                    |[
]|                                                                         |[
]|      Who's here?                                                        |[
\*-------------------------------------------------------------------------*/
I D(Who)(C*s,Z*z){/*
	*/C buf[1024]={0};/*
	*/Z*y;/*
	*/for(y=allZ;y;y=y->n){/*
		*/sprintf(buf,"Zocket %d%s",y->zo,y->zo==z->zo?" (you)\n":"\n");/*
		*/Zout(buf,z);/*
	*/}/*
	*/R s ? SPlay : SPlay;/*simply used to avoid compiler warnings*//*
*/}
/*-------------------------------------------------------------------------*\
]|  Assorted Functions                                                     |[
\*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*\
]|  VooDoo                                                                 |[
]|                                                                         |[
]|      Given a Z instance and a command line, Voodoo the command and then |[
]|      return its return value. The return value is a S-enumeration, or a |[
]|      "state" value. Thus, when you execute a command, it updates the    |[
]|      state accordingly. Incredibly simple stuff.                        |[
]|                                                                         |[
]|      Why 'VooDoo'? Well, 'do' is a keyword so I couldn't use that, and  |[
]|      I didn't feel like copying MERC and using 'do_', so I used VooDoo. |[
]|      (Though I could have just used 'Do', but I figured that would just |[
]|      confuse more people than it was worth.)                            |[
\*-------------------------------------------------------------------------*/
I VooDoo(C*s,Z*z){/*
	*/I x=*s;/*
	*/if(s==N)R SPlay;/*

	*/switch(x=tolower(x)){/*
		*/A 'd':/*
			*/if(!strPcmp("dam",s)) R D(Dam)(s,z);/*
		*/B;/*
		*/A 'q':/*
			*/if(!strPcmp("quit",s)) R D(Quit)(s,z);/*
		*/B;/*
		*/A 'w':/*
			*/if(!strPcmp("who",s)) R D(Who)(s,z);/*
		*/default:/*
			*/R D(Say)(s,z);/*
		*/B;/*
	*/}/*
	*/R SPlay;/*
*/}
/*-------------------------------------------------------------------------*\
]|  stateZ                                                                 |[
]|                                                                         |[
]|      Given a Z instance, process its input according to its state.      |[
]|      (This does nearly nothing. I've left in a way to shut down the mud |[
]|      and that's about it. You can't name yourself or really do anything |[
]|      other than talk.)                                                  |[
]|                                                                         |[
]|      Below would be an example of use:                                  |[
]|                                                                         |[
]|      SNew - a new socket/Z instance. Greetings, get Id.                 |[
]|      SGetID - read their name, pass along a message, set new state.     |[
]|      SPlay - convert their input into an in-game command.               |[
]|      SClose - close the Z instance's socket and free the Z instance.    |[
]|                                                                         |[
]|      ...god damn I hate that. I swear I rewrote this much nicer before. |[
]|      Where the hell did I put it? CVS anyone? Sigh...                   |[
\*-------------------------------------------------------------------------*/
V stateZ(Z*z){/*
	*/T*t=Zin(z);/*
	*/if(t==N&&z->st!=SNew)R;/*
	*//* Based on the Z state, do something. *//*
	*/switch(z->st){/*
		*//* a new connection, send out a greeting and set them to get id *//*
		*/A SNew:/*
			*/Zout(	"Quzah's Poorly Orchestrated Small MUD.\n"/*
					*/"Version:\t0.0.16k\n"/*
					*/"Created:\tApril.2000\n"/*
					*/"Author: \tJeremiah Myers (aka: Quzah)\n"/*
					*/"Email:  \tquzah@quzah.com\n\nEnjoy.\n\n"/*
				*/,z);/*
			*//*Normally you'd do something like...
			z->st=SGetID;
			Instead we're doing something like...*//*
			*/{/*
				*/static C buf[35]={0};/*
				*/sprintf(buf,"You are Zocket %d.\n> ",z->zo);/*
				*/Zout(buf,z);/*
			*/}/*
			*/z->st=SPlay;/*
		*/B;/*

		*//* take their input and turn it into a name; play or close *//*
		*/A SGetID:/*
			*//*Normally you'd check input and do something here...*//*
			*/z->st=SPlay;/*
		*/B;/*

		*//* take their input and translate it to an in game command *//*
		*/A SPlay:/*
			*/z->st=VooDoo(t->text,z);/*
			*/Zout("\n> ",z);/*
		*/B;/*

		*//* close the Z instance's socket and free the Z instance *//*
		*/A SClose:/*
			*/freeZ(z);/*
		*/B;/*
	*/}/*
	*/if(t)freeT(t);/*
*/}
/*-------------------------------------------------------------------------*\
]|  flowZ                                                                  |[
]|                                                                         |[
]|      The flow of the mud. Dam it to shut down. The concept is a simple  |[
]|      one: while !dam, listen, ready, read, process state, write, nap.   |[
\*-------------------------------------------------------------------------*/
V flowZ(I oZ){/*
	*/Z*z,*y;/*
	*/I x=-1;/*
	*/struct timeval	t;/*
	*/fd_set 	r,w;/*

	*/while(!dam){/*
		*/listenZ(oZ);/*

		*/if(allZ!=N){/*
			*/FD_ZERO(&r);/*
			*/FD_ZERO(&w);/*

			*//* add to "check me" list *//*
			*/for(z=allZ;z;z=y){/*
				*/y=z->n;/*
				*/if(z->zo>-1){/*
					*/FD_SET(z->zo,&r);/*
					*/FD_SET(z->zo,&w);/*
					*/if(z->zo>x)/*
						*/x=z->zo;/*
				*/}E freeZ(z);/*
			*/}/*

			*//* check me *//*
			*/X(select(x+1,&r,&w,(fd_set*)0,&t));/*
			*/for(z=allZ;z;z=y){/*
				*/y=z->n;/*
				*/if(FD_ISSET(z->zo,&r)&&readZ(z)<0)continue;/*
				*/stateZ(z);/*do one of them*//*
				*/if(FD_ISSET(z->zo,&w))writeZ(z);/*
			*/}/*
		*/}
#if NAPZ
		napZ(oZ,-1);
#endif
	}/*
	*//* trash all Z instances *//*
	*/for(z=allZ;z;z=y){y=z->n;freeZ(z);}/*
*/}
/*-------------------------------------------------------------------------*\
]|  Main                                                                   |[
]|                                                                         |[
]|      Validate the port, initialize the mud, call the mud flow, return.  |[
\*-------------------------------------------------------------------------*/
I main(I argc,C**argv){/*
    */I s,p;/*

    */p=argc>1?atoi(argv[1]):4000;/*
    */p=p<1024||p>9999?4000:p;/*
    */printf("Using port: %d\n",p);/*

    */X(s=initZ(p));/*
	*/flowZ(s);/*
    */R 0;/*
*/}