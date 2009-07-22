import traceback
import sys
import string
import db

def shutdown():
    log('util.shutdown() called')

    db.sync()
    # Exit.
    sys.exit(0)

def exc_str(info = None):
    if not info:
        info = sys.exc_info()
        
    return string.join(apply(traceback.format_exception, info), '')
    
def log_exc(info = None):
    log(exc_str(info))

def log(text):
    print text

exc_info = sys.exc_info
