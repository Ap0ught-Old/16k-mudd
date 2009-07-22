# login system
import os,g,player
from string import *

class Login:
	def __init__(self):
		#self.s will get set to the socket that's logging in in Srv.hande_accept
		self.showintro=1
		self.interp=self.get_name
		self.eyn="Enter your name: " # just to save space for 16k
		self.eap="Enter a password: " # likewise

	def cnf_newname(self,inp):
		if upper(inp)=='Y' or inp=="":
			self.s.wrn(self.eap)
			self.interp=self.get_newpw
		else:
			self.s.wrn(self.eyn)
			self.interp=self.get_name

	def cnf_newpw(self,inp):
		if inp==self.password:
			p=player.Player()
			p.short=self.name
			p.pw=self.password
			p.kws.append(lower(self.name))
			p.s=self.s
			p.s.o=p
			self.s=None
			g.startroom.insert(p)
			p.p_evt((0,"look",p,p.loc,(p.loc,"in")))
		else:
			self.s.wr("Passwords don't match.")
			self.s.wrn(self.eap)
			self.interp=self.get_newpw

	def get_name(self,inp):
		if len(inp)<3:
			self.s.wr("Name is too short, the minimum length is 3 characters.")
			self.s.wrn(self.eyn)
			return
		if len(inp)>20:
			self.s.wr("Name is too long, the maximum length is 20 characters.")
			self.s.wrn(self.eyn)
			return
		if len(split(inp))>1:
			self.s.wr("Your name may only be one word long.")
			self.s.wrn(self.eyn)
			return
		self.name=capitalize(inp)
		if os.path.exists(os.path.normcase("players/%s/%s")%(lower(self.name[0]),self.name)):
			#FIXME: load char
			self.s.wrn("Password: ")
			self.interp=self.get_pw
		else:
			self.s.wrn("%s is a new character. Is this name ok? [Y/n]"%(self.name))
			self.interp=self.cnf_newname

	def get_newpw(self,inp):
		if len(inp)<3:
			self.s.wr("Password is too short, the minimum length is 3 characters.")
			self.s.wrn(self.eap)
			return
		if len(inp)>12:
			self.s.wr("Password is too long, the maximum length is 12 characters.")
			self.s.wrn(self.eap)
			return
		if len(split(inp))>1:
			self.s.wr("The password may only be one word long.")
			self.s.wrn(self.eap)
			return
		self.password=inp
		self.s.wrn("Confirm password: ")
		self.interp=self.cnf_newpw

	def get_pw(self,inp):
		pass

	def show_intro(self):
		self.s.wr("Welcome to Serenity.")
		self.s.wrn(self.eyn)
