#!/usr/bin/python
# I'm not sure if the above is right. *nix people don't kill me please.

import comm,world
w="%%%%N%%%%%...$...%%..Z$Z..%%...$...%W@@@#@@@E%...$...%%..Z$Z..%%...$...%%%%%S%%%%"
rooms=world.mkwrld(w,9)
s=comm.Srv()
s.begin()
s.run()
s.finish()
