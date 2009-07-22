/* Slight mud
** ksb '00
** for the 16k comp - http://www.andreasen.org/16k.shtml
** 
** All in one source file, have fun
**
** Feel free to use this source code as you wish, providing you give proper
** recognition to its author, Karl Bastiman (aka Kenny).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

#define RC rooms[count]
#define UC users[count]
#define UU users[uid]
#define BP users[uid].bufpos
#define CB users[uid].buff
#define SU short uid
#define CM mobs[curmob]
#define MC mobs[count]
#define WT wuser(uid,text)
#define WU wuser(uid
#define SH short
#define FC for (count=0;count
#define CP count++)
#define PR psuedo_rand
#define VD void
#define SF sprintf(text,
#define RT return
#define US users
#define MB mobs
#define CT continue


struct chr {
	char name[9];
	int socket;
	SH lvl;
	SH hp,maxhp;
	long xp;
	long bufpos;
	char buff[4096];
	SH room;
	SH login;
	SH fight;  /* will be a mob vnum */
};

struct rm {
	char name[23];
	SH vnum;
	char desc[791];
	SH dir[10]; /* n,e,s,w,ne,nw,se,sw,u,d */
};

struct mob {
	char name[23];
	SH vnum;
	SH room;
	char sdesc[80];
	SH lvl;
	SH hp,maxhp;
	SH status; /* 0=dead, 1=corpse, 2=alive, 3=asleep */
	SH fight; /* will be a character number */
	SH deadtime;
};

#define XU 40
#define XM 30
#define XR 100

struct chr US[XU];
struct rm rooms[XR];
struct mob MB[XM];

char *comlist[]={
"quit","look","north","south","east","west","ne","se","sw","nw","up",
"down","say","score","consider","kill","flee",
"*"
};

enum coms {
xQUIT,xLOOK,xNORTH,xSOUTH,xEAST,xWEST,xNE,xSE,xSW,xNW,xUP,
xDOWN,xSAY,xSCORE,xCONSIDER,xKILL,xFLEE
}foundc;

char inpstr[4096];
char text[4096];
char *comstr;
SH word_count=0;
int lsock=0;
char word[10][255];
SH curmob=-1;
SH ht=0,hr=0;

VD init_socket();
VD scan();
VD new_connect();
SH getinp(SU);
VD terminate(char *str);
VD qquit(SU);
VD activity(SU);
SH read_rooms();
VD login(SU);
VD username(SU);
SH getroom(SH vnum);
VD wall(char *str, SH exc);
char *getdir(SH dn);
VD quit1(SU);
VD flee(SU);
VD say(SU);
VD move(SU, SH dir);
VD look(SU);
VD wsock(int socket,char *str);
VD init_vars();
VD killx(SU);
VD read_MB();
VD wuser(SU, char *str);
VD wroom(SH vnum, char *str, SH exc);
SH read_mob(SH vnum);
VD autox();
VD score(SU);
VD consider(SU);
SH getmob(SU,char *str);

/**************************************************************************/
int main()
{
	init_vars();
	read_rooms();
	read_MB();
	init_socket();
	while(1) {
		scan();
		autox();
	} 
}

int PR(int max)
{
        RT (1+(int) ((double)max*rand()/(RAND_MAX+1.0)));
}

