package mm;

import mm.*;

import java.util.*;
import java.net.*;
import java.io.*;

public class MObj extends Object implements Serializable {

	/* Object attributes... */

	static final public int		A_PLR		= 0x0001,
					A_PLACE		= 0x0002,
					A_WORLD		= 0x0004,
					A_ITEM		= 0x0008;

	/* Initial object attributes... */

	static final public int		BS_PLAYER	= 0x0001,
					BS_PLACE	= 0x0002,
					BS_WORLD	= 0x0004,
					BS_ITEM		= 0x0008;

	/* Login states... */

	static final int	S_QNAME		= 1,
				S_ONLINE	= 2,
				S_RECONN	= 3;

	/* String constants... */

	static final String	WORLD		= "world",
				ROOM_ZERO	= "Medusas Garden",
				SFILE		= "medusa.save",
				EAN		= "Enter another name:",
			        NITV		= "Not in the void!";

	/* Command lookup tables... */

	static HashMap	qcmap	= new HashMap(10),
			cmap	= new HashMap(20),
			soul;

	/* Master HashMap that holds everything in the world... */

	public static HashMap things;

	/* References to the special objects for the world and the first room... */

	public static MObj world, rz;

	/* Object attributes - base and current */

	int b_attr, c_attr;

	/* Object name... */

	public String name;

	/* Sock and I/O buffers, only set for connected players... */

	transient Socket sock;

	transient BufferedReader in;

	transient BufferedWriter out;

	/* Players current login state... */

	transient int state;

	/* Vector of all Objects inside this one... */

	public Vector contents = new Vector(5);

	/* The Object that this one is inside... */

	public MObj inside;

	/* Hashtable mapping exit directions to destination names... */

	Hashtable exits = new Hashtable(10);

	/* Initialize the values in the command lookup tables... */

	static void init() {

		qcmap.put("'", new Cmd_Say());
		qcmap.put(",", new Cmd_Emote());
		qcmap.put(".", new Cmd_Gossip());

		cmap.put("look", new Cmd_Look());
		cmap.put("zap", new Cmd_Zap());
		cmap.put("link", new Cmd_Link());
		cmap.put("unlink", new Cmd_Unlink());
		cmap.put("go", new Cmd_Go());
		cmap.put("north", new Cmd_Go("north"));
		cmap.put("south", new Cmd_Go("south"));
		cmap.put("east", new Cmd_Go("east"));
		cmap.put("west", new Cmd_Go("west"));
		cmap.put("up", new Cmd_Go("up"));
		cmap.put("down", new Cmd_Go("down"));
		cmap.put("out", new Cmd_Go("out"));
		cmap.put("say", new Cmd_Say());
		cmap.put("emote", new Cmd_Emote());
		cmap.put("gossip", new Cmd_Gossip());
		cmap.put("tell", new Cmd_Tell());
		cmap.put("get", new Cmd_Get());
		cmap.put("put", new Cmd_Put());
		cmap.put("soul", new Cmd_Soul());
	}

	/* Constructor - name and base attributes... */

	public MObj(String n_name, int bits) {
		name = n_name;
		b_attr = bits;
		c_attr = bits;
	}

	/* Returns name prefixed with 'a statue of' if the object is a disconnected player... */

	public String fname() {

		String fn = name;

		if (is(A_PLR)
		 && sock == null) {
			fn = "a statue of "+fn;
		}

		return fn;
	}

	/* Current attribute manipulation routine... */

	public boolean is(int bits) {
		return ((c_attr & bits) != 0);
	}

	public void set(int bits) {
		c_attr |= bits;
	}

	public void clear(int bits) {
		c_attr &= ~bits;
	}

	/* Login a new player... */

