namespace eval Zen {

    P read {a} {
	S f [open $a r]
	while {[eof $f] == 0} {
	    S l [gets $f]
	    if {$l != ""} {
		lapp o $l
	    }
	}
	ret $o
    }

    P init {} {
	V nl {} ql {} ml {} ul {} tl {}

	S ul [read user]
	S nl [read noun]
	S ml [read mantra]
	S ql [read question]

	# Minimum of 15 seconds, maximum of 40.
	after [rtime] Zen::mantra
	after 300000 Zen::fidget
    }

    # Sends a mantra to a random player in the room.

    P rtime {} {
	ret [expr round(rand() * 25000) + 15000]
    }

    P mantra {} {
	V qu
	V nl
	V ql
	V ml
	V ul
	V pq
	V tl
	S l {}

	# Minimum of 15 seconds, maximum of 40.
	after [rtime] Zen::mantra

	FE p $::sl {
	    if {$::loc($p) == 2} {
		lapp l $p
	    }
	}

	# Find a random player...
	if {[catch {
	    S r [expr round(rand() * [llength $l])]
	    if {$r >= [llength $l]} {
		S r [expr [llength $l] - 1]
	    }

	    S s [lindex $l $r]
	}] == 1} {
	    # No suitable player found
	    ret
	}

	if {$s == ""} {
	    ret
	}

	# Is this player answering a question already?
	if {$qu($s) > 0} {
	    ret
	}

	# Is this going to be a mantra or a question?
	if {[expr rand() * 4] > 1} {
	    # Mantra

	    # Pick a random line from the mantras
	    S o [getLine $ml]
	    
	} {
	    # Question - Set qu($s) to a Viable containing the after ID
	    S qu($s) [after 60000 Zen::asleep $s]

	    # Pick a random question from the questions
	    S o [getLine $ql]
	}

	# Fill in the %nx bits with nouns
	
	FE i {"\x81" "\x82" "\x83" "\x84" "\x85" "\x86" "\x87" "\x88" "\x89"} {
	    regsub -all "$i" $o [getLine $nl] o
	}
	
	# Fill in the %ux bits with usernames
	FE i {"\x91" "\x92" "\x93"} {
	    regsub -all "$i" $o [getLine $ul] o
	}
	
	S pq($s) $o
	send $s $o
	
    }
    
    P getLine {l} {
	# Pick a random element from this list.
	S i [expr round(rand() * [llength $l])]
	if {$i >= [llength $l]} {
	    S i [expr [llength $l] -1]
	}
	S o [lindex $l $i] 
	ret $o
    }
    
    P asleep {s} {
	starb $s "$::name($s) has fallen asleep."
	send $s "Stay awake during meditation, dingbat!"
	S ::is($s) [time]
	Command::cmdquit $s {}
    }
    
    P interpret {s m} {
	V qu
	V pq
	V nl
	V ul
	V tf
	
	# Interpret for the Zen room.  Most commands do not work in here.
	
	if {$qu($s) > 0} {
	    after cancel $qu($s)
	    S qu($s) 0
	    
	    # Store the input if appropriate.
	    
	    # pq might not exist, so we catch on this - V init takes
	    # up more than the 7 bytes for the catch.
	    
	    catch {
		if {[regexp {name} $pq($s)] == 1} {
		    lapp ul $m
		} {
		    lapp nl $m
		}
	    }

	    starb $s "$::name($s) mutters something."
	    send $s "Thankyou, enlightened one."
	    S ::at($s) [calc $s]
	    id $s
	    ret 1
	}
	
	# tf($s) stores the number of times this person has fidgetted since
	# it was last reset.
	incr tf($s)
	
	# Nuke their idle time.
	if {$m == "quit" || [regexp {^go .*$} $m]} {
	    ret 0
	}

	id $s
	starb $s "$::name($s) shuffles around."

	switch $tf($s) {
	    1 {send $s "Fidgeting prevents true enlightenment."}
	    2 {send $s "You're disturbing other meditators."}
	    3 {send $s "Quit fidgeting!  You'll get kicked out."}
	    4 {
		S tf($s) 0
		kick $s
		ret 1
	    }
	}
	ret 0
    }

    P id {s} {
	S ::ta($s) [expr $::ta($s) + [time] - $::is($s)]
	S ::is($s) [time]
    }

    P kick {s} {
	
	send $s "You've been booted from the Zen Room for fidgeting."
	starb $s "$::name($s) is booted from the Zen Room."
	S ::loc($s) 1
	Command::cmdlook $s {}
	starb $s "$::name($s) appears through the door from the Zen Room."
	send $s "In all the hubbub, you forget what you were trying to do."
    }

    # This procedure clears everyone's fidget count.
    
    P fidget {} {
	V tf
	after 300000 Zen::fidget

	FE s $::sl {
	    S tf($s) 0
	}
    }
}





