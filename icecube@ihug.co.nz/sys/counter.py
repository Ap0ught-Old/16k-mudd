class counter(Object):
    singleton = 1
    
    def __init__(S):
        S.counts = {}

    def next(S, path):
        if not S.counts.has_key(path):
            S.counts[path] = 1
            
        id = S.counts[path]
        S.counts[path] = id + 1
        S._dirty = 1
        S.save()      # make sure we do this immediately!
        return `id`
            
