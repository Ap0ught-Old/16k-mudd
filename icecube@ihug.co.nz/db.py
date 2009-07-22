# pygmy -- database management

# Magic object names needed:
#
# sys.counter  (singleton)

import string
import new
import pickle
import util
import os
import namespace
import sys
import gzip
from errno import *
from types import *

# Location of top of softcode + data files
# (sys/ mud/ etc are below this)
DATADIR = './'

# Exceptions thrown by various loader functions
NoSuchClass = "db.NoSuchClass"
NoSuchObject = "db.NoSuchObject"
LoadError = "db.LoadError"

# Convert dotpath a.b.c to directory pathname /foo/data/a/b/c
def dpath(p):
    return DATADIR + string.replace(p, '.', '/')

# Root object for persistence. A bit magic, since the loader messes with
# it directly
class Object:
    cur_version = 1   # loadhook support

    # Not __init__ -- this is *always* called, before __init__ even, by the
    # loader! (and can't be overridden in subclasses). Sets up stuff that
    # really shouldn't be modified elsewhere as it will break object
    # persistence.
    #
    # (it's deliberately not called __init__ as we don't want to end up
    # calling it twice in the case where a subclass doesn't provide an
    # implementation of __init__)
    def __setup__(S, name, inst):
        S.__name__ = name
        S.__instance__ = inst
        S._version = S.cur_version
        # no need to set _dirty, assigning to _version does it for us
        # S._dirty = 1

    # Catch attribute setting to set the dirty flag too. Yes, this doesn't
    # catch changes within mutable values -- too bad.
    def __setattr__(S, attr, value):
        S.__dict__['_dirty'] = 1  # do this first so we can clear _dirty easily
        S.__dict__[attr] = value

    def save(S):
        # Persist object, but only if it's actually dirty..
        
        if S._dirty:
            # write to a temp file in case we die halfway through pickling
            fn = dpath(S.__name__)
            f = open(fn + '.tmp', 'w')

            # Actually pickle the instance data
            try:
                persPickler(f,1).dump(S.__dict__)
                try: os.rename(fn + '.tmp', fn)
                except: pass
                
            finally:
                f.close()

                # this fails on success since we've already renamed it. no
                # problem..
                try: os.unlink(fn + '.tmp')
                except: pass

            # Saved ok. 
            S._dirty = 0

# Not implemented here, but will be called on object load if it exists --
# use this to upgrade object data from S.version to S.cur_version
# as needed.
#    def loadhook(S):
#        pass

# Return (class, instance) of arbitary object
def objinfo(obj):
    # Instance of Object; extract class name and instance ID
    if isinstance(obj, Object):
	return (obj.__class__.__name__, obj.__instance__)
    
    # Lazy reference to an Object instance, extract classname/instance
    # ID from it.
    elif isinstance(obj, lazy):
	return (obj._class, obj._inst)

    # return None

# Persistent pickling -- save instances of Object as references to their
# name.
class persPickler(pickle.Pickler):
    def persistent_id(S, obj):
        if type(obj) == InstanceType:
	    return objinfo(obj)

        # Otherwise pickle as normal
        # return None

class persUnpickler(pickle.Unpickler):
    def persistent_load(S, pid):
        # We always end up pickling (class, name) above. Get a lazy reference
        # to it.
        try: return getlazy(pid[0], pid[1])
        
        # (??) Replace references to nonexistent objects with None
        except NoSuchObject: return None
        
# Object cache
ocache = {}

# Class cache
ccache = {}

# Compressed instance data cache
gzcache = None

# Placeholder object for a, a.b in a.b.c where datadir/a/b/c.py exists

class placeholder:
    def __init__(S, path):
        S._path = path

    def __getattr__(S, attr):
        newp = S._path + '.' + attr

        # if it's a dir, we get a placeholder, else it should
        # be a class
        if os.path.isdir(dpath(newp)):
            v = placeholder(newp)
        else:
            v = getclass(newp)

        # avoid hitting __getattr__ every time
        S.__dict__[attr] = v
        return v

# Class namespace object
#
# Behaves somewhat like a module, but with a couple of bits of magic behaviour:
#
# for a.b.c:
#
# a/b/c.py is read, and the resulting global dictionary appears under a.b.c
# (ie. a.b.c.function)
#
# If it declares a class 'c' that inherits from Object, a.b.c becomes callable,
# acting like a class (constructing instances of c on call)
#
# Additionally a.b.c.instance(name) will return instance "name" of a.b.c
# (ie. a.b.c%name) or raise NoSuchObject.
#
# If class 'c' has an attribute 'singleton' that is non-0, a.b.c() returns
# a singleton instance of c (creating as needed). The instance ID is
# a.b.c%single.
#
# (I've forgotten what the "i" in iclass is meant to mean now. Oh well.
#  "interface to class"? ;)