VD autox()
{
	SH count,dam,hit,xpa,hpg;
	/* scan for fights */
	ht++;
	hr=0;
	if (ht==15) { ht=0;hr=1; }
	FC<XU;CP {
		if (UC.socket==-1) CT;
		if (hr) {
			/* hourly tick */
			UC.hp+=(int)(UC.maxhp*.2);
			UC.hp=UC.hp>UC.maxhp?UC.maxhp:UC.hp;
		}
		if (UC.fight==-1) CT;
		hit=PR(MB[UC.fight].hp-(8*MB[UC.fight].lvl));
		dam=0;
		if (hit<=UC.hp) {
			dam=(int)(PR((UC.hp-hit)/2)*(1-(.2*(MB[UC.fight].lvl-UC.lvl))));
			if (dam<0) dam=0;
			MB[UC.fight].hp-=dam;
		}
		SF"You %s the %s.[%d/%d]\n",hit<=UC.hp?"hit":"miss",MB[UC.fight].name,dam,MB[UC.fight].hp);
		wuser(count,text);
		SF"%s %s the %s.\n",UC.name,hit<=UC.hp?"hits":"misses",MB[UC.fight].name);
		wroom(UC.room,text,count);
		if (MB[UC.fight].hp<=0) {
			SF"The %s is DEAD!\n",MB[UC.fight].name);
			wroom(UC.room,text,-1);
			xpa=(((MB[UC.fight].lvl-UC.lvl)+2)*25)+PR(100);
			if (xpa<0) xpa=0;
			SF"You recieve %d xp.\n",xpa);
			wuser(count,text);
			UC.xp+=xpa;
			if ((int)(UC.xp/1000)>UC.lvl) {
				wuser(count,"You have gained a level!\n");
				hpg=PR(13)+7;
				SF"Your gain is %d hp.\n",hpg);
				wuser(count,text);
				UC.maxhp+=hpg;
				SF"%s has reached level %d.\n",UC.name,UC.lvl);
				wall(text,count);
				UC.lvl++;
			}
			MB[UC.fight].fight=-1;
			MB[UC.fight].status=0;
			UC.fight=-1;
		}	
			
	}
	FC<XM;CP {
		if (hr) {
			if (MC.status==0) MC.deadtime++;
			if (MC.deadtime==5) {
				MC.deadtime=0;
				MC.status=2;
				MC.fight=-1;
				MC.hp=MC.maxhp;
			}
		}
		if (MC.fight==-1) CT;
		hit=PR(US[MC.fight].hp+(US[MC.fight].lvl*5));
		dam=0;
		if (hit<=MC.hp) {
			/* dam=(int)((MC.hp-hit)/5+((MB[UC.fight].lvl-UC.lvl)*10)); */
			dam=(int)(PR((MC.hp-hit)/4)*(1-(.2*(US[MC.fight].lvl-MC.lvl))));
			if (dam<0) dam=0;
			US[MC.fight].hp-=dam;
		}
		SF"The %s %s you.[%d/%d]\n",MC.name,hit<=MC.hp?"hits":"misses",dam,US[MC.fight].hp);
		wuser(MC.fight,text);
		SF"%s %s %s.\n",MC.name,hit<=MC.hp?"hits":"misses",US[MC.fight].name);
		wroom(MC.room,text,MC.fight);
		if (US[MC.fight].hp<=0) {
			wuser(MC.fight,"You are DEAD!\n");
			xpa=(int)((US[MC.fight].xp-(US[MC.fight].lvl*1000))/2);
			US[MC.fight].xp-=xpa;
			if (US[MC.fight].xp<1000) US[MC.fight].xp=1000;
			US[MC.fight].hp=1;
			US[MC.fight].fight=-1;
			MC.fight=-1;
		}	
	}
}

VD read_MB()
{
		FILE *fp;

		if (!(fp=fopen("../mobs/mobs.idx","r")))
		{
			exit(-1);
		}

		while (fgets(text,255,fp)) {
			if (text[0]=='#') { fclose(fp); break; }
			read_mob(atoi(text));
			fgets(text,255,fp);
			CM.room=atoi(text);
			CM.status=2;
			CM.deadtime=0;
		}	
}

SH read_mob(SH vnum)
{
	FILE *fp;
	SH stat=0;
	SH count=-1;

	if (!(fp=fopen("../mobs/mobs.def","r"))) 
	{ 
		exit(-1); 
	}

	curmob++;

	while (fgets(text,255,fp)) {
		text[strlen(text)-1]='\0';
		if (text[0]=='#') { fclose(fp); break; }
		if (text[0]=='%') { stat=stat==4?0:stat+1; CT;}
		switch (stat) {
			case 0 : CM.vnum=atoi(text); break;
			case 1 : strcpy(CM.name,text); break;
			case 2 : strcpy(CM.sdesc,text); break;
			case 3 : CM.lvl=atoi(text); break;
			case 4 : CM.maxhp=CM.hp=atoi(text); break;
			default: exit(-2);
		}
		if ((CM.vnum==vnum)&&(stat==4)) { fclose(fp); break; }
	}
}

