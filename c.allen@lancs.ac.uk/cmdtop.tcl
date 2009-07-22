lapp ::cmdtable {top Command::cmdtop}
P cmdtop {s m} {
	FE p $::sl {
		if {$::state($p) == $::SP} {
		    S j "$::name($p)"
		    lapp j "$::at($p)"
		    lapp j "[expr $::at($p) * 100 / $::ta($p)]"
		    lapp l $j
		}
	}
	S o "Top meditators:"
	FE e [lsort -decreasing -index 1 -integer $l] {
		append o [format "\n%-16s %5d %3d%%" [lindex $e 0] [lindex $e 1] [lindex $e 2]]
	}
	send $s $o
}



