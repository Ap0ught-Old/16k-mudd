/* 
 * protoMUD
 *
 * Copyright (c) 2000 Blane Bramble and Digital Biscuit Technology
 *
 * Written for the 16k MUD competition
 *
 * Created:			 5th April 2000
 * Last updated: 	25th April 2000
 *
 * This program was compiled on RedHat Linux 6.0
 *
 * Warning: Mixture of sloppy and tight code follows. Try and understand
 * it at your own risk!
 *
 * License: non-commercial use of this code, or it's derivatives is
 * fine as long as it is credited - within the source code if the
 * source code is publically available, or within the copyright or
 * credits message if the source code is not available. Commercial
 * usage is forbidden without a specific license.
 *
 * Email to: 	blane@adonisystems.co.uk
 *	 			geolin@iowa-mug.net
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <signal.h>

/* AllocateMEMory - allocate a block of mem of the correct size and typecast */
#define amem(xx) (xx*)malloc(sizeof(xx))

/* Is the line currently in the game? InGame */
#define IG(xx) (ln[xx].ol==100)

/* Define a verb in the verb table */
#define VB(xx) {#xx,v##xx},

/* It's suprising just how much I use for(;;) */
#define LP(vv,nn) for(vv=0;vv<nn;vv++)

/* Max input length */
#define MI 200
/* Max players at once */
#define MU 100
/* Max name length */
#define MN 20

/* Some standard messages */
#define AV " has just arrived.\n"
#define LV " has just left.\n"
#define CANT(xx) "You can't " #xx " that!\n"

/* annoying but useful to save space */
#define i int
#define l long
#define c char
#define cp char *
#define v void
#define vp void *
#define r return

#define cmp strcasecmp
#define cpy strcpy

#define tds typedef struct

#define FDS FD_SET
#define FDIS FD_ISSET

/* PoinTer - holds a text tag and a pointer to the instance for that tag */
tds PT {
	c nm[MN];
	vp pt;
	i ty;
	};

/* Verb Table - pointer to a verb and the function to call */
tds VT {
	cp vb;
	v (*fn)(i,cp);
	};

/* Output BuFfer - pointer to the buffered text and link to next list item */
tds BF {
	cp txt;
	BF*nxt;
	};

/* PLayer */
tds PL {
	c nm[MN];
	c pw[MN];
	l sc;
	i sl;
	i hp[2];
	PT lc;
	vp wp;
	vp fi;
	PL*nxt;
	};

/* LoCation */
tds LC {
	c nm[MN];
	PT ex[12];
	c*sd,*ld;
	LC*nxt;
	};

/* OBject */
tds OB {
	c nm[MN];
	PT lc,rl;
	vp fi;
	c*sd,*ld;
	i vl,pw,mv[2],ag[2],rs[2],hp[2];
	OB*nxt;
	};

/* LiNe - online status */
tds LN {
	c nm[MN];
	BF*bf;
	i so,ol,ln;
	PL*pl;
	};

PL*pl=NULL;
LC*lc=NULL;
OB*ob=NULL;

LN ln[MU];

/* Control Socket */
i cs;
i run;

/* Ticker count */
i tc=5;

/* Timer counts */
l sec,psec;

/* Paired as OPPOSTIES. This is important */
cp xt[]={
	"north","south","east","west",
	"ne","sw","nw","se",
	"up","down",
	"in","out",
	NULL
	};

/*
 * 			Game Support Routines
 *
 * These are the routines required by the actual game code
 *
 */
i lup(cp a[],cp s)
{
i n=0;
while(a[n]!=NULL)
	{
	if(cmp(a[n],s)==0)
		r n;
	n++;
	}
r -1;
}

/* Remove the first word from a line of input, and return a pointer to the
 * string following the word
 */
cp word(cp s,cp w)
{
while(isspace(*s))
	s++;
while(!isspace(*s)&&*s!=0)
	*w++=*s++;
while(isspace(*s))
	s++;
*w=0;
r s;
}

/*
 * Capitilise the first letter of a word, lowercase the rest
 *
 */
v name(cp s)
{
if(*s==0)
	r;
*s=toupper(*s);
s++;
while(*s++)
	*s=tolower(*s);
}

/*
 * This routine takes a PoinTer item, and looks up the text portion of
 * it in the list of locations, objects and players. If the text is
 * matched, it will set the pointer portion to point to the matching
 * item and return 1 for a location, 2 for an object or 3 for a player
 *
 * Why? Because we use pointers within the game (a players current
 * location is a pointer to that location), but need a non changing
 * tag for saving and loading.
 */