VD init_vars()
{
	SH count;

	FC<XU;CP
	{
		UC.fight=-1;
		UC.socket=-1;
	}

	FC<XM;CP
	{
		MC.fight=-1;
	}
}

VD init_socket()
{
	int mains=0;
	struct sockaddr_in bind_addr;
	int on=0;
	int size=0;
	SH count;

	size=sizeof(struct sockaddr_in);
	bind_addr.sin_family=AF_INET;
	bind_addr.sin_addr.s_addr=INADDR_ANY;

	if ((lsock=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		exit(-1);
	}

	setsockopt(lsock,SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));

	bind_addr.sin_port=htons(9999);
	if (bind(lsock,(struct sockaddr *)&bind_addr,size)==-1)
	{
		exit(-1);
	}

	if (listen(lsock,10)==-1)
	{
		exit(-1);
	}

	fcntl(lsock,F_SETFL,O_NDELAY);

	for(count=0;count<XU;CP UC.socket=-1;
}

VD scan()
{
	fd_set fset;
	struct timeval tv;
	SH count=0;

	FD_ZERO(&fset);
	FD_SET(lsock,&fset);

	FC<XU;CP
	{
		if (UC.socket>-1) FD_SET(UC.socket,&fset);
	}

	tv.tv_sec=2;
	tv.tv_usec=0;

	if (select(FD_SETSIZE,&fset,0,0,&tv) == -1) RT;

	if (FD_ISSET(lsock,&fset)) new_connect();

	FC<XU;CP
	{
		if (UC.socket>-1)
		{
			if (FD_ISSET(UC.socket,&fset))
			{
				activity(count);
			}
		}
	}

}

VD new_connect()
{
	struct sockaddr_in newsock;
	int len,snew,dummy=1;
	char *tmpstr;
	SH count;

	len=sizeof(newsock);
	snew=accept(lsock,(struct sockaddr *)&newsock,&len);

	FC<XU;CP
	{
		if (UC.socket<0) {
			UC.socket=snew;
			UC.login=1;
			wsock(snew,"\nPlease enter character name:\n");
			RT;
		}
	}

	wsock(snew,"\nSorry maximum connections reached\n\n");
}

SH getinp(SU)
{
	long len=0,c=0;
	int i=0;

	memset(inpstr,0,4095);
	if (!(len=read(UU.socket,inpstr,4096))) 
	{
		qquit(uid);
		RT 0;
	}
	if ((inpstr[len-1]<=32)&&(!BP)) RT 1;

	/* We can assume split input here */
	for (i=0;i<len;i++) {
		if (inpstr[i]==8 || inpstr[i]==127) 
		{
			if (BP)
			{	
				BP--;
				CT;
			}
		}
	
		CB[BP]=inpstr[i];

		if (inpstr[i]<32 || BP+2==4096)
		{
			terminate(CB);
			strcpy(inpstr,CB);
			BP=0;
			CB[0]='\0';
			RT 1;
		}

		BP++;
	}

	RT 0;
}

VD terminate(char *str)
{
	int i;
	for (i=0;i<4096;i++)
	{
		if (*(str+i)<32)
		{
			*(str+i)='\0';
			RT;
		}
	}

	str[i-1]='\0';
}


VD qquit(SU)
{
	close(UU.socket);
	UU.socket=-1;
}

VD activity(SU)
{
	char *tmpstr;
	char *loginstr;
	char *sst;
	char SHc[4096];
	SH len=0;
	SH slogin;
	int count=0;
	char com[4096];
	time_t ln,ll,tl;

	if (!getinp(uid)) RT; /* no actual usable input */
	if ((unsigned char)*inpstr==255) RT;
	if (*inpstr<=32) RT;

	inpstr[strlen(inpstr)-2]='\0';
	if (UU.login>0) {
		login(uid);
		RT;
	}

        comstr=strchr(inpstr,' ');
        if (!comstr)
        {
                comstr=strchr(inpstr,'\0');
                strncpy(com,inpstr,comstr-inpstr);
                com[(comstr-inpstr)]='\0';
        }
        else
        {
                strncpy(com,inpstr,comstr-inpstr);
                com[(comstr-inpstr)]='\0';
                comstr++;
        }

	/* Commands go here */
	foundc=-1;
	len=strlen(com);
	for (count=0;comlist[count][0]!='*';CP
	{
		if (strncmp(com,comlist[count],len)==0)
		{
			foundc=count;
			break;
		}
	}

	switch(foundc) {
		case xQUIT	:quit1(uid);		break;
		case xLOOK	:look(uid);		break;
		case xNORTH	:move(uid,0);		break;
		case xSOUTH	:move(uid,2);		break;
		case xEAST	:move(uid,1);		break;
		case xWEST	:move(uid,3);		break;
		case xUP  	:move(uid,8);		break;
		case xDOWN	:move(uid,9);		break;
		case xNE	:move(uid,4);		break;
		case xNW	:move(uid,5);		break;
		case xSE	:move(uid,6);		break;
		case xSW	:move(uid,7);		break;
		case xSAY	:say(uid);		break;
		case xSCORE :score(uid);		break;
		case xCONSIDER : consider(uid); break;
		case xKILL	:killx(uid);		break;
		case xFLEE	:flee(uid);		break;
		default: 
			WU,"Command unknown.\n");
			break;
	}
}

SH read_rooms()
{
	FILE *fp;
	SH stat=0;
	SH count=-1;

	if (!(fp=fopen("../rooms/rooms.def","r"))) 
	{ 
		exit(-1); 
	}

	while (fgets(text,255,fp)) {
		if (text[0]=='#') { fclose(fp); break; }
		if (text[0]=='%') { stat=stat==3?0:stat+1; CT;}
		switch (stat) {
			case 0 : count++; RC.vnum=atoi(text); break;
			case 1 : strcpy(RC.name,text); break;
			case 2 : strcat(RC.desc,text); break;
			case 3 : RC.dir[text[0]-48]=atoi(text+1); break;
			default: exit(-2);
		}
	}
}

VD login(SU)
{
	switch(UU.login) {
		case 1 : username(uid);		break;
		default : break;
	}
}

VD username(SU)
{
	if ((strlen(inpstr)<3)||(strlen(inpstr)>10)) {
		WU,"Incorrect length, must be between 3 and 10 chars\n");
		WU,"Try another name:\n");
		RT;
	}

	strcpy(UU.name,inpstr);

	UU.login=0;
	UU.lvl=1;
	WU,"\n\n");
	
	/* defaults */
	UU.hp=20;
	UU.maxhp=20;
	UU.xp=1000;
	UU.room=1;
	UU.fight=-1;
	SF"%s has signed on.\n",UU.name);
	wall(text,uid);
	look(uid);
}

