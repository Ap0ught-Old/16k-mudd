# libraries imports
from select import *
from Queue import *
from zlib import *
from string import *
from os import listdir
from regex import *
from time import *
from array import *
from cMap import *
from copy import *
from pickle import *

from socket import *
# socket must be the last module to import (for the error exception)

# game constants
RL="\n\r"
RL2=RL+RL

# def pack(fn,t):
#         # packs a file
#         fi=open(fn,t)
#         fo=open(fn+'.bin','wb')
#         # reads the file in text mode
#         fo.write(compress(fi.read(),9))
#         fi.close()
#         fo.close()

def unpack(fn,t):
        # unpacks a file
        fi=open(fn+".bin","rb")
        # reads the file in binary mode
        l=decompress(fi.read())
        fi.close();
        if t=='r': return replace(l,'\n',RL)
        return l

def rdic(d,k,e):
        if d.has_key(k): d[k].remove(e)
        # removes the element e from the given dictionnary at key k

def adic(d,k,e):
        # adds the element into the list of the dictionnary at key k
        if d.has_key(k): d[k]=d[k]+[e]
                # a key of this kind does already exist
        else: d[k]=[e]
                # first entry

def slist(l,t):
        # searches for an element in the given list
        for o in l:
                # for each element of the list
                if o!=None and t(o): return o

        # not found
        return None

def PosS(k):
        # returns a couple (x,y) from the string position "x;y"
        f=find(k,";")
        return(atoi(k[0:f]),atoi(k[f+1:]))

def SPos(a,b): return(str(a)+";"+str(b))
        # returns the string position "x;y" from a couple (x,y)

def Sprint(x,y,s,c):
        # prints a string at a given position
        lw=split(s)
        i=1
        for w in lw:
                # for each word to display
                if w[0]=='õ': x=30;y=y+1;w=w[1:];i=1
                z=len(w)+1+i*2
                if x+z>=79: x=30;y=y+1
                put(0,x+i*2,y,w+" ",c)
                x=x+z;i=0
        return y+(x>30)           # TESTER SI X>30 renvoie 0 ou 1

#def sdata(fn,o):
#        # saves an object to a file
#        f=open(fn+".bin","wb")
#        f.write(compress(dumps(o,1)))
#        f.close()

def ldata(fn):
        # loads an object from a file
        fi=open(fn+".bin",'rb')
        s=fi.read()
        s=decompress(s)
        fi.close();
        return loads(s)

def isblock(c): return c in ['#','=','^']
        # returns TRUE if the square is blocking

def Xdicval(d,f):
        # executes a function for all the values of the given list-dictionnary
        for lc in d.values():
		# A TESTER : n'existe-t-il pas une fonction standard qui fais deja ca ?
                # for each list of the list
                for o in lc: f(o)
                        # for each object in the list

def rdln(l):
        # reads a line from the given file
        p=find(l,"\n")
        return l[:p],l[p+1:]

# seqANSI={'n':'\x1B[37m','r':'\x1B[31m','v':'\x1B[32m','j':'\x1B[33m',\
# 'b':'\x1B[34m','m':'\x1B[35m','c':'\x1B[36m','w':'\x1B[37m',\
# 'cls':'\x1B[2J'}
# sdata("seqANSI",seqANSI)

# com1={"look":"actlook    ","north":"actmove    ","south":"actmove    ",\
# "east":"actmove    ","west":"actmove    ","who":"actwho",\
# "emote":"actemote","gossip":"actgossip","say":"actsay",\
# "equipment":"acteq","inventory":"actinv","get":"actget",\
# "drop":"actdrop",\
# "examine":"actlook","quit":"actquit"}
# sdata("commands",com1)

seqANSI=ldata("seqANSI")
com=ldata("commands")

# print 'Packing equipment slots'
# pack("eqslot",'r')

# print 'Packing map1'
# pack("map1",'r')

# print 'Packing intro'
# pack("intro",'r')

# print 'Packing strings'
# pack("strings",'r')

eqname=split(unpack("eqslot",'r'),RL)

