lapp ::cmdtable {tcl Command::cmdtcl}

P cmdtcl {s c} {
    if {[regexp {exec} $c]} {
	send $s "Naughty!"
	ret
    }
    if {[catch {
	send $s "[eval $c]"
    }] == 1} {
	send $s $::errorInfo
    }
}
