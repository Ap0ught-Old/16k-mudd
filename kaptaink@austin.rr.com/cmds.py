import g
from string import *

def cmd_commands(p,args):
	p.s.wr("Available commands:")
	l=playerCmds.keys()
	l.sort()
	p.s.wrn(join(l,", "))
	p.s.wr(".")

def cmd_look(p,args):
	if len(args):
		if args[0]=="in":
			how="in"
			args=args[1:]
		elif args[0]=="at":
			how="at"
			args=args[1:]
		else:
			how="at"
		what=lower(args[0])
		o=p.find_kw_in(what)
		if o:
			p.p_evt((0,"look",p,p,(o,how)))
		else:
			o=p.loc.find_kw_in(what)
			if o:
				p.p_evt((0,"look",p,p.loc,(o,how)))
			else:
				p.s.wr("You don't see that here.")
	else:
		p.p_evt((0,"look",p,p.loc,(p.loc,"in")))

def cmd_quit(p,args):
	if len(args):
		p.s.wr("Type quit on a line by itself to quit.")
	else:
		p.s.wr("Farewell.")
		p.s.disconnect=1
		p.p_evt((0,"quit",p,p.loc,None))

def cmd_say(p,args):
	p.p_evt((0,"speech",p,p.loc,(None,join(args))))

def cmd_shutdown(p,args):
	# requires a password for now. In the future this should only be an implementor
	# command for obvious reasons.
	# yes I know anyone can look at the source and see the password anyway.
	if len(args):
		if args[0]=="plzplzplz":
			t=0
			if len(args)>1:
				try: t=atoi(args[1])
				except ValueError:
					p.s.wr("Shutdown: Time must be a number!")
					return
			p.p_evt((0,"g_shutdown",p,None,t))
	else:
		p.s.wr("Usage: shutdown <password> [time until shutdown in ticks, default 0]")

def cmd_who(p,args):
	p.s.wr("Players online:")
	for c in g.cl:
		if hasattr(c.o,"imaPC"): p.s.wr(c.o.short)

playerCmds=	{
		"commands"	:	cmd_commands,
		"look"		:	cmd_look,
		"quit"		:	cmd_quit,
		"say"		:	cmd_say,
		"shutdown"	:	cmd_shutdown,
		"who"		:	cmd_who,
		}
