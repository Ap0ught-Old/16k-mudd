
protoMUD - can you handle this much game in so little code?

Copyright (c) 2000 Blane Bramble and Digital Biscuit Technology

Written for the 16kb MUD competition

License: non-commercial use of this code, or it's derivatives is fine as
long as it is credited - within the source code if the source code is
publically available, or within the copyright or credits message if the
source code is not available. Commercial usage is forbidden without a
specific license.

Email to:	blane@adoniystems.co.uk
		geolin@iowa-mug.net

Source from:	http://www.iowa-mug.net/


Features:

*	100 Simultaneous connections
*	Aggressive mobiles
*	Say, shout and emote commands for interaction
*	Auto-set code resets objects and mobiles continuously as the
	game progresses


Compiling:

The source code should be compiled with gcc, and has been tested on RedHat
Linux 6.0 and Mandrake Linux 7.0. To compile, simply use:

gcc -o protoMUD protoMUD.cc


Running:

protoMUD should happily run in the background, so to execute it use:

./protoMUD &

protoMUD will automatically create a few simple locations, mobiles and
objects when it is run.


Connecting:

The default connection port is 4444, so you should be able to telnet to
that port to connect:

telnet localhost 4444


Shutting down:

Any player can shut the system down by executing the SHUTDOWN command.


Commands:

The following commands are supported:

HELP				Displays the supported commands
SHUTDOWN			Close the server down
WHO				List who is playing
SCORE				Show your score and statistics
LOOK				Redisplay the room description
INVENTORY			Display your inventory
GET <object>			Pick up an object
DROP <object>			Put an object down
OFFER <object>			Offer an object to the gods
GO <direction>			Move in the indicated direction
<direction>			Move in the indicated direction
QUIT				Log out of the game
KILL <mobile>			Attack a mobile
WIELD <object>			Use a weapon
SLEEP				Rest to recover some stamina
SAY <text>			Talk to a player in the same room
SHOUT <text>			Talk to everyone on the game
EMOTE <text>			Express an emotion
FLEE <direction>		Run away!


Notes:

The victim in combat is not considered to be actively engaged in combat
until they have been struck. Thus, if a mobile attacks you, you will
be able to avoid combat by moving to another location if you have not
yet been struck. This works for mobiles too - until you actually land
a blow they will still be free to move location. Put simply, the
intent to start a fight is not enough, you need to actually hit them!

The EMOTE command can be used to express emotions etc. to players in
the same room. Examples are:
	EMOTE grin
	EMOTE point at the Dragon

If you FLEE from a combat you will lose 25% of your score. If you die, you
will lose 50% of your score.

Valid movement directions are NORTH, SOUTH, EAST, WEST, NE, SE, NW, SW,
UP, DOWN, IN and OUT. There are no other valid abbreviations.

Player-player combat is not supported, so there is no PK. There are also
no levels.

