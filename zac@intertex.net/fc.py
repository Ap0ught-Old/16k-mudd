# fc.py - Fresnel class definitions
#
# $Id$
#
# Important abbreviations used are:
#   _ - object identifier (integer or None)
#
#   H - object hook functions (dictionary)
#
#   G - gender of an object (object)
#   Q - quantity of a heap (integer)
#
#   I - inventory (list)
#   E - environment (Object)
#
#   T - exits of a room (dictionary)
#
#   N - names (list of strings)
#   P - plurals (list of strings)
#   S - short description (string)
#   D - full description (string)
#
#   W - weight of the object in kilograms (integer)
#   V - volume (capacity) of the object (integer)
#
#   X - flag bits of the object (integer)
#   U - equip bits of the object (integer)
#
#   A - attributes of a living (dictionary)
#   Y - flag bits of a living (integer)
#   Z - flag bits of a player (integer)
#   K - player connection object (Konnection)

import copy, fb, md5

from ff import *
from asyncore import dispatcher

# Konnection Phases
# KP_LONG = -2 # Entering multi-line text
# KP_MORE = -1 # Reading large texts
KP_PLAY = 0
KP_USER = 1
KP_PASS = 2
KP_NEWU = 3
KP_NEWP = 4
KP_NEWG = 5

# Konnection Translation Table - NOT a 'raw string'
K_TRANS = re.compile('[^\n\x20-\x7e]')

# Validation Regexes
V_NAME = re.compile(r'^[a-z]{3,16}$')
V_PASS = re.compile(r'''^.+[0-9-_!@#$%^&*()[]{}<>=+;:'",./?].+$''')
V_PASS = re.compile(r'.')
V_CMDS = re.compile(r"^[a-z\t':]+$")


# Konnection
class K (dispatcher):
  def __init__ (s, k):
    dispatcher.__init__(s, k)
    s.buf = s.out = s.text = s.last = ''
    s.lag = 0; s.body = None; s.play = KP_USER

    s.msg(fb.TGreet)

  def __reduce__ (s): return None

  # line () - returns the next entered line or None
  def line (s):
    if s.lag > 0:
      s.lag = s.lag - 1
    else:
      p = sfi(s.buf, '\n')
      if p >= 0:
        l = s.buf[:p]; s.buf = s.buf[p+1:]
        if l == '!!' or l == '%%':
          return s.last
        else:
          s.last = l; return l
    return None

  # phase () - handles the current login phase
  def phase (s):
    l = s.line()
    if l is None: return

    if s.play == KP_USER:
      s.name = l
      if not V_NAME.match(l):
        s.msg(fb.TInvUser)
      if not fb.passwd.has_key(l):
        s.msg(fb.TNoUser)
        s.play = KP_NEWU
      else:
        s.msg('Pass: '); s.play = KP_PASS
    elif s.play == KP_PASS:
      m = md5.md5(l); s.pswd = m.digest()
      if s.pswd != fb.passwd[s.name]:
        s.msg('Sorry.\n'); s.close()
      s.body = fb.users[s.name];
      if s.body.K is not None: s.body.K.close()
      s.body.K = s
      if isW(s.body):
        s.body.mov(fb.WEntry)
      else: s.body.mov(fb.PEntry)
      s.msg('\n\n' + fb.TOldEntry); s.buf = 'look\n' + s.buf; s.play = KP_PLAY
    elif s.play == KP_NEWU:
      l = slo(l)
      if l in ['y', 'yes']:
        s.msg(fb.TNewPass)
        s.play = KP_NEWP
      else:
        s.msg('\nUser: '); s.play = KP_USER
    elif s.play == KP_NEWP:
      if len(l) < 5 or not V_PASS.match(l):
        s.msg(fb.TInvPass)
      else:
        m = md5.md5(l); s.pswd = m.digest()
        s.msg(fb.TNewGender)
        s.play = KP_NEWG
    elif s.play == KP_NEWG:
      l = slo(l)
      if l == 'm': g = fb.GMale
      elif l == 'f': g = fb.GFemale
      elif l == 'n': g = fb.GNeuter
      elif l == 'a': g = fb.GAmbig
      else:
        s.msg(fb.TInvGender)
        return

      s.body = P(N=[ s.name ], S=cap(s.name), D=cap(s.name), G=g, K=s); s.body.mov(fb.PEntry)
      fb.users[s.name] = s.body; fb.passwd[s.name] = s.pswd; fb.save_pass()

      s.msg('\n\n' + fb.TNewEntry); s.buf = 'look\n' + s.buf; s.play = KP_PLAY

  # handle_read () - dispatcher callback
  def handle_read (s, trans=re.sub):
    inp = s.recv(1024)
    if inp is None:
      if s.body is not None: s.body.disco()
      s.close()
    else:
      s.buf = s.buf + trans(K_TRANS, '', inp)
      if s.play: s.phase()

  # handle_write () - dispatcher callback
  def handle_write (s):
    try:
      if s.out != '': s.out = s.out[s.send(s.out):]
    except:
      if s.body is not None: s.body.disco()
      s.close()

  # handle_close () - dispatcher callback
  def handle_close (s):
    if s.body is not None: s.body.disco()

  # msg (data) - queue up a string to be output
  def msg (s, d, trans=string.replace): s.out = s.out + trans(d, '\n', '\r\n')


