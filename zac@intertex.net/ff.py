# ff.py - Fresnel function definitions
#
# $Id$
#

import fb, fc, re, string, zlib, pickle, sys, traceback
from copy import deepcopy

cap = string.capitalize; slo = string.lower; sfi = string.find;
sjo = string.join; ssp = string.split; sst = string.strip

# isX (object)
#
# Returns true if the given object is an X:
#   C - Container
#   H - Heap
#   L - Living
#   P - Player
#   R - Room
#   W - Wizard
#
def isC (o): return isinstance(o, fc.C)
def isH (o): return isinstance(o, fc.H)
def isL (o): return isinstance(o, fc.L)
def isP (o): return isinstance(o, fc.P)
def isR (o): return isinstance(o, fc.R)
def isW (o): return isP(o) and o.Z & fb.PWizard


# hasX (object)
#
# Returns true if the given object has the given property X:
#   A - fb.LAggro
#   L - fb.OLite
#   U - fb.OEquip
#
#   S - is in a safe room
#   V - can be seen by u
#
# def hasA (o): return isL(o) and o.Y & fb.LAggro
def hasL (o): return o.X & fb.OLite
def hasU (o): return o.X & fb.OEquip

# def hasS (o):
#   while o is not None and not isR(o): o = o.E
#   if o is not None and isR(o) and o.X & RSafe: return 1
#   return 0

# def hasV (o, u):
#   if isW(u): return 1
#   if hasL(u.E) and not o.X & OInvis: return 1
#   return 0


# dice (string)
#
# Rolls the dice specified in the string, and returns the result.
#
# def dice (s):
#   pass


# fld (filename)
#
# Loads a pickled, zlib-compressed file
#
def fld (fn):
  f = open(fn, 'r')
  return pickle.loads(zlib.decompress(f.read()))


# fsv (filename, data)
#
# Saves a data-element to a pickled, zlib-compressed file
#
def fsv (fn, d):
  f = open(fn, 'w')
  f.write(zlib.compress(pickle.dumps(d), zlib.Z_BEST_COMPRESSION))


# hook (ob, hook, args)
#
# Calls the given hook on the given object with the given args, safely.
#
# Currently disabled for performance and debugging reasons.
#
def hook (o, h, *a, **kw):
  if h[0] == '!':
    return 0
  else: return []
#   def _hmap (o, h, a, kw):
#     try:
#       return apply(h, (o,) + a, kw)
#     except: return None
#
#   if type(o) == type([]):
#     return map(lambda x, h=h, a=a, kw=kw: apply(hook, (x, h) + a, kw), o)
#   elif h[0] == '!':
#     return 0
#     return len(filter(lambda x, o=o, a=a, kw=kw: _hmap(o, x, a, kw), o.H[h]))
#   else:
#     return [ None ]
#     return map(lambda x, o=o, a=a, kw=kw: _hmap(o, x, a, kw), o.H[h])

# oc (ob)
#
# Returns a copy of the object passed
#
def oc (o):
  if isP(o):
    return None
  elif isR(o):
    i = o.I; o.I = []; r = deepcopy(o); o.I = i; r.rc()
    fb.room.append(r); return r
  i = o.I; o.I = []; r = deepcopy(o); o.I = i;
  r.E = fb.Void; r.rc(); r.mov(o.E); return r


# om (name, list, user=None)
#
# Matches one or more objects out of a list.
#
def om (n, l, u=None):
  if n == 'me' and u in l:
    return u
  elif sfi(n, ':') >= 0:
    pass # Some day, this will be for wizards to specify objects
  elif sfi(n, '*') >= 0:
    pass # Some day, this will be for specifying multiple objects
  elif sfi(n, '.') >= 0:
    p, n = ssp(n, '.', 1)

    if p == 'me':
      l = filter(lambda x: x.E == u)
      return om(n, l, s)
    elif p == 'here':
      l = filter(lambda x: x.E == u.E)
      return om(n, l, s)
    else:
      l = filter(lambda x, n=n: n in x.N, l)
      try:
        return l[int(p)]
      except (ValueError, IndexError), err: pass    
  else:
    l = filter(lambda x, n=n: n in x.N, l)
    if len(l) > 0: return l[0]
  return None


