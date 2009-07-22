St = sys.static()

class player(Object):
    def __init__(S, **kwargs):
        S.conn = None
        S.ftp = None
        S.admin = 0
    
    def set_conn(S, conn):
        if S.conn:
            S.conn.close()
        S.conn = conn

    def set_ftp(S, ftp):
        if S.ftp:
            S.ftp.close()
        S.ftp = ftp

    def write(S, data):
        if S.conn:
            try: S.conn.write(data)
            except: pass

    def edit(S, str, cbo, cbf):
        if not S.ftp: raise RuntimeError, "no ftp connection"
        return S.ftp.edit(str, cbo, cbf)

    def cmd(S, line):
        args = string.split(line, ' ', 1)
        if not args: return
        c = 'c_' + string.lower(args[0])      # common commands
        a = 'a_' + string.lower(args[0])      # admin commands
        if len(args) == 1: args = [args, '']
        if S.admin and hasattr(S, a): apply(getattr(S, a), args[1:])
        elif hasattr(S, c): apply(getattr(S, c), args[1:])
        else: S.write(St.badcmd)

    # User commands

    def c_who(S, x):
        S.write(St.whohdr)
        for i in sys.user_conn.all:
            if i.state == 'cmd':
                S.write(i.player.name)
                if i.player.admin: S.write(' (admin)')
                S.write('\n')
            
    def c_say(S, text):
        for i in sys.user_conn.all:
            if i.player != S: i.write(S.name + " says '" + text + "'.\n")

        S.write("You say '" + text + "'.\n")

    def c_quit(S, x):
        if S.conn:
            S.write(St.bye)
            S.conn.die = 1

    # Minimal error checking on these
    def a_mkadmin(S, plr):
        mud.player.instance(string.lower(plr)).admin = 1
        db.sync()

    def a_edit(S, path):
        S.editing = path
        try: str = db.getcode(path)
        except db.NoSuchObject: str = ''
        S.edit(str, S, 'newcode')

    def newcode(S, text):
        err = db.update(S.editing, text)
        if err: S.write("Errors in %s:\n%s\n" % (S.editing, err))
        else: S.write(S.editing + " updated ok.\n")

    def a_eval(S, expr):
        S.write('==> ' + `eval(expr)` + '\n')

    def a_exec(S, code):
        exec code