VD wsock(int socket,char *str)
{
        write(socket,str,strlen(str));
}

VD wuser(SU, char *str)
{
	wsock(UU.socket,str);
}

VD wroom(SH vnum, char *str, SH exc)
{
	SH count=0;
	
	for(count=0;count<XU;CP {
		if (count==exc) CT;
		if (UC.socket<0) CT;
		if (UC.room!=vnum) CT;
		wuser(count,str);
	}
}

VD wall(char *str, SH exc)
{
	SH count=0;
	
	for(count=0;count<XU;CP {
		if (count==exc) CT;
		if (UC.socket<0) CT;
		wuser(count,str);
	}
}

VD quit1(SU)
{
	WU,"Goodbye, see you soon!\n");
	SF"%s has signed off.\n",UU.name);
	wall(text,uid);
	qquit(uid);
}

VD say(SU)
{
	if (!*comstr) {
		WU,"Say what?\n");
		RT;
	}

	SF"You say '%s'\n",comstr);
	WT;
	SF"%s says '%s'\n",UU.name,comstr);
	wroom(UU.room,text,uid);
}

SH getroom(SH vnum)
{
	SH count=0;
	
	FC<XR;CP {
		if (rooms[count].vnum==vnum) RT count;
	}

	RT 1;
}

VD move(SU, SH dir)
{
	SH newr=0;
	if (rooms[getroom(UU.room)].dir[dir]>0) {
		SF"%s leaves %s.\n",UU.name,getdir(dir));
		wroom(UU.room,text,uid);
		newr=rooms[getroom(UU.room)].dir[dir];
		UU.room=newr;
		SF"%s has arrived.\n",UU.name);
		wroom(UU.room,text,uid);
		look(uid);
	}
	else
	{
		WU,"Alas, you cannot go that way.\n");
	}
}