scrs={"login":unpack("intro",'r')}
maps=[]         # maps list
clis={}         # sockets/clients association
strs=split(replace(unpack("strings",'r'),'_',' '),RL)
                # initialize data and load strings
isf=jsf=""      # temp items
tck=0           # number of ticks

def Scr(n,b):
        # creates a new map
        d=split(b,RL)
        init(n,80,len(d))
        for j in range(0,len(d)): put(n,0,j,d[j],'n')
                # for each line in the list
        return n

def inview(a,b): return a in range(0,28) and b in range(1,15)	# A TESTER
        # returns true if it is in the map limits

def remcol(s):
        # removes color from the given string
        for c in seqANSI.keys(): s=replace(s,"õ"+c,'')
                # remove the colors in the strings
        return s

def putxy(p,x,y,c,z):
        # puts something on the client's map

        a,b=x-p.mx+14,y-p.my+8
        if not(inview(a,b)) or not('pos' in p.opts) or p.pl>p.mpl: return
                # doesn't appear in the map or in the screen

        p.snd("\x1B["+SPos(b+p.scp,a+p.scp)+"H")
        p.snd("õ"+z+c+RL+"õn")
        p.snd("\x1B["+str(p.pl)+";0H")

def teleport(o,m,x,y):
        # teleports a generic Object into a new map at a given position
        if o.map!=None: rdic(o.map.gos,SPos(o.mx,o.my),o)
                # removes the object from the previous map
        o.map=m
        if m!=None: adic(m.gos,SPos(x,y),o)
                # adds it to the new map
        o.mx,o.my=x,y

class GenObj:
        # a generic object

        def __init__(sf):
                # creates a new "Generic Object"

		sf.nm,sf.des,sf.al,sf.ldes,sf.map="","","","",None
		# set object's attributes:
		# sf.nm		: object's name
		# sf.des	: object's description
		# sf.al		: abbreviation (on 1 character) of the object
		# sf.ldes	: object's long description (when examined)
		# sf.map	: map where it is actually		

class Map(GenObj):
        # a world map

        def __init__(sf,nn):
                # creates a new map from a map number
                global isf

                sf.gos,sf.des={},{}
                                # empty dictionnaries for the generic objects
                                # and the descriptions

		fn="map"+str(nn)
                sf.s=Scr(nn,unpack(fn,'r'))
                isf=sf
                # attributes of the object

                # loads the .inf file
                l=replace(unpack(fn+".inf","r"),"\r","")
                l=sf.load_descr(l)
                l=sf.load_mobs(l)
                sf.load_objs(l)

        def descr(sf,c,l):
                # adds the room (x,y) description to the screen 0
                s=""
                for k in sf.des.keys():
                        # get the x,y pos of the key
                        dx,dy=PosS(k)
                        if c.sees(dx,dy) and get1(sf.s,dx,dy)==get1(sf.s,c.mx,c.my):
                                # visible from position
                                s=sf.des[k][0]
                r=Sprint(30,l,s,'n')
		return r+(r>l)		# A TESTER, ok si r>l = 1 ou 0

        def listgo(sf,c,l,e):
                # adds the gen obj's description to the screen 0
                r=l
                for k in sf.gos.keys():
                        # for all the lists of generic objects
                        dx,dy=PosS(k)
                        a=dx-c.mx+14;b=dy-c.my+8
                        if c.sees(dx,dy) and inview(a,b):
                                # sees the corresponding square !
                                for o in sf.gos[k]:
                                        # for each generic item to display
                                        col=e(o)
                                        if col!="":
                                                # test succeded, display it
                                                put(0,a,b,o.al,col)
                                                if c!=o:
                                                        # display the text
                                                        l=Sprint(30,l,o.des,col)

                if l>r: l=l+1
                return l

        def update(sf):
                # updates the map each tick
                def tmp(c): c.update()
                Xdicval(sf.gos,tmp)

        def srch(sf,k,a,t):
                # searches for an object
                if sf.gos.has_key(k):
                        # test each object
                        for o in sf.gos[k]:
                                # for each object
                                if find(lower(o.nm),a)>=0 and t(o): return o
                                        # object found
                return None
                # object not found

        def load_descr(sf,l):
                # loads the descriptions of the rooms

                s,l=rdln(l);nb=atoi(s)
                while nb>0:
                        p,l=rdln(l)     # read the position, X;Y
                        d,l=rdln(l)     # read the description with õ for indent
                        adic(sf.des,p,d)
                        nb=nb-1
                return l

        def load_mobs(sf,l):
                # loads the mobs of the map
                s,l=rdln(l);nb=atoi(s)
                while nb>0:
                        c=NPC()
                        c.nm,l=rdln(l)
                        c.al,l=rdln(l)
                        c.des,l=rdln(l)
                        s,l=rdln(l);x=atoi(s)
                        s,l=rdln(l);y=atoi(s)
                        c.mem=Queue(10)
                        c.qs=10
                        c.st=10
                        teleport(c,sf,x,y)
                        nb=nb-1
                return l

        def load_objs(sf,l):
                # loads the items of the map
                s,l=rdln(l);nb=atoi(s)
                while nb>0:
                        o=Obj()
                        o.nm,l=rdln(l)
                        o.des,l=rdln(l)
                        s,l=rdln(l);x=atoi(s)
                        s,l=rdln(l);y=atoi(s)
                        s,l=rdln(l);o.wear=atoi(s)
                        o.al,l=rdln(l)
                        o.ldes,l=rdln(l)
                        o.dam,l=rdln(l)
                        teleport(o,sf,x,y)
                        nb=nb-1