	public void login(Socket n_sock) {

		sock = n_sock;

		/* Obtain I/O queues... */

		try {
			in = new BufferedReader(
					new InputStreamReader(sock.getInputStream()));
			out = new BufferedWriter(
					new OutputStreamWriter(sock.getOutputStream()));
		} catch (Exception e) {

			e.printStackTrace();

			try {
				sock.close();
			} catch (Exception x) {
				x.printStackTrace();
			}

			return;
		}

		/* Add it to MM's list of players... */

		MM.socks.add(this);

		/* Handle login vs reconnect... */

		switch (state) {

			default:

				things.put(name, this);

				state = S_QNAME;
				send(	"Welcome to Medusas Garden\n"+
					"What is your name?");
				break;

			case S_RECONN:

				state = S_ONLINE;
				send(	"Reconnected!");
				break;
		}
	}

	/* A player disconnecting from the mud... */

	public void logout(boolean ok) {

		/* Send a message if a graceful logout... */

		if (ok) {
			issue(	"Bye @a0", 
				"@a0 turns to stone!");
		}

		/* Close the socket... */

		try {
			if (sock != null) {
				sock.close();
			}

		} catch (Exception e) {

			e.printStackTrace();

		}

		/* Lose the socket (even if the close failed)... */

		sock = null;

		/* Send a message if a messy logout... */

		if (!ok) {
			inside.issue( "@a0 turns to stone!");
		}

		/* Remove from MMs list of players... */

		MM.socks.remove(this);
	}

	/* Sometimes a player will manage to commit suicide... */

	public void suicide() {

		issue(	"You disappear up your own backside!",
			"@a0 dissapears up thier own backside!");
		purgeItem(this);
	}

	/* Handle input from a player... */

	public void process(String str) {

		/* Switch on login state... */

		switch (state) {

			/* Input should be the players name... */

			case S_QNAME:

				MObj other;

				/* No-one may login as a guest... */

				if (str.startsWith("guest")) {
					send(	"You may not use that name!\n"+EAN);
					break;
				}

				/* See if thier player object already exists... */

				other = (MObj) things.get(str);

				if (other != null) {

					/* If it does and it's a connected player, send it a message... */
					/* If it's socket is in a mess, this will force a write error   */
					/* and disconnet the player so it can be reconnected to...      */

					if (other.sock != null) {
						other.send("A shiver runs up your spine!");

						if (other.sock != null) {
							send(	"Already logged in!\n"+EAN);
							break;
						}
					}

					/* You can only reconnect to a disconnected player... */

					if (!other.is(A_PLR)) {
						send(	"You cannot connect to that!\n"+EAN);
						break;
					}

					/* Switch the socket to the disconnected player... */

					other.state = S_RECONN;

					other.login(sock);

					sock = null;

					/* Lose this guest object... */

					MM.socks.remove(this);

					things.remove(name);

					/* Say Hi and have a look around... */

					if (other.inside == null) {
						rz.addItem(other);
						other.issue(	"Welcome back @a0",
								"@a0 appears!");
					} else {
						other.issue(	"Welcome back @a0",
								"@a0 returns to normal!");
					}

					other.dispatch("look");

					break;
				}

				/* Rename this object from guestxxx to the name the player gave us... */

				things.remove(name);

				name = str;

				things.put(name, this);

				/* We're now online... */

				state = S_ONLINE;

				/* Place into the first room... */

				rz.addItem(this);

				/* Say Hi and have a look around... */

				issue( 	"Welcome @a0",
					"@a0 appears!" );

				dispatch("look");

				break;

			/* Input should be a command... */

			case S_ONLINE:

				dispatch(str);

				break;

			/* Oh-oh */

			default:

				send("Strange state: "+state);
				logout(true);
				break;
		}
	}

	/* Pass commands from the player onto the appropriate command handler... */

