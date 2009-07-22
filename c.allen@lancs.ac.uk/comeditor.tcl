namespace eval Command {
    
    P addCommand {name a body} {
	removeCommand $name
	# Write it out to its own file for loading back in next boot.
	S fp [open cmd$name.tcl w]
	puts $fp "lapp ::cmdtable \{$name Command::cmd$name\}"
	# Omit the trailing \} since that's already there.
	puts $fp "P cmd$name \{$a\} \{\n$body\n"
	close $fp
	# Load it back in so that the function now works.
	source "cmd$name.tcl"
	# Sort the commands into alphabetical order.
	S ::cmdtable [lsort -index 0 $::cmdtable]
    }
    
    P removeCommand {name} {
	# Figure out a way to unload the procedure
	# Remove the command from the command table.
	FE cmd $::cmdtable {
	    if {[lindex $cmd 0] == "$name"} {
		S ::cmdtable [lreplace $::cmdtable [lsearch $::cmdtable $cmd] [lsearch $::cmdtable $cmd]]
	    }
	}
	catch {rename cmd$name.tcl ""}
    }
    
    P edit {s name} {
	if {$name == ""} {
	    send $s "Syntax: cmdedit <command>"
	    ret
	}

	catch {unset str}
	if {[catch {
	    S str "P cmd$name \{[info args cmd$name]\} \{[info body cmd$name]\}"
	}] == 1} {
	    puts $::errorInfo
	    S str "P cmd$name \{s p\} \{\n\}"
	}
	Editor::edit $s $str Command::edit_cb
    }
    
    P edit_cb {s str} {
	# Need to parse the results...
	FE l [split $str \n] {
	    if {[regexp {P cmd([^ ]+) \{(.*)\} \{} $l d n a] == 1} {
		# Found the P definition line
	    } {
		append data $l\n
	    }
	}
	addCommand $n $a $data
	send $s "Command $n added"
    }
}
 