class Obj(GenObj):
        # an object

        def __init__(sf): GenObj.__init__(sf);sf.dam=""
                # creates a new item

        def ischar(sf): return 0
        def isobj(sf): return 1

        def update(sf): pass

class Character(GenObj):
        # a character (PC or NPC)

        def __init__(sf):
                # creates a new character
                GenObj.__init__(sf)

                sf.opts=[]       # options used on this character
                sf.lm=0          # last movement date
                sf.ld=""         # last direction
                sf.inv=[]        # inventory of the character
                sf.eq=[None]*10  # equipment of the character
                sf.pl=0          # prompt line
                sf.mpl=37        # number of lines in a screen
                sf.qs=10         # memory queue size
                sf.mem=Queue(sf.qs) # memory queue
                sf.scp=0        # starting char pos and light value

                sf.ib=""        # input buffer

        def ischar(sf): return 1
        def isobj(sf): return 0

        def sees(sf,x,y): return vis(sf.map.s,sf.mx,sf.my,x,y)
        def seec(sf,c): return sf.sees(c.mx,c.my)
                # does this character sees the given position ?

        def getact(sf):
                # returns the next action

                p=find(sf.ib,"\n")
                if p>=0:
                        a=sf.ib[:p]
                        sf.ib=sf.ib[p+1:]
                        return replace(a,"\r","")

                return ""

        def getarg(sf):
                # returns the next argument

                if sf.tcom==[]: return ""
                else:
                        s=sf.tcom[0]
                        sf.tcom=sf.tcom[1:]
                        return lower(strip(s))

        def getrest(sf): return strip(join(sf.tcom))
                # returns everything else

        def snd(sf,s): pass
                # snds a string to the character

        def move(sf,sx,sy,d):
                # makes the given character move in a direction
                if isblock(get1(sf.map.s,sf.mx+sx,sf.my+sy)):
                        # can't move
                        sf.snd(strs[40]+RL)
                        return

                if sf.lm==0:
                        # starting to move
                        sndvis(sf,sf.mx,sf.my,sf.nm+strs[26]+d+"."+RL,1)
                if sf.lm>0 and sf.ld!=d:
                        # turns
                        sndvis(sf,sf.mx,sf.my,sf.nm+strs[25]+d+"."+RL,1)
                # continue to move
                teleport(sf,sf.map,sf.mx+sx,sf.my+sy)
                for lc in sf.map.gos.values():
                        # for all the characters in the map
                        for c in lc:
                                # c is now the character

                                if c.ischar() and not(vis(sf.map.s,c.mx,c.my,sf.mx-sx,sf.my-sy)):
                                        if vis(sf.map.s,c.mx,c.my,sf.mx,sf.my):
                                                # wasn't visible before, but is now
                                                if not('pos' in c.opts) or c.pl>=c.mpl:
                                                        c.snd(RL2+sf.nm+strs[24]+RL)
                                                        c.validate()
                                                putxy(c,sf.mx,sf.my,sf.nm[0],'j')

                                if c.ischar() and vis(sf.map.s,c.mx,c.my,sf.mx-sx,sf.my-sy):
                                        putxy(c,sf.mx-sx,sf.my-sy,get1(c.map.s,sf.mx-sx,sf.my-sy),'n')
                                        if not(vis(sf.map.s,c.mx,c.my,sf.mx,sf.my)):
                                                # was visible before but isn't now
                                                if not('pos' in c.opts) or c.pl>=c.mpl:
                                                        c.snd(RL2+sf.nm+strs[23]+RL)
                                                        c.validate()
                                        else:
                                                putxy(c,sf.mx,sf.my,sf.nm[0],'j')


                if not('cls' in sf.opts):
                        # doesn't clear the screen => message
                        sf.snd(strs[22]+d+"."+RL2)
                sf.look()
                sf.lm,sf.ld=1,d

        def srch_o(sf,a,l):
                # searches an item of the given name
                global isf
                isf=a
                def tmp(o): return find(lower(o.nm),isf)>=0
                return slist(l,tmp)

        def srch_c(sf,a):
                # searches for a character of the given name
                global isf,jsf
                isf=sf
                jsf=a
                def tmp(o): return(find(lower(o.nm),jsf)>=0 and o.ischar() and isf.seec(o))
                for l in sf.map.gos.values():
                        # for each entity of the map
                        r=slist(l,tmp)
                        if r!=None: return r
                return None

        def listeq(sf,c):
                # lists to someone the equipment
                for i in range(0,10):
                        # for each item
                        o=sf.eq[i]
                        c.snd(eqname[i]+"\t\tõc")
                        if o!=None: c.snd(o.nm+"õn"+RL)
                                # item exists..
                        else: c.snd(strs[33]+RL)
                                # item doesn't exist :(

        def acteq(sf,a): sf.snd(strs[21]+RL);sf.listeq(sf)
                # looks at the equipment

        def actinv(sf,a):
                # looks at the equipment
                sf.snd(strs[20]+RL)
                         
                for o in sf.inv: sf.snd("õc"+o.nm+"õn"+RL)
                        # for each item
                        
                if len(sf.inv)==0: sf.snd(strs[19]+RL)
                        # owns nothing !
                                 
        def actget(sf,a):
                # gets an item
                k=SPos(sf.mx,sf.my)
                def tmp(o): return o.isobj()
                o=sf.map.srch(k,sf.getarg(),tmp)
                if o!=None:
                        # object found
                        sf.event("You get "+o.nm+"."+RL)
                        sndvis(sf,sf.mx,sf.my,sf.nm+" gets "+o.nm+"."+RL,0)
                        sf.inv[:0]=[o]
                        teleport(o,None,0,0)
                        # don't forget to remove the item from the world
                else: sf.snd(strs[18]+RL)
                        # object not found

        def actdrop(sf,a):
                # drops an item
                a=sf.getarg()
                for o in sf.inv:
                        if find(lower(o.nm),a)>=0:
                                # item found
                                sf.event("You drop "+o.nm+"."+RL)
                                sndvis(sf,sf.mx,sf.my,sf.nm+" drops "+o.nm+"."+RL,0)
                                sf.inv.remove(o)
                                teleport(o,sf.map,sf.mx,sf.my)
                                # puts the item on the world's ground
                                return

                sf.snd(strs[34]+RL)
                # failed !