	void dispatch(String str) {

		String cmd, parms;

		/* Resolve prefix commands like ' , and . ... */

		String ch = str.substring(0,1);

		Cmd proc;

		proc = (Cmd) qcmap.get(ch);

		if (proc != null) {
			proc.ex(this, str.substring(1));
			return;
		}

		/* Split into command and parms... */

		cmd = w1(str);

		parms = wr(str);

		/* Handle a few simple/important commands directly... */

		if (cmd.equals("shutdown")) {
			MM.shutdown = true;
			System.out.println("Shutting down");
			return;
		}

		if (cmd.equals("quit")) {
			logout(true);
			return;
		}

		if (cmd.equals("save")) {
			saveWorld();
			return;
		}

		/* Look up and invoke the command handler... */

		proc = (Cmd) cmap.get(cmd);

		if (proc != null) {
			proc.ex(this, parms);
			return;
		}

		/* soul command? */

		Msg sm = (Msg) soul.get(cmd);

		if (sm != null) {
			sendSoul(sm, parms);
			return;
		}

		/* Oh-oh */

		send("Unrecognized command");
	}

	/* Issue a soul message... */

	public void sendSoul(Msg sm, String str) {

		if (inside == null) {
			send(NITV);
			return;
		}	

		MObj v = inside.findItem(w1(str));

		MObj i = findItem(wr(str));

		if (i == null) {
			i = inside.findItem(wr(str));
		}	

		sm.a = this;
		sm.v = v;
		sm.i = i;

		issue(sm);

	}

	/* Create a new room... */

	public static void createRoom(String name) {

		createItem(name, BS_PLACE, world);
	}

	/* Create a new item... */

	public static void createItem(String name, int bits, MObj cont) {

		MObj item = (MObj) things.get(name);

		if (item == null) {

			/* Create... */

			item = new MObj(name, bits);

			/* Index... */

			things.put(name, item);

			/* Imbed... */

			if (cont != null) {
				cont.addItem(item);
			}
		}
	}

	/* Get rid of an item... */

	public static void purgeItem(MObj tgt) {

		/* Some places cannot be pruged... */

		if (tgt == world
		 || tgt == rz ) {
		   return;
		}

		/* Players get logged out first... */

		if (tgt.is(A_PLR)) {
			tgt.send("You are oblitterated by the hand of fate!");
			tgt.logout(true);
		}

		/* Pull the object from its container... */

		if (tgt.inside != null) {
			tgt.inside.removeItem(tgt);
		}

		/* Purge all contained items... */

		for (	Enumeration e = tgt.contents.elements();
			e.hasMoreElements();
			) {

			MObj item = (MObj) e.nextElement();

			purgeItem(item);
		}	

		/* Remove from the global index to complete dereferencing... */

		things.remove(tgt.name);
	}

	/* Create or restore the world... */

	public static void createWorld() {

		/* Try and deserialize the last world... */

		if (!loadWorld()) {

			/* If that went wrong, build the world and room zero from scratch... */

			things = new HashMap(500);

			world = new MObj(WORLD, BS_WORLD);

			things.put(WORLD, world);

			createRoom(ROOM_ZERO);

			rz = (MObj) things.get(ROOM_ZERO);

			soul = new HashMap(100);
		}
	}

	/* Serialize the world to the save file... */

	public static void saveWorld() {

		try {

			/* Open... */

			FileOutputStream os = new FileOutputStream(SFILE);
			ObjectOutputStream p = new ObjectOutputStream(os);

			/* Write... */

			p.writeObject(things);
			p.writeObject(soul);

			/* Close... */

			p.flush();
			os.close();

		} catch (Exception e) {

			e.printStackTrace();
		}
	}

	/* Inflate the world from a serialized file... */

