/*
 * sixteen.c -- Mud server using sixteen kibibyte of source
 * Copyright (C) 2000 Telford Tendys <telford@triode.net.au>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <gdbm.h>
#include <math.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

/* ----------------------------------------------------------------------
 * Macro mayhem!
 * Anything can be anything in C
 */
#define ATTN 10 /* attention span */
#define CHN 55 /* channels available */
#define A assert
/* #define Aok(x) A(x>-1) */
#define Aok(x) if((x)<0){ perror("hmmm"); A((x)>-1); }
#define R return
#define BK break
#define FCH(x) do{is1=is0;is0=i;for(i=0;i<CHN;i++){x} i=is0;is0=is1;}while(0)
#define Nb(x) fcntl(x,F_SETFL,O_NONBLOCK|fcntl(x,F_GETFL));
#define Q(x) (Qt[i].x)
#define Q0(x) (Qt[is0].x)
#define W(d) do{if(Q(q))write(Q(q),d,strlen(d));}while(0)
#define W0(d) write(Q0(q),d,strlen(d))
#define CLR(x) memset(&x,0,sizeof(x))
#define CMD(x) (!strcmp(Q(b),x))
#define PP(x) do{W("\r\e[2K");{x}prompt();}while(0)
#define RND(x) (x>unif())
#define LOG printf
#define U(x) g.x-dsv[j].x
#define GURU !strcmp(Q(n),"guru")
#define DMV do{F("%s",Q(j)?Q(b)+Q(j):"");memcpy(dtmp,ktmp,sizeof(dtmp));dsc.dsize=strlen(dtmp)+1;}while(0)
#define DSTO gdbm_store(DB,k,dsc,GDBM_REPLACE);
#define AN(x) (index("aeiou",x[0])?"an":"a")

/* ----------------------------------------------------------------------
 * Global variables are good if you want to save program size,
 * they also save executable size for that matter but doing
 * things with globals means you have to be a little bit more
 * careful about the order in which you operate
 */

/*
 * Terrain modifiers
 */
typedef struct{
	double v,w,r;
} vt;
vt T;
typedef struct{
	double v,w,r,o;
} ot;
double Ta,Tm;
/*
 * for the bind(), listen() and accept() stuff
 */
struct sockaddr_in I,In;
typedef struct pS{
/*
 * b is the typing buffer
 * n is the character's name
 */
	char b[256],n[256];
/*
 * q if the file descriptor of the player's socket
 * i is the typing position in the buffer
 * j is the position of the second word in the command (i.e. the argument)
 * (x,y) is the coordinate of the character on the map
 * l is the line for mapping (last line is zero)
 * The various (xl,yl) pairs are locations recently mentioned in
 * the ``looking around'' routines (don't mention them twice)
 * ws is a count of the number of spears available
 * wr is a count of the number of rocks available
 * wv is a count of the number of volcanic stones available
 * hp is hit-points (max is 3)
 * dl is a delay to restrict fast changes of who-list entries
 */
	int q,i,j,x,y,l,xl[ATTN],yl[ATTN],ws,wr,wv,hp,dl;
} pt;
/*
 * Allow up to FCH characters in the game at a time
 * Last is bodgy scratch space that can pretend to work like a character
 */
pt Qt[CHN+1];
/*
 * Some scratch buffer space for formatting strings and such
 */
char ktmp[300];
char dtmp[300];
/*
 * The DB file contains the game data, it uses the gdmb library
 * which is a very standard C library, widely available and similar
 * to the old BSD standard dbm library.
 */
GDBM_FILE DB;
/*
 * These are for the database too:
 * k is always the key of whatever we are searching or storing,
 * many things are stored only as a key, all keys are strings.
 * flg is just some arbitrary data that is used to say that a
 * key does exist (gdbm does not allow a key to relate to empty
 * data so we need something)
 * ter is a terrain datum.
 */
datum k, d, flg = {"!",1}, ter = {(char*)&T,sizeof(vt)}, dsc = {dtmp,0};
/*
 * i is the current index in the array of characters
 * e is whatever value was returned from a system call
 * is0 is the index saver
 * (rx,ry) are coordinates used in radius and direction calculations
 * (gx,gy) are coordinates used by the `go' command for walk towards something
 */
int i,s,e,l,is0,is1,rx,ry,gx,gy;

/* ----------------------------------------------------------------------
 * Random number generator -- give a double between 0 and 1 (uniform)
 *
 * | Random Number Generators and Simulation |
 * | Deak Istvan                             |
 * | ISBN = 9630553163                       |
 * | page 51                                 |
 */