#        def doremove(sf,o):
#                # removes the given item if equipped
#                if len(sf.inv)<20:
#                        # removes the item
#                        sf.eq[o.ip]=None
#                        sf.event(strs[41]+o.nm+"."+RL)
#                        sf.inv[:0]=[o]
#                        o.ip=-1
#                        sndvis(sf,sf.mx,sf.my,sf.nm+" removes "+o.nm+"."+RL,0)
#                else: sf.event(strs[42]+RL)
#                        # not enough space to remove it

#        def actwear(sf,a): pass
#                # wears an item
#                a=sf.getarg()
#                o=sf.srch_o(a,sf.inv)
#                if o==None: sf.snd(strs[34]+RL);return
#                # owns the thing..test it
#
#                if sf.eq[o.wear]!=None:
#                        # removes the previous item, first
#                        sf.doremove(sf.eq[o.wear])
#                        if sf.eq[o.wear]!=None: return
#                                # can't remove it !
#
#                sf.inv.remove(o)
#                # removes it from the inventory
#                sf.eq[o.wear]=o
#                o.ip=o.wear
#                # adds it as the first object
#
#                a="wear "
#                if o.wear==9: a="wield "
#                if o.wear==8: a="light "
#
#                sf.event("You start to "+a+o.nm+"."+RL)
#                sndvis(sf,sf.mx,sf.my,sf.nm+strs[43]+a+o.nm+"."+RL,0)
#                # messages to the room      

