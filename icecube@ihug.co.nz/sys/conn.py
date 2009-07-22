St = sys.static()

def init():
    # listeners. Put this here not globally to avoid a recursive reference loop.
    ports = [ (12345, sys.user_conn),
              (12351, sys.ftp_conn) ]

    # Set up listeners
    for i in ports:
        comm.listen( ('', i[0]), i[1] )

class conn(Object):
    def start(S):
        S.die = 0
        S.tostate('start')
        
    def to(S, state):
        S.state = state
        t, s = S.fsm[state]
        S.write(t)
        if S.state == 'close':
            S.close()
            return
        if s: S.to(s)

    def comm_close(S):
        db.destroy(S)

    def comm_readline(S, l):
        try: S.to(apply(getattr(S, S.state), [l]))
        except: S.write(util.exc_str())

        if S.die: S.close()

    write = comm.write  # passes S as arg 1, perfect.
    close = comm.close