unsigned long s0=1,s1=2,s2=3,s3=4;
double unir(double x){R x/(double)4294967296.0;}
double unif(){
	s3^=s0<<16;
	s3&=0x7FFFFFFF;
	s0^=s0>>15;
	s0^=s1<<17;
	s1^=s1>>15;
	s1^=s2<<17;
	s2^=s2>>15;
	s2^=s3<<17;
	s3^=s3>>15;
	switch(3L&s3){
		case 1:R unir(s1);
		case 2:R unir(s2);}
	R unir(s0);}
double tri(){R unif()-unif();}

/* ----------------------------------------------------------------------
 * The good old swiss-army knife of formatting gets
 * a single letter name because it is so useful.
 * Note that it also keys up the database so that we are ready
 * for a query as soon as the string is formatted, this allows
 * arbitrary generation of DB keys with very little effort
 * (and no extra code other than what would have been used for
 * output formatting anyhow).
 */
F(char*ft,...){
	va_list A;
	va_start(A,ft);
	CLR(ktmp);
	vsnprintf(ktmp,300,ft,A);
	va_end(A);
/*
 * debugging is easy when just about everything that happens
 * goes through here first and gets a string representation
 */
/*
	printf( "DEBUG#%d: F(%s)\n", i, ktmp );
*/
	k.dptr=ktmp;
	k.dsize=(1+strlen(ktmp));}

/* ----------------------------------------------------------------------
 * This is a measure of distance between the current character
 * and whoever is on the top of the stack
 */
double sqr(double x){R x*x;}
double radius(){
	rx=Q(x)-Q0(x);
	ry=Q(y)-Q0(y);
	wrap(&rx);
	wrap(&ry);
	R sqrt(sqr(rx)+sqr(ry));}

/* ----------------------------------------------------------------------
 * Figure out which direction one character is in relation to another
 * Depends on a side-effect of the radius() call to set rx and ry.
 * Correctly handles torroidal topology for world surface (spherical
 * geometry is MUCH harder than doughnut geometry).
 */
char *drct(int fl){
	double phi;
	char *p;
	gx=gy=0;
	radius();
	phi=atan2(ry,rx)/M_PI;
	switch(7&(int)floor(phi*4+4.5+fl)){
		case 0:gx=1;p=" (e)";BK;
		case 1:gx=gy=1;p=" (ne)";BK;
		case 2:gy=1;p=" (n)";BK;
		case 3:gx=-1;gy=1;p=" (nw)";BK;
		case 4:gx=-1;p=" (w)";BK;
		case 5:gx=gy=-1;p=" (sw)";BK;
		case 6:gy=-1;p=" (s)";BK;
		default:gy=-1;gx=1;p=" (se)";}
	if(radius()<12)p="";
	R p;}

/* ----------------------------------------------------------------------
 * Descriptions for distances
 */
char *disa[]={"right here","very close","nearby","some way off","in the distance"};
char *dis(){
	int j=floor(radius()/20);
	if(j<5)R disa[j];
	R "a long way in the distance";}

/* ----------------------------------------------------------------------
 * Very small function to kick the current player off the
 * MUD and recycle the slot
 */
kick(int j){
	if(Q(q)){close(Q(q));LOG("KICK %d\n",i);}
	Q(q)=0;
	if(j&&Q(n)[0]){
		F("%s has left the game.\r\n",Q(n));
		FCH(PP(W(ktmp);););}
	CLR(Qt[i]);}

/* ----------------------------------------------------------------------
 * This sets the limits of the playfield by wrapping the edges
 */
wrap(int *p){while(*p<=-2048)*p+=4096;while(*p>2048)*p-=4096;}
mwrap(){wrap(&Q(x));wrap(&Q(y));}

/* ----------------------------------------------------------------------
 * Move to a random location
 */
rj(){Q(x)=400*tri();Q(y)=400*tri();}

/* ----------------------------------------------------------------------
 * Free database results (can be called extra times
 * without causing crash so if in doubt, call it).
 * There should be no other calls to free() necessary.
 */
frd(){if(d.dptr)free(d.dptr);d.dptr=0;}

/* ----------------------------------------------------------------------
 * Commonly used test for data found in the database
 * Takes input from global variable k (i.e. the key)
 * Gives output as global variable d (i.e. the data)
 * and returns 1 if there is some data or 0 if not.
 * (This is only suitable for string data)
 */
fnd(){frd();d=gdbm_fetch(DB,k);if(!d.dptr)R 0;if(!d.dptr[0])R 0;R 1;}

