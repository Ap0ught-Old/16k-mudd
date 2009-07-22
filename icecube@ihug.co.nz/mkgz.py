#!/usr/bin/env python

# Create data.gz

import gzip
import sys
import string
import pickle
import db

sd = {}
for i in sys.argv[1:]:
    sd[string.replace(i, '/', '.')] = db.persUnpickler(open(i)).load()

f = gzip.open('data.gz', 'w')
pickle.Pickler(f, 1).dump(savedict)
f.close()
