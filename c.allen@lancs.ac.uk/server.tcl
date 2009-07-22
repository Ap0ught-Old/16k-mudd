# Alias V as Viable
rename variable V
rename lappend lapp
rename return ret
rename set S
rename proc P
rename foreach FE

source comtable.tcl
source comeditor.tcl
source mudftp.tcl
source editor.tcl
source room.tcl
source zen.tcl
Zen::init
S PORT 4000
# Total users variable
S TU 0
# STATE_VALIDATE
S SV 0
# STATE_PLAYING
S SP 1

# STATE_MUDFTP (INITIAL)
S SM 2
# Auth'd waiting for command.
S SA 3
# Waiting for file pushed to user.
S SW 4
# Waiting to get file back from user.
S SR 5

# Shutdown Viable.
S SD 0
S sl {}

# Login failure message
S FM "Invalid username/password."

# Load up all the commands.
S files {}
if {[catch {glob cmd*.tcl} files] == 0} {
    FE f $files {
	namespace eval Command "source $f"
    }
}
# Sort the commands into alphabetical order.
S ::cmdtable [lsort -index 0 $::cmdtable]

namespace eval Server {
    P initSocket {} {
	socket -server Server::accept $::PORT
	# Initialise mudftp server.
	Mudftp::init [expr $::PORT+6]

	vwait SD

	FE s $::sl {
	    close $s
	}

	exit $::SD
    }

    P accept {s rhost rport} {
	addConnection $s $rhost $::SV
	puts $s $::M
    }

    # Generic method to add a connection to the list of connections.

    P addConnection {s h st} {
	# Initialise all variables associated with this connection.
	S ::mudftp($s) 0
	S ::desc($s) "I'm too boring to set a description."
	S ::name($s) 0
	S ::pw($s) 0
	S ::loc($s) 1
	S ::Zen::qu($s) 0
	S ::Zen::tf($s) 0
	S ::ga($s) 0
	# Total available time
	S ::ta($s) 1
	# Idle since (in zen room)
	S ::is($s) [time]
	# Logged on since
	S ::ls($s) [time]
	# This should be read from the pfile.
	# Actual idle time
	S ::at($s) 0

	fconfigure $s -buffering line -blocking FALSE -translation auto
	fileevent $s readable "Server::data $s"
	S ::hostname($s) $h
	S ::state($s) $st
	
	lapp ::sl $s
    }

    P data {s} {
	if [eof $s] {
	    disconnect $s
	    ret
	}

	S l [gets $s]
	if {$::state($s) < $::SM && $l == ""} {
	    ret
	}
	interpret $s $l
    }

    # Looks at message m from socket s.

    P interpret {s m} {
	if {$::state($s) >= $::SM} {
	    Mudftp::interpret $s $m
	    ret
	}

	if {$::state($s) == $::SV} {

	    # Expect create username password
	    # Expect connect username password

	    if {[regexp {^connect ([^ ]+) ([^ ]+)$} $m d u p] == 1} {
		S u [nm $u]
		connect $s $u $p
		ret
	    }
	    if {[regexp {^create ([^ ]+) ([^ ]+)$} $m d u p] == 1} {
		S u [nm $u]
		create $s $u $p
		ret
	    }
	    # Allow them to quit.
	    if {$m == "quit"} {
		disconnect $s
		ret
	    }
	    puts $s $::M
	    ret
	}

	if {$m == ""} {
	    ret
	}

	# If this person is in the Zen room, use that interpret.
	if {$::loc($s) == 2} {
	    if {[::Zen::interpret $s $m] == 1} {
		ret
	    }
	}
	
	if {$::state($s) == $::SP} {
	    S p {}
	    if {[regexp {^[ ]*([^ ]+) (.*)[ ]*$} $m d c p] == 0} {
		if {[regexp {^[ ]*([^ ]+)[ ]*$} $m d c] == 0} {
		    send $s "Error."
		    ret
		}
	    }
	    FE i $::cmdtable {
		if {[string match "[string tolower $c]*" "[lindex $i 0]"]} {
		    if {[catch {
			[lindex $i 1] $s $p
		    }]} {
			send $s $::errorInfo
		    }
#		    S ::is($s) [time]
		    ret
		}
	    }
	}
	
	send $s "'[lindex $m 0]': unknown command."
    }
    