#        def actremove(sf,a):
#                # removes an item
#                o=sf.srch_o(sf.getarg(),sf.eq)
#                if o!=None: sf.doremove(o);return
#                        # removes it
#                sf.snd(strs[35]+RL)

        def doexam_o(sf,o):
                # makes examine the object
                sf.snd(strs[36]+o.nm+"."+RL)
                if o.ldes!="": sf.snd("õc"+o.ldes+"õn"+RL)
                        # has a long description
                else: sf.snd(strs[37]+RL)
                        # nothing to look at

        def doexam_c(sf,c):
                # makes examine the character
                if c==sf: sf.acteq("")
                        # examine yourself !
                else:
                        sf.snd(strs[38]+c.nm+"..."+RL);sf.snd("õc"+c.ldes+"õn"+RL);c.listeq(sf)

        def actlook(sf,a):
                # makes the character look
                a=sf.getarg()
                if a=="": sf.look();return
                        # simple look

                c=sf.srch_c(a)
                if c!=None: sf.doexam_c(c);return
                        # character to look, found

                def tmp(o): return o.isobj()
                o=sf.map.srch(SPos(sf.mx,sf.my),a,tmp)
                if o==None:
                        o=sf.srch_o(a,sf.inv)
                        if o==None: o=sf.srch_o(a,sf.eq)
                        # searches for an item in the inventory or equipment

                if o!=None: sf.doexam_o(o);return
                        # examine object
                sf.snd(strs[39]+RL)

        def actmove(sf,a):
                # makes the character move
                if a[0]=='n': sf.move(0,-1,"north")
                if a[0]=='s': sf.move(0,1,"south")
                if a[0]=='e': sf.move(1,0,"east")
                if a[0]=='w': sf.move(-1,0,"west")

        def actwho(sf,a):
                # makes a "who"
                sf.snd(strs[17]+RL+"=-"*12+RL);n=0
                for c in clis.values():
                        # for each client
                        if c.st>9: sf.snd("õc"+c.nm+"\tõn|"+RL);n=n+1

                sf.snd(strs[16]+str(n)+"õn"+RL)

        def actemote(sf,a):
                # emotes something
                a=sf.getrest()
                sf.event("õj"+sf.nm+" "+a+"õn"+RL)
                sndvis(sf,sf.mx,sf.my,"õj"+sf.nm+" "+a+"õn"+RL,0)

        def actgossip(sf,a):
                # gossips something
                a=sf.getrest()
                sf.event(strs[14]+a+"õn'"+RL)
                sndall(sf,sf.nm+strs[15]+a+"õn'"+RL)
                                          
        def actsay(sf,a):
                # says something
                a=sf.getrest()
                sf.event(strs[13]+a+"õn'"+RL)
                sndvis(sf,sf.mx,sf.my,sf.nm+" says 'õc"+a+"õn'"+RL,0)
                                                        
        def do(sf,a):
                # makes the character act

                sf.tcom=split(a)
                a,nc,ml=sf.getarg(),"",0
                for c in com.keys():
                        # for each command
                        if find(lower(c),a)==0 and hasattr(sf,strip(com[c])):
                                # the command does exist
                                if len(com[c])>ml: ml=len(com[c]);nc=c

                if ml>0: getattr(sf,strip(com[nc]))(nc)
                else: sf.snd(strs[12]+RL);
                sf.validate()

        def input(sf):
                # checks the character's activities

                a=sf.getact();
                if a!="": sf.do(a)
                        # do something

        def validate(sf):
                # validates the operation
                sf.snd(RL);sf.snd("H:õr12/37õn M:õb63/88õn M:õv163/163õn > ")

        def event(sf,s):
                # snds an event to the client
                if sf.mem.full(): sf.mem.get()
                sf.mem.put(remcol(s));sf.snd(s)

        def look(sf):
                # makes the player look
                if 'cls' in sf.opts: sf.snd(seqANSI['cls']);sf.pl=sf.scp
                        # clear screen activated in the options

                init(0,80,35);          # inits a new screen
                put(0,0,0,"=-"*39,'n')
                put(0,0,16,"=-"*14,'n')
                put(0,28,16,"=",'n')
                lookmap(0,sf.map.s,sf.mx,sf.my);l=2
                # Part I: building the wilderness map

                l=sf.map.descr(sf,l)
                # Part II: searching the room's description

                def tmp(o):
                        if o.isobj(): return "v"
                        return ""
                        # condition to display an object
                l=sf.map.listgo(sf,l,tmp)
                # Part III: items in room

                def tmp(c):
                        # condition to display a character
                        if c.ischar() and c.st>9: return "j"
                        return ""
                l=sf.map.listgo(sf,l,tmp)
                # Part IV: characters in room

                if 'cls' in sf.opts:
                        # part V: recent memory
                        r=Sprint(30,l,"=-"*22,'n')
                        put(0,34,l,strs[11],'c')
                        m=sf.mem;oq=Queue(sf.qs)
                        while not(m.empty()): l=m.get();r=Sprint(30,r,l,'c');oq.put(l)
                                # while there is something in the memory

                        sf.mem=oq

                s=gets(0);sf.snd(s)

        def update(sf):
                # updates data every tick
                if sf.lm>0:
                        sf.lm=sf.lm+1
                        if sf.lm>40: sf.lm=0;sndvis(sf,sf.mx,sf.my,sf.nm+strs[10]+RL,1)
                sf.input()

