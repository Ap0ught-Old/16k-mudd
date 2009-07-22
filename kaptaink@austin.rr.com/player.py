import obj,cmds
from string import *

OO=obj.Object

class Player(OO):
	def __init__(self):
		obj.Object.__init__(self)
		self.cmds=cmds.playerCmds
		self.imaPC=1
		self.aliases={"e":"east","w":"west","n":"north","s":"south","u":"up",
				"d":"down"}

	def interp(self,inp):
		cmd=map(strip,split(inp))
		if cmd:
			if self.aliases.has_key(cmd[0]):
				cmd[0]=self.aliases[cmd[0]]
			if self.loc.exits.has_key(cmd[0]):
				self.p_evt((0,"move_leave",self,self.loc,self.loc.exits[cmd[0]]))
			elif self.cmds.has_key(cmd[0]):
				self.cmds[cmd[0]](self,cmd[1:])
			else:
				self.s.wr("Huh?!")

	def hEvt_look(self,e):
		if (e[2]!=self) and (e[2] in self.loc.inv):
			if e[4][1]=="at":
				self.s.wr("%s looks at %s."%(e[2].short,e[4][0].short))
			elif e[4][1]=="in":
				if not e[4][0]==self.loc:
					self.s.wr("%s looks inside %s."%(e[2].short,e[4][0].short))
				# we don't send a message if they are just looking around
				# the current room.
		OO.hEvt_look(self,e)

	def hEvt_move_enter(self,e):
		if e[2]==self:
			pass
		elif e[3]==self.loc:
			dir="to an unknown location"
			for x in e[3].exits.items():
				if x[1]==e[4]: dir=x[0]
			self.s.wr("%s enters from the %s."%(e[2].short,dir))
		OO.hEvt_move_enter(self,e)

	def hEvt_move_leave(self,e):
		if e[2]==self:
			pass
		elif e[3]==self.loc:
			dir="to an unknown location"
			for x in e[3].exits.items():
				if x[1]==e[4]: dir=x[0]
			self.s.wr("%s leaves, heading %s."%(e[2].short,dir))
		OO.hEvt_move_leave(self,e)

	def hEvt_quit(self,e):
		if e[3]==self.loc or e[3]==self:
			self.s.wr("%s has quit the game."%(e[2].short))
		OO.hEvt_quit(self,e)

	def hEvt_speech(self,e):
		if e[4][0]==self:
			self.s.wr("%s whispers to you, '%s'"%(e[2].short,e[4][1]))
		elif (e[4][0]==None) and (e[3]==self.loc):
			if e[2]==self: self.s.wr("You say, '%s'"%(e[4][1]))
			else: self.s.wr("%s says, '%s'"%(e[2].short,e[4][1]))
		OO.hEvt_speech(self,e)