/* ----------------------------------------------------------------------
 * Read from the socket of the current player and load
 * it into the buffer. Evil characters are thrown away
 * and backspace and delete both work like backspace.
 * Long buffers are silently truncated. 
 *
 * It also has a side-effect of attempting to fill unused
 * player slots with an accept, if the accept fails then
 * no harm done (OK, a tiny bit of system time is used in
 * redundant polling of accept() so what!)
 */
Rd(){
	if(!Q(q)){
		l=sizeof(In);
		e=accept(s,&In,&l);
		if(e>0){
/* local login only, comment out next two lines for global internet access */
/*
			LOG("CONNECT #%d=%08lX\n",i,ntohl(In.sin_addr.s_addr));
			if(0x7F000001!=ntohl(In.sin_addr.s_addr)
			   &&0xC0A8027B!=ntohl(In.sin_addr.s_addr)){LOG("Illegal addr\n");close(e);R 0;}
*/
			kick(0);
			Q(q)=e;
			do{rj();rsum();}while(Tm<0.5);
			prompt();
			Nb(Q(q));}
		R 0;}
	while(1){
		char *p;
		if(Q(j)>Q(i)||Q(j)<0)Q(j)=Q(i);
		p=Q(b)+Q(i);
		e=read(Q(q),p,1);
		if(e<=0){
			if(!e||errno!=EAGAIN)kick(1);
			R 0;}
		if(Q(i)<250)Q(i)++;
		if('\n'==*p||'\r'==*p){
			if(Q(i)>1){*p=0;Q(i)=0;R 1;}
			--Q(i);
			continue;}
		if(!Q(j)||Q(j)==Q(i)-1)
			if(isspace(*p)){*p=0;Q(j)=Q(i);continue;}
		if(isprint(*p)) continue;
		if((8==*p||127==*p)&&Q(i))--Q(i);
		--Q(i);}}

/* ----------------------------------------------------------------------
 * description engine converts terrain data into descriptive sentences
 * that are strung end-to-end. A static data space is used to collect
 * all the descriptions and no check is made whether it overflows.
 * Thus, the buffer length must be set longer than the longest combination
 * of descriptions. The descriptive sentences are all in code rather than
 * in a separate file because you can do extra tricks with code
 */

ot dsv[]={
	{.85,0,0,.7},/*       000  1.10 */
	{0,.85,0,.7},/*       001  1.10 */
	{0,0,.8,.7},/*        002  1.06 */
	{.1,.1,.1,1.1},/*     003  1.11 */
	{.4,0,0,.2},/*        004  0.45 */
	{0,.4,0,.2},/*        005  0.45 */
	{0,0,.4,.2},/*        006  0.45 */
	{-.2,0,.4,1},/*       007  1.10 */
	{.25,0,.1,.1},/*      008  0.29 */
	{0,.25,-.1,.1},/*     009  0.29 */
	{-.1,0,.25,.1},/*     010  0.27 */
	{.4,0,0,1},/*         011  1.07 */
	{.6,.6,0,.5},/*       012  0.98 */
	{.6,0,.6,.5},/*       013  0.98 */
	{0,.6,.6,.5},/*       014  0.98 */
	{0,.2,.2,.1},/*       015  0.30 */
	{.5,.5,.5,.3},/*      016  0.92 */
	{.3,.2,.1,.9},/*      017  0.97 */
	{.3,.1,.6,.8},/*      018  1.05 */
	{0,0,.1,.1},/*        019  0.14 */
	{.1,.05,0,.1},/*      020  0.15 */
	{0,.1,0,.1},/*        021  0.14 */
	{.04,.07,0,.1},/*     022  0.13 */
	{.05,.05,.05,.06},/*  023  0.11 */
	{.5,.3,.2,.7},/*      024  0.93 */
	{0,.4,0,1},/*         025  1.07 */
	{0,-.1,.1,.1},/*      026  0.17 */
	{0,.4,.7,.5},/*       027  0.95 */
};