i res(PT*pt)
{
LC*lcl=lc;
OB*lob=ob;
if(pt==NULL)
	r 0;
while(lcl!=NULL)
	{
	if(cmp(lcl->nm,pt->nm)==0)
		{
		pt->pt=(vp)lcl;
		r 1;
		}
	lcl=lcl->nxt;
	}
while(lob!=NULL)
	{
	if(cmp(lob->nm,pt->nm)==0)
		{
		pt->pt=(vp)lob;
		r 2;
		}
	lob=lob->nxt;
	}
r 0;
}

/* De-resolve
 *
 * Opposite of resolve - sets the name to the text associated with
 * the instance pointer. Relies on the name string being the first item
 * in the relevant structure.
 *
 */
v deres(PT*pt)
{
cpy(pt->nm,(cp)pt->pt);
}

/*
 * This routine takes a PoinTer item and a string and resolves the string
 * using the routine above.
 *
 */
i resn(PT*pt,cp s)
{
strncpy(pt->nm,s,MN);
r res(pt);
}


/*
 * This is the main output routine - buffers output into a linked list for
 * later processing.
 *
 */
v tx(i n,cp s)
{
if(ln[n].ol)
	{
	BF*nw;
	nw=amem(BF);
	nw->txt=strdup(s);
	nw->nxt=NULL;
	if(ln[n].bf==NULL)
		ln[n].bf=nw;
	else
		{
		BF*ll=ln[n].bf;
		while(ll->nxt!=NULL)
			ll=ll->nxt;
		ll->nxt=nw;
		}
	}
}

/*
 * Transmit a number to a player.
 *
 */
v txn(i n,l vl)
{
static c bf[20];
sprintf(bf,"%ld",vl);
tx(n,bf);
}


v room(LC*ll,cp s)
{
i n;
LP(n,MU)
	{
	if(IG(n)&&ln[n].pl->lc.pt==ll)
		tx(n,s);
	}
}

v room2(LC*ll,LN*l2,cp s1,cp s2)
{
i n2;
LP(n2,MU)
	{
	if(IG(n2)&&ln[n2].pl->lc.pt==ll)
		{
		if(&ln[n2]==l2)
			tx(n2,s1);
		else
			tx(n2,s2);
		}
	}
}

OB*newob(cp snm,cp sst,cp slt,i vl,i rs)
{
OB*nw,*ll=ob;
i n;
nw=amem(OB);
cpy(nw->nm,snm); name(nw->nm);
nw->sd=strdup(sst);
nw->ld=strdup(slt);
nw->mv[0]=nw->ag[0]=0;
nw->vl=vl; nw->rs[0]=rs;
nw->nxt=NULL;
if(ll==NULL)
	ob=nw;
else
	{
	while(ll->nxt!=NULL)
		ll=ll->nxt;
	ll->nxt=nw;
	}
r nw;
}

v newlc(cp snm,cp sst,cp slt)
{
LC*nw,*ll;
i n;
PT pt;
if(resn(&pt,snm))
	r;
nw=amem(LC);
cpy(nw->nm,snm); name(nw->nm);
nw->sd=strdup(sst);
nw->ld=strdup(slt);
nw->nxt=NULL;
LP(n,12)
	{
	*nw->ex[n].nm = 0;
	nw->ex[n].pt=NULL;
	}
if((ll=lc)==NULL)
	lc=nw;
else
	{
	while(ll->nxt!=NULL)
		ll=ll->nxt;
	ll->nxt=nw;
	}
}

i setx(cp l1,cp x,cp l2)
{
PT p1;
LC*ll;
i n;
if(resn(&p1,l1)!=1)
	r 0;
ll=(LC*)p1.pt;
LP(n,12)
	{
	if(cmp(xt[n],x)==0)
		{
		resn(&ll->ex[n],l2);
		r 1;
		}
	}
r 0;
}

i setx2(cp l1,cp x,cp l2)
{
if(setx(l1,x,l2))
	r setx(l2,xt[lup(xt,x)^1],l1);
r 0;
}

v setwp(OB*wp,i pw)
{
wp->pw=pw;
}

v setmb(OB*mb,i mv,i ag,i hp)
{
mb->mv[0]=mb->mv[1]=mv;
mb->ag[0]=mb->ag[1]=ag;
mb->hp[0]=mb->hp[1]=hp;
}

i xcnt(LC*ll)
{
i n,n2=0;
LP(n,12)
	{
	if(ll->ex[n].pt!=NULL)
		n2++;
	}
r n2;
}

