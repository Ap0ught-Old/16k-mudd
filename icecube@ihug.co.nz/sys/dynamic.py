class dynamic(Object):
    singleton = 1

    def list(S, name):
        name = '_' + name
        if not hasattr(S, name):
            setattr(S, name, [])
        return getattr(S, name)

