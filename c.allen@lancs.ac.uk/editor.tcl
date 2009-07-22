namespace eval Editor {

    P edit {s st cb} {

	# If this user has a mudFTP connection, push the string to them through it.

	if {$::mudftp($s) != 0} {
	    # Yep they've a connection.
	    Mudftp::push $::mudftp($s) $st $cb $s
	    # Wait until the connection is complete -- this will halt the entire proess though?  Ugh.
	} {
	    send $s "Editing requires mudFTP.
	    ret $st
	}
    }
}
