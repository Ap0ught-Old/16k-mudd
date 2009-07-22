Tinyadamud

This entry for the 16K MUD competition is writen in Ada (ISO/IEC 8652:1995).
GNAT, a free Ada compiler built on gcc technology, is available from:
ftp://cs.nyu.edu/pub/gnat/

For Linux related information, please visit:
http://www.adapower.com/linux/

For windows related information, please visit:
http://www.adapower.com/windows/

For Macintosh related information, please visit:
http://www.adapower.com/lab/macos/index.html

The source code delivered in this entry were developed using GNAT 3.11 under
MachTen 4.1.1 on a Macintosh computer.  The AdaSockets 0.1.3 library was
used as a socket library:
http://www-inf.enst.fr/ANC/

The source files included in this entry are:
tinyadamud.adb
signals.ads
signals.adb
sockets-mud.ads
sockets-mud.adb

These files follow the GNAT file naming convention, but are not required.

Once GNAT and AdaSockets are installed, and the environment variable
ADA_INCLUDE_PATH is set to point to the AdaSockets library, Tinyadamud
can be built using this command:
gnatmake tinyadamud.adb

The executable "tinyadamud" is created.  The default port is 6565.

Tinyadamud is very limited.  Besides providing crude chat capabilities,
it allows the creation of rooms with descriptions.  Unfortunately,
there was not room for any editing features.  Rooms can be linked.
The character save feature was removed after room descriptions pushed
the source code over the 16K limit.

I hope to make available Simple_Ada_Mud sometime in the near future.
This will be available through a link on my mud page:
http://members.aol.com/drveg/mud/index.html

--djk