	public static boolean loadWorld() {

		try {

			/* Open... */

			FileInputStream is = new FileInputStream(SFILE);
			ObjectInputStream p = new ObjectInputStream(is);

			/* Suck... */

			things = (HashMap)p.readObject();
			soul = (HashMap)p.readObject();

			/* Close... */

			is.close();

		} catch (java.io.FileNotFoundException e) {

			return false;

		} catch (Exception e) {

			e.printStackTrace();
			return false;
		}

		/* Locate the world and room zero objects... */

		world = (MObj) things.get(WORLD);

		rz = (MObj) things.get(ROOM_ZERO);

		if (world == null
		 || rz == null) {

			world = null;
			rz = null;

			return false;
		}

		/* Successful only if it all worked... */

		return true;
	}

	/* Put one Object inside another... */

	public void addItem(MObj item) {

		if (item.inside != null) {
			item.inside.removeItem(item);
		}

		contents.add(item);

		item.inside = this;
	}

	/* Remove an Object from whatever it's in... */

	public void removeItem(MObj item) {

		contents.remove(item);

		item.inside = null;
	}

	/* Find an object inside another... */

	public MObj findItem(String tgt) {

		for (	Enumeration e = contents.elements();
			e.hasMoreElements();
			) {

			MObj item = (MObj) e.nextElement();

			/* Just check the beginning of the name to allow abbreviations... */

			if (item.name.startsWith(tgt)) {
				return item;
			}
		}

		return null;
	}

	/* Extract a soul from a string and add/update its definition... */

	public static void addSoul(String str) {

		String sn = w1(str);
		str = wr(str).replace('/','|').replace(' ','/').replace('|',' ');

		String am = w1(str).replace('/',' ');
		str = wr(str);

		String om = w1(str).replace('/',' ');
		str = wr(str);

		String vm = w1(str).replace('/',' ');
		str = w1(str).replace('/',' ');

		soul.remove(sn);
		soul.put(sn, new Msg(null, null, null, am, vm, str, om));
	}

	/* Various flavous of issuing messages... */

	public void issue(	MObj v1, MObj v2,
				String m_a, String m_v1, String m_v2, String m_o ) {
		issue( new Msg(this, v1, v2, m_a, m_v1, m_v2, m_o) );
	}

	public void issue(String m_a, String m_o ) {
		issue( new Msg(this, null, null, m_a, null, null, m_o) );
	}

	public void issue(String m_o ) {
		issue( (String) null, m_o);
	}

	/* The actual issue routine... */

	public void issue(Msg msg) {

		/* Rooms and worlds just distribute the message... */

		if (is(A_WORLD | A_PLACE)) {
			msg.dist(this);
			return;
		}

		/* otherwise, the objects containg object distributes it... */

		if (inside != null) {

			msg.dist(inside);

			/* If inside something other than a room, that gets to receive it as well... */

			if (!inside.is(A_WORLD | A_PLACE)) {
				inside.handle(msg);
			}
		}
	}

	public void handle(Msg msg) {

		/* Players receive the appropriate message by role... */

		if (is(A_PLR)) {
			msg.sendTo(this);
		}

		/* Rooms distribute to to everyone in the room... */

		if (is(A_PLACE)) {
			msg.dist(this);
		}
	}

	/* Send a string of bytes to a players socket... */	

	public void send(String str) {

		try {

			if (out != null
			 && sock != null) {
				out.write(cap(str)+"\n");
				out.flush();
			}

		} catch (Exception e) {

			e.printStackTrace();

			/* Disconnect the player if there were problems... */

			logout(false);
		}
	}

	/* Capitalize the first character of a string... */

	public String cap(String str) {

		return (str.substring(0,1).toUpperCase())+(str.substring(1));
	}

	/* First word... */

	public static String w1(String str) {
		int i = str.indexOf(' ');
		if (i == -1) {
		  return str;
		}
		return str.substring(0,i);
	}
	
	/* Rest of string... */
	
	public static String wr(String str) {
		int i = str.indexOf(' ');
		if (i == -1) {
		  return "";
		}
		return str.substring(i+1);
	}
}

/* A class for holding messages... */

class Msg extends Object {

	/* Roles are actor, primary victim and secondary victim... */

	MObj	a, v, i;