i pcnt(vp lc)
{
i n,n2=0;
LP(n,MU)
	{
	if(IG(n)&&ln[n].pl->lc.pt==lc)
		n2++;
	}
r n2;
}

cp a_an(cp s)
{
c c2;
if((c2=toupper(*s))=='A'||c2=='E'||c2=='I'||c2=='O'||c2=='U')
	r "an ";
else
	r "a ";
}

i objhere(i n,cp pre,vp pt)
{
OB*ll=ob;
i n2=0;
while(ll!=NULL)
	{
	if(ll->lc.pt==pt)
		{
		if(n2++==0)
			tx(n,pre);
		else
			tx(n,", ");
		tx(n,a_an(ll->sd)); tx(n,ll->sd);
		}
	ll=ll->nxt;
	}
if(n2!=0)
	tx(n,".\n");
r n2;
}

v look(i n)
{
LC*lc1;
i n2;
lc1=(LC*)(ln[n].pl->lc.pt);
tx(n,lc1->nm); tx(n,": "); tx(n,lc1->sd); tx(n,"\n");
tx(n,lc1->ld); tx(n,"\n");
if((n2=xcnt(lc1))==0)
	tx(n,"There are no exits!\n");
else
	{
	i n3;	
	if(n2==1)
		tx(n,"The only exit is ");
	else
		tx(n,"Exits lead ");
	LP(n3,12)
		{
		if(lc1->ex[n3].pt!=NULL)
			{
			tx(n,xt[n3]);
			if(--n2==1)
				tx(n," and ");
			else if(n2!=0)
				tx(n,", ");
			}
		}
	tx(n,".\n");
	}
LP(n2,MU)
	{
	if(IG(n2)&&ln[n2].pl->lc.pt==lc1&&n2!=n)
		{
		tx(n,ln[n2].nm);
		tx(n," is here.\n");
		}
	}
objhere(n,"You can see ",(vp)lc1);
}

/*
 * Log a player into the game
 *
 */
v login(i n)
{
ln[n].pl->wp=ln[n].pl->fi=NULL;
ln[n].pl->sl=0;
cpy(ln[n].nm,ln[n].pl->nm);
tx(n,"Welcome "); tx(n,ln[n].pl->nm);
tx(n," you are logged in. HELP for help.\n\n");
if(res(&ln[n].pl->lc)==0)
	resn(&ln[n].pl->lc,"start");
look(n);
tx(n,"* ");
}

v logout(i n)
{
OB*ll=ob;
ln[n].pl->wp=ln[n].pl->fi=NULL;
while(ll!=NULL)
	{
	if(ll->lc.pt==ln[n].pl)
		{
		ll->lc.pt=ln[n].pl->lc.pt;
		deres(&ll->lc);
		}
	ll=ll->nxt;
	}
tx(n,"Logged out\n");
ln[n].ol=0;
ln[n].pl=NULL;
}


i fight(OB*at,OB*df)
{
i t,a,d;
if(at==NULL)
	a=1;
else
	a=1+at->pw;
if(df==NULL)
	d=1;
else
	d=1+df->pw;
t=10+a+d;
if((rand()%t)<(5+a))
	r (1+(rand()%a));
r -(1+(rand()%d));
}

/*
 * Produce a welcome banner for new connection
 *
 */
v welcome(i n)
{
ln[n].ol=1;
tx(n,"\n\nWelcome to protoMUD\n\n\n");
tx(n,"Please enter your name: ");
}

/*
 * Player vs Mobile combat routines
 *
 */
v pvm(i n)
{
PL*pl=ln[n].pl;
OB*mb=(OB*)pl->fi;
i dm;
LC*lc=(LC*)mb->lc.pt;
if(pl->lc.pt!=lc)
	{
	pl->fi=NULL;
	r;
	}
if((dm=fight((OB*)pl->wp,mb))>0)
	{
	room(lc,"\n");
	room2(lc,&ln[n],"You",pl->nm);
	room2(lc,&ln[n]," hit the "," hits the ");
	room(lc,mb->sd);
	room(lc,".\n");
	mb->fi=(vp)&ln[n];
	mb->hp[1]-=dm;
	ln->pl->sc+=(dm*mb->pw);
	if(mb->hp[1]<=0)
		{
		room(lc,"The "); room(lc,mb->sd); room(lc," is dead!\n");
		resn(&mb->lc,"limbo");
		pl->sc+=(mb->pw*mb->pw);
		pl->fi=NULL;
		mb->fi=NULL;
		}
	}
else
	tx(n,"You miss.\n");
}

