import rexec
import __builtin__

# make_namespace constructs the namespace that all user code
# sees. Policy decisions here!

# Override list of not-ok builtins so myRExec.modules['__builtin__'] is
# right (rexec uses that to initialise other modules' __builtin__ attr)

class myRExec(rexec.RExec):
    def make_builtin(S):
        # no futzing around with restricted versions of open, thanks.
        return S.copy_except(__builtin__, ('__import__', 'open', 'reload', 'raw_input', 'execfile'))

# __builtin__ -> everything except dangerous things
# driver -> limited view of our driver module
# sys -> nothing useful in the standard module. Reuse it for object space
# string -> everything
# re -> everything
# math -> everything
# whrandom -> everything
# mud -> object space
def get():
    # We just use RExec for its module building routines
    r = myRExec()

    import sys
    import comm
    import util
    import db
    import re
    import string

    # Yes, we reconstruct this on every call. Returning a copy of the dict
    # still referring to the same underlying objects is a possibility, but
    # it means that runaway softcode could corrupt the "standard" namespace
    # for all other softcode.
    return {
        '__builtins__': r.modules['__builtin__'],
        'db':          r.copy_only(db, ('destroy', 'update', 'getcode', 'sync', 'refresh', 'objinfo', 'NoSuchObject', 'NoSuchClass', 'LoadError')),
        'comm':        r.copy_only(comm, ('error', 'write', 'close', 'listen')),
        'util':        r.copy_except(util, ()),
        'sys':         db.placeholder('sys'),  # softcode namespace
        'mud':         db.placeholder('mud'),  # softcode namespace
        'string':      r.copy_except(string, ()),
        're':          r.copy_except(re, ()),
        'Object':      db.Object,      # top-level persistent object
        '__name__':    'softcode'   # this makes repr() look nicer
        }
