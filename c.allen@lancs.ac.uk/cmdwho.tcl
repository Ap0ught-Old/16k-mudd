lapp ::cmdtable {who Command::cmdwho}
P cmdwho {s a} {
    S c 0
    S o "Users of crappotalk on [date]"

    FE so $::sl {
	if {$::state($so) == $::SP} {
	    incr c
	    append o [format "\n%-15s %-40s" $::name($so) $::Room::name($::loc($so))]
	}
    }
    append o "\n&dYou see $c users."
    send $s $o	
}