class iclass:
    def __init__(S, path, dict):
        # Grab the dictionary we're given; we'll override bits of it later
        S.__dict__ = dict

        # Now set up some magic names
        S._path = path

        # Make sure these attrs are always there
        S._class = None
        S._single = 0

        # If there's a class named the same as the last component
        # of the path, that's what we create instances of.
        lastbit = string.split(path, '.')[-1]
        try:
            c = getattr(S, lastbit)
            if issubclass(c, Object):
                # Ok, we have an appropriately named subclass of Object
                S._class = c

                # check singleton status
                if hasattr(c, 'singleton') and c.singleton:
                    S._single = 1
                else:
                    S._single = 0

                # set canonical name for class
                c.__name__ = path
                
        except: pass
        
    def _new(S, name, args, kwargs):
        # can only call _new if we have a matching class to instantiate
        if not S._class: raise RuntimeError, "instances of this type not available"

        # check that it isn't already there!
        try:
            getinst(S._path, name)
            raise RuntimeError, "tried to create an instance that already exists"
        except NoSuchObject: pass  # we *expect* this
        
        # construct the new object. Do *not* call __init__ yet!
        newobj = new.instance(S._class, {})

        # Call Object.__setup__ to do required object setup, eg. set object
        # name.
        fullname = S._path + '%' + name
        Object.__setup__(newobj, fullname, name)

        # Call the real __init__ if it's there
        if hasattr(newobj, '__init__'):
            apply(newobj.__init__, args, kwargs)

        # Remember to cache the new instance!
        ocache[fullname] = newobj
        return newobj

    # Call interface for an iclass:
    #   sys.foo() -- creates a new instance with autogenerated instance ID
    #           *or* retrives a singleton instance, creating as necessary
    #
    #   sys.foo(a,b,c) -- create a new instance with autogenerated instance ID
    #                     passing given args to ctor
    #
    #   sys.foo(a,b,c,name='foo') -- create a new instance with instance ID
    #                                'foo', passing all args to ctor
    
    def __call__(S, *args, **kwargs):
        if S._single:
            # Singletons always refer to the same instance, creating it as
            # needed.

            # (note: I don't pass provided args to the ctor deliberately --
            # it doesn't make sense for a persistent singleton to be
            # potentially constructed from different places with different
            # args..)
            if args or kwargs:
                raise RuntimeError, "can't provide args to get-singleton call"
            
            try: s = getlazy(S._path, 'single')
            except NoSuchObject: s = S._new('single', (), {})
            return s

        # Non-singleton. Create new instance

        if kwargs.has_key('name'):
            name = kwargs['name']
        else:
            # No name provided, make our own
            # note: there's a softcode dependency here, but it means we don't
            # need a parallel persistence system just for the counter bit.
            # sys.counter better be a singleton!!
            name = soft_sys.counter().next(S._path)

        return S._new(name, args, kwargs)
        
    def instance(S, name):
        # Get an instance of this class
        if not S._class: raise RuntimeError, "instances of this type not available"

        # .. actually, just get a lazy ref to it
        # (getlazy does existence checks though)
        return getlazy(S._path, name)

# Lazy reference to a class instance. Only load the actual underlying object
# when needed (get/set/del attribute)
class lazy:
    def __init__(S, klass, inst):
        # Use __dict__ to avoid triggering setattr
        S.__dict__['_class'] = klass
        S.__dict__['_inst'] = inst

    def __getattr__(S, attr):
        # Load underlying object; then we are it so just retry
        S._load()
        return getattr(S, attr)

    def __setattr__(S, attr, value):
        # Load underlying object; then we are it so just retry
        S._load()
        setattr(S, attr, value)
        
    def __delattr__(S, attr):
        # Load underlying object; then we are it so just retry
        S._load()
        delattr(S, attr)

    def _load(S):
        # Something tried to use us, replace ourselves with the real object
        newobj = getinst(S._class, S._inst)
        S.__class__ = newobj.__class__
        S.__dict__ = newobj.__dict__

# Return a lazy reference to klass%inst (or maybe the object itS, if
# cached)
def getlazy(klass, inst):
    # Check cache
    path = klass + '%' + inst
    if ocache.has_key(path):
        return ocache[path]
    
    # Verify existance of klass and inst    
    getclass(klass) # discard return value; raises on error
    load_gz()
    if not os.path.isfile(dpath(path)) and not gzcache.has_key(path): raise NoSuchObject, path

    # Construct our lazy ref
    return lazy(klass, inst)