deng(){
	char buf[1024];
	double w,w0,w1;
	ot g;
	int j,k;

	CLR(buf);l=0;
	g.v=T.v;g.w=T.w;g.r=T.r;
	g.o=1.4-sqr(g.v)-sqr(g.w)-sqr(g.r);
	if(g.o>0)g.o=sqrt(g.o);else g.o=0;
	while(1){
		w0=sqrt(sqr(g.v)+sqr(g.w)+sqr(g.r)+sqr(g.o));
		w1=10;
/*
 * debugging to get descriptions right
 */
/*
		if(GURU){
			F("{%g,%g,%g,%g} ",g.v,g.w,g.r,g.o);
			strcat(buf,ktmp);}
*/
		for(j=sizeof(dsv)/sizeof(ot);j--;){
			w=sqrt(sqr(U(v))+sqr(U(w))+sqr(U(r))+sqr(U(o)));
			if(w<w1){k=j;w1=w;}}
		if(w1>w0)BK;
		j=k;
		g.v=U(v);g.w=U(w);g.r=U(r);g.o=U(o);
		CLR(ktmp);
		switch(j){
			case 0:
				F("You are in the heart of a dense thicket. "
				  "Springy brush presses against you and twisted briar entangles"
				  "your feet. ");
				BK;
			case 1:
				F("Waist deep in water, you watch the circular ripples drift away. ");l|=4;
				BK;
			case 2:
				F("A fresh breeze across your face feels cool and almost primal "
				  "as you pick your way across this craggy mountain landscape. ");
				BK;
			case 3:
				F("You are crossing a wide, empty %s. ",T.w<.2?"desert":"plain");
				BK;
			case 4:
				F("Trees and shrubs dot the area. ");
				BK;
			case 5:
				F("Marsh flies rise up out of a boggy sump, angry at being disturbed. ");
				BK;
			case 6:
				F("Large boulders are strewn around like petanque for giants. ");
				BK;
			case 7:
				F("You scramble over bare, rocky foothills and stony, broken ground. ");
				BK;
			case 8:
				if(l&4)
					F("Some slimy water-weed sticks to your leg. ");
				else
					F("A sapling sprouts from between two nearby stones. ");
				BK;
			case 9:
				F("The muddy, damp earth squelches around your boots. ");
				BK;
			case 10:
				F("The occasional loose breccia provides uncertain footing. ");
				BK;
			case 11:
				F("You are wandering through light scrubland. ");
				BK;
			case 12:
				F("You are in the midst of heavy jungle. "
				  "Tangled vines descend from the treetops far above. ");l|=2;
				BK;
			case 13:
				F("This is gnarly country. "
				  "The heavy set trees and wiry shrubs jealously guard their rocky, "
				  "mountainous territory. ");
				BK;
			case 14:
				F("Rushing water spashes over rocky cliffs and kicks up a spray as mountain "
				  "streams expend their playful energy... better tread carefully! ");
				BK;
			case 15:
				F("A few plashing streamlets wend their way past. ");
				BK;
			case 16:
				F("Spiny gorse grows in sodden patches amongst mountain fir trees, "
				  "challenging you to pick your way across morraines, unstable "
				  "scree and hidden, trecherous burns. ");
				BK;
			case 17:
				F("The light wind pushes long, slow, lazy waves over the rolling "
				  "hills of grassland like a living green ocean. ");
				BK;
			case 18:
				F("A vast, open expanse of grassy tundra lies around you. ");
				BK;
			case 19:
				F("A%s above you, scanning for prey. ",
				  l&2?" python hangs from a branch":(T.w<.3?" vulture circles":"n eagle wheels"));
				BK;
			case 20:
				F("Cheerful birds twitter %s. ", T.v>.6?"in the trees":"close by");
				BK;
			case 21:
				F("A%s smell of dampness wafts around. ", T.w<.4?" slight":" strong");
				BK;
			case 22:
				if(l&4)
					F("Something swims up to your foot, then quickly swims away. ");
				else
					F("A soft `tok!' comes from a local frog. ");
				BK;
			case 23:
				if(l&1){F("SPLAT! No more fly... ");}else{F("A fly zips back and forth. ");l|=1;}
				BK;
			case 24:
				F("Open woodland unfolds around you. ");
				BK;
			case 25:
				F("You are wading up to your knees in water. ");l|=4;
				BK;
			case 26:
				if(l&4)
					F("Wading birds has built a nest "
					  "that is not much more than a pile of mud. ");
				else
					F("Some cracked old bones represent the remains "
					  "of what was once a sizable creature. ");					
				BK;
			case 27:
				F("You have reached a high point where your breath forms fog "
				  "and the cold sun glints from patches of ice. ");
				BK;
			default: F(">!<");}
		strcat(buf,ktmp);
	}
	if(buf[0]){
		strcat(buf,"\r\n");
		W(buf);}}

/* ----------------------------------------------------------------------
 * Insert an ``attention-span'' item which is a list where everything
 * gets pushed on the top and the last one in falls off the bottom.
 * Items on the list are room coordinates where landmarks exist or
 * possibly where characters are sitting.
 *
 * It will not insert an item that is already on the list (nor does
 * that item get promoted to the top) and will return 0 for an item
 * already on the list or 1 for a new item (item not already on the list)
 *
 * f==0 means really do the insert, f==1 means just check for existing
 */