# di (ob)
#
# Returns a list of every object in the inventory of the given object. A
# deep search is performed and the result is smashed into a single list.
#
def di (o, r=None):
  if r is None: r = []
  for i in o.I:
    try:
      r.index(i)
    except ValueError:
      r.append(i);
      if len(i.I) > 0: r.extend(di(i, r))
  return r


# li (ob, space=0, wizard=0)
#
# If ob is a list, list the short descriptions of every object in the
# list. If it is a single object, return the long description followed
# by the contents of the object if any.
#
def li (o, s=0, w=0, u=None):
  r = ''
  if type(o) == type([]):
    for i in o:
      if (not w and i.X & fb.OInvis) or i is u or i.S == '': continue
      r = r + (' ' * s) + i.S
      if hasL(i): r = r + ' (lit)'
      if hasU(i): r = r + ' (equipped)'
      r = r + '\n'
  else:
    r = o.D + '\n'; l = li(o.I, 2, w, u)
    if l != '':
      if isR(o):
        r = r + 'You see:\n'
      elif isL(o):
        r = r + cap(o.G.n) + ' is carrying:\n'
      else:
        r = r + '\nIt contains:\n'
      r = r + l
  return r


# afac (object)
#
# If the object is armor, return the absorption percentage. If
# it is not, return None.
#
# def afac (o):
#   pass


# wfac (object)
#
# If the object is a weapon, return the dice-string for its
# damage. Otherwise, return None.
#
# def wfac (o):
#   pass


# msg (message, receipients)
#
# Outputs the given message to the given recipients.
#
def msg (m, r, **kw):
  if type(r) != type([]): r = [ r ]
  kw['text'] = m; map (lambda x, kw=kw: x.rcv(kw), r)


# eng (object, message)
#
# Performs english substitution on the message for the given object.
# This function could definitely be made better.
#
def eng (o, m, sre=string.replace):
  t = m['text']; s = m.get('sub', None); d = m.get('dob', None); i = m.get('iob', None)

  if m.has_key('exc') and o in m['exc']:
    return ''

  if o is s:
    t = sre(t, '\tES', '');
  else:
    t = sre(t, '\tES', 's');

  if s is not None and sfi(t, '\tS') >= 0:
    if o is s:
      t = re.sub('\tS[ns]', 'you'); t = re.sub('\tS[NS]', 'You')
      t = sre(t, '\tSp', 'your'); t = sre(t, '\tSP', 'Your')
    else:
      t = sre(t, '\tSn', s.S); t = sre(t, '\tSN', cap(s.S))
      t = sre(t, '\tSs', s.G.n); t = sre(t, '\tSS', cap(s.G.n))
      t = sre(t, '\tSp', s.G.p); t = sre(t, '\tSP', cap(s.G.p))

  if d is not None and sfi(t, '\tD') >= 0:
    if o is d:
      t = re.sub('\tD[ns]', 'you'); t = re.sub('\tD[NS]', 'You')
      t = sre(t, '\tDp', 'your'); t = sre(t, '\tDP', 'Your')
    else:
      t = sre(t, '\tDn', d.S); t = sre(t, '\tDN', cap(d.S))
      t = sre(t, '\tDo', d.G.o); t = sre(t, '\tDO', cap(d.G.o))
      t = sre(t, '\tDp', d.G.p); t = sre(t, '\tDP', cap(d.G.p))

# if i is not None and sfi(t, '\tI') >= 0:
#   if o is i:
#     t = re.sub('\tI[ns]', 'you'); t = re.sub('\tI[NS]', 'You')
#     t = sre(t, '\tIp', 'your'); t = sre(t, '\tIP', 'Your')
#   else:
#     t = sre(t, '\tIn', i.S); t = sre(t, '\tIN', cap(i.S))
#     t = sre(t, '\tIo', i.G.o); t = sre(t, '\tIO', cap(i.G.o))
#     t = sre(t, '\tIp', i.G.p); t = sre(t, '\tIP', cap(i.G.p))

  return t
