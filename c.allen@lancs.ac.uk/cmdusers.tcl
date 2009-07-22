lapp ::cmdtable {users Command::cmdusers}
P cmdusers {s a} {
	S i 0
	FE so $::sl {
		incr i
		S h $::hostname($so)
		S u $::name($so)

		if {$::state($so) == $::SV} {
			S u "Unknown"
		}

		append o [format "%-20s %-40s\n" $u $h]
	}

	append o "You see $i user(s)."
	send $s $o
}