# Get the actual instance klass%inst
def getinst(klass, inst):
    # Check cache
    path = klass + '%' + inst
    if ocache.has_key(path):
        return ocache[path]

    # Check class
    c = getclass(klass)   # might raise
    if not c._class:
        # Ow, the code must have changed under us        
        raise LoadError, "class went missing for " + path
    try:
        # Load the instance dictionary
        f = open(dpath(path), 'r')
        try:
            try:
                d = persUnpickler(f).load()
            except IOError, e:
                load_gz()
                if gzcache.has_key(path):
                    d = gzcache[path]

        finally:
            f.close()

        # Make sure the data is sane (this can go wrong if things are renamed)
        d['__name__'] = path
        d['__instance__'] = inst
            
    except IOError, e:
        if e[0] != ENOENT: raise
        raise NoSuchObject, path

    # Construct the new instance with given dict
    i = ocache[path] = new.instance(c._class, d)

    # Call loadhook if it's there to upgrade the object
    if hasattr(i, 'loadhook'):
        try: i.loadhook()
        except: util.log_exc()
        # might as well continue on errors?
        
    return i

# Permanently destroy an instance
def destroy(obj):
    if isinstance(obj, Object):
        klass = obj.__class__.__name__
        inst = obj.__instance__
    elif isinstance(obj, lazy):
        klass = obj._class
        inst = obj._inst
    else:
        raise RuntimeError, "can only destroy Object instances"

    name = klass + '%' + inst

    # Check for cached objects; someone might still hold a reference to
    # the object to destroy and try to use it later.
    if ocache.has_key(name):
        o = ocache[name]

        # Turn the cached object into a lazy reference; this should cause a
        # NoSuchObject to be raised if anything still references that object
        # and tries to use it.
        l = lazy(klass, inst)
        o.__class__ = l.__class__
        o.__dict__ = l.__dict__

        del ocache[name]  # ocache only holds full instances!

    # Kill the actual instance data
    try: os.unlink(dpath(name))
    except: pass

# Load the "class" (really the iclass wrapper) for a path
def getclass(path):
    # check cache
    if ccache.has_key(path):
        return ccache[path]

    try:
        # Remember our namespace; this will become the iclass' namespace on
        # success since execfile() modifies it.

        ccache[path] = None # dummy value to avoid recursion
        
        ns = namespace.get() # restricted execution environment
        execfile(dpath(path) + '.py', ns)
    except IOError, e:
        del ccache[path]
        
        if e[0] != ENOENT: raise

        # missing file.
        raise NoSuchClass, path

    # Construct iclass object and cache
    c = ccache[path] = iclass(path, ns)
    return c


# Change the underlying code for a class.

# reload from filesystem (nice for development..)
def refresh(path):
    # Remember the old class in case of errors
    try: oldclass = getclass(path)
    except: oldclass = None

    # Try to load the new version
    
    # .. clear cache
    try: del ccache[path]
    except: pass

    try:
        # .. and refetch (this might raise compile errors etc)
        newclass = getclass(path)
        
        # Successfully loaded. Now update any cached object instances based
        # on it        

        # Note! We can't let any exceptions out of here once we
        # upgrade an object! be careful..
        
        for i in ocache.keys():
            if i[:len(path)+1] == path + '%':
                # Instance of class
                if not newclass._class:
                    # ugh. There are instances without a class now. help!
                    raise LoadError, path + ": instances exist in cache, but new code doesn't allow them"
                
                # Set the new class, then upgrade
                o = ocache[i]
                o.__class__ = newclass._class
                if hasattr(o, 'loadhook'):
                    try: o.loadhook()
                    except: util.log_exc()

    except:
        # Help! Help! Something went wrong. Restore what we can.
        # .. restore cache
        if oldclass: ccache[path] = oldclass
        
        raise                    

# Update class on fs to be the given chunk of code, then reload it.
# Gotcha: this does not raise exceptions; instead it returns the traceback
# text (from util.exc_str()). This is because by the time you catch the
# exception the filesystem has reverted to the old code, so code context in
# the traceback is misleading. A return value of None is success.
def update(path, code):
    # first write it out..
    fname = dpath(path) + '.py'

    # save the old version in case the load fails
    oldfname = fname + '~'
    if os.path.exists(fname):
        os.rename(fname, oldfname) # let errors raise; we need that backup!

    try:
        # Update the code on disk        
        try:
            f = open(fname, 'w')
            f.write(code)
        finally:
            f.close()

        # Do the actual refresh
        refresh(path)
        
        # Done!

    except:
        str = util.exc_str()
        
        # .. try to restore the backup
        try: os.unlink(fname)
        except: pass

        try: os.rename(oldfname, fname)
        except: pass

        # Signal failure.
        return str

    # Completed ok. Clean up backup
    try: os.unlink(oldfname)
    except: pass
    # return None

# Sync cache -- write all dirty objects
def sync():
    for i in ocache.keys():
        try: ocache[i].save()
        except: util.log_exc()

def getcode(path):
    try: f = open(dpath(path) + '.py')
    except IOError: raise NoSuchObject, path
    try: return f.read()
    finally: f.close()

# Load compressed data
def load_gz():
    global gzcache
    if gzcache is None:
        gzcache = {}
        try:
            f = gzip.open('data.gz', 'r')
            gzcache = pickle.Unpickler(f).load()
            f.close()
        except: pass

# Shorthand for softcode.sys
soft_sys = namespace.get()['sys']