char *getdir(SH dn)
{
	switch(dn) {
		case 0 : RT "north";
		case 1 : RT "east";
		case 2 : RT "south";
		case 3 : RT "west";
		case 4 : RT "northeast";
		case 5 : RT "northwest";
		case 6 : RT "southeast";
		case 7 : RT "southwest";
		case 8 : RT "up";
		case 9 : RT "down";
		default : RT "error!";
	}
}

VD look(SU)
{
	SH rc=0;
	SH count=0;

	rc=getroom(UU.room);

	WU,rooms[rc].name);
	SF"[Exits:");
	FC<10;CP {
		if (rooms[rc].dir[count]>0) {
			strcat(text," ");
			strcat(text,getdir(count));
		}
	}
	strcat(text,"]\n");
	WT;
	WU,rooms[rc].desc);
	/* Show the MB present */
	FC<=curmob;CP {
		if (MC.status==0) CT;
		if (MC.room==UU.room) {
				SF"%s is here.\n",MC.sdesc);
				WT;
		}
	}
	FC<XU;CP {
		if (UC.socket<0) CT;
		if (count==uid) CT;
		if (UC.room==UU.room) {
			SF"%s the level %d Adventurer is here.\n",UC.name,UC.lvl);
			WT;
		}
	}
}

VD score(SU)
{
		SF"You are %s.\n",UU.name);
		WT;
		SF"Hit %d(%d) Exp %d Level %d\n",UU.hp,UU.maxhp,UU.xp,UU.lvl);
		WT;
}

VD consider(SU)
{
		SH mob;
		long con;

		mob=getmob(uid,comstr);
		if (mob<0) {
				WU,"Not here.\n");
				RT;
		}

		con=MB[mob].lvl-UU.lvl;
		if (con>=6) {
				WU,"No chance!\n");
				RT;
		}
		if (con>=4) {
				WU,"HAHA!\n");
				RT;
		}
		if (con>=2) {
				WU,"Perhaps!\n");
				RT;
		}
		if (con>=-1) {
				WU,"Perfect!\n");
				RT;
		}
		if (con>=-3) {
				WU,"Easy.\n");
				RT;
		}

		WU,"Pushover.\n");
}

SH getmob(SU,char *str)
{
		SH count;

		FC<XM;CP
		{
				if (MC.status<2) CT;
				if (MC.room==UU.room) {
						if ((strstr(MC.name,str))||(strstr(MC.sdesc,str))) {
										RT count;
						}
				}
		}
	RT -1;
}

VD killx(SU)
{
	SH mob;

	if (UU.fight!=-1) {
		WU,"You are already fighting!\n");
		RT;
	}

	mob=getmob(uid,comstr);
	if (mob==-1) {
		WU,"No one of that name here!\n");
		RT;
	}
	if (MB[mob].fight>-1) {
		WU,"That is already fighting someone!\n");
		RT;
	}

	UU.fight=mob;
	MB[mob].fight=uid;
}

VD flee(SU)
{
	SH chance=0;
	SH fd=-1,sr=0;

	chance=PR(100);

	if (chance>75) {
		WU,"You failed!\n");
	} else
	{
		while (fd==-1) {
			sr=PR(10)-1;
			if (rooms[getroom(UU.room)].dir[sr]>0)
				fd=sr;
		}
		WU,"You flee!\n");
		SF"%s has fled %s.\n",UU.name,getdir(fd));
		wroom(UU.room,text,uid);
		UU.room=rooms[getroom(UU.room)].dir[fd];
		MB[UU.fight].fight=-1;
		UU.fight=-1;
	}
}

