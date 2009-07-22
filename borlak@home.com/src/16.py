#!/usr/bin/python

# Version: 1.02

import cgi, os, sys, string, re

def showForm():
  print """
<HTML><BODY>
<FORM METHOD="post" ENCTYPE="multipart/form-data">
Select options that fit your language below:"""
  for opt,optname in options.items():
    print "<BR><INPUT TYPE=checkbox NAME=%s> %s\n" % (opt, optname)

  print """
<BR>
Enter filename to upload: <INPUT TYPE=file NAME=file>
<BR>
<INPUT TYPE=submit NAME=submit VALUE="Process file">
</FORM>
</BODY></HTML>
"""

def handleInput(form):
  file = form["file"]
  if not file.file:
    raise Exception, "Not a file"
  data = file.value
  print "<PRE>\n"
  analyzeFile(data, form)
  print "</PRE>\n"


def analyzeFile(data, opt):
  print "File size:                   %6d bytes" % len(data)
  print "Lines:                       %6d" % string.count(data, '\n')

  data = string.replace(data, '\r', '')
  print "\\r striped:                  %6d" % len(data)

  if opt.has_key('h'):
    data = re.compile('#.*$', re.MULTILINE).sub('',data)
    print "# removed:                   %6d" % len(data)

  if opt.has_key('s'):
    data = re.compile('//.*$', re.MULTILINE).sub('',data)
    print "// removed:                  %6d" % len(data)

  if opt.has_key('c'):
    data = re.compile(r'/\*.*?\*/',re.DOTALL).sub('',data)
    print "/* removed:                  %6d" % len(data)

  if opt.has_key('l'):
    data = re.compile('^[ \t]+', re.MULTILINE).sub('',data)
    data = re.compile('[ \t]+', re.MULTILINE).sub(' ',data)
    print "leading blanks removed:      %6d" % len(data)

  if opt.has_key('b'):
    data = re.compile(r'^\s*\n', re.MULTILINE).sub('',data)
    print "blank lines stripped:        %6d" % len(data)

  print "\nFinal result:                %6d" % len(data)
  print "                             ======\n"

  if len(data) > 16384:
    print "You are OVER the limit!"
  else:
    print "Size is acceptable."

  print "\nModified file follows"
  print "====================="
  print cgi.escape(data)

options = {
  'h': "Strip everything after # (Python, Perl)",
  's': "Skip everything after // (C++, Java)",
  'c': "Skip everything between /* */ (C, Java, C++)",
  'l': "Skip leading blanks, compress multiple blanks (everything but Python)",
  'b': "Strip any fully blank lines (all languages"
}

def handleCommandLine():
  opt = {}
  data = ""
  
  for arg in sys.argv[1:]:
    if arg[0] == '-':
      if not options.has_key(arg[1]):
        print "Unknown option: %c" % arg[1]
        sys.exit(1)
      opt[arg[1]] = 1
    else: # must be a file
      try:
        data = data + open(arg).read()
      except:
        print "Error reading file %s" % arg

  if not data:
    print "No input files specified. Following are valid options:"
    for opt,desc in options.items():
      print "-%c: %s" % (opt,desc)
    print "\nFor example:\n%s -l -c -b file1.c file2.c (for C programs)" % sys.argv[0]
    print "%s -h -b file.py (for Python)"  % sys.argv[0]
  else:
    analyzeFile(data, opt)
  
# Called as CGI or command line?
if not os.environ.has_key('REQUEST_METHOD'):
  handleCommandLine()
else:
  print "Content-type: text/html\n"
  sys.stderr = sys.stdout
  form = cgi.FieldStorage()
  if form.has_key('submit'):
    handleInput(form)
  else:
    showForm()

