
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#define N "\r\n"

#define MOTD "Welcome to Tank Mage Deathmatch."N
#define TOOMANY "Sorry.  Too many players!"N
#define PROMPT "> "

#define NRUNES 4
struct{char*Name,*name;int cost,maint;}runes[NRUNES]={
 {"Rune of Earth","earth",2,1,},/*1*/
 {"Rune of Air","air",1,1},/*2*/
 {"Rune of Fire","fire",1,2},/*4*/
 {"Rune of Water","water",0,1}};/*8*/

typedef struct{char*name,*s;int cost,dam,neg;}attack;
#define NATT 5
attack attacks[NATT]={
 {"lunge","s",1,1,24},/*1*/
 {"hack","s",1,1,17},/*2*/
 {"slash","es",1,1,18},/*4*/
 {"chop","s",1,1,20},/*8*/
 {"block","s",3,0}};/*16*/

typedef struct{char*Name;int dam,req[2],neg;}combo;
#define NCOMBO 5
combo combos[NCOMBO]={
 {"Shield Rush",3,{4,4},16},
 {"Forward Combo",3,{0,2},18},
 {"Reverse Combo",3,{1,3},20},
 {"Body Strike",3,{1,2},24},
 {"Overhead Chop",5,{3,0},17}};

typedef struct{char*Name,*name;int cost,dam,pro,ar,req,neg;}spell;

#define NSPELLS 5
spell spells[NSPELLS]={
 {"Magic Missile","magic missile",1,2,0,0,2,1},
 {"Fireball","fireball",2,6,0,1,6,8},
 {"Stone Skin","stone skin",1,0,8,0,9,0},
 {"Ice Shards","ice shards",2,5,0,0,10,5},
 {"Poison Cloud","poison cloud",2,6,0,1,12,2}};

void error(char*msg){
 printf("Error %s\n",msg);
 exit(1);
}

int ran(int r){return r*(rand()/(RAND_MAX+1.0));}

void append(char*str,char ch){
 char chstr[2]={ch,0};
 strcat(str,chstr);
}

typedef struct _obj{struct _obj*next,*parent,*child;}obj;

typedef struct _room{
 obj o;
 char*desc;
 struct _room*exits[6];/*NSEWUD*/
 int ptype;/*None Rune Health Sword RunePower*/
 int prep,pcnt;
}room;

typedef struct _plr plr;

#define MAXBUF 2048 /* Power of two */
#define MAXPLAY 32

#define PLOOP(p) for(i=0,p=players;i<MAXPLAY;i++,p++)

#define NRUNES 4 /* Earth Air Fire Water */

struct _plr{
 obj o;
 int id,state,fd,bpos,blen;
 void(*ip)(plr*p,char ch);
 char name[16],Name[16],op[MAXBUF],line[128];
 struct pollfd*p;
 int HP,AP,pro,runes[NRUNES],runep[NRUNES],irunes[NRUNES];
 int kills,sp,att[3];
 plr*tar;
};

void init_plr(plr*p){
 int i;
 p->HP=p->AP=12+ran(7);
 p->sp=0;
 p->kills=0;
 for(i=0;i<3;i++)p->att[i]=-1;
 for(i=0;i<NRUNES;i++)p->runes[i]=p->irunes[i]=p->runep[i]=0;
 p->tar=NULL;
}

char*exit_names[]={"north","south","east","west","up","down"};

#define EQ(x,y) (strcmp((x),(y))==0)

#define DSC(x) {NULL,NULL,NULL},x
#define EX1(x) x==-1?NULL:rooms+x
#define EXITS(n,s,e,w,u,d) EX1(n),EX1(s),EX1(e),\
	EX1(w),EX1(u),EX1(d)

