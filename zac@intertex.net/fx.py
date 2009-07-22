# fx.py - Fresnel executables
#
# $Id$
#
# All commands are stored here in alphabetical order. Player
# commands, then wizard commands.

import fb, fc, fx
from ff import *

# Player Commands

# emote (string)
#
def _emote (u, r, a, o1, o2):
  if a:
    if a[0] == "'":
      msg('%s%s\n' % (u.S, a), u.E)
    else: msg('%s %s\n' % (u.S, a), u.E)
  else: msg('You do nothing.\n', u)


# go (direction)
#
# def _go (u, r, a, o1, o2):
#   pass


# look (object)
#
def _look (u, r, a, o1, o2):
  if len(r) == 0:
    o = u.E
    if (isR(o)):
      if (isW(u)):
        msg('\n[%s] %s ' % (repr(o._), o.S), u);
      else: msg('\n%s ' % o.S, u);
      if len(o.T.keys()) > 0:
        msg('(%s)\n' % sjo(o.T.keys(), ', '), u)
      else: msg('(no exits)\n', u)
  elif len(r) == 2: o = o2
  else: o = o1

  if o is None:
    msg('You do not see that here.\n', u)
  else: msg(li(o, u=u), u)


# quit ()
#
def _quit (u, r, a, o1, o2):
  msg(fb.TQuit, u); k = u.K; u.disco(); k.close()


# say (string)
#
def _say (u, r, a, o1, o2):
  if a:
    msg('\tSN say\tES: %s\n' % a, u.E, sub=u)
  else: msg('You mumble to yourself.\n', u)


# tell (player, string)
#
def _tell (u, r, a, o1, o2):
  print o1
  if o1 is None or o1.K is None:
    msg('That person is not connected.\n', u)
  elif u == o1 or not o2:
    msg('You mumble to yourself.\n', u)
  else:
    msg('\tSN tell\tES \tDn: %s\n' % o2, [ u, o1 ], sub=u, dob=o1)


# who ()
#
# def _who (u, r, a, o1, o2):
#   pass


# Wizard Commands

# ex (code)
#
def _ex (u, r, a, o1, o2):
  try:
    exec a
  except:
    err = sys.exc_info()
    err = sjo(apply(traceback.format_exception, err), '')
    msg(err, u)


# goto (user or room number)
#
def _goto (u, r, a, o1, o2):
  o = u.E
  try:
    print repr(a)
    if fb.users.has_key(a):
      if u.mov(fb.users[a].E): raise fb.Error
    else:
      d = int(a);
      if u.mov(fb.room[d]): raise fb.Error
    msg(fb.TBamfOut, o, sub=u)
    msg(fb.TBamfIn, u.E, sub=u, exc=[u])
    _look(u, (), '', None, None)
  except fb.Error: msg('You stay where you are.\n', u)
