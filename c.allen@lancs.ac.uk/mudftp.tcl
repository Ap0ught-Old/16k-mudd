namespace eval Mudftp {

    P init {p} {
	V ID
	socket -server Mudftp::accept $p
	S ID 0
    }

    P accept {sock host port} {
	Server::addConnection $sock $host $::SM
    }

    P interpret {s l} {
	# What state is the connection currently in?

	# waiting to auth
	if {$::state($s) == $::SM} {
	    Mudftp::auth $s $l
	    ret
	}
	# waiting for command from client
	if {$::state($s) == $::SA} {
	    command $s $l
	    ret
	}
	# waiting for file to push to client
	if {$::state($s) == $::SW} {
	    ret
	}
	# waiting for client to send file back
	if {$::state($s) == $::SR} {
	    retrieve $s $l
	    ret
	}
    }

    # PUSH file to the client.
    P push {s d ca ps} {
	V cb 
	V psock
	V ID
	S psock($s) $ps
	S cb($s) $ca
	S c 0
	FE l [split $d \n] {
	    incr c
	}
	puts $s "SENDING [incr ID] $c c"
	puts $s $d
	S ::state($s) $::SR
    }

    # Retrieve file from client
    P retrieve {s l} {
	V data
	V count
	V total
	V cb
	V psock
	if {[regexp {^PUT [^ ]+ ([0-9]+)$} $l d total($s)] == 1} {
	    S count($s) 0
	    S data($s) {}
	    ret
	}
	append data($s) $l\n
	incr count($s)
	# Have we received the entire file?
	if {$total($s) == $count($s)} {
	    S ::state($s) $::SW
	    puts $s "OK csum"
	    # Call the callback function.
	    $cb($s) $psock($s) $data($s)
	}
    }

    # Wait for PUSH command
    P command {s l} {
	if {$l == "PUSH"} {
	    puts $s OK
	    S ::state($s) $::SW
	    ret
	}
	puts $s "FAILED PUSH only"
	Server::disconnect $s
    }

    # Allow the user to authenticate
    P auth {so l} {
	V total
	V count
	V data
	V cb
	V psock
	if {[regexp {^([^ ]+) ([^ ]+)$} $l d u p] == 1} {
	    S u [Server::nm $u]
	    FE s $::sl {
		if {$::name($s) == $u} {
		    if {$::pw($s) == $p} {
			puts $so "OK mudFTP 2.0"
			S ::mudftp($s) $so
			S ::name($so) $u\(mudftp\)
			S ::pw($so) $p
			S ::state($so) $::SA
			S psock($so) 0
			S total($so) 0
			S count($so) 0
			S data($so) {}
			S cb($so) {}
			ret
		    }
		}
	    }
	}
	puts $so FAILED
	Server::disconnect $so
    }
}
