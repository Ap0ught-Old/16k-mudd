lapp ::cmdtable {quit Command::cmdquit}
P cmdquit {s a} {
    # If in the zenroom, update their total idle time.
    if {$::loc($s) == 2} {
	S ::at($s) [calc $s]
	S ::ta($s) [expr $::ta($s) + [time] - $::is($s)]
    }
    send $s "Quitting.\nIdle [expr $::at($s) * 100 / $::ta($s)]%."
    Server::save $::name($s) $::pw($s)
    ::sendToAllBut $s "$::name($s) has quit."
    Server::disconnect $s

}



