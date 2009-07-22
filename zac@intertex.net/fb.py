# fb.py - Fresnel base data
#
# $Id$
#

import fc, fx
from ff import *

# Error! Error!
class Error (StandardError):
  pass

# Global Password List
passwd = {}

# Global User List
users = {}

# Global Room List
room = []

# Rooms
#
# Below are the base rooms for the mud. When the mud is saved, only stuff
# actually in a room is saved. If it's not in a room, it will never be
# seen again. All rooms have a unique number: their index in the room
# list.
#
# Void   - everything is initially created here. Nothing can be without
#          an environment.
#
# Dump   - Objects that have been halted are placed here, then have their
#          references removed.
#
# Cryo   - The bodies of players are stored here while not connected.
#
# Edge   - Room numbers are never re-used. When a room is deleted for
#          whatever reason, it is replaced by the Edge, which provides
#          a notice of a bug and an exit to PEntry.
#
# WEntry - Wizards enter the game in this room.
#
# PEntry - Players enter the game in this room.
#
# Empty  - This room is copied with the wizard command 'dig'.
#

def load_res ():
  import fb

  fb.res = fld('lib/res');
  for k in fb.res.keys(): fb.__dict__[k] = fb.res[k]

  fb.room   = fld('lib/room')
  fb.cmds   = fld('lib/cmds')
  fb.passwd = fld('lib/passwd')

  fb.Void   = room[0]
  fb.Dump   = room[1]
  fb.Cryo   = room[2]
  fb.Edge   = room[3]
  fb.WEntry = room[4]
  fb.PEntry = room[5]
  fb.Empty  = room[6]

  cm = []

  for r in room:
    for i in r.I:
      if isP(i):
        fb.users[i.N[0]] = i
        if r is not Cryo: cm.append(i)

  while len(cm) > 0:
    p = cm.pop(); p.mov(Cryo)


def save_res ():
  fsv('lib/res', res)
  fsv('lib/cmds', cmds)


def save_room ():
  fsv('lib/room', room)


def save_pass ():
  fsv('lib/passwd', passwd)


# Everything below this point is actually loaded from the resource files.
# This data is purely for reference. You can use the wizard 'res' and 'ex'
# commands to edit any resource from within the game. The below were
# written in Python purely to speed up the process of constructing the
# game, and were then crunched to resource files.

# GMale   = fc.G('he',   'him',  'himself',    'his')
# GFemale = fc.G('she',  'her',  'herself',    'her')
# GNeuter = fc.G('it',   'it',   'itself',     'its')
# GAmbig  = fc.G('ve',   'ver',  'verself',    'ver')
# GPlural = fc.G('they', 'them', 'themselves', 'their')

# Bits
# B00 = 1 << 0;  B01 = 1 << 1;  B02 = 1 << 2;  B03 = 1 << 3;  B04 = 1 << 4;  B05 = 1 << 5;  B06 = 1 << 6;  B07 = 1 << 7;
# B08 = 1 << 8;  B09 = 1 << 9;  B10 = 1 << 10; B11 = 1 << 11; B12 = 1 << 12; B13 = 1 << 13; B14 = 1 << 14; B15 = 1 << 15;
# B16 = 1 << 16; B17 = 1 << 17; B18 = 1 << 18; B19 = 1 << 19; B20 = 1 << 20; B21 = 1 << 21; B22 = 1 << 22; B23 = 1 << 23;
# B24 = 1 << 24; B25 = 1 << 26; B26 = 1 << 26; B27 = 1 << 27; B28 = 1 << 28; B29 = 1 << 29; B30 = 1 << 30; B31 = 1 << 31;

# Object/Room Flags
# OLite     = B00
# OEquip    = B01
# OInvis    = B02
# ONoGet    = B03
# ONoPut    = B04
# ONoDrop   = B05
# ONoGive   = B06
# OWHold    = B16
# OWParry   = B17
# OWAltDam  = B18
# OUAttrib  = B16|B17|B18
# OUFactor  = B19|B20|B21|B22|B23; OUFShift = 19
# RWizard   = B24
# RSafe     = B25
# RQuiet    = B26
# RDark     = B27
# ROutdoors = B28

# Equipment Flags
# UWear   = B00|B01|B02|B03|B04|B05|B06|B07|B08|B09|B10|B11|B12|B13|B14|B15|B16|B17|B18|B19|B20|B21|B22|B23|B24
# UHold   = B25|B26
# UItem   = B27|B28|B29|B30
# UNeck   = B00
# UHead   = B01|B02|B22|B23
# URFoot  = B03
# URHand  = B04
# URLeg   = B05
# URArm   = B06|B07
# UGroin  = B08|B16
# URTorso = B09|B10
# UCTorso = B11|B12|B13
# ULTorso = B14|B15
# ULArm   = B17|B18
# ULLeg   = B19
# ULHand  = B20
# ULFoot  = B21
# UBack   = B24
# UPri    = B25
# USec    = B26
# URead   = B27
# UUse    = B28
# UQuaff  = B29
# ULight  = B30

# Living Flags
# LPlayer = B00
# LAggro  = B01

# Player Flags 
# PWizard = B00