ins(int x,int y,int f,int i){
	int j;
	for(j=ATTN;j--;){if(x==Q(xl)[j]&&y==Q(yl)[j])R 0;}
	if(f)R 1;
	memmove(Q(xl),Q(xl)+1,sizeof(int)*(ATTN-1));
	Q(xl)[ATTN-1]=x;
	memmove(Q(yl),Q(yl)+1,sizeof(int)*(ATTN-1));
	Q(yl)[ATTN-1]=y;
	R 1;}

/* ----------------------------------------------------------------------
 * Movement is on a cartesian grid with solid boundaries
 * and terrain of varying difficulty. The characters can never
 * see how difficult it might be to move in a given direction,
 * they can only try it and find out (they can take an educated
 * guess based on the map and the terrain description).
 *
 * If a landmark is found, set l and return, otherwise return
 * with l clear.
 */
move(int dx,int dy){
	int dist = 100;

	if(Q(j))dist=strtol(Q(b)+Q(j),0,0);
	if(!dist)dist=1;
	if(GURU){Q(x)+=dx*dist;Q(y)+=dy*dist;mwrap();R;}
	if(dist>100)dist=100;
	l=0;
	while(dist){
		Q(x)+=dx;Q(y)+=dy;
		rsum();
		if(RND(Tm)){
			mwrap();
			F("M %d %d",Q(x),Q(y));
			if(gdbm_exists(DB,k)){l=1;R;}}
		else{
			Q(x)-=dx;Q(y)-=dy;
			if(Q(wr)<5&&T.r>.65&&RND(.06)){++Q(wr);W("You found a rock\r\n");R;}
			if(Q(ws)<5&&T.v>.74&&RND(.04)){++Q(ws);W("You found a spear\r\n");R;}
			if(Q(wv)<5&&T.w<.25&&RND(.01)){++Q(wv);W("You found a volcanic bomb\r\n");R;}
			if(0==Tm){W("You cannot move in that direction.\r\n");R;}}
		dist--;}}

/* ----------------------------------------------------------------------
 * go is like move but there is a target location rather
 * than a general direction. The goal is in Q0(x),Q0(y)
 * Don't bother stopping for landmarks other than the goal.
 */
go(){
	int dist = 100;

	if(GURU){Q(x)=Q0(x);Q(y)=Q0(y);R;}
	while(dist){
		if(Q(x)==Q0(x)&&Q(y)==Q0(y)){
			if(100==dist)
				W("You would go to where you already are.\r\n");
			else
				l=1;
			R;}
		radius();
		drct(0);
		Q(x)+=gx;Q(y)+=gy;
		rsum();
		if(!RND(Tm)){
			Q(x)-=gx;Q(y)-=gy;
			if(0==Tm){W("You cannot move in that direction.\r\n");R;}}
		mwrap();
		dist--;}}

/* ----------------------------------------------------------------------
 * Most characters get a silly prompt, the guru knows where he/she is
 * so guru gets a prompt with numbers in it.
 */
prompt(){
	if(!Q(n)[0]){
		W("name passwd: ");}
	else if(GURU){
		F("(%d,%d) %d# ",Q(x),Q(y),i);
		W(ktmp);}
	else{
		W("=> ");}}

/* ----------------------------------------------------------------------
 * Place a new room at (Q(x),Q(y)) with terrain created randomly
 * and stored in T, only random terrain can be created. There is a
 * minimum room size (implemented as a mask on low bits) which prevents
 * the hash table filling up with pointless tiny rooms.
 */
rnew(){
	T.v=unif();
	T.w=unif();
	T.r=unif();
	if(RND(.3))CLR(T);
	if(RND(.15))T.v=1;
	if(RND(.15))T.w=1;
	if(RND(.15))T.r=1;
	Q(x)&=~15;
	Q(y)&=~15;
	mwrap();
	F("R %d %d",Q(x),Q(y));
	e=gdbm_store(DB,k,ter,GDBM_REPLACE);}

/* ----------------------------------------------------------------------
 * Pull a single room from the hashtable and put the
 * terrain values into T, note that storage is sparse
 * so any room which does not exist counts as zero
 */
int m0,m1;
rget(int x, int y)
{
	vt*Tp;
	double w=(m1-fabs(x-Q(x)))*(m1-fabs(y-Q(y)));

	w/=m1;
	w/=m1;
	w/=sqrt(m1);
	wrap(&x);
	wrap(&y);
	F("R %d %d",x,y);
	d=gdbm_fetch(DB,k);
	if(Tp=(vt*)d.dptr){
		Ta+=w;
		T.v+=w*Tp->v;
		T.w+=w*Tp->w;
		T.r+=w*Tp->r;
		frd();}}

