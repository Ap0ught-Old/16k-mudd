# Pygmy3 -- connection interface
# (note: this only handles line-based interfaces for now, for the sake of
# space; asynchat does most of the work for us already)

# Basic structure:
# - Softcode calls connection.listen(addr, klass)
# - On connection acceptance, driver instantiates a new 'klass' object
#   passing in peer address.
# - That new object can call comm.write(S, data) and
#   comm.close(S) as needed. (this is easier than trying to
#   handle references to nonpersistent objects)
# - The driver may call obj.conn_readline(data), obj.conn_close() as needed.

import asynchat
import asyncore
import socket
import util
import db
import string
from types import *

# Map of client object names to Client objects
clientmap = {}

AC = asynchat.async_chat

# One Client object per incoming connection. Each Client has an associated
# softcode peer that actually handles the incoming data.
class Client(AC):
    def __init__(S, conn, peer, klass):
        AC.__init__(S, conn)
        
        S.next_cmd = ''
        S.terminator = '\n'

        try:
            # Build the softcode object to handle this connection
            S.conn_obj = klass(peer)
            cmap[S.conn_obj.__name__] = S
            S.conn_obj.start()
        except:
            S.conn_obj = None
            util.log_exc()
            S.close()
            raise

    def handle_error(S, *info):
        util.log_exc(info)

    def log(S, msg):
        # this is just noise.
        pass

    def collect_incoming_data(S, buf):
        # Build up a pending command
        S.next_cmd = S.next_cmd + buf

    def found_terminator(S):
        # First clear the pending data out
        cmd = S.next_cmd
        S.next_cmd = ''

        # Next clear out stray \r's
        cmd = string.replace(cmd, '\r', '')
        
        # Then dispatch it
        try:
            S.conn_obj.comm_readline(cmd)
        except:
            util.log_exc()

    def close(S):
        if S.conn_obj:
            # Tell our softcode friend about it
            del cmap[S.conn_obj.__name__]
        
            try:
                S.conn_obj.comm_close()
            except:
                util.log_exc()

            S.conn_obj = None

        # Really do the close.
        AC.close(S)

    def write(S, data):
        S.push(string.replace(data, '\n', '\r\n'))

AD = asyncore.dispatcher

# One Listener per listening port. On connection, a new Listener is created.
class Listener(AD):
    def __init__(S, addr, klass):
        AD.__init__(S)        
        S.klass = klass

        # Set up the listener
        S.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        S.set_reuse_addr()
        S.bind(addr)
        S.listen(1)
                 
    def handle_error(S, *info):
        util.log_exc(info)

    def log(S, msg):
        # noise noise noise
        pass

    def handle_accept(S):
        incoming = S.accept()
        if incoming:
            # Build connection. Adds itS to the right lists automatically
            try: Client(incoming[0], incoming[1], S.klass)
            except: pass

# External hooks: write or close a connection. This all gets keyed by object
# name to avoid having the softcode object have a reference to a Client, for
# two reasons:
# - circular references are bad
# - we don't want to try to persist a Client object, ever.
#
# Nice side effect: we could swap out the softcode peers safely and they'd
# work when they came back. If there was an intervening server restart, they'd
# just discover that their writes/closes failed.
#
# (see sys.user_conn for an implementation that avoids leaving lots of dead
#  connections around over server restart)

def write(conn_obj, data):
    cmap[conn_obj.__name__].write(data)

def close(conn_obj):
    try: cmap[conn_obj.__name__].close()
    except: pass

# Softcode interface to add/remove listeners.

# .. well. We never need to remove them. Comment that bit out..

#listeners = {}

#def remove_listener(addr):
#    listeners[addr].close()
#    del listeners[addr]

def listen(addr, klass):
    l = Listener(addr, klass)
#    listeners[addr] = l

# Called from the main loop
idle = asyncore.poll