room rooms[]={
 /*0*/{DSC("The Village Church"),EXITS(-1,4,-1,2,-1,-1)},
 /*1*/{DSC("Wizards' Hall"),EXITS(-1,-1,-1,-1,2,-1),/*Rune*/1,15},
 /*2*/{DSC("An Elevator"),EXITS(-1,-1,0,-1,3,1)},
 /*3*/{DSC("The Church Attic"),EXITS(-1,-1,-1,-1,-1,2),/*RunePower*/4,20},
 /*4*/{DSC("The Village Green"),EXITS(0,-1,5,8,-1,7)},
 /*5*/{DSC("The Village Square"),EXITS(6,14,9,4,-1,-1)},
 /*6*/{DSC("Bargain Bazaar"),EXITS(15,5,13,-1,-1,-1),/*Sword*/3,30},
 /*7*/{DSC("A Deep Well"),EXITS(-1,-1,-1,12,4,-1)},
 /*8*/{DSC("The Only Road Out of Town"),EXITS(-1,-1,4,10,-1,-1)},
 /*9*/{DSC("The Esplanade"),EXITS(13,17,-1,5,-1,-1)},
 /*10*/{DSC("Troll's Bridge"),EXITS(-1,-1,8,11,-1,12)},
 /*11*/{DSC("The Dark Forest"),EXITS(-1,-1,10,-1,-1,-1),/*Rune*/1,20},
 /*12*/{DSC("Troll's Lair"),EXITS(-1,-1,7,-1,10,-1),/*Sword*/3,30},
 /*13*/{DSC("The Jetty"),EXITS(-1,9,-1,6,-1,16)},
 /*14*/{DSC("Harry's Place"),EXITS(5,-1,-1,-1,-1,-1),/*Health*/2,20},
 /*15*/{DSC("The Pub"),EXITS(-1,6,-1,-1,-1,-1),/*Health*/2,20},
 /*16*/{DSC("Swimming at Sea"),EXITS(-1,-1,-1,-1,13,-1)},
 /*17*/{DSC("The Corn Fields"),EXITS(9,-1,-1,-1,-1,-1),/*Health*/2,20}};

#define NROOMS (sizeof(rooms)/sizeof(room))

plr players[MAXPLAY];

struct pollfd pdata[MAXPLAY+1];

void sendp(plr*p,char*msg){
 int pos=(p->bpos+p->blen)&(MAXBUF-1);
 while(*msg){
  p->op[pos++]=*(msg++);
  p->blen++;
  pos&=MAXBUF-1;
 }
}

void sendpf(plr*p,char*fmt,...){
 static char temp[896];
 va_list ap;
 va_start(ap,fmt);
 vsprintf(temp,fmt,ap);
 va_end(ap);
 sendp(p,temp);
}

void say(plr*p,plr*i,char*str){
 /* send a message to all players in the room except p and i */
 obj*o;
 plr*d;
 o=p->o.parent->child;
 while(o){
  d=(plr*)o;
  if(d!=p&&d!=i)sendp(d,str);
  o=o->next;
 }
}

void sayf(plr*p,plr*i,char*fmt,...){
 static char temp[896];
 va_list ap;
 va_start(ap,fmt);
 vsprintf(temp,fmt,ap);
 va_end(ap);
 say(p,i,temp);
}

void shout(plr*p,char*str){
 int i;
 obj*r;
 if(p){
  r=p->o.parent;
  for(i=0;i<NROOMS;i++){
   p->o.parent=&(rooms+i)->o;
   say(p,p,str);
  }
  p->o.parent=r;
 }else{
  plr fake;
  for(i=0;i<NROOMS;i++){
   fake.o.parent=&(rooms+i)->o;
   say(&fake,NULL,str);
  }
 }
}

void moveobj(obj*ob,obj*dest){
 obj*prev,*cur;
 if(ob->parent){
  cur=ob->parent->child;
  prev=NULL;
  while(cur!=ob){
   prev=cur;
   cur=cur->next;
  }
  if(prev)prev->next=ob->next;
  else ob->parent->child=ob->next;
 }
 if(dest){
  cur=dest->child;
  if(cur){
   while(cur->next)cur=cur->next;
   cur->next=ob;
  }else dest->child=ob;
 }
 ob->parent=dest;
 ob->next=NULL;
}

void check_alias(char*s,char*a){
 char*c=s;
 while(*c)if(*c=='='||*c==';')return;else c++;
 while(*a){
  c=s;while(*c==*a){c++;a++;}
  if(*c==0&&*a=='='){a++;while(*a!=';')*(s++)=*(a++);*s=0;return;}
  while(*(a++)!=';');
 }
}

