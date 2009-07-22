# sys.user_conn

St = sys.static()
Dy = sys.dynamic()
all = Dy.list('user_conn')

def init():
    # Kill old connections
    for i in all[:]:
        if i: i.comm_close()

    del all[:]

class user_conn(sys.conn.conn):
    fsm = St.user_fsm

    def __init__(S, peer):
        all.append(S)
        St._dirty = 1
        S.player = None
	
    def comm_close(S):
        all.remove(S)
        if S.player: S.player.set_conn(None)
        sys.conn.conn.comm_close(S)
	
    # Validators
    def login(S, line):
        l2 = string.lower(line)
        if not re.search('^[a-z]+$', l2): return 'badname'
        else:
            try:
                S.player = mud.player.instance(l2)
                return 'oldpwd'
            except db.NoSuchObject:
                S.newname = line
                return 'newpwd1'
	    
    def oldpwd(S, line):
        if line != S.player.pwd: return 'wrongpwd'
        else:
            S.player.set_conn(S)
            return 'welcome'
	
    def newpwd1(S, line):
        if not line: return 'login'
        else:
            S.pwd = line
            return 'newpwd2'

    def newpwd2(S, line):
        if S.pwd != line: return 'nomatch'
        else:
            try:
                p = mud.player.instance(S.newname)
                return 'cantcreate'
            except db.NoSuchObject:
                S.player = mud.player(name=string.lower(S.newname))
                S.player.pwd = S.pwd
                S.player.name = S.newname
                if sys.config().mkadmin:
                    S.player.admin = 1
                    sys.config().mkadmin = 0
                S.player.set_conn(S)
                return 'welcome'

    def cmd(S, line):
        S.player.cmd(line)
        return 'cmd'

