#! /usr/bin/env python -O
#
# $Id$
#
# Fresnel - a mud in under 16k - by Doug Swarin
#
# See doc/LICENSE for licensing terms.
#
# By default, the mud runs at 8 cycles per second on port
# 2002. These can both be changed below in main().
#

import fb, fc, time, socket
from ff import *
from asyncore import dispatcher, poll

class _emote: pass

class Fresnel (dispatcher):
  def __init__ (s, p):
    dispatcher.__init__(s)
    s.create_socket(socket.AF_INET, socket.SOCK_STREAM)
    s.set_reuse_addr(); s.bind(('', p)); s.listen(20)

  # handle_accept () - dispatcher callback
  def handle_accept (s):
    c, a = s.accept(); fc.K(c)


def main ():
  fb.load_res()

  tm = time.time; mud = Fresnel(2002)

  try:
    while 1:
      tf = tm() + 0.125 # 8 cycles/second
      poll(); map(lambda x: x.run(), fb.users.values())
      te = tm()
      if te < tf: time.sleep(tf - te)
  except KeyboardInterrupt, err:
    fb.save_res(); fb.save_room(); fb.save_pass()


if __name__ == '__main__':
  main()
