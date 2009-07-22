import g,obj

# simple world generator for now

def mkwrld(w,dim):
	rl=[]
	dim
	i=0
	while i<(dim*dim):
		x=w[i]
		o=obj.Object()
		if x=="%":
			o.short="Circle Road."
			o.desc_in="You are on circle road. It runs around the town."
		elif x=="N":
			o.short="North Gate."
			o.desc_in="You are at the north gate."
		elif x=="S":
			o.short="South Gate."
			o.desc_in="You are at the south gate."
		elif x=="W":
			o.short="West Gate."
			o.desc_in="You are at the west gate."
		elif x=="E":
			o.short="East Gate."
			o.desc_in="You are at the east gate."
		elif x=="$":
			o.short="Elpee Way."
			o.desc_in="It is a muddy street."
		elif x=="Z":
			o.short="A shop."
			o.desc_in="If this mud did more, you could buy stuff here."
		elif x=="@":
			o.short="A smaugy street."
			o.desc_in="There's lots of smog here. Heh."
		elif x=="#":
			o.short="A dirty square."
			o.desc_in="It's a square, that is dirty."
			g.startroom=o
		rl.append(o) # append blank room that's ok see below
		i=i+1
	i=0
	while i<(dim*dim):
		# link up exits
		x=w[i]
		if x==".":
			pass
		else:
			t=i+1
			if (t<(dim*dim)) and ((i%9)!=8):
				if w[t]!=".":
					rl[i].exits["east"]=rl[t]
			t=i-1
			if (t>=0) and ((i%9)!=0):
				if w[t]!=".":
					rl[i].exits["west"]=rl[t]
			t=i-9
			if (t>=0):
				if w[t]!=".":
					rl[i].exits["north"]=rl[t]
			t=i+9
			if (t<(dim*dim)):
				if w[t]!=".":
					rl[i].exits["south"]=rl[t]
		i=i+1
	return rl