/* Mobile vs Player fight routine
 */
v mvp(OB*mb)
{
i dm;
LC*lc=(LC*)mb->lc.pt;
LN*ln=((LN*)mb->fi);
PL*pl=ln->pl;
if(pl==NULL||pl->lc.pt!=lc)
	{
	mb->fi=NULL;
	r;
	}
room(lc,"\n");
if((dm=fight(mb,(OB*)(pl->wp)))>0)
	{
	room(lc,"The "); room(lc,mb->sd);
	room(lc," hits ");
	room2(lc,ln,"you",ln->nm);
	room(lc,".\n");
	pl->fi=mb;
	pl->hp[1]-=dm;
	if(pl->hp[1]<=0)
		{
		room2(lc,ln,"You",ln->nm); room(lc," died!\n");
		pl->sc/=2;
		pl->hp[1]=pl->hp[0];
		logout(ln->ln);
		welcome(ln->ln);
		pl->fi=NULL;
		mb->fi=NULL;
		}
	}
}


/*
 *			Game Verb Functions
 *
 * To avoid forward references, this is where all the game
 * verbs will be declared.
 *
 */

extern VT vt[];

v vlook(i n,cp s)
{
look(n);
}

v vquit(i n,cp s)
{
logout(n);
welcome(n);
}

v vhelp(i n,cp s)
{
i n2=0;
tx(n,"Supported commands:\n");
while(vt[n2].vb!=NULL)
	{
	tx(n,vt[n2].vb);
	tx(n,"\t");
	n2++;
	}
tx(n,"\n");
}

v vshutdown(i n,cp s)
{
i n2;
LP(n2,MU)
	if(IG(n2))
		logout(n2);
run=0;
}

v vscore(i n,cp s)
{
PL*pl=ln[n].pl;
tx(n,"Name:  "); tx(n,ln[n].nm);
tx(n,"\nScore: "); txn(n,pl->sc);
tx(n,"\nHP:    "); txn(n,pl->hp[1]); tx(n,"/"); txn(n,pl->hp[0]);
if(pl->fi!=NULL)
	{
	tx(n,"\nYou are fighting the ");
	tx(n,((OB*)pl->fi)->sd);
	}
tx(n,"\n");
}

v vwho(i n,cp s)
{
i n2;
tx(n,"Players:\n");
LP(n2,MU)
	{
	if(IG(n2))
		{
		tx(n,ln[n2].nm);
		tx(n,"\n");
		}
	}
}


v vinventory(i n,cp s)
{
if(objhere(n,"You are holding ",(vp)ln[n].pl)==0)
	tx(n,"You have nothing.\n");
if(ln[n].pl->wp!=NULL)
	{
	tx(n,"You are wielding the ");
	tx(n,((OB*)ln[n].pl->wp)->sd);
	tx(n,"\n");
	}
}

v voffer(i n,cp s)
{
c w[MI];
PT pt,p2;
s=word(s,w); name(w);
if(resn(&pt,w)!=2||((OB*)pt.pt)->lc.pt!=ln[n].pl)
	tx(n,CANT(offer));
else if(cmp(((LC*)ln[n].pl->lc.pt)->nm,"temple")==0)
	{
	resn(&((OB*)pt.pt)->lc,"limbo");
	tx(n,"You offer up the "); tx(n,((OB*)pt.pt)->sd); tx(n,".\n");
	ln[n].pl->sc+=((OB*)pt.pt)->vl;
	}
else
	tx(n,"Nothing happens.\n");
}

v vwield(i n,cp s)
{
c w[MI];
PT pt;
s=word(s,w); name(w);
if(resn(&pt,w)!=2||((OB*)pt.pt)->lc.pt!=ln[n].pl&&!((OB*)pt.pt)->pw)
	tx(n,CANT(wield));
else
	{
	ln[n].pl->wp=pt.pt;
	tx(n,"You wield the "); tx(n,((OB*)pt.pt)->sd); tx(n,".\n");
	}
}

v vdrop(i n,cp s)
{
c w[MI];
PT pt;
s=word(s,w); name(w);
if(resn(&pt,w)!=2||((OB*)pt.pt)->lc.pt!=ln[n].pl)
	tx(n,CANT(drop));
else
	{
	((OB*)pt.pt)->lc.pt=ln[n].pl->lc.pt;
	deres(&((OB*)pt.pt)->lc);
	tx(n,"You drop the "); tx(n,((OB*)pt.pt)->sd); tx(n,".\n");
	}
}