char *dir_aliases="n=north;s=south;e=east;w=west;u=up;d=down;";

#define CMD(x) void x(plr*p,char*args)

CMD(do_look){
 int i;
 obj*o;
 room*r;
 r=(room*)p->o.parent;
 if(*args){
  check_alias(args,dir_aliases);
  for(i=0;i<6;i++)if(EQ(exit_names[i],args))break;
  if(i==6){sendp(p,"Look where?"N);return;}
  r=r->exits[i];
  if(!r){sendp(p,"You can't look that way."N);return;}
  sayf(p,p,"%s peers %s"N,p->Name,args);
 }
 sendpf(p,"%s"N"Exits:",r->desc);
 for(i=0;i<6;i++)if(r->exits[i])sendpf(p,"  %s",exit_names[i]);
 sendp(p,N);
 i=0;
 o=r->o.child;
 while(o){
  if(o!=&p->o){
   if(i==0)sendp(p,"You see:"N);
   sendpf(p,"%s (%d)"N,((plr*)o)->Name,((plr*)o)->id);
   i++;
  }
  o=o->next;
 }
 if(i==0)sendp(p,"There is nothing here."N);
}

void rvoke(plr*p){/* revoke all runes*/
  int i;
  for(i=0;i<NRUNES;i++){p->runes[i]+=p->irunes[i];p->irunes[i]=0;}
}

CMD(do_go){
 room*r=(room*)p->o.parent;
 int i;
 check_alias(args,dir_aliases);
 for(i=0;i<6;i++){
  if(EQ(args,exit_names[i])){
   if(r->exits[i]){
    if(!p->AP){
     sendp(p,"You are too weak to move."N);
     return;
    }
    p->AP-=2;if(p->AP<0)p->AP=0;
    sayf(p,p,"%s goes %s."N,p->Name,args);
    moveobj(&p->o,(obj*)r->exits[i]);
    sayf(p,p,"%s arrives."N,p->Name);
    do_look(p,"");
    rvoke(p);/* revoke runes */
   } else sendp(p,"You can't move that way."N);
   return;
  }
 }
 sendp(p,"Where?"N);
}

CMD(do_quit){sendp(p,"Thanks for playing!"N);p->state=2;}

CMD(do_score){
 int i;
 sendpf(p,"You are %s, currently targeting %s."N,p->Name,
	p->tar?p->tar->Name:"no-one");
 sendpf(p,"You have %d HP, %d AP, sword+%d and %d kills"N,
	p->HP,p->AP,p->sp,p->kills);
 sendp(p,"Your runes:");
 for(i=0;i<NRUNES;i++)
  sendpf(p,"    %s+%d  %d  %d",(runes+i)->Name+8,
	p->runep[i],p->runes[i],p->irunes[i]);
 sendp(p,N);
}

CMD(do_inv){
 sendpf(p,"You are carring:"N"%s's Sword+%d"N"%s's Shield"N,
	p->Name,p->sp,p->Name);
}

CMD(do_say){
 sendpf(p,"You say: %s"N,args);
 sayf(p,p,"%s says: %s"N,p->Name,args);
}

CMD(do_shout){
 if(!args[0]){sendp(p,"Shout what?"N);return;}
 sendpf(p,"You shout: %s"N,args);
 shout(p,p->Name);shout(p," shouts: ");shout(p,args);shout(p,N);
}

CMD(do_who){
 plr*w;
 int i;
 sendp(p,"Playing:"N);
 PLOOP(w)if(w->state)sendpf(p,"%4d: %-16s(%d kills)"N,w->id,w->Name,w->kills);
}

#define INROOM(a,b) ((a)->o.parent==(b)->o.parent)

int validtar(plr*tar,plr*p){
 if(!tar)return 0;
 if(tar==p)return 0;
 if(tar->state!=1)return 0;
 if(!INROOM(tar,p))return 0;
 return 1;
}