# Executable (basically, a command)
class X:
  def __init__ (s, **kw):
    # 'name', 'rule', 'func', 'fail' are all required attributes of a command
    for k in kw.keys():
      setattr(s, k, kw[k])

  def __call__ (s, u, a):
    if hasattr(s, 'wiz') and not isW(u):
      msg('Eh?\n', u); return

    x = s.fail

    for r in s.rule:
      if len(r) == 0:
        x = s.func(u, r, a, None, None)
      elif len(r) == 1:
        o = s.mat(u, r[0], a); x = s.func(u, r, a, o, None)
      elif len(r) == 2:
        if sfi(a, ' ') >= 0:
          a1, a2 = ssp(a, ' ', 1); o1 = s.mat(u, r[0], a1); o2 = s.mat(u, r[1], a2)
          x = s.func(u, r, a, o1, o2)
      elif len(r) == 3:
        if sfi(a, r[1]) >= 0:
          a1, a2 = ssp(a, r[1], 1); o1 = s.mat(u, r[0], a1); o2 = s.mat(u, r[2], a2)
          x = s.func(u, r, a, o1, o2)
      if not x: break

    if x: msg(x + '\n', u)

  # mat (rule, data) - matches an object or string to a rule
  def mat (s, u, r, d):
    n = sst(d)
    if r == 'STR' or r == d: return d
    elif r == 'OBJ': return om(n, u.I + u.E.I, u=u)
#   elif r == 'OBE': return om(n, u.E.I, u=u)
#   elif r == 'OBI': return om(n, u.I, u=u)
#   elif r == 'NON': return om(n, filter(lambda x: not isL(x), u.E.I), u=u)
#   elif r == 'CON': return om(n, filter(isC, u.I + u.E.I), u=u)
#   elif r == 'PLU': return om(n, filter(isH, u.I + u.E.I), u=u)
#   elif r == 'LIV': return om(n, filter(isL, u.E.I), u=u)
#   elif r == 'PLL': return om(n, filter(isP, u.E.I), u=u)
    elif r == 'PLR': return om(n, fb.users.values(), u=u)
    else: return None


# Gender (thanks to Greg Egan for the 've' ambiguous gender)
class G:
  def __init__ (s, n, o, r, p):
    s.n = n  # Nominative/Subjective
    s.o = o  # Objective
    s.r = r  # Reflexive
    s.p = p  # Possessive


# Object
class O:
  def __init__ (s, *a, **kw):
    s._ = None; s.H = {}; s.Q = 1
    s.G = fb.GNeuter; s.E = fb.Void; s.I = [];
    s.N = s.S = s.D = 'object'
    s.W = s.V = s.X = s.U = 0
    for k in kw.keys():
      setattr(s, k, kw[k])

  # rc (force=0) - recalculates any necessary data about the object
  def rc (s, f=0):
    pass

  # hlt () - clean up after oneself
  def hlt (s):
    if s.E != fb.Dump: s.mov(fb.Dump, 0, 1);
    try:
      s.E.I.remove(s)
    except: pass

  # mov (dest, combine=1, force=0) - moves the object into the requested destination, returning 0 on success
  def mov (s, d, c=1, f=0):
    if not isC(d): return 4
    if not f and d == s.E: return 3
    if not f and d.V < s.W: return 2

    if hook(d, '!enter', s): return 1

    hook(s.E, 'leave', s); hook(s.E.I, 'depart', s); s.X = s.X & ~fb.OEquip
    try:
      s.E.I.remove(s)
    except: pass
    s.E.V = s.E.V + s.W; s.E.rc(); s.E = d; d.I.append(s); d.V = d.V - s.W; d.rc();
    hook(d, 'enter', s); hook(d.I, 'arrive', s); s.rc(); return 0

  # spl (amount) - returns a new Heap if the object can be split into the requested quantity
  def spl (s, n): return None

  # rcv (message) - receives a message and acts upon it
  def rcv (s, m): pass

  # cmd (verb, args) - perform a command
  def cmd (s, v, a): pass