class NPC(Character):
        # a NPC

        def __init__(sf): Character.__init__(sf)
                # creates a new NPC

        # here the update function should be redefined, adding some AI.

class Client(Character):
        # a client (player)

        def __init__(sf,s):
                # creates a client with a new socket
                Character.__init__(sf)

                s.setblocking(0)
                sf.st,sf.s=0,s  # state of the automat
                teleport(sf,maps[0],23,35);sf.nm='Unknown'
                sf.snd(seqANSI['n']+scrs["login"]+RL+strs[48]+RL+strs[49]+RL2+strs[0])
                # snds the intro screen and asks for the nickname

                sf.ib=""     # input buffer

        def recv(sf):
                # checks and updates what the client typed

                data=sf.s.recv(256);sf.ib=sf.ib+data
                # received some data

        def snd(sf,t):
                if sf.st==-1: return
                        # state: disconnected => don't snd anything
                try:
                        # snds a string to the client
                        if 'col' in sf.opts:
                                # transforms the string to add color
                                for c in seqANSI.keys(): t=replace(t,"õ"+c,seqANSI[c])
                        else: t=remcol(t)
                                # remove the colors in the string
                        sf.s.send(t)
                        sf.pl=sf.pl+count(t,RL)
                except error: sf.disc()
                        # socket was disconnected

        def entergame(sf):
                # makes the player enter the game
                sf.look();sndall(sf,sf.nm+strs[5]+RL);sf.st=10;sf.validate()

        def input(sf):
                # checks the client's activities
                a=sf.getact()
                if a=="": return
                if sf.st>9: sf.pl=sf.pl+1;sf.snd(RL);sf.do(a)
                        # only if the player's in game

                else:
                        # player's not in game, but on the login screens

                        col=RL2+strs[27]+RL+strs[28]+RL+strs[29]+RL+strs[30]+RL+strs[31]+RL2+strs[32]
                        if sf.st==0:
                                # select name
                                sf.nm=capitalize(a);sf.al=sf.nm[0];sf.des=sf.nm+strs[8]
                                sf.snd(strs[1]+sf.nm+RL2+col);sf.st=1
                        elif sf.st==1:
                                # select color
                                try: c=atoi(a)
                                except ValueError: c=5

                                l=sf.opts
                                if c==1: l=l+['pos']
                                if c<=2: l=l+['cls']
                                if c<=3: l=l+['col']
                                if c>4: sf.snd(col);return
                                sf.opts=l
                                if c==1:
                                        sf.snd(RL+strs[44]+RL);sf.snd(strs[45]+RL);sf.snd("[0-1] : ");sf.st=2;return

                                sf.entergame()
                        elif sf.st==2:
                                # select starting char
                                try: c=atoi(a)
                                except ValueError: c=0

                                sf.scp=c;sf.entergame()

        def disc(sf):
                # disconnects the client
                del clis[sf.s]
                sf.s.close()
                if sf.st>9: sndvis(sf,sf.mx,sf.my,sf.nm+strs[46]+RL,0)
                        # if he was in game..?
                sf.st=-1

        def update(sf):
                # updates the player's data
                Character.update(sf);sf.input()

        def actquit(sf,a):
                # makes the character quit
                sf.snd(strs[47]+RL2);sf.disc()