CMD(do_target){
 plr*o;
 int i,t=atoi(args);
 if(!*args){
  /* find another player in the room */
  t=0;/* this could be removed? */
  PLOOP(o)if(validtar(o,p)){t=i+1;break;}
 } else if(t==0){
  /* try to match the name */
  plr*x;
  PLOOP(x)if(validtar(x,p)&&EQ(args,x->name)){t=x->id;break;}
 }
 /* find the id */
 if(t==p->id)t=0;
 if(t>0&&t<=MAXPLAY){
  p->tar=players+t-1;
  if(validtar(p->tar,p)){
   sendpf(p,"You target %s."N,p->tar->Name);
   sendpf(p->tar,"You are targetted by %s."N,p->Name);
  }else sendp(p,"Bad target."N);
 }else{
  p->tar=NULL;
  sendp(p,"You stop targeting."N);
 }
 rvoke(p);/* reset runes */
}

CMD(do_invoke) {
 int i;
 for(i=0;i<NRUNES;i++)if(EQ(args,(runes+i)->name)){/* find rune */
  if(p->runes[i]==0){
   sendpf(p,"You don't have a %s available."N,(runes+i)->Name);
   return;
  }
  if(p->AP==0){
   sendp(p,"You don't have enough AP"N);
   return;
  }
  /* invoke the rune */
  p->runes[i]--;
  p->irunes[i]++;
  p->AP-=(runes+i)->cost;
  if(p->AP<0)p->AP=0;
  sendpf(p,"You invoke %s"N,(runes+i)->Name+8);
  sayf(p,p,"%s invokes %s"N,p->Name,(runes+i)->Name+8);
  return;
 }
 sendp(p,"Which rune?"N);
}

CMD(do_revoke){sendp(p,"You revoke your runes."N);rvoke(p);}

int userunes(plr*p,int r){
 /* spend the runes if the player has them invoked */
 int i;
 for(i=0;i<NRUNES;i++)if((r&(1<<i))&&p->irunes[i]==0)return 0;
 for(i=0;i<NRUNES;i++)if(r&(1<<i)){p->irunes[i]--;p->runes[i]++;}
 return 1;
}

int bonus(int*p,int r){
 /* calculate the rune bonuses */
 int i,b=0;
 for(i=0;i<NRUNES;i++)if(r&(1<<i))b+=p[i];
 return b;
}

void hit(plr*p,int dam,plr*src){
 room*r;
 int i;
 if(dam<=p->pro){p->pro-=dam;return;}
 dam-=p->pro;p->pro=0;
 p->HP-=dam;
 sendpf(p,"You are wounded by %s."N,src->Name);
 sendpf(src,"You wound %s."N,p->Name);
 sayf(src,p,"%s wounds %s."N,src->Name,p->Name);
 if(p->HP<=0){
  sendpf(p,"You died.  %s killed you."N,src->Name);
  sendpf(src,"You killed %s."N,p->Name);
  sayf(src,p,"%s killed %s."N,src->Name,p->Name);
  src->kills++;
  p->HP=12+ran(7);
  rvoke(p);
  for(i=0;i<NRUNES;i++){
   if(p->runes[i]>0&&ran(2)==1)p->runes[i]--;
   if(p->runep[i]>0&&ran(2)==1)p->runep[i]--;
  }
  if(p->sp>0&&ran(2)==1)p->sp--;
  r=rooms+ran(NROOMS);
  moveobj(&p->o,&r->o);
  sayf(p,p,"%s appears."N,p->Name);
 }
}