v vkill(i n,cp s)
{
c w[MI];
PT pt;
s=word(s,w); name(w);
if(resn(&pt,w)!=2||((OB*)pt.pt)->lc.pt!=ln[n].pl->lc.pt||!((OB*)pt.pt)->mv[0])
	tx(n,CANT(kill));
else if(ln[n].pl->fi!=NULL)
	tx(n,"One fight at a time\n");
else if(ln[n].pl->wp==NULL)
	tx(n,"You need a weapon\n");
else
	{
	tx(n,"You attack the "); tx(n,((OB*)pt.pt)->sd); tx(n,".\n");
	ln[n].pl->fi=pt.pt;
	}
}

v vsleep(i n,cp s)
{
if(ln[n].pl->fi!=NULL)
	tx(n,"You are fighting!\n");
else if(ln[n].pl->hp[1]<ln[n].pl->hp[0])
	{
	tx(n,"You sleep...\n");
	ln[n].pl->sl=1;
	}
else
	tx(n,"You don't need to.\n");
}

v vget(i n,cp s)
{
c w[MI];
PT pt;
s=word(s,w); name(w);
if(resn(&pt,w)!=2||((OB*)pt.pt)->lc.pt!=ln[n].pl->lc.pt||((OB*)pt.pt)->mv[0])
	tx(n,CANT(get));
else
	{
	((OB*)pt.pt)->lc.pt=ln[n].pl;
	deres(&((OB*)pt.pt)->lc);
	tx(n,"You take the "); tx(n,((OB*)pt.pt)->sd); tx(n,".\n");
	}
}

v vgo(i n,cp s)
{
c w[MI];
LC*ll,*od;
i n2;
if(ln[n].pl->fi!=NULL)
	{
	tx(n,"You are fighting!\n");
	r;
	}
s=word(s,w);
ll=(LC*)(ln[n].pl->lc.pt);
if((n2=lup(xt,w))!=-1&&ll->ex[n2].pt!=NULL)
	{
/* send message to room moving TO */
	od=(LC*)ll->ex[n2].pt;
	room(od,ln[n].pl->nm); room(od,AV);
	ln[n].pl->lc.pt=ll->ex[n2].pt;
/* send message to room moving FROM */
	room(ll,ln[n].pl->nm); room(ll,LV);
	look(n);
	}
else
	tx(n,"You can't go that way\n");
}

v vflee(i n,cp s)
{
if(ln[n].pl->fi==NULL)
	r;
ln[n].pl->sc/=4;
ln[n].pl->sc*=3;
ln[n].pl->fi=NULL;
vgo(n,s);
}

v vemote(i n,cp s)
{
c w[MI];
LN*ll=&ln[n];
LC*lc=(LC*)(ll->pl->lc.pt);
s=word(s,w);
room2(lc,ll,"You",ll->nm);
room(lc," ");
room(lc,w);
room2(lc,ll," ","s ");
room(lc,s);
room(lc,".\n");
}

v vsay(i n,cp s)
{
LN*ll=&ln[n];
LC*lc=(LC*)(ll->pl->lc.pt);
room2(lc,ll,"You",ll->nm);
room2(lc,ll," say '"," says '");
room(lc,s);
room(lc,"'.\n");
}

v vshout(i n,cp s)
{
i n2;
LP(n2,MU)
	{
	if(IG(n2)&&n2!=n)
		{
		tx(n2,"\n");
		tx(n2,ln[n].nm);
		tx(n2," shouts '"); tx(n2,s); tx(n2,"'.\n");
		}
	}
}

/*
 *			Game Parse table
 *
 * This is the parse/lookup table for verbs. It's global, but declared
 * after the functions to avoid wasteful forward references
 *
 */

VT vt[] = {
	VB(help)
	VB(shutdown)
	VB(who)
	VB(score)
	VB(look)
	VB(inventory)
	VB(get)
	VB(drop)
	VB(offer)
	VB(go)
	VB(quit)
	VB(kill)
	VB(wield)
	VB(sleep)
	VB(say)
	VB(shout)
	VB(emote)
	VB(flee)
	{NULL,NULL}
	};
/*
 *			Engine Support Routines
 *
 * Anything below here is probably more to do with the game
 * engine, input/output and initialisation etc.
 *
 */

/* Signal MANager */
v sman(i sg)
{
switch(sg)
	{
	case SIGTERM:
		vshutdown(0,"");	
		break;
	case SIGALRM:
		sec++;
		break;
	}
signal(sg,sman);
}