    P nm {n} {
	S n [string tolower $n]
	S n "[string toupper [string range $n 0 0]][string range $n 1 end]"
	ret $n
    }

    P disconnect {s} {
	S ::sl [lreplace $::sl [lsearch $::sl $s] [lsearch $::sl $s]]
	close $s
    }

    P connect {s u p} {
	# See if this person is already logged on.
	FE so $::sl {
	    if {$::name($so) == $u} {
		# Reconnect as this user.
		
		if {$::pw($so) != $p} {
		    puts $s $::FM
		    ret
		}
		
		S ::name($s) $::name($so)
		S ::pw($s) $::pw($so)
		S ::state($s) $::SP
		
		disconnect $so
		# Increase user count
		incr ::TU
		::sendToAllBut $s "$::name($s) has reconnected."
		send $s "You have reconnected."
		ret
	    }
	}

	# @@ Load up the user from file?

	if {[catch {
	    S f [open "users/$u" r]
	    # Read the line from f.
	    gets $f l
	    
	    if {$p == $l} {
		# Log this user on.
		logon $s $u $p
	    } {
		puts $s $::FM
	    }
	}]} {
	    puts $s $::FM
	}
	ret
    }

    P create {s u p} {
	# Check that this username doesn't already exist
	FE s $::sl {
	    if {$::name($s) == $u} {
		puts $s "Already connected.  Use connect to reconnect."
		ret
	    }
	}

	# Check that a userfile of this name doesn't exist...
	if {[catch {
	    S f [open "users/$u" r]
	    S t 1
	}] == 1} {
	    S t 0
	}

	if {$t == 1} {
	    close $f
	    puts $s "That name is already taken."
	    ret
	}
	
	logon $s $u $p
	
	ret
    }
    

    P logon {s u p} {
	S ::name($s) $u
	S ::pw($s) $p
	S ::state($s) $::SP
	save $u $p
	
	# Increase user count
	incr ::TU
	::sendToAllBut $s "$u has logged on."
	# Add this to the list of zennames
	lapp $Zen::ul $u
	puts $s "The 'commands' command will give you a list of commands.\nUse the 'ga' command to enable telnet GA/EOR."
	Command::cmdlook $s {}
    }

    P save {u p} {
	S f [open "users/$u" w]
	puts $f $p
	close $f
    }

}
#signal trap "SIGINT" {S SD 1}


# Sends a message to the specified socket.
P send {s m} {
    # regsub on the stuff to put in ANSI colour
    S l {{b 30} {r 31} {g 32} {y 33} {l 34} {m 35} {c 36} {w 37} {d 0} {@ 1}}
    FE c $l {
	S d [lindex &$c 0]
	regsub -all "$d" $m \033\[[lindex $c 1]m m
    }

    puts $s $m\033\[0m
    puts -nonewline $s "$Room::name($::loc($s)) > "
    if {$::ga($s)} {
	puts -nonewline $s "\377\357"
    }
    flush $s
}

P sendToAll {m} {
    FE s $::sl {
	if {$::state($s) == $::SP} {
	    send $s $m
	}
    }
}

P sendToAllBut {s m} {
    FE so $::sl {
	if {$so != $s && $::state($so) == $::SP} {
	    send $so $m
	}
    }
}

# sendToAllRoomBut - took too many bytes for a commonly used function.

P starb {s m} {
    FE so $::sl {
	if {$so != $s && $::state($so) == $::SP && $::loc($so) == $::loc($s)} {
	    send $so $m
	}
    }
}

P sendToAllRoom {s m} {
    FE so $::sl {
	if {$::state($so) == $::SP && $::loc($so) == $::loc($s)} {
	    send $so $m
	}
    }
}

# Returns the socket of the user with name $m.  Returns "" if no user.

P findUser {m} {
    FE s $::sl {
	if {[string match [string tolower $m]* [string tolower $::name($s)]]} {
	    ret $s
	}
    }
    ret
}

P date {} {
    ret [clock format [clock seconds]]
}

P time {} {
    ret [clock seconds]
}

P calc {s} {
    ret [expr $::at($s) + [time] - $::is($s)]
}

# Startup time
S ST [date]
# Greeting message
S M "Welcome to crappotalk.\nThe server started up: $::ST\nUse \"connect username password\" to reconnect to an existing user\nUse \"create username password\" to create a new one\nUse \"quit\" to exit."
Server::initSocket
