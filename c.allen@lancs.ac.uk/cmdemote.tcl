lapp ::cmdtable {emote Command::cmdemote}
P cmdemote {s a} {
    if {$a != ""} {
	sendToAllRoom $s "&@$::name($s) $a"
    }
}