v mbmove(OB*mb)
{
i n,x;
LC*lc;
if(mb->fi!=NULL)
	r;
if((x=xcnt((LC*)mb->lc.pt))<1)
	r;
lc=(LC*)mb->lc.pt;
n=rand()%x;
LP(x,12)
	{
	if(lc->ex[x].pt!=NULL)
		{
		if(n--==0)
			{
			mb->lc.pt=lc->ex[x].pt;
			deres(&mb->lc);
			break;
			}
		}
	}
room(lc,"\nThe ");
room(lc,mb->nm); room(lc,LV);
room((LC*)mb->lc.pt,mb->nm); room((LC*)mb->lc.pt,AV);
}

v setrs(OB*ob,cp lc)
{
/* Set the current location and reset location */
resn(&ob->lc,lc);
resn(&ob->rl,lc);
}

v tick(v)
{
OB*ll=ob;
i n;
if(!tc--)
	tc=5;
while(ll!=NULL)
	{
	if(cmp(((LC*)ll->lc.pt)->nm,"limbo")==0)
		{
		if(--ll->rs[1]<=0)
			{
			LC*lc;
			lc=(LC*)ll->rl.pt;
			ll->lc.pt=(vp)lc;
			deres(&ll->lc);
			room(lc,"\nFrom nowhere ");
			room(lc,a_an(ll->sd)); room(lc,ll->sd);
			room(lc," appears.\n");
			ll->rs[1]=ll->rs[0];
			ll->hp[1]=ll->hp[0];
			}
		}
	if(ll->fi!=NULL)
		{
		if(!tc)
			mvp(ll);
		}
	else if(ll->ag[0])
		if(--ll->ag[1]<=0)
			{
			i n;
			ll->ag[1]=ll->ag[0];
			if((n=pcnt(ll->lc.pt))>0)
				{
				i n2=0;
				n=rand()%n;
				LP(n2,MU)
					{
					if(IG(n2)&&ln[n2].pl->lc.pt==ll->lc.pt)
						if(n--==0)
							{
							ll->fi=(vp)&ln[n2];
							tx(n2,"The ");
							tx(n2,ll->sd);
							tx(n2," attacks!\n");
							break;
							}
					}
				}
			}
	if(ll->mv[0])
		if(--ll->mv[1]<=0)
			{
			ll->mv[1]=ll->mv[0];
			mbmove(ll);
			}
	ll=ll->nxt;
	}
LP(n,MU)
	{
	if(!tc&&IG(n))
		{
		if(ln[n].pl->fi!=NULL)
			pvm(n);
		if(ln[n].pl->sl)
			{
			if(++ln[n].pl->hp[1]==ln[n].pl->hp[0])
				{
				ln[n].pl->sl=0;
				tx(n,"\nYou wake up!\n");
				}
			}
		}
	}
}

/* Setup default values and create a control socket to listen on */
i init(v)
{
i n;
l vl;
struct sockaddr_in sad;
struct itimerval tv={{0,1000000l},{0,1000000l}};
LP(n,MU)
	{
	ln[n].ln=n;
	ln[n].ol=0;
	ln[n].nm[0]=0;
	ln[n].so=-1;
	ln[n].bf=NULL;
	}

if((cs=socket(AF_INET,SOCK_STREAM,0))<0)
	r 0;

vl = 1;
ioctl(cs,FIONBIO,&vl);
sad.sin_family=AF_INET;
sad.sin_addr.s_addr=INADDR_ANY;
sad.sin_port=htons(4444);
if(bind(cs,(sockaddr*)&sad,sizeof(sad))<0)
	r 0;

if(listen(cs,5)!=0)
	r 0;

signal(SIGTERM,sman);
signal(SIGALRM,sman);
setitimer(ITIMER_REAL,&tv,NULL);
r 1;
}

/*
 * Initialise our world by creating a few locations etc.
 *
 */
v setup(v)
{
PT pt;
if (resn(&pt,"limbo")==0)
	{
	newlc("limbo","In limbo","");
	newlc("start","Gateway to another world","You stand before the gateway to another world.\n");
	newlc("mist","In the mist","In the mist");
	newlc("temple","Temple of Blot","Try OFFERing something.\n");
	newlc("field","Large field","It's a field, it's large.\n");
	newlc("path","Mud path","On a muddy path.\n");
	newlc("track","Small track","Well-worn track\n");

	setx("start","north","mist");
	setx2("mist","north","field");
	setx2("mist","east","track");
	setx2("track","north","path");
	setx2("track","south","temple");
	setx2("path","west","field");
	}

if (resn(&pt,"sword")==0)
	{
	OB*sw;
	sw=newob("sword","sharp sword","",500,30);
	setrs(sw,"mist"); setwp(sw,10);

	sw=newob("dragon","large dragon","",0,40);
	setmb(sw,20,10,40); setwp(sw,15);
	setrs(sw,"field");

	sw=newob("troll","small troll","",0,40);
	setmb(sw,10,20,60); setwp(sw,7);
	setrs(sw,"mist");

	sw=newob("axe","large axe","",300,30);
	setrs(sw,"field"); setwp(sw,8);
	}
}