	/* Messages are for actor, victim and observers... */

	String msg_a, msg_v, msg_i, msg_o;

	/* Constructor... */

	public Msg (	MObj n_a, MObj n_v, MObj n_i,
			String m_a, String m_v, String m_i, String m_o ) {

		a = n_a;
		v = n_v;
		i = n_i;

		msg_a = n(m_a);
		msg_v = n(m_v);
		msg_i = n(m_i);
		msg_o = n(m_o);
	}

	/* Turn an empty string into a null string... */

	String n(String s) {

		if (s != null
		  &&s.trim().equals("")) {
		  s = null;
		}  
		return s;
	}	

	/* Format and send the appropriate message... */

	public void sendTo(MObj o) {
	
		if (o == a
		 && msg_a != null) {
			o.send(fmt(msg_a,o));
		} else {
			if (o == v
			 && msg_v != null) {
				o.send(fmt(msg_v,o));
			} else {
				if (o == i
				 && msg_i != null) {
					o.send(fmt(msg_i,o));
				} else {
					if (msg_o != null) {
						o.send(fmt(msg_o,o));
					}	
				}	
			}
		}
	}
		
	/* Send the message to every object within a given object... */

	public void dist(MObj place) {

		for (	Enumeration e = place.contents.elements();
			e.hasMoreElements();
			) {
			MObj item = (MObj) e.nextElement();
			item.handle(this);
		}
	}

	/* Format a message, substituting for @ax, @vx, @ix and @ox symbols... */

	String fmt(String t, MObj o) {

		String r = "";

		int x = t.indexOf('@');

		while (x != -1
		     &&x < t.length()-2) {

			if (x > 0) {
				r += t.substring(0,x);
			}	
			
			r += exp(t.charAt(x+1), t.charAt(x+2), o);
			
			t = t.substring(x+3);
			
			x = t.indexOf('@');
		}

		return r + t;
	}

	/* Work out a tokens value name... */

	String exp(char c, char m, MObj o) {
	
		MObj z;
	
		switch (c) {
			case 'a':
				z = a;
				break;
			case 'v':
				z = v;
				break;
			case 'i':
				z = i;
				break;
			case 'o':
				z = o;
				break;
			default:
				return "*@"+c+m+"*";
		}

		if (z==null) {
		  return "nothing";
		}

		switch (m) {
			case '0':
				return z.fname();
			default:
				return "*@"+c+m+"*";
		}		
	}
}

/* Base class for the command handlers... */

class Cmd extends Object {

	/* Work variables... */

	MObj me, inside, world, i2;

	String name;

	Hashtable exits;

	/* This method is always called and hand balls the call to handlers ex2 routine... */

	void ex(MObj n_me, String str) {

		world = MObj.world;

		me = n_me;

		name = me.name;

		inside = me.inside;

		if (inside != null) {
			exits = inside.exits;
			i2 = inside.inside;
		}

		ex2(str);
	};

	/* Dummy ex2 routine, overridden by subclasses... */

	void ex2(String str) {
	};

	/* Pass through methods... */

	void send(String msg) {
		me.send(msg);
	};

	MObj thing(String name) {
		return (MObj) MObj.things.get(name);
	}
}

/* Implementation of the 'look' command... */

class Cmd_Look extends Cmd {

