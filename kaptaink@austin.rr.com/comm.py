import asyncore,socket,time,evt,login,g
from string import *

ad=asyncore.dispatcher

# GS class, 'game socket' -- whatever it just sends and receives. =)
class GS(ad):
	def __init__(self,c,o):
		ad.__init__(self,c)
		self.o=o
		self.ib=''
		self.ob=''
		self.disconnect=0
		self.closed=0

	def writeable(self):
		# return 1 if we have data to send
		if self.ob:
			return 1
		return 0

	def handle_read(self):
		# Perhaps this should error check better on the recv but it's long enough
		# already and it hasn't caused any problems yet. This should be solid enough
		# and takes care of character mode clients and, as an added bonus, handles
		# backspaces.
		self.ib=self.ib+self.recv(1024)
		if len(self.ib)>1024:
			self.ib=self.ib[:1024]
			self.wr("Line too long.")
		ns=count(self.ib,'\n')
		while ns>0:
			p=find(self.ib,'\n')
			t=[]
			for ch in self.ib[:p]:
				if ch=='\b':
					if t!=[]: t.pop()
				else: t.append(ch)
			self.o.interp(strip(join(t,'')))
			self.ib=lstrip(self.ib[p:])
			ns=ns-1

	def handle_write(self):
		if not self.closed:
			self.send(self.ob)
			self.ob=''

	def handle_close(self):
		# asyncore is a piece of shit and likes to call this multiple times when
		# the socket is closed so we use a silly variable to keep track of
		# whether the socket is closed or not.
		if not self.closed:
			g.cl.remove(self)
			self.closed=1

	def handle_expt(self):
		if hasattr(self.o,"imaPC"):
			self.o.p_evt((0,"quit",self.o,self.o.loc,None))

	def wr(self,t):
		self.ob=self.ob+t+"\r\n" # write with trailing \r\n

	def wrn(self,t):
		self.ob=self.ob+t # write without trailing \r\n
		
# Srv class, the server -- accepts new connections, runs the game loop, blah blah.
class Srv(ad,evt.EvtUsr):
	def __init__(self):
		ad.__init__(self)
		evt.EvtUsr.__init__(self)
		self.create_socket(socket.AF_INET,socket.SOCK_STREAM)
		self.bind(("",1616))
		self.listen(5)
		self.done=0

	def begin(self):
		self.rcv_glbls("g_shutdown")
		self.rcv_glbls("g_msg")

	def run(self):
		# ct = current time, td = time delta. TC is caps to help avoid
		# confusion with ct.
		TC=time.clock
		ct=TC()
		while not self.done:
			evt.man.upd()
			asyncore.poll()
			for c in g.cl:
				if c.disconnect:
					g.cl.remove(c)
					c.close()
			td=TC()-ct
			if 0.1-td > 0:
				time.sleep(0.1-td)
			ct=TC()
		for c in g.cl:
			c.close()

	def finish(self):
		pass
			
	def handle_accept(self):
		# accept() returns a (socket,addr) tuple. GS inherits from
		# asyncore.dispatcher which wants the socket as its argument, thus the
		# t=GS(self.accept()[0]) bit.
		l=login.Login()
		sock=GS(self.accept()[0],l)
		l.s=sock
		g.cl.append(sock)
		l.show_intro()
		return sock

	# event handlers

	def hEvt_g_shutdown(self,e):
		self.p_evt((0,"g_msg",self,None,"Shutdown in %d ticks."%(e[4])))
		if e[4]==0:
			self.p_evt((0,"l_real_shutdown",self,self,None))
		else:
			self.p_evt((g.tick+e[4],"l_real_shutdown",self,self,None))

	def hEvt_l_real_shutdown(self,e):
		for c in g.cl:
			c.wr("MUD is shutting down.")
		self.done=1

	def hEvt_g_msg(self,e):
		for c in g.cl: c.wr(e[4])
