#!/usr/bin/env python

import sys
import db
import comm
import util
from time import *

# Mainline

# time between db syncs (approximate)
sync_t = 100

# time between ticks ('exact')
tick_t = 0.25

def run():
    try:
        main = db.soft_sys.main()
        main.init()
    except:
        util.log_exc()
        sys.exit(1) # don't sync..

    s_next = 0.0
    t_last = time()

    # t = now
    # t_last = start of last tick
    # next = scheduled start of next tick
    # s_next = scheduled time of next cache sync

    try:
        while 1:        
            try: main.tick()
            except: util.log_exc()
            
            # Calculate next tick time
            next = t_last + tick_t
            
            # Wait for next tick, doing I/O
            
            t = time()
            while t < next:
                comm.idle(next - t)
                t = time()
                
            # Don't use t; we don't want drift here
            t_last = next
            
            # Check for cache sync
            if t > s_next:
                s_next = t + sync_t
                db.sync()

    except:
        db.sync()
        raise

if __name__ == '__main__':
    run()