/* ----------------------------------------------------------------------
 * Read the room values for all rooms overlapping point (Q(x),Q(y))
 * and generate terrain values between 0 and 1, explaining how the
 * bit sliding stuff works is beyond the scope of this documentation.
 */
rsum(){
	int x0,x1,y0,y1;
	m0=-16;
	m1=8;

	CLR(T);
	Ta=.01;
	while(0x3FF&m1){
		x0=((Q(x)-m1)&m0)+m1;
		y0=((Q(y)-m1)&m0)+m1;
		x1=((Q(x)-m1)|~m0)+m1+1;
		y1=((Q(y)-m1)|~m0)+m1+1;
		m1<<=1;
		m0<<=1;
		rget(x0,y0);
		rget(x1,y0);
		rget(x0,y1);
		rget(x1,y1);}
	T.v/=Ta;
	T.w/=Ta;
	T.r/=Ta;
	Tm=(1-T.v)*(1-T.w)*(1-T.r);
	if(Tm<.1)Tm=0;
	if(Tm>1)Tm=1;}

/* ----------------------------------------------------------------------
 * Do a database query looking for a landmark, return 1 if it is found
 * and 0 if not. Leave the resulting data (i.e. the name of the
 * landmark) in d.dptr global variable.
 */
lm(int x,int y){
	F("m %d %d",x,y);
	if(fnd())R 1;
	R 0;}

/* ----------------------------------------------------------------------
 * Search for a named thing which might be a character or a landmark
 * Only one thing may be found and it is returned in Qt[CHN]
 * The name of the thing we are searching for is in Q(b)+Q(j)
 * which gets put into a local variable for convenience
 * Returns 1 for character found, 2 for landmark and 0 for nothing
 */
srch(){
	char *p=Q(b)+Q(j);
	int j;

	CLR(Qt[CHN]);
	FCH(if(Q(q)){
		if(radius()<100&&!strcmp(p,Q(n))){
			memcpy(Qt+CHN,Qt+i,sizeof(pt));
			Qt[CHN].j=i;}});
	is0=CHN;
	if(Q0(q)&&Q0(n)[0])R 1;
	for(j=ATTN;j--;){
		Q0(x)=Q(xl)[j];Q0(y)=Q(yl)[j];
		if(!Q0(x)&&!Q0(y))continue;
		if(lm(Q0(x),Q0(y))){
			if(!strcmp(d.dptr,p)){
				strcpy(Q0(n),d.dptr);
				frd();R 2;}}}
	R 0;}

char *wpn[]={"chucks a volcanic bomb","hurls a spear","throws a rock"};

/* ----------------------------------------------------------------------
 * This handles one single character and the possible
 * effects of a new connection. This includes getting the
 * player's name and password, checking if that name is
 * already owned by someone, checking if the password matches,
 * storing a new password if the name is new, etc.
 *
 * Once a password is entered, there is no way to change it :-)
 * We don't bother asking for a password twice, they can just
 * make sure they get it right (also we don't worry about no-echo
 * and such, telnet users can just be careful, users of a fancy
 * client can take the trouble to write a two line script for
 * logging in.
 */
