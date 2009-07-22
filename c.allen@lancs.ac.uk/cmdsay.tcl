lapp ::cmdtable {say Command::cmdsay}
P cmdsay {s p} {
	send $s "&mYou say, '$p&m'"
	starb $s "&m$::name($s) says, '$p&d&m'"
}



