package mm;

import mm.*;

import java.util.*;
import java.net.*;
import java.io.*;

public class MM extends Object {

	public static boolean shutdown;

	public static Vector socks = new Vector(30);

	static int gnum = 1;

	static void main(String args[]) {

		/* Initialize... */

		MObj.init();

		MObj.createWorld();

		/* Obtain server socket... */

		ServerSocket mud = null;

		try {

			mud = new ServerSocket(9876);

			mud.setSoTimeout(200);

		} catch (Exception x) {

			x.printStackTrace();
			System.exit(1);
		}

		/* Listen for players and check for commands... */

		System.out.println("Medusa waiting on port " + mud.getLocalPort());

		Socket sock;

		MObj player;

		Enumeration e;

		while ( !shutdown ) {

			/* Listen for a new player... */

			sock = null;

			try {

				sock = mud.accept();

				System.out.println(	"Connected by "
							+ sock.getInetAddress()
							+ " on port "
							+ sock.getPort() );

			} catch ( java.io.InterruptedIOException x ) {
			  ;

			} catch (Exception x) {

				x.printStackTrace();

			}

			/* If we have a new player, log them in as a guest... */

			if (sock != null) {
				player = new MObj("guest" + (gnum++), MObj.BS_PLAYER);

				player.login(sock);
			}

			/* Check all sockets for commands... */

			for (	e = socks.elements() ;
				e.hasMoreElements() ;
				) {

				player = (MObj) e.nextElement();

				/* If we have one, dispatch it... */

				try {
					if (player.in.ready()) {
						player.process(player.in.readLine());

						player.send("> ");
					}

				} catch (Exception x) {

					x.printStackTrace();

					player.logout(false);
				}
			}
		}

		/* Serialize the world so we can restart it... */

		MObj.saveWorld();

		/* Kick the players off... */

		bcast( "Shutting Down now.\nBye!");

		for (	e = socks.elements() ;
			e.hasMoreElements() ;
			) {

			player = (MObj) e.nextElement();

			player.logout(false);

			socks.remove(player);
		}

		/* Close the socket... */

		try {

			mud.close();  

		} catch (Exception x) {

			x.printStackTrace();
		}

		/* And we're all done... */
	}

	/* Send a message to every connected player... */

	public static void bcast(String str) {

		for (	Enumeration e = socks.elements() ;
			e.hasMoreElements() ;
			) {

			MObj player = (MObj) e.nextElement();

			player.send(str);
		}
	}
	
	/* Find a specific player... */

	public static MObj findPlayer(String str) {
	
		for (	Enumeration e = socks.elements() ;
			e.hasMoreElements() ;
			) {
	
			MObj player = (MObj) e.nextElement();

			if (player.name.startsWith(str)) {
				return player;
			}	
		}

		return null;
	}
}

