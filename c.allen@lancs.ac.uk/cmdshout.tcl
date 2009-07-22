lapp ::cmdtable {shout Command::cmdshout}
P cmdshout {s p} {
	send $s "&yYou shout '$p&y'"
	sendToAllBut $s "&y$::name($s) shouts '$p&d&y'"
}