	void ex2(String str) {

		/* Doesn't work in the void... */

		if (inside == null) {
			send("You are in the void!");
			return;
		}

		MObj item;

		/* No parms means look at the current room... */

		if (str == "") {

			/* Location name... */

			send("Location: "+inside.fname());

			/* Objects present... */

			inv(inside);

			/* Available exits... */

			send("Exits: ");

			if (exits.size() == 0
			 && (i2 == null
			  || i2 == world)) {
				send("None!");
			} else {
				String exl = "";
				for (	Enumeration e = exits.keys();
					e.hasMoreElements();
					) {

					String dir = (String) e.nextElement();

					exl += dir+" ";
				}

				/* Out is available if you are inside something that is inside something... */

				if (i2 != null
				 && i2 != world) {
					exl += "out ";
				}

				send(exl.trim()); 
			}

		} else {

			/* See if its an item in the surrounding room... */

			item = inside.findItem(str);

			if (item != null) {

				/* Show name... */

				me.issue(	null, item,
						"You look at @i0",
						null,
						"@a0 looks at you",
						"@a0 looks at @i0");

				/* Show contents... */

				inv(item);

				return;
			}

			/* Is it an exit... */

			String dest = (String) exits.get(str);

			/* Is it the 'out' automatic exit... */

			if (dest == null
			 && str.equals("out")
			 && i2 != null) {
				dest = i2.name;
			}

			if (dest != null) {

				/* Show exit destination... */
			
				me.issue(	str+" leads to "+dest,
						"@a0 looks "+str);
				return;
			}

			/* Tell them it's not there... */

			me.issue(	"You cannot see that!",
					"@a0 is looking for something!");
			
		}
	}

	/* Show the Objects inside an object... */

	void inv(MObj cont) {
	
		send("Visible:");

		boolean seen = false;

		MObj item;

		for (	Enumeration e = cont.contents.elements();
			e.hasMoreElements();
			) {

			item = (MObj) e.nextElement();

			if (item != me) {
				send("  "+item.fname());
				seen = true;
			}
		}

		if (!seen) {
			send ("  nothing");
		}
	}
}

/* Implementation of the 'zap' command... */

class Cmd_Zap extends Cmd {

	void ex2(String str) {

		/* Find the object (anywhere in the world)... */

		MObj tgt = thing(str);

		if (tgt == null) {
			send("No such object!");
			return;
		}

		/* Some things need to be protected... */

		if (tgt == world
		 || tgt == MObj.rz
		 || tgt == me) {
			send("You cannot zap that!");
			return;
		}

		/* Eradicate it (and its contents)... */

		MObj.purgeItem(tgt);

		send("Object deleted");
	}
}

/* Implementation of the 'link' command... */

class Cmd_Link extends Cmd {

	void ex2(String str) {

		/* Doesn't work in the void... */

		if (inside == null) {
			send(MObj.NITV);
			return;
		}

		/* Split into direction and object name... */

		int s1 = str.indexOf(' ');

		String n_exit, n_dest;

		if (s1 == -1) {
			send("Syntax: link exit dest");
			return;
		}

		n_exit = str.substring(0, s1).trim();
		n_dest = str.substring(s1+1).trim();

		if (n_exit.equals("")
		 || n_dest.equals("")) {
			send("Syntax: link exit dest");
			return;
		}

		/* Find the object, if it already exists... */

		MObj dest = thing(n_dest);

		/* a '.' means 'create it here'... */

		if (n_exit.equals(".")) {
			if (dest != null) {
				send("Object already exists!");
				return;
			}
			MObj.createItem(n_dest, MObj.BS_ITEM, null);
			dest = thing(n_dest);
		} else {

			/* Anything else means 'create an exit leading to it'... */
		
			if (dest == null) {
				MObj.createRoom(n_dest);
			}
			dest = thing(n_dest);
		}

		/* Check that worked... */

		if (dest == null) {
			send("Unable to create that!");
			return;
		}

		if (n_exit.equals(".")) {

			/* Add to room... */

			inside.addItem(dest);

		} else {

			/* Add to room exits... */
		
			exits.remove(n_exit);
			exits.put(n_exit, n_dest);
		}
	}
}

/* Implementation of the 'unlink' command... */

class Cmd_Unlink extends Cmd {

	void ex2(String str) {

		if (inside == null) {
			send(MObj.NITV);
			return;
		}

		exits.remove(str);
	}
}

/* Implementation of the 'go' command... */