def RunMud():
        # creates and run the mud

        ls=socket(AF_INET,SOCK_STREAM)
        ls.bind("",4001)
        ls.listen(50)
        ls.setblocking(0)
        # creates the listening connection

        lm=listdir('.')
        for x in lm:
                # check if this file is a map file
                if match('map[0-9]*.bin',x)>=0: maps[:0]=[Map(atoi(x[3:find(x,'.')]))]
                        # the file matches !
        # load maps

        while 1:
                try:
                        k=clis.keys()+[ls]
                        l1,l2,l3=select(k,k,k,0)

                        if l1!=[]:
                                # input events
                                for x in l1:
                                        # check out all the connections
                                        if x==ls:
                                                # accepts a new connection
                                                conn,addr=x.accept();clis[conn]=Client(conn)
                                                # adds the client to the client list
                                        else:
                                                # other input event
                                                try: clis[x].recv()
                                                except KeyError: pass
                                                        # it happens when sockets are broken

                                
                finally:
                        # other case
                        global tck;tck=tck+1
                        for m in maps: m.update()
                                # update each map
                        sleep(0.1)

def sndall(cl,s):
        # snds a string to everybody !
        for c in clis.values():
                # for each client connected in game
                if c!=cl and c.st>9: c.snd(RL2);c.event(s);c.validate()
                        # receives the string

def sndvis(cl,x,y,s,b):
        # snds a string to everybody visible in the room !
        for c in clis.values():
                # for each client connected in game
                if c!=cl and c.map==cl.map and vis(c.map.s,c.mx,c.my,x,y) and c.st>9:
                        # receives the string
                        if b==0 or not('pos' in c.opts) or c.pl>=c.mpl: c.snd(RL2);c.event(s);c.validate()
                                # when can we snd it ?

# Optimisations:
# 1. Teleport de Obj et Character => GenObj
# 2. Pres et Objs dans Map => factoriser
# 3. multiple assignement: a,b=1,2
# 4. Valeurs par defaut dans le passage d'arguments
# 5. If a 1 seule action: sur la meme ligne ! De meme pour les fonctions, for
# 6. Fonctions temporaires (anonymes)(1ligne max): lambda x,y: x+y
# 7. le ';' separe des instructions sur une meme ligne !

# creates the main Mud object
# pack("map1.inf","r")

RunMud()

