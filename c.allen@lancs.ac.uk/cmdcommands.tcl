lapp ::cmdtable {commands Command::cmdcommands}
P cmdcommands {s a} {
    S o "Commands available to you:\n"
    FE c $::cmdtable {
	append o "  [lindex $c 0]\n"
    }
    send $s $o
}



