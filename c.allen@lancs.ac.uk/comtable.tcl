lapp cmdtable {desc Command::desc}
lapp cmdtable {cmdedit Command::edit}

namespace eval Command {
    P desc {s a} {
	Editor::edit $s $::desc($s) Command::desc_cb
    }

    P desc_cb {s d} {
	S ::desc($s) $d
	send $s "Description now:\n$d"
    }
}

