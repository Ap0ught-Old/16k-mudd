lapp ::cmdtable {look Command::cmdlook}
P cmdlook {s c} {
    
    # Is there an argument provided?
    if {$c == ""} {
	# Look at the current room.
	S loc $::loc($s)
	S o "&c$Room::name($loc)\n&m\[&dExits:"
	FE e $Room::exits($loc) {
	    append o " [lindex $e 0]"
	}
	append o "&m\]&d\n$Room::desc($loc)"
	
	FE so $::sl {
	    if {$::state($so) == $::SP} {
		if {$::loc($so) == $::loc($s)} {
		   if {$so != $s} {
		      append o "\n$::name($so) is here."
		   }
		}
	    }
	}
	
	send $s $o
	ret
    }

    S u [findUser $c]
    if {$u != ""} {
	if {$::loc($u) == $::loc($s)} {
	    send $s "You look at $::name($u)\n$::desc($u)\nOn from $::hostname($u)\nOn since [clock format $::ls($u)]"
	    ::sendToAllBut $s "$::name($s) looks at $::name($u)"
	    ret
	}
    }
    
    send $s "Didn't find any '$c'"
}