class Cmd_Go extends Cmd {

	/* Allow instances to have a default parameter specified... */

	String def;

	Cmd_Go(String new_def) {

		def = new_def;

	}

	/* Standard constructed, needed for javaish reasons... */

	Cmd_Go() {
	}

	void ex2(String str) {

		String dname = null;

		MObj dest;

		/* Default a null parameter... */

		if (str.equals("")
		 && def != null) {
			str = def;
		}

		/* Look up the exit, if there are any... */

		if (inside != null) {

			dname = (String) exits.get(str);
			
			if ( dname == null
			  && str.equals("out")
			  && i2 != null) {
				dname = i2.name;
			}
		}

		if (dname != null) {

			/* Look up the destination object... */

			dest = thing(dname);

			/* If not found, trash the exit... */

			if (dest == null) {
				send("The exit vanishes!");
				exits.remove(str);
				return;
			}

		} else {
		
			/* If not an exit, it may be an object... */	

			dest = thing(str);

			if (dest == null) {
				send("No such object!");
				return;
			}
		}

		/* Trying to crawl up your own butt is fatal... */

		if (dest == me) {
			me.suicide();
			return;
		}

		/* Do the move and have a look around... */

		dest.addItem(me);

		me.dispatch("look");
	}
}

/* Implentation of the 'say' command... */

class Cmd_Say extends Cmd {

	void ex2(String str) {

		me.issue(	"You say '"+str+"'",
				"@a0 says '"+str+"'");
	}
}

/* Implementation of the 'emote' command... */

class Cmd_Emote extends Cmd {

	void ex2(String str) {

		me.issue( "@a0 "+str);
	}
}

/* Implementation of the 'gossip' command... */

class Cmd_Gossip extends Cmd {

	void ex2(String str) {

		MM.bcast(name+" gossips '"+str+"'");
	}
}

/* Implementation of the 'tell' command... */

class Cmd_Tell extends Cmd {

	void ex2(String str) {

		int s1 = str.indexOf(' ');

		MObj tgt = MM.findPlayer(str.substring(0,s1).trim());

		if (tgt == null) {
			send("No such player!");
			return;
		}	

		str = str.substring(s1+1).trim();

		send("You tell "+tgt.fname()+": "+str);
		tgt.send(me.name+" tells you: "+str);
	}
}

/* Implementation of the 'get' command... */

class Cmd_Get extends Cmd {

	void ex2(String str) {

		/* Nothing to get in the void... */

		if (inside == null) {
			send(MObj.NITV);
			return;
		}

		/* Locate the Object... */

		MObj item = inside.findItem(str);

		if (item == null) {
			send("That is not here!");
			return;
		}

		/* Some things you cannot put in your pocket... */

		if (item.is(MObj.A_PLACE | MObj.A_WORLD)) {
			send("You cannot get that!");
			return;
		}

		/* Trying to stuff yourself up your own butt is fatal... */

		if (item == me) {
			me.suicide();
			return;
		}

		/* Do the move and tell the world... */

		me.addItem(item);

		me.issue(	null, item,
				"You get @i0",
				null,
				"@a0 gets you",
				"@a0 gets @i0");
	}
}

/* Implementation of the 'put' command... */

class Cmd_Put extends Cmd {

	void ex2(String str) {

		/* Find the object... */

		MObj item = me.findItem(str);

		if (item == null) {
			send("You do not have that!");
			return;
		}

		/* Put it into the room or destroy it if we're in the void... */

		if (inside != null) {
			inside.addItem(item);
		} else {
			MObj.purgeItem(item);
		}

		/* Tell the world... */

		me.issue(	null, item,
				"You drop @i0",
				null,
				"@a0 drops you",
				"@a0 drops @i0");
	}
}

/* Implementation of the 'soul' command... */

class Cmd_Soul extends Cmd {

	void ex2(String str) {

		MObj.addSoul(str);
	}
}

