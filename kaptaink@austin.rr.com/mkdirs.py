# simple script to make dirs for serenity

from os import *

dirs=['a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z']

print "making dir 'players'"
mkdir(path.normcase("players"))
for dir in dirs:
	print "making dir 'players/%s'" % (dir)
	mkdir(path.normcase("players/%s" % (dir)))
