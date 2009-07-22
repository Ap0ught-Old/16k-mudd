lapp ::cmdtable {ga Command::cmdga}
P cmdga {s p} {
	if {$::ga($s)} {
		S ::ga($s) 0
		send $s "Turning GA off."
	} {
		S ::ga($s) 1
		send $s "Turning GA on."
	}
}



