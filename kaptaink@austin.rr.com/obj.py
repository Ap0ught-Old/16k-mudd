# object classes (yay)

import string,evt,g

class Object(evt.EvtUsr):
	def __init__(self):
		evt.EvtUsr.__init__(self)
		# these are the basic variables all objects must have
		self.short=""
		self.desc_ext="" # exterior desc
		self.desc_in=""  # interior desc
		self.exits={}
		self.kws=[]	# keywords
		self.loc=None	# the location of this object. None if not inside anything
		self.inv=[]	# the contents of the object.

# This next bunch of functions makes up the event handling crew. These are basic events
# can be called on any objects. Some of these you will want to define in some way for
# every object. Others may just use the base ones defined here.

	def hEvt_look(self,e):
		if e[4][0]==self:
			if e[4][1]=="at":
				e[2].s.wr(self.short)
				e[2].s.wr(self.desc_ext)
			elif e[4][1]=="in":
				e[2].s.wr(self.short)
				e[2].s.wr(self.desc_in)
				k=self.exits.keys()
				if k:
					e[2].s.wrn("Exits [")
					for x in k: e[2].s.wrn(" %s"%(x))
					e[2].s.wr(" ]")
				e[2].s.wr("You see:")
				for o in self.inv:
					e[2].s.wr("  %s"%(o.short))
		for o in self.inv:
			self.pass_evt(e,o)

	def hEvt_move_enter(self,e):
		if e[3]==self:
			self.insert(e[2])
			self.p_evt((0,"look",e[2],self,(self,"in")))
		for o in self.inv:
			self.pass_evt(e,o)

	def hEvt_move_leave(self,e):
		if e[3]==self: # this is the location the originator is leaving
			self.remove(e[2])
			self.p_evt((0,"move_enter",e[2],e[4],self))
		for o in self.inv:
			self.pass_evt(e,o)

	def hEvt_quit(self,e):
		if e[2] in self.inv:
			self.remove(e[2])
		for o in self.inv:
			self.pass_evt(e,o)

	def hEvt_speech(self,e):
		for o in self.inv:
			self.pass_evt(e,o)

# Utility functions

	def send_to_pcs_in(self,t,xcpt=[]):
		# send text to everything in the room that has a socket
		for o in self.inv:
			if hasattr(o,"imaPC"):
				if o not in xcpt: o.s.wr(t)

	def find_kw_in(self,kw):
		# find something in our inventory using a keyword
		which=1;this=0
		if kw[0] in string.digits:
			which=atoi(kw[0])
			kw=kw[2:]
		for o in self.inv:
			if kw in o.kws:
				this=this+1
				if this==which: return o
		return None

	def find_obj_in(self,obj):
		# try to match something in our inventory to obj
		for o in self.inv:
			if o==obj:
				return o
		return None

	def insert(self,obj):
		obj.loc=self
		self.inv.append(obj)

	def remove(self,obj):
		obj.loc=None
		self.inv.remove(obj)
