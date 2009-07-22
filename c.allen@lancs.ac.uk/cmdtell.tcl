lapp ::cmdtable {tell Command::cmdtell}
P cmdtell {s p} {

	if {[regexp {^([^ ]+) (.*)$} $p d u m] == 0} {
		send $s "Syntax: tell <player> <message>"
		ret
	}

	S v [findUser $u]
	
	if {$v != ""} {
	    if {$::state($v) == $::SP} {
		send $v "&c$::name($s) tells you, '$m&d&c'"
		send $s "&cYou tell $::name($v), '$m&d&c'"
		ret
	    }
	}

	send $s "I can't find anyone called '$u'."
}