/*
 * Write a string out by writing out the length of the string
 * followed by the string itself.
 *
 */
v fws(cp s,FILE*f)
{
i n;
n=strlen(s)+1; fwrite(&n,sizeof(n),1,f);
fwrite(s,n,1,f);
}

/*
 * Read in a string that has been written by FWS. Automatically
 * allocates the necessary memory.
 *
 */
cp frs(FILE*f)
{
i n;
cp s;
fread(&n,sizeof(n),1,f);
s=(cp)malloc(n);
fread(s,n,1,f);
r s;
}

/*
 * Load everything into memory!
 *
 */
v load(v)
{
FILE*fp;
if((fp=fopen("rooms.mm","r"))!=NULL)
	{
	LC*ll=lc;
	i n;
	while(!feof(fp))
		{
		LC*nw;
		nw=amem(LC);
		if(fread(nw,sizeof(LC),1,fp)<1)
			free(nw);
		else
			{
			nw->sd=frs(fp);
			nw->ld=frs(fp);
			if(ll==NULL)
				lc=nw;
			else
				ll->nxt=nw;
			nw->nxt=NULL;
			ll=nw;
			}
		}
	ll=lc;
	while(ll!=NULL)
		{
		LP(n,12)
			res(&ll->ex[n]);
		ll=ll->nxt;
		}
	fclose(fp);
	}
if((fp=fopen("players.mm","r"))!=NULL)
	{
	PL*ll=pl;
	while(!feof(fp))
		{
		PL*nw;
		nw=amem(PL);
		if(fread(nw,sizeof(PL),1,fp)<1)
			free(nw);
		else
			{
			res(&nw->lc);
			if(ll==NULL)
				pl=nw;
			else
				ll->nxt=nw;
			nw->nxt=NULL;
			ll=nw;
			}
		}
	fclose(fp);
	}
if((fp=fopen("objects.mm","r"))!=NULL)
	{
	OB*ll=ob;
	while(!feof(fp))
		{
		OB*nw;
		nw=amem(OB);
		if(fread(nw,sizeof(OB),1,fp)<1)
			free(nw);
		else
			{
			nw->fi=NULL;
			nw->sd=frs(fp);
			nw->ld=frs(fp);
			res(&nw->lc);
			if(ll==NULL)
				ob=nw;
			else
				ll->nxt=nw;
			nw->nxt=NULL;
			ll=nw;
			}
		}
	fclose(fp);
	}
}

/*
 * Save our game world out to disk.
 * 
 */
v save(v)
{
PL*ll=pl;
LC*lcl=lc;
OB*lob=ob;
FILE*fp;
i n;
fp=fopen("players.mm","w");
while(ll!=NULL)
	{
	fwrite(ll,sizeof(PL),1,fp);
	ll=ll->nxt;
	}
fclose(fp);
fp=fopen("rooms.mm","w");
while(lcl!=NULL)
	{
	fwrite(lcl,sizeof(LC),1,fp);
	fws(lcl->sd,fp);
	fws(lcl->ld,fp);
	lcl=lcl->nxt;
	}
fclose(fp);
fp=fopen("objects.mm","w");
while(lob!=NULL)
	{
	fwrite(lob,sizeof(OB),1,fp);
	fws(lob->sd,fp);
	fws(lob->ld,fp);
	lob=lob->nxt;
	}
fclose(fp);
}



/*
 * Parse an input line as appropriate for the current state of the user
 *
 */
