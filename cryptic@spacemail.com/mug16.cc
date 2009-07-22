/*
File size:                    26811 bytes
Lines:                         1377
\r stripped:                  26811
// removed:                   22024
/* removed:                   20524
leading blanks removed:       16321
blank lines stripped:         15814

Final result:                 15814
                             ======

Size is acceptable.

Modified file follows
=====================
*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <std/bastring.h>
class poi
{
public:
poi *next;
poi *prev;
void *thing;
poi(){next=NULL;prev=NULL;};
void add(void *t);
void * giv(int n);
int del(int n);
};
int poi::del(int n)
{
if(n==0)
{
if(prev!=NULL)
prev->next=next;
if(next!=NULL)
next->prev=prev;
delete this;
return 1;
}
else
{
if(next==NULL)
return 0;
else
return(next->del(--n));
}
}
void * poi::giv(int n)
{
if(n==0)
return(thing);
else
{
if(next==NULL)
return NULL;
else
return(next->giv(--n));
}
}
void poi::add(void *t)
{
poi *bus;
for(bus=this;bus->next!=NULL;bus=bus->next);
bus->next=new poi;
bus->next->prev=bus;
bus->next->thing=t;
}
string sfo(int n);
class exi;
class swi;
class room;
class area;
class exi
{
public:
unsigned int id;
string *desc;
room *r;
exi(int n){id=n;desc=NULL;r=NULL;};
};
class swi
{
public:
unsigned int id;
bool on;
unsigned int type;
string desc;
swi(int i,string d){id=i;desc=d;on=0;};
};
class room
{
public:
unsigned int id;
string desc;
string title;
poi *exits;
poi *switches;
room(int i,string t,string d){id=i;desc=d;title=t;exits=new poi;switches=new poi;};
string shows(int n);
string showr(int n);
string showe(int n);
void adex(exi *e);
string shex(int n);
void adsw(swi *s);
string shsw(int n);
private:
};
string
room::showr(int n)
{
return (title+"\n"+desc+"\n");
}
string
room::showe(int n)
{
string s="Exits:";
s+=(n?"\n":" ")+shex(n);
return s;
}
string
room::shows(int n)
{
string s="Switches:";
s+=(n?"\n":" ")+shsw(n);
return s;
}
void
room::adsw(swi *s)
{
switches->add((swi *)s);
}
void
room::adex(exi *e)
{
exits->add((exi *)e);
}
string
room::shex(int v)
{
string s;
int i=1;
exi * twist;
twist=(exi *)exits->giv(i);
while(twist!=NULL)
{
s+=sfo(i)+". ";
if(v)
s+=*(twist->desc)+"\n";
++i;
twist=(exi *)exits->giv(i);
}
if(!v)
s+="\n";
return s;
}
string
room::shsw(int v)
{
string s;
int i=1;
swi * twist;
twist = (swi *)switches->giv(i);
while(twist != NULL)
{
s+=sfo(i)+". "+twist->desc+(twist->on?" [on] ":" [off] ");
if(v)
s+="\n";
++i;
twist=(swi *)switches->giv(i);
}
if(!v)
s+="\n";
return s;
}
class area
{
public:
unsigned int id;
string desc;
poi *rooms;
poi *exits;
poi *switches;
area(int i,string s)
{
id=i;desc=s;
rooms=new poi;
exits=new poi;
switches=new poi;
};
};
string
sfo(int n)
{
string s;
do
{
s=(char)((n%10)+48)+s;
n=n/10;
}
while(n>0);
return s;
}
#define INPU_LENGTH 256
#define TIMEOUT 60
#define MAX_PLAYERS 23
#define M 714025
#define IA 1366
#define IC 150889
class d
{
public:
int fd;
struct in_addr host;
int port;
int score;
string name;
int auth;
long int on;
int build;
string comm;
string unic;
d *next;
d *prev;
room *here;
};
d *socko;
d *last;
extern int errno;
long ranseqval;
int players=0;
int port;
void phree(d *p, d **atend);
void shutit(d *p, d **atend);
float ran2(long *idum);
int rain(int span);
void deskribe(d *s);
int writeutf(int fd,const void *buf,size_t count);
int broadcast(string mes);
int narocast(d *b,string mes);
int herecast(d *bus,string mes);
void charem(string *s,char c);
void look(d *bus);
int go(d *bus,int i);
int thro(d *bus,int i);
void make_area(area *a,int n);
room * make_room(area *a,int n,int y);
int gcount=2,gexi=0;
main(int argc, char* argv[])
{
int x=1,running=0;
struct sockaddr_in maia;
static struct timeval null_time;
fd_set readfs,writefs,excepfs;
d *bus,*cus;
struct timeval timer;
long int endtime,endgame;
long int dura=300,endg=60;
string proc,out;
int aw,uh;
unsigned int poif;
int cup_cap=3;
string tea_spirit="Wei Lin";
if(argc<2)
{
printf("Usage: %s <port number>\n",argv[0]);
exit(0);
}
else
{
port=strtol(argv[1],(char **)NULL,10);
if((errno==ERANGE)||(port<1)||(port> 65535))
{
printf("Error: argument given was not in valid range.\n");
printf("Exiting.");
exit(1);
}
}
ranseqval = (time(0)+getpid());
ranseqval = abs(ranseqval);
ranseqval = 0-ranseqval;
area *th=new area(1,"Tea House");
room *r1=new room(1,"Tea House","The tea house is constructed from a series of adjoining spherical pods of varying capacity and shape. A single teapot is stored in the largest, central pod. It is not to be touched by guests.");
room *tea_room=r1;
th->rooms->add((room *)r1);
make_area(th,4);
swi *sw;
for(int i=0;i<26;++i)
{
string s="the ";
s+=(char)(97+i);
s+=" switch";
sw=new swi(i,s);
sw->type=97+i;
th->switches->add((swi *)sw);
r1=(room *)th->rooms->giv(rain(gcount-2)+2);
r1->adsw(sw);
printf("Switch %d placed in %s\n",i,r1->title.c_str());
}
socko=new d;
last=socko;
socko->fd=socket(AF_INET,SOCK_STREAM,0);
setsockopt(socko->fd,SOL_SOCKET,SO_REUSEADDR,(char *)&x,sizeof(x));
maia.sin_family=AF_INET;
maia.sin_port=htons(port);
maia.sin_addr.s_addr=INADDR_ANY;
bzero(&(maia.sin_zero),8);
if(bind(socko->fd,(struct sockaddr *)&maia,sizeof(struct sockaddr))<0)
{
printf("bind error\n");
exit(1);
}
if(listen(socko->fd,10)<0)
{
printf("listen error\n");
exit(1);
}
signal(SIGPIPE,SIG_IGN);
running=1;
gettimeofday(&timer,NULL);
endtime=timer.tv_sec+23+dura;
endgame=endtime-endg;
while(running)
{
FD_ZERO(&readfs);
FD_ZERO(&writefs);
FD_ZERO(&excepfs);
for(bus=socko;bus!=NULL;bus=bus->next)
{
FD_SET(bus->fd, &readfs);
FD_SET(bus->fd, &writefs);
FD_SET(bus->fd, &excepfs);
}
if(select(FD_SETSIZE,&readfs,&writefs,&excepfs,&null_time)==-1)
{
printf("Error: select failed\n");
exit(1);
}
for(bus=socko->next;bus!=NULL;bus=bus->next)
{
if(FD_ISSET(bus->fd,&excepfs))
{
printf("exception: %d\n",bus->fd);
bus=bus->prev;
shutit(bus->next,&last);
}
}
for(bus=socko->next;bus!=NULL;bus=bus->next)
{
if(FD_ISSET(bus->fd,&readfs)&&bus)
{
int in;
char inpu[INPU_LENGTH];
in=read(bus->fd,inpu,INPU_LENGTH-1);
if(errno==EWOULDBLOCK)
{
printf("EWOULDBLOCK %d\n",errno);
exit(1);
}
else if(in<0)
{
perror("read error: ");
if(errno==EPIPE);
else
{
printf("Weird read error, killing connection\n");
bus=bus->prev;
shutit(bus->next,&last);
}
}
else if(in==0)
{
bus=bus->prev;
shutit(bus->next,&last);
}
else if(in>0)
{
bus->comm.append(inpu,in);
if(bus->comm.size()>1024)
{
bus->comm[1023]='\n';
bus->comm.erase(1024);
narocast(bus,"error. shorten input.\n");
}
}
}
}
for(bus=socko->next;bus!=NULL;bus=bus->next)
{
unsigned int nlp;
nlp=bus->comm.find('\n');
if(nlp!=string::npos)
{
proc.assign(bus->comm,0,nlp);
bus->comm.assign(bus->comm,nlp+1,string::npos);
if(proc.size()>0)
{
int pval=atoi(proc.c_str());
char c=proc[0];
if(!bus->auth)
{
charem(&proc,'\r');
int ps=proc.size();
if(ps>15||ps< 3)
narocast(bus,"id length must be 3 - 15 characters.\n");
else if(proc.find_first_not_of("abcdefghijklmnopqrstuvwxyz") != string::npos)
{
narocast(bus,"error. use only lower-case letters.\n");
}
else
{
bus->name=proc;
proc="["+bus->name+" is formed]\n";
broadcast(proc);
narocast(bus,"Welcome.\n");
bus->here=tea_room;
look(bus);
bus->auth=1;
if(uh)
{
++bus->auth;
narocast(bus,tea_spirit+" bows and gives you a cup of tea.\n");
}
}
}
else
{
if((bus->auth)&&(proc.find("drink tea")==0))
{
if(bus->auth<2)
narocast(bus,"Tea has not yet been served.\n");
else if(bus->auth>cup_cap+1)
narocast(bus,"Tea will be served upon your return.\n");
else
{
if(++bus->auth>(cup_cap+1))
{
herecast(bus,bus->name+" finishes their cup of tea.\n"+tea_spirit + " catches the empty tea cup as it falls.\n");
bus->here=(room *)th->rooms->giv(rain(gcount-2)+2);
if(bus->here==NULL)
{
bus->here=tea_room;
printf("tea not properly made\n");
}
narocast(bus,"You finish your tea.\n\n");
look(bus);
}
else
{
narocast(bus,"The tea tastes nice.\n");
herecast(bus,bus->name+" drinks tea.\n");
}
}
}
if(proc.find_first_of("wstgik")==0)
{
poif=proc.find(' ');
if(poif!=string::npos)
{
proc.assign(proc,poif+1,string::npos);
charem(&proc,'\r');
if(c=='w')
{
proc="["+bus->name+"] "+proc+"\n";
broadcast(proc);
}
if(c=='s')
{
proc="<"+bus->name+"> "+proc+"\n";
narocast(bus,proc);
herecast(bus,proc);
}
if(c=='g')
go(bus,atoi(proc.c_str()));
if(c=='t')
thro(bus,atoi(proc.c_str()));
if(c=='i'||c=='k')
{
if(bus->build&&bus->here!=tea_room)
{
if(c=='i')
bus->here->title=proc;
else
bus->here->desc=proc;
herecast(bus,"The room shifts and changes.\n");
narocast(bus,"The room shifts and changes.\n");
--bus->build;
}
else
narocast(bus,"You are not tea based.\n");
}
}
else
narocast(bus,"This command requires an argument.\n");
}
if(c=='h')
{
narocast(bus,"\nmughi command set:\n\nquit -- disconnect\nw [text] -- world [text]\ns [text] -- utter [text]\nt # -- throw switch #\ng # / # -- leave via exit #\ni [text] -- change room title to [text]\nk [text] -- change room description to [text]\ndrink tea -- drink tea.\n\n");
}
if(c=='l')
look(bus);
if(c=='o')
{
int i=0,j;
char fer[80];
gettimeofday(&timer,NULL);
for(cus=socko->next;cus!=NULL;cus=cus->next)
{
++i;
j=(timer.tv_sec - cus->on);
snprintf(fer,80,"%-15s %02d:%02d:%02d\t%03d\n",
cus->name.c_str(),
(int)(j/3600),(int)((j%3600)/60),j%60,cus->score);
narocast(bus,fer);
}
narocast(bus,sfo(i)+" entities exist.\n");
}
if(pval>0)
{
go(bus,pval);
}
if(proc.find("quit")==0)
{
proc ="["+bus->name+" is unformed]\n";
broadcast(proc);
bus->auth=0;
bus->on=timer.tv_sec-(TIMEOUT+7);
}
}
}
}
}
gettimeofday(&timer,NULL);
if((timer.tv_sec>endgame)&&uh)
{
for(bus=socko->next;bus!=NULL;bus=bus->next)
{
if(bus->auth>(1+cup_cap))
narocast(bus,"A gong sounds somewhere in the distance.\n");
else
{
narocast(bus,tea_spirit+" bows and takes away your unfinished tea.\n");
bus->auth=1;
}
}
uh=0;
}
if(timer.tv_sec>endtime)
{
string upm;
bool ded;
swi *tch;
int i=1;
for(bus=socko->next;bus!=NULL;bus=bus->next)
{
if(bus->auth>(1+cup_cap))
{
ded=0;
for(tch=(swi *)th->switches->giv(i);tch!=NULL;
tch=(swi *)th->switches->giv(++i))
{
if(tch->on)
{
if(bus->name.find((char)tch->type)!=string::npos)
{
ded=!ded;
++bus->build;
}
}
}
if(ded)
{
upm+=bus->name+" was not within tea.\n";
narocast(bus,"You are not within tea.\nDisconnected.\n\n");
bus->auth=0;
bus->on=timer.tv_sec-(TIMEOUT+7);
}
else
{
upm+=bus->name+" is within tea.\n";
++bus->score;
bus->here=tea_room;
herecast(bus,bus->name+" returns to the tea room.\n");
look(bus);
}
}
if(bus->auth)
bus->auth=2;
}
broadcast(upm);
broadcast(tea_spirit+" serves tea.\n");
gettimeofday(&timer,NULL);
endtime = timer.tv_sec+23+dura;
endgame = endtime-endg;
uh=1;
}
if(FD_ISSET(socko->fd,&readfs))
{
int desc;
struct sockaddr_in sock;
int sizah=sizeof(sock);
if((getsockname(socko->fd,(struct sockaddr *)&sock,&sizah ))< 0)
printf("getsockname error\n");
else
{
if((desc=accept(socko->fd,(struct sockaddr *)&sock,&sizah))<0)
printf("accept error\n");
else
{
d *anew;
if(fcntl(desc,F_SETFL,O_NDELAY)<0)
{
printf("O_NONBLOCK error\n");
exit(1);
}
anew=new d;
anew->fd=desc;
anew->host=sock.sin_addr;
anew->port=ntohs(sock.sin_port);
anew->auth=0;
anew->comm="";
anew->unic="";
anew->name="";
anew->score=0;
anew->build=0;
players=0;
for(bus = socko->next; bus != NULL; bus = bus->next)
++players;
if(players<MAX_PLAYERS)
{
anew->prev=last;
anew->next=NULL;
gettimeofday(&timer,NULL);
anew->on=timer.tv_sec;
last->next=anew;
last=anew;
narocast(anew,"\n[mughi v.23]\n\n[tea spirit: "+tea_spirit+"]\n\n[login] ");
printf("addr: %s, port: %d on fd: %d\n", inet_ntoa(anew->host),anew->port,anew->fd);
deskribe(socko);
}
else
{
printf("Ignoring %s\nClose value: %d\n",
inet_ntoa(anew->host),
close(anew->fd));
free(anew);
}
}
}
}
for(bus = socko->next;bus!=NULL;bus = bus->next)
{
if(FD_ISSET(bus->fd,&writefs))
{
int si;
unsigned int wpos;
wpos=bus->unic.find('\0');
if(wpos!=string::npos)
{
out.assign(bus->unic,0,wpos);
si=out.size();
if(si)
{
aw=write(bus->fd,out.c_str(),si);
if(aw!=si)
si=aw-1;
}
bus->unic.assign(bus->unic,si+1,string::npos);
}
}
}
for(bus=socko->next;bus!=NULL;bus=bus->next)
{
if(!bus->auth)
{
if(timer.tv_sec>(bus->on+TIMEOUT))
{
bus=bus->prev;
shutit(bus->next,&last);
}
}
}
for(bus=socko->next;bus!=NULL;bus=bus->next)
{
if(bus->unic.size()>0)
break;
}
if(bus==NULL)
usleep(1000);
fflush(stderr);
fflush(stdout);
}
close(socko->fd);
return(0);
}
int
narocast(d *b,string mes)
{
if(b->unic.size()>10240)
{
printf("Disconnecting %d for having too much stuff in output buffer\n",
b->fd);
b=b->prev;
shutit(b->next,&last);
}
b->unic+=mes;
b->unic+='\0';
return(1);
}
int
herecast(d *bus,string mes)
{
d *bl;
for(bl=socko->next;bl!=NULL;bl=bl->next)
{
if((bl->auth)&&(bl->here==bus->here)&&(bl!=bus))
narocast(bl,mes);
}
}
int
broadcast(string mes)
{
d *bus;
for(bus=socko->next;bus!=NULL;bus=bus->next)
{
if(bus->auth)
narocast(bus,mes);
}
return(1);
}
void
shutit(d *p,d **atend)
{
printf("shutit: %d - %s\n",p->fd,inet_ntoa(p->host));
printf("close value: %d\n",close(p->fd));
phree(p,atend);
}
void
phree(d *p,d **atend)
{
if(p->prev!=NULL)
(p->prev)->next=p->next;
else
printf("phree() just got told to delete first member..\nhop you know what you are doing...\n");
if(p->next!=NULL)
(p->next)->prev=p->prev;
else
*atend=p->prev;
delete p;
}
void
deskribe(d *s)
{
int i;
printf("\ndeskribe:\n");
for(i=0;s!=NULL;s=s->next,++i)
{
printf("%d : %d \n",i,s->fd);
}
}
int
rain(int span)
{
return (int) (span * ran2(&ranseqval));
}
float
ran2(long *idum)
{
static long iy,ir[98];
static int iff = 0;
int j;
if (*idum < 0 || iff == 0) {
iff = 1;
if ((*idum=(IC-(*idum)) % M ) < 0) *idum = -(*idum);
if ( *idum < 0 ) *idum = -(*idum);
for (j=1;j<=97;j++){
*idum=(IA*(*idum)+IC) % M;
ir[j]=(*idum);
}
*idum=(IA*(*idum)+IC) % M;
iy=(*idum);
}
j = (int) (1 + 97.0*iy/M);
if (j > 97 || j < 1) printf("\nRAN2 -- over array bounds");
iy=ir[j];
*idum=(IA*(*idum)+IC) % M;
ir[j]=(*idum);
return (float) iy/M;
}
void
charem(string *s,char c)
{
unsigned int i = s->find(c);
while(i != string::npos)
{
s->erase(i,1);
i = s->find(c);
}
}
void
look(d *bus)
{
d *cus;
narocast(bus,"\n");
narocast(bus,bus->here->showr(1));
for(cus=socko->next;cus!=NULL;cus=cus->next)
{
if(cus!=bus)
{
if(cus->here==bus->here)
{
narocast(bus,cus->name+" exists.\n");
}
}
}
narocast(bus,bus->here->shows(1));
narocast(bus,bus->here->showe(1));
}
int
go(d *bus,int i)
{
if(i>0)
{
exi *tw;
tw=(exi *)bus->here->exits->giv(i);
if(tw!=NULL)
{
if(tw->r!=NULL)
{
string s=bus->name+" goes "+*(tw->desc)+".\n";
herecast(bus,s);
s=bus->name+" arrives from "+bus->here->title+".\n";
bus->here=tw->r;
herecast(bus,s);
look(bus);
}
else
narocast(bus,"This exit doesn't lead anywhere.\n");
}
else
narocast(bus,"Unknown exit.\n");
}
return 1;
}
int
thro(d *bus,int i)
{
if(i>0)
{
swi *tw;
tw =(swi *)bus->here->switches->giv(i);
if(tw!=NULL)
{
tw->on=!tw->on;
string s="You turned "+tw->desc+(tw->on?" on.\n":" off.\n");
narocast(bus,s);
s=bus->name+" turned "+tw->desc+(tw->on?" on.\n":" off.\n");
herecast(bus,s);
}
else
{
narocast(bus,"Unknown switch.\n");
}
}
else
narocast(bus,"Please supply the number of the switch to be thrown.\n");
return 1;
}
void
make_area(area *a,int n)
{
make_room(a,n,1);
room *rp;
int nor;
for(nor=1;nor<gcount;++nor)
{
rp=(room *)a->rooms->giv(nor);
printf("%s ... \n",rp->title.c_str());
}
}
room *
make_room(area *a,int n,int y)
{
static int an=0,cn=0,rn=0;
string de;
string ti;
exi *e;
room *re,*ro;
de = "A small, circular recess cut into the wall.";
printf("make room
if(n<2)
{
ti="Alcove " +sfo(an++);
ro=new room(gcount,ti,de);
++gcount;
}
else
{
int noe=3;
if(y)
noe++;
printf("%d possible\t",noe);
noe=rain(noe)+1;
printf("%d actual\n",noe);
if(y&&noe==1)
ti="Alcove "+sfo(an++);
else
{
ti=(noe<2)?"Corridor "+sfo(cn++):"Room "+sfo(rn++);
de=(noe<2)?"A short connecting corridor.":"This room is without form.";
}
ro=new room(gcount,ti,de);
++gcount;
for(int i=0;i<noe;++i)
{
re=make_room(a,n-1,0);
e=new exi(gexi++);
e->r=re;
e->desc=&(re->title);
a->exits->add((exi *)e);
ro->adex(e);
e=new exi(gexi++);
e->r=ro;
e->desc=&(ro->title);
a->exits->add((exi *)e);
re->adex(e);
}
}
a->rooms->add((room *)ro);
printf("Made %s\n",ti.c_str());
return(ro);
}

