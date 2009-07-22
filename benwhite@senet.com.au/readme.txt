

                         Tank Mage Deathmatch
                      16K Mud Server Competition Entry
                           Ben White  April 2000
                   http://www.senet.com.au/~benwhite/


PRELIMINARIES

This program is provided as is.  Use it at your own risk.  If you use any
of it, please give me some credit.

I come from an LPMud background.  This game plays similar to a stock LPMud,
or any other room based mud.  I tried to design the combat engine for this mud
so it requires a bit of strategy to win, not just who has the most hit points.
Of course, fitting a design into 16K is another matter...  If you have any
good ideas on how this could be improved, please forward them on.

You might notice the mud map is somewhat similar to the stock LPMud 2.4.5
mudlib.  Don't you just love the nostalgic feel?

I would like to thank Sam Davey, Mik Newton, Chris Crouch and Peter Bawden, for
their ideas in making this mud better, and their assistance with player
testing.


COMPILING

The program all resides in a single source file, tmd.c.  Just type
"gcc -o tmd tmd.c" to compile.

This program requires the poll() function, which is at least available on the
machines I have available to me (linux and Compaq Tru64 unix).  According
to the Tru64 man pages, it is a part of the XPG4-UNIX standard (X/Open CAE
Specs?).  I hope this means it's fairly widely available...


RUNNING

The program takes one argument: the port number on which to listed.
Each player can connect to the designated port, and is prompted for a name.
Names must be unique and contain between 3 and 15 chars.  All characters are
converted to lowercase.

The program may be killed with a SIGINT (ctrl-c).


PLAYING

Each player will start in the designated start room.  You can move between
rooms using, n, s, e, w, u, d commands, depending on exits.  You will also
start with a number of health points (HP).

The aim of the game is to defeat other players in magical and melee combat by
spending attack points (AP) to cause each other damage.  Damage is represented
as a loss of HP.  If your HP drops to zero or below, you can consider yourself
killed, and you will restart in a randomly selected room.

Powerups also appear from time to time in selected rooms, and have various
beneficial effects.


COMBAT

Combat is based on attacks and spells.  Attacks are performed by spending AP;
the attacks either damage the opponent or are blocked.  Spells can be cast
by invoking the required runes at a cost of AP; spells either protect, do
damage, or are negated if the target has invoked the negation runes.
Maintaining invoked runes also costs AP; if the player does not have enough
AP to maintain invoked runes, then the runes are automatically invoked.

Every three seconds, you receive another (HP + 7) / 8 AP to spend, up to a
maximum equal to your HP.  No matter what the cost of something, if you have
at least 1 AP, you can perform it; this ensures that players close to death
can still do something useful.

Moving from one room to another costs 2 AP, and all invoked runes are revoked.
Changing targets also causes all runes to be revoked.


TARGETING

You will target all your actions at a specific player.  This player must be
in the same room as you for you to target them.  All rune invocations are
recorded relative to this player.  Invoked runes are lost when changing
targets or moving from room to room.

If you use the target command with no parameters, then you will target the
first player in the room (other than yourself).  If you want to target a
specific player in the room, you can enter either their name or number as
a parameter.  To determine their number, use either the look or who
command.


ATTACKS

Attack costs, damage, and negations:

Lunge 1 AP, damage 1 HP, negated by Chop  or Block
Hack  1 AP, damage 1 HP, negated by Lunge or Block
Slash 1 AP, damage 1 HP, negated by Hack  or Block
Chop  1 AP, damage 1 HP, negated by Slash or Block
Block 3 AP, damage 0 HP

Your attack will be blocked by your target if one of the negation attacks
was performed within the last 9 seconds, and it was among the last three
attacks.


COMBO ATTACKS

If you perform certain attacks in the right order, you will receive a bonus
attack, or combo attack.  Combo attacks can only be blocked by the target if
they have performed all the listed attacks within the last 9 seconds, and they
were among the last three attacks performed.  Once you perform a combo, the
attacks you used for it can no longer be used as defences from other attacks.

Shield Rush:
  Damage: 3 HP
  Requires: Block, Block
  Blocked by: Block

Forward Combo:
  Damage: 3 HP
  Requires: Lunge, Slash
  Blocked by: Block and Hack

Reverse Combo:
  Damage: 3 HP
  Requires: Hack, Chop
  Blocked by: Block and Slash

Body Strike:
  Damage: 3 HP
  Requires: Hack, Slash
  Blocked by: Block and Chop

Overhead Chop:
  Damage: 5 HP
  Requires: Chop, Lunge
  Blocked by: Block and Lunge


RUNES

Spell rune invocation costs (and maintenance costs):
Maintenance costs are charged every three seconds.
Invoked runes are automatically revoked when changing targets, moving from
room to room, used in a spell, or automatically used to defend against a spell.

