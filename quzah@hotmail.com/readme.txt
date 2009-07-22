Quzah's Poorly Orchestrated Small MUD.
Copyright 2000, Jeremiah Myers.

This archive contains three files.
	readme.txt - this file.
	qposmud.c - my April 2000 entry into the 16k mud server competition.
	amused.c - a slightly amusing alteration, exploiting Erwin's CGI.

I started this whole thing entirely wrong. As a result, my entry is
lacking in features. The reason it lacks features is because I spend far
too much time rewriting--in fact, I was still rewriting the sockets on
the very last day of the competition!

The CGI 'exploit' is as follows:
	...
	for(x=0;x<10;x++)/*
		*/printf("do something here %d\n",x);/*
		*/printf("do something else %d\n",x);/*
	*/}
	...

Nothing major, just a nifty little trick with the way it strips out the
comment blocks. Using extra comments, you could in theory convert every
line in your code to a single line. (You natrually wouldn't want to do
this on your #def or #include ... lines.) I did this in 'amused.c' to a
small extent. I didn't feel like having one huge line of code, because
I didn't want to break the CGI script, and I didn't feel like running
into a potential "line too damn long" error with the compiler. At any
rate, it was slightly amusing, and if you run both files through the
CGI you'll notice a ~300 byte savings on what lines I did do this.

This mud compiles under cygwin, under win98, using the following command
line, without warnings or errors:

	gcc qposmud.c -W -Wall -lm

It should run under BSD/RedHat/whatever, but I've no way of testing it.
As a result, there is no startup script. (Because I don't know how to
write one.) Anyway, here's my small entry. Enjoy.

See the .c files for my license.

Quzah.