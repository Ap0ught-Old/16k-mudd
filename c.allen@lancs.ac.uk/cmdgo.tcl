lapp ::cmdtable {go Command::cmdgo}
P cmdgo {s p} {
    
    FE e $Room::exits($::loc($s)) {
	if {[lindex $e 0] == $p} {
	    
	    # If in the zenroom, update their total idle time.
	    if {$::loc($s) == 2} {
		# Total idle.
		S ::at($s) [calc $s]
		# Time available for idle.
		S ::ta($s) [expr $::ta($s) + [time] - $::is($s)]
	    }
	    
	    starb $s "$::name($s) goes to $Room::name([lindex $e 1])."
	    S ::loc($s) [lindex $e 1]
	    starb $s "$::name($s) has arrived."
	    send $s "You move to $Room::name($::loc($s))."
	    Command::cmdlook $s {}

	    # If the new location is the zen room, start the idle counter.
	    if {$::loc($s) == 2} {
		S ::is($s) [time]
	    }

	    ret
	}
    }
    
    send $s "You cannot go that way."
}