v parse(i n,cp s)
{
i n2;
c w[MI];
PL*ll;
switch(ln[n].ol)
	{
	case 0:
		r;
	case 1:
		word(s,w); name(w);
		if(*w==0)
			{
			welcome(n);
			break;
			}
		strncpy(ln[n].nm,w,MN-1);
		ll=pl;
		while(ll!=NULL)
			{
			if(cmp(ll->nm,w)==0)
				{
				tx(n,"Password: ");
				ln[n].pl=ll;
				ln[n].ol=2;
				r;
				}
			ll=ll->nxt;
			}
		tx(n,"New player.\nEnter a password: ");
		ln[n].ol=10;
		r;
	case 2:			/* check password */
		word(s,w); name(w);	
		if(cmp(ln[n].pl->pw,w)==0)
			{
			i n2;
			LP(n2,MU)
				{
				if(n2!=n&&IG(n2)&&ln[n2].pl==ln[n].pl)
					{
					welcome(n);
					break;
					}
				}
			if(n2==MU)
				{
				login(n);
				ln[n].ol=100;
				}
			}
		else
			{
			tx(n,"Password failure\n");
			welcome(n);
			}
		r;
	case 10:		/* new player */
		{
		PL*nw,*ll=pl;
		word(s,w); name(w);
		if(*w==0)
			{
			welcome(n);
			r;
			}
		ln[n].pl=nw=amem(PL);
		cpy(nw->nm,ln[n].nm);
		cpy(nw->pw,w);
		nw->sc=0; nw->hp[0]=nw->hp[1]=20;
		cpy(nw->lc.nm,"start");
		nw->nxt=NULL;
		if(ll==NULL)
			pl=nw;
		else
			{
			while(ll->nxt!=NULL)
				ll=ll->nxt;
			ll->nxt=nw;
			}
		login(n);
		ln[n].ol=100;
		r;
		}
	case 100:
		s=word(s,w);
		n2=0;
		while(vt[n2].vb!=NULL)
			{
			if(cmp(vt[n2].vb,w)==0)
				{
				if(ln[n].pl->sl)
					tx(n,"You wake up.\n");
				ln[n].pl->sl=0;
				vt[n2].fn(n,s);
				tx(n,"* ");
				r;
				}
			n2++;
			}
		if(*w!=0)
			{
			if(lup(xt,w)!=-1)
				vgo(n,w);
			else
				tx(n,"Sorry, I dont understand\n");
			}
		if(IG(n))
			tx(n,"* ");
		r;
	default:
		r;
	}
}

i main(v)
{
i n;
struct timeval tv;

run=init();

load();

setup();

while(run)
	{
	fd_set fdr,fdw;
	FD_ZERO(&fdr);
	FD_ZERO(&fdw);
	FDS(cs,&fdr);

	LP(n,MU)
		{
		if(ln[n].ol)
			{
			FDS(ln[n].so,&fdr);
			FDS(ln[n].so,&fdw);
			}
		}
	if(sec!=psec)
		{
		tick();
		psec=sec;
		}
	tv.tv_sec=1; tv.tv_usec=0;
	if(select(FD_SETSIZE,&fdr,&fdw,NULL,&tv) > 0)
		{
/* New Terminal Connected */
		if(FDIS(cs,&fdr))
			{
/* New Socket, Addrlength, Line */
			i ns,al,li;
			struct sockaddr_in sa;
			al=sizeof(sa);
			if((ns=accept(cs,(sockaddr*)&sa,(socklen_t*)&al))>-1)
				{
				i n2;
				l fl=1;
				ioctl(ns,FIONBIO,&fl);
				LP(n2,MU)
					{
/* Found an unused slot, so connect */
					if(!ln[n2].ol)
						{
						ln[n2].so=ns;
						ln[n2].ol=1;
						welcome(n2);
						break;
						}
					}
/* No free connections, so close socket */
				if(n2==MU)
					{
					shutdown(ns,2);
					close(ns);
					}
				}
			}
		LP(n,MU)
			{
			if(ln[n].ol&&FDIS(ln[n].so,&fdr))
				{
				c li[MI];
				i t;
				if((t=recv(ln[n].so,&li,MI-1,0))<1)
					{
					if(IG(n))
						logout(n);
					shutdown(ln[n].so,2);
					close(ln[n].so);
					ln[n].ol = 0;
					}
				else
					{
/* 0 terminate the input string */
					li[t-2]=0; 
					parse(n,li);
					}
				}
			if(ln[n].ol&&FDIS(ln[n].so,&fdw))
				{	 
/* if a user is ready for output and has any output buffered, send the head of the list */
				if(ln[n].bf!=NULL)
					{
					BF*nx;
					send(ln[n].so,ln[n].bf->txt,strlen(ln[n].bf->txt),0);
					free(ln[n].bf->txt);
					nx = ln[n].bf->nxt;
					free(ln[n].bf);
					ln[n].bf = nx;
					}
				}
			}
		}
	}

save();
LP(n,MU)
	{
	if(ln[n].ol)
		{
		shutdown(ln[n].so,2);
		close(ln[n].so);
		}
	}
r 0;
}

