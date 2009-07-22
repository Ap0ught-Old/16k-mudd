lapp ::cmdtable {shutdown Command::cmdshutdown}
P cmdshutdown {s p} {

	puts $::name($s)
	puts $p

	if {$::name($s) == "Zoia" && $p == "yes"} {
		sendToAll "Shutting down."
		S ::SD 1
		file delete reboot
		ret
	}

	sendToAll "Rebooting NOW!"
	S ::SD 1
}