# Heapable items (especially currency)
class H (O):
  pass
#   def __init__ (s, *a, **kw):
#     apply(O.__init__, (s,) + a, kw)
#     s._W = s.W; s.W = s._W * s.Q
#
#   def rc (s, f=0):
#     if (s.Q == 1):
#       s.G = fb.GNeuter
#     else: s.G = fb.GPlural
#     s.W = s._W * Q; O.rc(s, f)
#
#   def mov (s, d, c=1, f=0):
#     if O.mov(s, d, c=c, f=f) == 0 and c and s._ is not None:
#       for i in s.I:
#         if i._ != s._ or hook(i, '!combine', s): continue
#         hook(i, 'combine', s); s.Q = s.Q + i.Q; i.hlt(); s.rc()
#
#   def spl (s, n):
#     if s.Q < n: return None
#     if s.Q == n: return s
#
#     if hook(s, '!split', n): return None
#     x = copy.copy(s); s.Q = s.Q - n; x.Q = n
#     s.rc(); x.rc(); s.E.rc();
#     hook(s, 'split', s.Q); hook(x, 'split', n); return x


# Container
class C (O):
  def __init__ (s, *a, **kw):
    apply(O.__init__, (s,) + a, kw)
    s._W = s.W; s._V = s.V

  def rc (s, f=0):
    if f:
      s.V = s._V
      for i in s.I: i.rc(1); s.V = s.V - i.W
    s.W = s._W + (s._V - s.V); O.rc(s, f)


# Room
class R (C):
  def __init__ (s, *a, **kw):
    kw['V'] = 1000000000; kw['E'] = None; s.T = {}
    apply(C.__init__, (s,) + a, kw)

  def rc (s, f=0):
    if s.X & fb.RDark:
      s.X = s.x & ~fb.OLite
      for i in s.I:
        if i.X & fb.OLite: s.X = s.X | fb.OLite; break
    C.rc(s, f)

  def rcv (s, m): map(lambda x, m=m: x.rcv(m), s.I)


# Living
class L (C):
  def __init__ (s, *a, **kw):
    s.A = { 'st' : 0, 'dx' : 0, 'ag' : 0, 'co' : 0, 'in' : 0, 'hp' : 100 };
    s.Y = 0; s.Z = 0
    apply(C.__init__, (s,) + a, kw)

    if not kw.has_key('W') or not kw.has_key('V'):
      s._W = 100000 + int(10000 * s.A['co']); s._V = 50000 + int(5000 * s.A['st']); s.rc(1)

  def rc (s, f=0):
    s.X = s.X & ~fb.OLite
    for i in s.I:
      if i.X & fb.OLite: s.X = s.X | fb.OLite; break
    C.rc(s, f)

  def mov (s, d, c=1, f=0):
    if not isR(d): return 4
    return O.mov(s, d, c=c, f=f)

  def cmd (s, v, a):
    if not V_CMDS.match(v) or not fb.cmds.has_key(v):
      msg('Eh?\n', s); return

    fb.cmds[v](s, a);


# Player
class P (L):
  def __init__ (s, *a, **kw):
    apply(L.__init__, (s,) + a, kw)
    s.Y = s.Y | fb.LPlayer    

  def __getstate__ (s):
    d = s.__dict__.copy(); d['K'] = None; return d

  def __setstate__ (s, d):
    for k in d.keys(): s.__dict__[k] = d[k]

  # run () - the actual main user loop
  def run (s):
    if s.K is None: return
    l = s.K.line()

    if l:
      # This line is primarily for debugging purposes
      print 'Run %s [%s]: %s' % (repr(s), s.S, l)

      v, a = ssp(l + ' ', ' ', 1); a = sst(a); s.cmd(v, a)

    if l is not None: msg('> ', s)

  # disco () - disconnect from the game and return to Cryo
  def disco (s):
    s.K = None; msg('%s has left the game.\n' % s.S, s.E); s.mov(fb.Cryo)

  def rcv (s, m):
    if s.K is not None: s.K.msg(eng(s, m))