CMD(do_cast){
 spell*s=spells;
 int i;
 for(i=0;i<NSPELLS;i++,s++)if(EQ(args,s->name)){
  if(!userunes(p,s->req)){
   sendp(p,"You don't have enough runes."N);
   return;
  }
  if(s->pro>0){/* protection spell */
   if(p->pro<s->pro)p->pro+=s->pro;
   sendp(p,"You feel protected."N);
   sayf(p,p,"%s casts %s."N,p->Name,s->name);
   return;
  }
  if(!validtar(p->tar,p)){
   sendp(p,"Your misdirected spell is wasted."N);
   return;
  }
  /* cast it! */
  sendpf(p,"You cast %s at %s."N,s->Name,p->tar->Name);
  sendpf(p->tar,"%s casts %s at you."N,p->Name,s->Name);
  sayf(p,p->tar,"%s casts %s at %s."N,p->Name,s->Name,p->tar->Name);
  /* apply the damage if not protected */
  if(!userunes(p->tar,s->neg))hit(p->tar,s->dam+bonus(p->runep,s->req),p);
  if(s->ar){/* area spell does half damage to other players in the room */
   plr*t;
   PLOOP(t)if(t->state==1&&t!=p&&t!=p->tar&&INROOM(t,p)&&!userunes(t,s->neg))
    hit(t,s->dam/2,p);
  }
  return;
 }
 sendp(p,"Cast what?"N);
}

int defend(plr*p,int n){
 /* returns true if any of the attack flags are set */
 int i;
 for(i=0;i<3;i++)if(p->att[i]>=0&&((1<<p->att[i])&n))return 1;
 return 0;
}

int defendall(plr*p,int n){
 /* returns true if all of the attack flags are set */
 int i,f=0;
 for(i=0;i<3;i++)if(p->att[i]>=0)f|=1<<p->att[i];
 return(f&n)==n;
}

combo*getcombo(int*att){
 combo*c=combos;
 int i;
 for(i=0;i<NCOMBO;i++,c++)if(att[0]==c->req[1]&&att[1]==c->req[0])
  {att[0]=att[1]=-1;return c;}
  /*{att[2]=att[1];att[1]=att[0];return c;}*/
 return NULL;
}

CMD(do_attack){
 attack*a=attacks;
 combo*c;
 int i,d;
 for(i=0;i<NATT;i++,a++)if(EQ(args,a->name)){
  if(!validtar(p->tar,p)){sendp(p,"You aren't attacking anyone."N);return;}
  if(p->AP>0){
   p->AP-=a->cost;
   if(p->AP<0)p->AP=0;
   if(p->att[0]>=0){p->att[2]=p->att[1];p->att[1]=p->att[0];}
   p->att[0]=i;
   d=a->dam;
   if(d>0){
    sendpf(p,"You %s at %s"N,args,p->tar->Name);
    sendpf(p->tar,"%s %s%s at you."N,p->Name,args,a->s);
    sayf(p,p->tar,"%s %s%s at %s."N,p->Name,args,a->s,p->tar->Name);
    if(!defend(p->tar,a->neg))hit(p->tar,a->dam+p->sp,p);
    else{
     sendp(p,"Your attack is blocked."N);
     sendp(p->tar,"You block the attack."N);
     sayf(p,p->tar,"%s blocks the attack."N,p->tar->Name);
    }
   }else{/* must be defensive */
    sendp(p,"You block."N);
    sayf(p,p,"%s blocks."N,p->Name);
   }
   if(!validtar(p->tar,p))return;
   c=getcombo(p->att);
   if(c){
    sendpf(p,"You attack %s with a %s."N,p->tar->Name,c->Name);
    sendpf(p->tar,"%s attacks you with a %s."N,p->Name,c->Name);
    sayf(p,p->tar,"%s attacks %s with a %s."N,p->Name,p->tar->Name,c->Name);
    if(!defendall(p->tar,c->neg))hit(p->tar,c->dam+p->sp,p);
    else{
     sendp(p,"Your attack is blocked."N);
     sendp(p->tar,"You block the attack."N);
     sayf(p,p->tar,"%s blocks the attack."N,p->tar->Name);
    }
   }
  }else sendp(p,"You are too weak."N);
  return;
 }
 sendp(p,"Attack how?"N);
}

CMD(do_help){sendp(p,"See readme.txt"N);}

int cnt=0;

