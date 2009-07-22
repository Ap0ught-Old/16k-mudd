# event system
#
# This is sloppy, ugly, and filled with magic numbers. It should probably be redone.
#
# The man variable here shouldn't be used as a global. All of the event backend is isolated
# here and EvtMan never needs to be called directly. Inherit from EvtUsr and call methods
# on yourself.
#
# See also event.txt for documentation on the different types of events. I throw events
# around as tuples for speed and space reasons.
#

#EVT_QUEUE_SIZE = 256 <-- search for magic number to change.

import g

class EvtMan:
	def __init__(self):
		self.g_rcvrs={}	# these objects want to get global events
		self.q=[]
		for x in range(0,1024):
			self.q.append([])

	def upd(self):
		# throw events left and right
		tx=g.tick=g.tick+1
		t=tx%1024
		for e in self.q[t]:
			if e[0]==tx:
				self.dispatch(e)
				self.q[t].remove(e)

	def dispatch(self,e):
		if e[1][:2]=="g_":
			for rcvr in self.g_rcvrs[e[1]]:
				try: getattr(rcvr,"hEvt_%s"%(e[1]))(e)
				except AttributeError: pass
		else:
			if e[2].grab_out.has_key(e[1]):
				for m in e[2].grab_out[e[1]]:
					if not m(e): return
			if e[3].grab_in.has_key(e[1]):
				for m in e[3].grab_in[e[1]]:
					if not m(e): return
			try: 
				if not getattr(e[3],"hEvt_%s"%(e[1]))(e): return
			except AttributeError: pass #fixme should probably log something here

man=EvtMan()

class EvtUsr:
	def __init__(self):
		self.grab_out={}
		self.grab_in={}

	def p_evt(self,e):
		# post an event
		if e[0]:
			man.q[e[0]%1024].append(e)
		else:
			man.dispatch(e)

	def rcv_glbls(self,type):
		if man.g_rcvrs.has_key(type): man.g_rcvrs[type].append(self)
		else: man.g_rcvrs[type]=[self]

	def ntrcpt_in(self,o,type,m):
		if o.grab_in.has_key(type):
			o.grab_in[type].append(m)
		else:
			o.grab_in[type]=[m]

	def ntrcpt_out(self,o,type,m):
		if o.grab_out.has_key(type):
			o.grab_out[type].append(m)
		else:
			o.grab_out[type]=[m]

	def stop_ntrcpt_in(self,o,type,m):
		o.grab_in[type].remove(m)

	def stop_ntrcpt_out(self,o,type,m):
		o.grab_out[type].remove(m)

	def pass_evt(self,e,o):
		if o.grab_in.has_key(e[1]):
			for m in o.grab_in[e[1]]:
				if not m(e): return
		try: 
			if not getattr(o,"hEvt_%s"%(e[1]))(e): return
		except AttributeError: pass #fixme should probably log something here		
