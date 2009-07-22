# Static data object

class static(Object):
    singleton = 1

    def __init__(S):
        S.loadhook()
	
    def loadhook(S):
	    # Heh.
        pass
        #S.badcmd = 'Unknown command.\n'
        #S.whohdr = 'Users currently connected:\n'
        #S.bye = 'Quitting. Goodbye!\n'

        #S.user_fsm = {
        #    'start': ('pyGmy incarnation #1.\nTo create a new character, log in as the name you want.\n\n', 'login'),
        #    'login':  ('login: ', None),
        #    'badname': ('Names may only be alphabetic.\n', 'login'),
        #    'oldpwd': ('Existing player.\nPassword: ', None),
        #    'wrongpwd': ('Wrong password.\n', 'login'),
        #    'newplr':   ('New player.\n', 'newpwd1'),
        #    'newpwd1': ('Choose a password (blank to abort): ', None),
        #    'newpwd2': ('Reenter your password: ', None),
        #    'nomatch': ("Passwords don't match.\n", 'newpwd1'),
        #    'cantcreate': ('Sorry, that name is now taken.\n', None),
        #    'welcome': ('Successfully logged in.\n', 'cmd'),
        #    'cmd': ('', None)
        #    }

        #S.ftp_fsm = {
        #    'start': ('', 'auth'),
        #    'auth': ('', None),
        #    'welcome': ('OK pyGmyMudFtp 2.0 ready.\n', 'waitpush'),
        #    'waitpush': ('', None),
        #    'push': ('OK pushing you data\n', 'idle'),    
        #    'idle': ('', None),
        #    'error': ('FAILED some random error\n', 'idle'),
        #    'perror': ('FAILED only push mode supported\n', 'waitpush'),
        #    'noop': ('OK idling.\n', 'cmd'),
            #    'send': ('', None),  tostate() never called
        #    'recv': ('', None),
        #    'close': ('FAILED\n', None),
        #    }                

    def __getattr__(S, attr):
        if attr[0] == '_':
            raise AttributeError, attr
        else:
            return '<missing static data ' + attr + '>'
    