void timer(void){
 room*r;
 plr*p;
 int i,t;
 char*b,*be;
 /* update powerups in all roomes */
 for(i=0,r=rooms;i<NROOMS;i++,r++)if(r->ptype&&(r->pcnt>0?--r->pcnt==0:1)){
  p=(plr*)r->o.child;
  if(p){
   be=" Bonus";
   /* determine powerup type */
   switch(r->ptype){
    case 1:t=ran(NRUNES);p->runes[t]++;b=(runes+t)->Name;be="";break;
    case 2:p->HP+=ran(3)+1;b="Health";break;
    case 3:if(p->sp<3)p->sp++;b="Sword";break;
    case 4:t=ran(NRUNES);if(p->runep[t]<2)p->runep[t]++;b=(runes+t)->Name;
   }
   sendpf(p,"You pick up a %s%s."N,b,be);
   sayf(p,p,"%s picks up a %s%s."N,p->Name,b,be);
   r->pcnt=r->prep;
  }
 }
 /* update players every 3 seconds */
 cnt++;
 if(cnt>2){
  cnt=0;
  PLOOP(p)if(p->state==1){
   /* increase AP */
   p->AP+=(p->HP+7)/8;if(p->AP>p->HP)p->AP=p->HP;
   /* maintain invoked runes */
   for(t=0;t<NRUNES;t++)if(p->irunes[t]){
    if(p->AP>0){
     p->AP-=(runes+t)->maint*p->irunes[t];
     if(p->AP<=0)p->AP=0;
    }else{p->runes[t]+=p->irunes[t];p->irunes[t]=0;}
   }
   /* Cycle through attack history */
   p->att[2]=p->att[1];
   p->att[1]=p->att[0];
   p->att[0]=-1;
  }
 }
}

char*aliases="n=go n;s=go s;e=go e;w=go w;u=go u;d=go d;sc=score;"
"ln=look n;ls=look s;le=look e;lw=look w;lu=look u;ld=look d;"
"ia=invoke air;ie=invoke earth;iw=invoke water;if=invoke fire;"
"cm=cast magic missile;cs=cast stone skin;ci=cast ice shards;"
"cp=cast poison cloud;cf=cast fireball;al=attack lunge;"
"ah=attack hack;as=attack slash;ac=attack chop;ab=attack block;"
"r=revoke;l=look;";

#define NCOMMANDS 15

struct{char*str;void(*fn)(plr*,char*);}commands[NCOMMANDS]={
 {"look",do_look},{"score",do_score},{"go",do_go},
 {"quit",do_quit},{"say",do_say},{"shout",do_shout},{"who",do_who},
 {"invoke",do_invoke},{"target",do_target},{"cast",do_cast},
 {"attack",do_attack},{"t",do_target},{"help",do_help},
 {"revoke",do_revoke},{"i",do_inv}};

char*match(char*s,char*m){
 while(*s&&*s==*m){s++;m++;}
 if(*s)return NULL;
 if(*m){if(*m==' ')m++;else return NULL;}
 return m;
}

void command(plr*p){
 int i;
 char*args;
 check_alias(p->line,aliases);
 for(i=0;i<NCOMMANDS;i++){
  args=match((commands+i)->str,p->line);
  if(args){(commands+i)->fn(p,args);return;}
 }
 sendp(p,"What?"N);
}

void read_line(plr*p,char ch){
 int len;
 len=strlen(p->line);
 if(ch=='\n'||ch=='\r'){
  if(len>0)command(p);
  p->line[0]=0;
  if(ch=='\r')sendp(p,PROMPT);
 }else if(ch==' '){
  if(len>0&&len<127&&p->line[len-1]!=' ')append(p->line,ch);
 }else if(len<127)append(p->line,ch);
}

plr*checkp(char*name){
 plr*p;
 int i;
 PLOOP(p)if(name!=p->name&&EQ(name,p->name))return p;
 return NULL;
}

void new_connect(plr*p,char ch){
 ch=tolower(ch);
 if(ch>='a'&&ch<='z'){
  if(strlen(p->name)<15)append(p->name,ch);
 }else if(ch=='\n'){
  if(strlen(p->name)<3){
   p->name[0]=0;
   sendp(p,"Too short."N"Name: ");
  }else if(checkp(p->name)){
   p->name[0]=0;
   sendp(p,"That name is already used."N"Name: ");
  }else{
   strcpy(p->Name,p->name);
   p->Name[0]+='A'-'a';
   sendpf(p,"Welcome %s."N,p->Name);
   shout(p,p->Name);shout(p," joins the game."N);
   p->o.parent=p->o.child=p->o.next=NULL;
   moveobj(&p->o,&rooms->o);
   sayf(p,p,"%s appears!"N,p->Name);
   do_look(p,"");
   sendp(p,PROMPT);
   p->ip=read_line;
   p->line[0]=0;
  }
 }
}