Rune of Air   1 AP (1 AP)
Rune of Fire  2 AP (1 AP)
Rune of Water 1 AP (1 AP)
Rune of Earth 1 AP (2 AP)


SPELLS

Spells can be cast after the required runes have been invoked.  Area effect
spells do normal damage to the target, and half damage to all others in the
room (excluding the caster).

Magic Missile:
  Cost: 1 AP
  Damage: 2 HP
  Requires: Rune of Air
  Negated by: Rune of Earth

Fireball: Area
  Cost: 2 AP
  Damage: 6 HP
  Requires: Rune of Air, Run of Fire
  Negated by: Rune of Water

Stone Skin: Defensive
  Cost: 1 AP
  Protection: 8 HP
  Requires: Rune of Water, Rune of Earth

Ice Shards:
  Cost: 2 AP
  Damage: 5 HP
  Requires: Rune of Water, Rune of Air
  Negated by: Rune of Earth, Rune of Fire

Poison Cloud: Area
  Cost: 2 AP
  Damage: 6 HP
  Requires: Rune of Water, Rune of Fire
  Negated by: Rune of Air


POWERUPS

Powerups appear from time to time in selected rooms.  If there are players
in the room, then the first player in the room will receive the powerup.
Otherwise, the first player to enter the room will receive the powerup.
Rune powerups are the only way to get runes.

The following powerups are available:
  Rune: Earth, Air, Fire, or Water (randomly selected)
  Health Bonus: increase your HP
  Sword Bonus:  increase your sword damage, to a maximum of +3
  Rune Bonus:   increase the damage of spells, up to +2 per rune


DEATH

After being killed, all your runes will automatically be revoked.  You may
have a random number of runes deducted, and sword and rune power bonuses
will also be randomly lost.  On return from death, you will receive a random
number of HP, and will restart in a randomly selected room.


COMMANDS

score, go <dir>, target, target <player>, look, look <dir>, say, shout, who,
invoke <rune>, revoke, cast <spell>, attack <type>, help, quit, i

Shortened aliases are available:

n	go north
e	go east
s	go south
w	go west
u	go up
d	go down
l	look
ln	look north
ls	look south
le	look east
lw	look west
lu	look up
ld	look down
t	target
ia	invoke air
ie	invoke earth
if	invoke fire
iw	invoke water
r	revoke
cm	cast magic missile
cf	cast fireball
cp	cast poison cloud
cs	cast stone skin
ci	cast ice shards
ah	attack hack
as	attack slash
ac	attack chop
al	attack lunge
ab	attack block
sc	score


SCORE

The score command will tell you the following:
  Your name, and who you are targeting
  Your health points, attack points, sword bonus and number of kills
  Rune bonus, and number of available runes and invoked runes for each type


TIPS

When you see a player, use t to target them.  Sword attacks can be performed
using ah, as, ac, and al.

You'll want to use ab to block attacks.  Remember that a single ab will block
attacks for 9 seconds, if you only use two other sword attacks.  Doing a shield
rush (ab,ab) will remove your blocks and leave you vunerable to attack.

If you want to do a combo attack, perform a block first to leave you at least
partially protected from attack.  To block a combo attack, you'll have to
anticipate what your oppenent is going to perform, and perform the negation
move.  This is virtually impossible if your opponent has a lot of AP.

The Overhead Chop is the most powerful combo (chop-lunge).  So if your
opponent starts a chop, try to do a lunge to block it.  Then return with a
Forward Combo (slash, since you already have the lunge).  This also means that
if you do an Overhead Chop, immediately follow up with a hack to defend against
a Forward Combo.  Then you can finish with a chop or slash for another combo.

You will want to get hold of powerups, especially health bonuses.  These are
the only way you can recover lost HP.

If your oppenent starts invoking runes, try to figure out what spell they're
casting.  Hopefully you'll either have the runes invoked already to be able
to counteract the spell, or be able to quickly invoke them.  Otherwise, try
running or getting stuck into them with your sword.

Lastly, try not to move from room to room too quickly, because it wastes AP.



THE MAP

Here is the current map:

           3
          /
         2--0  15
        /   |  |
       1    |  6--13
            |  |   |\
   11-10-8--4--5---9 16
     /     /   |   |
    12----7   14  17


0   The Village Church
1   Wizards' Hall (Rune bonus)
2   The Elevator
3   The Church Attic (Rune Power bonus)
4   The Village Green
5   The Village Square
6   Bargain Bazaar (Sword bonus)
7   The Deep Well
8   The only road out of town
9   The Esplanade
10  Troll's Bridge
11  The Dark Forest (Rune bonus)
12  Troll's Lair (Sword bonus)
13  The Jetty
14  Harry's Place (Health bonus)
15  The Pub (Health bonus)
16  Swimming at Sea
17  The Corn Fields (Health bonus)