play(){
	if(Rd()){
		Q(l)=0;l=0;
		if(Q(dl)){
			W("Who-list entry remains unchanged.\r\n");
			Q(dl)=0;}
		if(!Q(n)[0]){
			F("N %s",Q(b));
			e=gdbm_store(DB,k,flg,GDBM_INSERT);
			F("P %s %s",Q(b),Q(b)+Q(j));
			if(e){
				if(!gdbm_exists(DB,k)){
					close(Q(q));
					Q(q)=0;
					R;}
				{char *p=Q(b);FCH(if(!strcmp(p,Q(n)))kick(0););}/* no duplicates! */
				W("Welcome back!\r\n");}
			else{
				gdbm_store(DB,k,flg,GDBM_INSERT);
				W("Welcome newbie!\r\n");}
			Q(hp)=3;
			strcpy(Q(n),Q(b));
			F("%s has joined the game.\r\n",Q(n));
			FCH(if(i!=is0)PP(W(ktmp);););
			goto lookout;}
		else{
			if(CMD("shout")||CMD("!")){
				FCH(if(i!=is0)if(radius()<700){
					F("%s%s shouts, `%s'\r\n",Q0(n),drct(0),Q0(b)+Q0(j));
					PP(W(ktmp););});}
			else if(CMD("say")||CMD("'")){
				FCH(
					if(i!=is0){
						if(radius()<60){
							F("%s%s says, `%s'\r\n",Q0(n),drct(0),Q0(b)+Q0(j));
							PP(W(ktmp););}
						else if(radius()<150){
							F("You hear muttering%s.\r\n",drct(0));
							PP(W(ktmp););}});}
			else if(CMD("emote")||CMD(":")){
				F("%s %s\r\n",Q(n),Q(b)+Q(j));
				FCH(
					if(i!=is0){
						if(radius()<50)PP(W(ktmp););
						else if(radius()<110){
							F("Someone is trying to attract your attention%s.\r\n",
							  drct(0));
							PP(W(ktmp););}});}
			else if(CMD("quit")){kick(1);}
			else if(CMD("n")){move(0,1);}
			else if(CMD("nw")){move(-1,1);}
			else if(CMD("ne")){move(1,1);}
			else if(CMD("s")){move(0,-1);}
			else if(CMD("sw")){move(-1,-1);}
			else if(CMD("se")){move(1,-1);}
			else if(CMD("e")){move(1,0);}
			else if(CMD("w")){move(-1,0);}
			else if(CMD("who")){
				FCH(if(Q(n)[0]){
					F("W %s",Q(n));
					if(fnd()){
						F("%s%s.\r\n",d.dptr,drct(4));
						W0(ktmp);}
					if(!d.dptr){
						F("%s is a newbie%s.\r\n",Q(n),drct(4));
						W0(ktmp);}});}
			else if(CMD("entry")){
				DMV;F("w %s",Q(n));DSTO;Q(dl)=270;
				W("Please wait quietly while your who-list entry is updated.\r\n");}
			else if(CMD("face")){DMV;F("D %s",Q(n));DSTO;}
			else if(CMD("shutdown")&&GURU){gdbm_close(DB);exit(0);}
			else if(CMD("mark")&&GURU){DMV;F("M %d %d",Q(x),Q(y));DSTO;}
			else if(CMD("mname")&&GURU){DMV;lm(Q(x),Q(y));DSTO;}
			else if(CMD("kick")&&GURU){
				FCH(if(!strcmp(Q0(b)+Q0(j),Q(n))){kick(1);});}
			else if(CMD("hyper")&&GURU){
				FCH(if(!strcmp(Q0(b)+Q0(j),Q(n))){Q0(x)=Q(x);Q0(y)=Q(y);});}
			else if(CMD("map")){Q(l)=39;}
			else if(CMD("kill")){
				if(Q(j)){
					if(1!=srch())goto kill_nf;
					is0=CHN;
					if(Q0(j)==i){W("No!\r\n");}
					else{
						int j=3,tg=0;

						if(Q(wv)){--Q(wv);j=0;}
						else if(Q(ws)){--Q(ws);j=1;}
						else if(Q(wr)){--Q(wr);j=2;}
						if(j==3){W("You have no ammo.\r\n");}
						else{
							double r=radius()/30+(j?0:3);
							int x,y,p;

							F("%s %s...\r\n",Q(n),wpn[j]);
							FCH(if(Q(n)[0]&&i!=is0)if(radius()<100)PP(W(ktmp);););
							for(p=j?1:10;p--;){
								is0=CHN;
								x=Q0(x)+r*tri();
								y=Q0(y)+r*tri();
								CLR(Q0(n));
								FCH(if(Q(n)[0]&&Q(x)==x&&Q(y)==y){
									strcpy(Qt[CHN].n,Q(n));
									Qt[CHN].j=i;tg++;
									if(--Q(hp)>0){
										F("You've been hit by %s!\r\n",Q0(n));
										PP(W(ktmp););}
									else
										kick(1);});
								is0=CHN;
								if(Q0(n)[0]){
									F("%s gets hit!\r\n",Q0(n));
									FCH(if(Q(n)[0]&&i!=is0&&i!=Qt[CHN].j)
										if(radius()<100)PP(W(ktmp);););}}
							F("You caused %s damage.\r\n",
							  tg?tg<2?"some":tg<4?"serious":"awesome":"no");
							W(ktmp);}}}
				else{
 kill_nf:
					W("Kill what?\r\n");}}
			else if(CMD("go")){
				if(Q(j)){
					if(!srch())goto go_nf;
					go();}
				else{
 go_nf:
					W("Go where?\r\n");}}
			else if(CMD("l")||CMD("look")){
				if(Q(j)){
					switch(srch()){
						case 0: W("Look at what?\r\n"); BK;
						case 1:
							F("D %s",Q0(n));
							if(fnd()){
								F("%s\r\n",d.dptr);
								W(ktmp);
								frd();}
							else{F("%s looks uninteresting.\r\n",Q0(n));W(ktmp);}
							BK;
						case 2:
							F("M %d %d",Q0(x),Q0(y));
							if(fnd()){
								F("%s\r\n",d.dptr);
								W(ktmp);
								frd();}}}
				else{
					int j;
 lookout:
					F("M %d %d",Q(x),Q(y));
					if(fnd()){
						ins(Q(x),Q(y),0,i);
						F("%s\r\n",d.dptr);
						W(ktmp);
						frd();}
					else{
						rsum();
						deng();}
					FCH(
						if(Q(q)&&is0!=i){
							if(radius()<100){
								if(lm(Q(x),Q(y)))
									F("%s is at the %s, %s%s.\r\n",Q(n),d.dptr,dis(),drct(4));
								else
									F("%s is %s%s.\r\n",Q(n),dis(),drct(4));
								W0(ktmp);}});
					for(j=ATTN;j--;){
						is0=CHN;Q0(x)=Q(xl)[j];Q0(y)=Q(yl)[j];
						if(!Q0(x)&&!Q0(y)||radius()>200||Q0(x)==Q(x)&&Q0(y)==Q(y))continue;
						if(lm(Q0(x),Q0(y))){
							F("The %s is %s%s.\r\n",d.dptr,dis(),drct(0));
							W(ktmp);}}
					l=0;}}
			else{
				F("No such command `%s'\r\n", Q(b));
				W(ktmp);}
			if(l)goto lookout;
			prompt();}}
	else if(Q(dl)){
		if(--Q(dl)<=0){
			Q(dl)=0;
			F("w %s",Q(n));
			fnd();
			ktmp[0]='W';
			gdbm_store(DB,k,d,GDBM_REPLACE);
			PP(W("Who-list update complete.\r\n"););}}
	else if(Q(n)[0]){
		int x,y,j;

		x=Q(x);
		y=Q(y);
		if(Q(l)){
			char X[99];
			Q(y)-=21-Q(l);
			Q(x)-=30;
			CLR(X);
			for(j=0;j<21;j++){
				if(x==Q(x)&&y==Q(y)){
					X[j]='i';}
				else{
					rsum();
					X[j]="@#*=+-:~,,...    "[(int)floor(Tm*16)];}
				Q(x)+=3;}
			X[21]='\r';
			X[22]='\n';
			PP(W(X););
			Q(x)=x;
			Q(y)=y;
			Q(l)-=3;}
		else{
			if(Q(hp)<3&&RND(.001)){PP(W("You feel a bit better\r\n"););++Q(hp);}
			for(j=10;j--;){
				x=Q(x)+90*tri();
				y=Q(y)+90*tri();
				wrap(&x);wrap(&y);
				if(ins(x,y,1,i)){
					if(lm(x,y)){
						ins(x,y,0,i);
						F("You notice %s %s.\r\n",AN(d.dptr),d.dptr);
						PP(W(ktmp););}}}
			ktmp[0]=0;
			FCH(if(Q(n)[0]&&i!=is0){
				if(radius()<100){
					if(ins(Q(x),Q(y),0,is0)){
						if(!ktmp[0])W0("\r\e[2K");
						F("%s moves around %s%s.\r\n",Q(n),dis(),drct(4));
						W0(ktmp);}}});
			if(ktmp[0])prompt();
		}
	}}

/* ----------------------------------------------------------------------
 * Now into the main loop
 * This is pretty much what you would expect...
 * set everything up, open files etc. then loop around the
 * player slots acting on any events that might be found
 * (classic polling loop, predictable, reliable and slightly inefficient)
 */
main(){
/*
 * Testing for random generator:
 *
 * while(1){printf( "%g\n", unif());}
 */
	CLR(Qt);
	s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	Aok(s);
	Nb(s);
	I.sin_family=AF_INET;
	I.sin_addr.s_addr=htonl(INADDR_ANY);
	I.sin_port=htons(9010);
	e=bind(s,&I,sizeof(I));
	Aok(e);
	e=listen(s,10);
	Aok(e);
	DB = gdbm_open("mydata",0,GDBM_WRCREAT,0600,0);
	F("INIT");
	if(!gdbm_store(DB,k,flg,GDBM_INSERT)){
		LOG("Please wait...\n");
		i=0;
		for(l=20000;l--;){Q(x)+=50*tri();Q(y)+=50*tri();mwrap();rnew();}}
	LOG("Ready!\n");
	while(1){
		FCH(if(RND(.001))sleep(1);play(););}}