int quit_time=0;

void sint(int sig){
 quit_time=1;
}

int main(int argc,char*argv[]){

 char ch;
 short port;
 int i,j,r,rsock,lsock,t0,t1;
 struct sockaddr_in sname;
 struct sockaddr*sockptr=(struct sockaddr*)&sname;
 plr*p;

 if(argc!=2){
  puts("Usage: tmd <port>");
  return 1;
 }

 port=atoi(argv[1]);

 signal(SIGPIPE,SIG_IGN);
 signal(SIGINT,sint);

 PLOOP(p){p->id=i+1;p->state=0;}

 lsock=socket(AF_INET,SOCK_STREAM,0);
 if(lsock==-1)error("creating lsock");

 sname.sin_family=AF_INET;
 sname.sin_port=htons(port);
 sname.sin_addr.s_addr=INADDR_ANY;

 i=1;
 setsockopt(lsock,SOL_SOCKET,SO_REUSEADDR,(char*)&i,sizeof(i));

 if(bind(lsock,sockptr,sizeof(sname))==-1)error("binding");

 if(listen(lsock,5)==-1)error("listening");

 printf("Playing on port %d\n",port);

 t1=time(NULL);

 while(!quit_time){

  /* setup poll structure */
  j=0;
  PLOOP(p){
   if(p->state>0){
    p->p=pdata+j++;
    p->p->fd=p->fd;
    p->p->events=POLLIN;
    if(p->blen>0)p->p->events|=POLLOUT;
    p->p->revents=0;/* poll() on Red Hat 5.0 likes this */
   }
   if(p->state==2&&p->blen==0)shutdown(p->fd,2);
  }

  /* don't forget the listening socket */
  (pdata+j)->fd=lsock;
  (pdata+j)->events=POLLIN;

  /* poll! */
  r=poll(pdata,j+1,1000);

  PLOOP(p)if(p->state){

   /* Check for dropped links */
   if((p->p->revents&POLLERR)||(p->p->revents&POLLHUP)){
    moveobj((obj*)p,NULL);
    shout(NULL,p->Name);
    shout(NULL," just quit!"N);
    p->state=0;
    close(p->fd);
    puts("Connection closed");
    continue;
   }

   /* check for pending characters */
   if(p->p->revents&POLLIN){
    recv(p->fd,&ch,1,0);
    p->ip(p,ch);
   }

   /* check for available output space */
   if(p->p->revents&POLLOUT){
    send(p->fd,p->op+p->bpos,1,0);
    p->blen--;
    if(p->blen==0)p->bpos=0;
    else p->bpos=(p->bpos+1)%MAXBUF;
   }

  }

  /* Check for new connections */
  if((pdata+j)->revents&POLLIN){
   i=sizeof(sname);
   rsock=accept(lsock,sockptr,&i);
   if(i==-1)error("accepting");
   printf("Connect from %s\n",inet_ntoa(sname.sin_addr));
   PLOOP(p)if(p->state==0){
    init_plr(p);
    p->state=1;
    p->name[0]=0;
    p->fd=rsock;
    p->blen=p->bpos=0;
    p->ip=new_connect;
    sendp(p,MOTD);
    sendp(p,"Name: ");
    break;
   }
   if(i==MAXPLAY){
    send(rsock,TOOMANY,strlen(TOOMANY),0);
    shutdown(rsock,2);
   }
  }

  /* update 1sec timer */
  t0=t1;t1=time(NULL);if(t1>t0)timer();

 }/* while (!quit_time) */

 /* Close it all down */
 PLOOP(p)if(p->state){shutdown(p->fd,2);close(p->fd);}

 close(lsock);

 return 0;/* bye bye! */

}

