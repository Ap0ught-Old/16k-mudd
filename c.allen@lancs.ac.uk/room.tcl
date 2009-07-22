namespace eval Room {
    S ID 0
    P addRoom {n d e} {
	V ID
	V name
	V desc
	V exits
	V roomlist
	S i [incr ID]
	S name($i) $n
	S desc($i) $d
	S exits($i) $e
	lapp roomlist $i
    }
}
source room.dat