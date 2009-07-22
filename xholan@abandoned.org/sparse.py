#Eli Fulkerson, 2000

#I unfortunately had to do some minor gymnastics to come in over the limit,
# I was some 500 bytes over when I had completed my feature list, and have
# been forced to compress some verbose variable names as a result.
# conn --> is now co
# room --> is now rm
# socket --> is now sk

#see sparse.htm for verbose documentation

from socket import *
from select import *
from string import *
from whrandom import *
from time import clock
 
HOST = ''
PORT = 4000

#color constants
endl = chr(27) + "[37m\n\r"
#color implementation
def c(color):
    return chr(27) + "[" + `color` + "m"

def sellOre(args,curr, rate, co):
    args = int(args)
    if args > curr:
        co.sk.send("You don't have that much.\n\r")
        return curr
    else:
        co.sk.send("You sell " + `args` + "units for " + `args * .01 * rate` + "credits.\n\r")
        co.ship.cash = co.ship.cash + (args * .01 * rate)
        return curr - args
    
def buyOre(args,curr, rate, co):
    invalid = 0
    for x in args[:]:
        if not x in ["1","2","3","4","5","6","7","8","9","0"]:
            invalid = 1
    if invalid == 0:
        args = int(args)
    else:
        args = 0        
    if co.ship.cash < 1:
        co.sk.send("Your credit isnt that good.\n\r")
        return curr
    else:
        if curr + args > 100:
            co.sk.send("You can't hold that much.  Purchasing "  + `(100 - curr)` + " units for " + `(100 - curr) * .01 * rate` + " credits.\n\r")
            co.ship.cash = co.ship.cash - ((100 - curr) * .01 * rate)
            return 100
        else:
            co.sk.send("Purchasing "  + `args` + " units for " + `args * .01 * rate` + "credits.\n\r")
            co.ship.cash = co.ship.cash - (args * .01 * rate)
            return curr + args

class Universe:
    def __init__(self):
        #room generator 
        self.rm  = []
        #create rooms
        for x in range(1,1000):
            newRoom = Room(x)
            self.rm.append(newRoom)
        #core
        for x in range(1,250):
            self.addExit(x, x+1)
            if (x+1) % 5 == 0:
                self.addExit(x, x+5)
            if (x +1) % 10 == 0:
                self.addExit(x, x+10)
            if (x+1) % 50 == 0:
                self.addExit(x, x+50)
        #near worlds
        for x in range(251,505):
            self.addExit(x, x + 1)
            self.addExit(x, x+ 2)
        #nebula
        for x in range(500,999):
            rand =  randint(501,998)
            self.addExit(x, rand)
            rand = randint(501,998)
            self.addExit(x,rand)
        #chaos
        for x in range(1,40):
            rand1 = randint(2,998)
            rand2 = randint(2,998)
            if not rand1 == rand2:
                self.addExit(rand1,rand2)

    print("The Universe Flickers into Being.")            

    def addExit(self, side1,side2):
        newExit = Exit()
        newExit.right = self.rm[side1]
        self.rm[side2].exit.append(newExit)
        newExit = Exit()
        newExit.right = self.rm[side2]
        self.rm[side1].exit.append(newExit)
            
class Exit:
    def __init__(self):
        self.right = Room(-1)
        self.mined = 0
        
class Room:
    def  __init__(self, sector):
        self.name = `sector`
        self.exit = []
        if sector < 251:
            self.desc = "Somewhere Near the Galactic Core"
            self.sun = randint(7,13)
        elif sector < 501:
            self.desc = "Somewhere In the Outer Worlds"
            self.sun = randint(5,13)
        elif sector < 751:
            self.desc = "Somewhere In the Nebula"
            self.sun = randint(0,12)
        else:
            self.desc = "Somewhere Beyond the Nebula"
            self.sun = randint(0,9)
        if self.sun > 7 and randint(1,2) < 2 and not sector == -1:
            self.planet = Planet()
        else:
            self.planet = -1

    def sundesc(self):
        if self.sun == 0:
            return "A spiral of rock and gas is being drawn carefully into a black hole."
        elif self.sun < 8:
            return "There is no star nearby."
        elif self.sun < 10:
            return "This system is lit by a dim dwarf star."
        elif self.sun < 12:
            return "This system is lit by an average yellow sun."
        else:
            return "This system is lit by a supergiant."
                       
class Ship:
    def __init__(self):
        self.time = 0
        self.second = 0
        self.docked = 0
        self.name = "unnamed"
        self.rm = Room(-1)
        self.fuel = 1000
        self.hp = 1000
        self.max = 1000
        self.ore1 = 0
        self.ore2 = 0
        self.ore3 = 0
        self.cash = 100
        
    def damage(self,num):
        self.hp = self.hp - num
        
    def useFuel(self, fuel):
        temp = self.fuel - fuel
        if temp< 0:
            return 0
        else:
            self.fuel = self.fuel - fuel
            return 1
    
    def regen(self):
        #regen 1 point of hp and 1 point of fuel per second.
        if not self.second == int(clock()) % 60:
            if self.fuel < self.max:
                self.fuel = self.fuel + 1
            if self.hp < self.max:
                self.hp = self.hp + 1
        self.second =  int(clock()) % 60
    
    def rename(self, args, co, players):
        args = split(args)
        if len(args) > 1 :
            old = self.name
            self.name = args[1]
            co.act(players,  c(32) + "Info: " + old + " has been renamed to " + self.name + "." + endl)
        else:
            co.sk.send("You must specify a new name.\n\r")
        
class Planet:
    def __init__(self):
        self.rm = Room(-1)
        self.ore1 = randint(25,250)
        self.ore2 = randint(25,250)
        self.ore3 = randint(25,250)
        self.name = choice(["Kra", "A", "E", "I", "O", "U", "Thie", "Mar", "Bel", "Ji", "Li", "Hra", "Fra", "Dra"]) + choice(["ll", "lb", "lod", "b", "c", "d", "fr", "m", "v", "r", "rout", "t", "sil", "em"]) + choice(["eth", "son", "tes", "las", "mis", "nos", "al", "is", "oth", "och", "in", "ir", "er", "ud", "ik"])

class Connection:
    def __init__(self, sk, address):
        self.sk = sk
        self.ip = address[0]
        self.dataqueue = ""
            
    def actRoom(self, player, playerlist, message):
        for co in playerlist:
            if player.ship.rm == co.ship.rm and not player.ship == co.ship:
                co.sk.send(message) 
    
    def act(self, playerlist, message):
        for co in playerlist:
            co.sk.send(message)
   
    def enqueue(self, string):
        self.dataqueue = self.dataqueue + string
        
    def queue(self):
        return self.dataqueue
        
    def resetqueue(self):
        self.dataqueue = ""
         
    def fileno(self):
        return self.sk.fileno()
 
    def receive(self):
        return self.sk.recv(1024)
    
    def showRoom(self, players):
        #What area of space is this?
        self.sk.send(self.ship.rm.desc + endl)
        
        #What sector are we in?
        self.sk.send("Sector: " + self.ship.rm.name + endl)
        
        #What wormgates leave from here?
        self.sk.send("Wormgates: ")
        temp = self.ship.rm.exit
        for a in range(len(temp)):
            self.sk.send(temp[a].right.name)
            #Is there a planet there?
            if not temp[a].right.planet == -1:
                self.sk.send("p")
            #Is there a black hole there?
            if temp[a].right.sun ==  0:
                self.sk.send("x")
            self.sk.send(" ")
        self.sk.send(endl)

        #What does this sector look like?
        temp = self.ship.rm.sundesc()
        self.sk.send(temp + endl)

        #Is there a planet here?
        if not self.ship.rm.planet == -1:
            self.sk.send("The planet " + self.ship.rm.planet.name + " is here.\n\r")
        
        #Are there other ships here?
        for co in players:
            if co.ship.rm == self.ship.rm and not co.ship == self.ship:
                self.sk.send("Another ship, the '"+ co.ship.name + "', is here")
                if co.ship.docked == 1:
                    self.sk.send(", docked.\n\r")
                else:
                    self.sk.send(".\n\r")
            
        #Anything else?
        #prompt
        self.sk.send("\n\rFuel: " + `self.ship.fuel` + " Hp: " + `self.ship.hp` + ">")
        self.sk.send(endl)

    #if var == 1, then turn mine on.  if var == 0, then turn mine off.
    def mine(self,side1,side2, uni,var):
        temp = uni.rm[int(side1) - 1].exit
        temp2 = uni.rm[int(side2) - 1].exit
        foundGate = 0
        for a in range(len(temp)):
            if temp[a].right.name == side2:
                temp[a].mined = var
                foundGate = 1
                for b in range(len(temp2)):
                    if temp2[b].right.name == side1:
                        temp2[b].mined = var
        return foundGate
            
    def mineExit(self,args,uni,players):
        args = split(args)
        if len(args) > 1:
            gate = args[1]
            if self.mine(gate, self.ship.rm.name, uni, 1) == 1:                    
                self.sk.send("You launch a subspace mine, which is quickly swallowed by the wormgate that leads to sector " + gate + ".\n\r")
                self.actRoom(self, players, "The " + self.ship.name + " fires a subspace mine into the wormgate that leads to sector " + gate + ".\n\r")
            else:
                self.sk.send("Not a valid gate.\n\r")

    def torpedo(self,args,uni,players):
        args = split(args)
        foundship = 0
        if len(args) > 1:
            target = args[1]
            for player in players:
                if player.ship.rm.name == self.ship.rm.name and target == player.ship.name:
                    if self.ship.useFuel(500) == 1:
                        self.sk.send("You target the '" + player.ship.name + "' and launch a torpedo.  It impacts milliseconds later.\n\r")
                        self.actRoom(player, players, "The '" + player.ship.name + "' rocks as a torpedo strikes it.\n\r")
                        player.sk.send(c(31) + "Your ship rocks violently as the '" + self.ship.name + "' fires upon you!" + endl)
                        player.ship.damage(randint(200,500))
                        foundship = 1
                    else:
                        self.sk.send("You don't have enough fuel.\n\r")
            if foundship == 0:
                self.sk.send("Invalid target.\n\r")
        else:
            self.sk.send("You must specify a target.\n\r")

    def enterGate(self,args,uni, players):
        args = split(args)
        foundGate = -1
        if len(args) > 1 :
            gate = args[1]
            temp = self.ship.rm.exit
            for a in range(len(temp)):
                if temp[a].right.name == gate:
                    foundGate = temp[a].right
                    mined = temp[a].mined
                    if mined > 0:
                        self.mine(gate, self.ship.rm.name, uni, 0)
            if foundGate == -1:
                self.sk.send("Not a valid gate.\n\r")
            else:
                self.actRoom(self, players, "The '" + self.ship.name + "' falls toward the wormgate to sector " + gate + " and is swallowed by darkness.\n\r")
                self.sk.send("Space ripples as your vessel falls through the wormgate to sector " + gate + ".\n\r")
                if not mined == 0:
                    self.sk.send(c(31) + "Your vessel is rocked as a subspace mine explodes within the wormhole!" + endl)
                    self.actRoom(self,players, "There is a flash of light deep within the wormhole's maw as it winks shut.\n\r")
                    self.ship.damage(randint(50,300))
                self.ship.rm = uni.rm[int(gate) - 1]
                if not mined == 0:
                    self.actRoom(self,players, "A small shockwave rocks your vessel as a wormgate winks open\n\r")
                self.actRoom(self, players, "A pulsing wormgate opens briefly, disgorging a vessel, the '" + self.ship.name + "'.\n\r") 
                self.showRoom(players)
            #if black hole!
            if self.ship.rm.sun == 0:
                self.sk.send("Your entire ship screams and strains as it emerges from the wormgate within the grasp of a fearsome black hole!  The wormgate twists upon itself and pulls you back, perhaps to safety...\n\r")
                self.ship.damage(randint(200, 600))
                self.ship.rm = uni.rm[randint(2,998)]
                self.actRoom(self, players, "A pulsing wormgate opens briefly, disgorging a vessel, the '" + self.ship.name + "'.\n\r")    
        else:
            self.sk.send("Enter what wormgate?\n\r")

class Handler:
    def __init__(self):
        self.clist = [ ]
        self.listener = socket(AF_INET, SOCK_STREAM)
        self.listener.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        self.listener.bind(HOST, PORT)
        self.listener.listen(32)
        print "Ready on port " + `PORT` + "."
 
    def run(self, uni):
        ready = self.clist[:]
        #regen and death check
        for co in ready:
            #if you are dead
            if co.ship.hp < 1:
                co.sk.send("It is over.\n\r")
                co.actRoom(co, self.clist[:], "With a flash of white light, the " + co.ship.name + "'s shielding finally gives way.  Your scanners are momentarily blinded as the energy of the explosion washes over your vessel.\n\r")
                self.clist.remove(co)
                print "Connection closed to " + co.ip + " (" + co.ship.name + ")."
                co.act(self.clist,  c(32) + "Info: The vessel '" + co.ship.name + "' has been destroyed." + endl)
            co.ship.regen()
        
        #process input
        ready.append(self.listener)
        ready = select(ready, [], [], 0.1)[0]
        for co in ready:
            if co is self.listener:
                sk, address = self.listener.accept()
                co = Connection(sk, address)
                self.clist.append(co)
                print "Connection accepted from " + co.ip + "."
                
                co.sk.send("Welcome Aboard.\n\r")
                co.sk.send(c(33)+ "Please see http://www.crosswinds.net/~forcharity/sparse.htm for documentation." + endl)
                co.act(self.clist,  c(32) + "Info: Someone has entered the game." + endl)
                co.ship = Ship()
                co.ship.time = int(clock())
                co.ship.rm = uni.rm[501]
                co.showRoom(self.clist[:])
            else:
                co.ship.regen()
                data = co.receive()
                if not data:
                    print "Connection closed to " + co.ip + "."
                    self.clist.remove(co)
                else:
                    if not data[-1] == '\n':
                        co.enqueue(data) 
                    else:
                        co.enqueue(data)
                        temp = co.queue() 
                        
                        #'look' command
                        if temp[0]== "l":
                            co.showRoom(self.clist[:])
                        
                        #'enter' command
                        elif temp[0] == "e":
                            if co.ship.useFuel(15):
                                if co.ship.docked == 0:
                                    co.enterGate(temp, uni, self.clist[:])
                                else:
                                    co.sk.send("Not while docked.\n\r")
                            else:
                                co.sk.send("You don't have enough fuel.\n\r")
                        
                        #'radio' command
                        elif temp[0] == "r":
                            temp = join(split(temp)[1:])
                            if temp:
                                #fizpop converter
                                co.act(self.clist[:], c(33) + "The '" + co.ship.name + "' radios, '" + temp  + "'"+ endl)
                            else:
                                co.sk.send("Radio what?\n\r")
                        
                        #'?help' command
                        elif temp[0] == "?":
                            co.sk.send("Your info:\n\r")
                            co.sk.send("\n\rFuel: " + `co.ship.fuel` + "(" + `co.ship.max` + ")\n\r")
                            co.sk.send("Hp: " + `co.ship.hp` + "(" + `co.ship.max` + ")\n\r")
                            co.sk.send("Ore1: " + `co.ship.ore1` + "\n\r")
                            co.sk.send("Ore2: " + `co.ship.ore2` + "\n\r")
                            co.sk.send("Ore3: " + `co.ship.ore3` + "\n\r")
                            co.sk.send("$" + `co.ship.cash ` + "\n\r")
                        
                        #'name' command
                        elif temp[0] == "n":
                            co.ship.rename(temp, co, self.clist[:])
                                              
                        #'dock' command
                        elif temp[0] == "d":
                            if co.ship.useFuel(10):
                                if co.ship.docked == 1:
                                    co.sk.send("You release your docking clamp, and gently slip away from the station.\n\r")
                                    co.actRoom(co,self.clist[:], "The " + co.ship.name + " disengages its docking clamp and slowly drifts away from the station.\n\r")
                                    co.ship.docked = 0
                                else:
                                    pl = co.ship.rm.planet
                                    if pl == -1:
                                        co.sk.send("There is no station here to dock with.\n\r")
                                    else:
                                        co.sk.send("You engage your docking clamp, securing your hull against " + pl.name + "'s spacedock.\n\r")
                                        co.sk.send("Trade rates: " + `pl.ore1 * .01` + "% for ore1, "+ `pl.ore2 * .01` + "% for ore2, "+ `pl.ore3 * .01` + "% for ore3.\n\r")
                                        co.actRoom(co,self.clist[:], "The " + co.ship.name + " engages its docking clamp and links with " + pl.name + "'s spacedock.\n\r")
                                        co.ship.docked = 1
                            else:
                                co.sk.send("You don't have enough fuel to use your docking clamp.\n\r")
                        
                        #'quit' command
                        elif temp[0] == "q":
                            co.ship.damage(1000000)
                        
                        #'viewer' command
                        elif temp[0] == "v":
                            if co.ship.useFuel(50):
                                co.sk.send(ljust("Name:", 15) + ljust("Fuel:", 10)+ ljust("Damage:", 10)+"Sector:\n\r")
                                for player in self.clist[:]:
                                    co.sk.send(ljust(player.ship.name, 15) + ljust(`player.ship.fuel`, 10)  + ljust(`player.ship.hp`, 10)+ player.ship.rm.name + "\n\r")
                                co.act(self.clist,  "Your sensors pick up a subspace ping.  Someone knows your location.\n\r")
                            else:
                                co.sk.send("You don't have enough fuel.\n\r")
                        
                        #'who' command
                        elif temp[0] == "w":
                            co.sk.send(ljust("Name:", 15)  + ljust("Seconds Online:", 25)+"IP:\n\r")
                            for player in self.clist[:]:
                                co.sk.send(ljust(player.ship.name, 15) + ljust(`int(clock()) - player.ship.time`, 25)+ player.ip + "\n\r")

                        #'buy' command.  purchase upgrade, ore1,ore2,ore3
                        elif temp[0] == "b":
                            if co.ship.docked == 0:
                                co.sk.send("You aren't docked at a planet.\n\r")
                            else:
                                args = split(temp)
                                if len(args) < 2:
                                    co.sk.send("Buy what?\n\r")
                                else:
                                    found = 0
                                    if len(args) > 2:
                                        if args[1] == "ore1":
                                            co.ship.ore1 = buyOre(args[2], co.ship.ore1, co.ship.rm.planet.ore1, co)
                                        if args[1] == "ore2":
                                            co.ship.ore2 = buyOre(args[2], co.ship.ore2, co.ship.rm.planet.ore2,co)
                                        if args[1] == "ore3":
                                            co.ship.ore3 = buyOre(args[2], co.ship.ore3, co.ship.rm.planet.ore3, co)
                                    elif args[1] == "ship":
                                        if co.ship.cash > co.ship.max:
                                            co.ship.cash = co.ship.cash - 1000
                                            co.ship.max = co.ship.max + 100
                                            co.sk.send("Your max hp and fuel have been increased by 100!\n\r")
                                        else:
                                            co.sk.send("You need " + `co.ship.max` + " credits for an upgrade.\n\r")
                                    else:
                                        co.sk.send("But how many?\n\r")
                                        
                        #'sell' command.  sell ore1,ore2,ore3
                        elif temp[0] == "s":
                            if co.ship.docked == 0:
                                co.sk.send("You aren't docked at a planet.\n\r")
                            else:
                                args = split(temp)
                                if len(args) < 2:
                                    co.sk.send("Sell what?\n\r")
                                else:
                                    if len(args) > 2:
                                        if args[1] == "ore1":
                                            co.ship.ore1 = sellOre(args[2], co.ship.ore1, co.ship.rm.planet.ore1, co)
                                        if args[1] == "ore2":
                                            co.ship.ore2 = sellOre(args[2], co.ship.ore2, co.ship.rm.planet.ore2,co)
                                        if args[1] == "ore3":
                                            co.ship.ore3 = sellOre(args[2], co.ship.ore3, co.ship.rm.planet.ore3, co)
                                    else:
                                        co.sk.send("But how many?\n\r")

                        #'torpedo' command
                        elif temp[0] == "t":
                            if co.ship.docked == 0:
                                co.torpedo(temp, uni,self.clist[:])
                            else:
                                co.sk.send("Not while docked.\n\r")
                            
                        #'mine' command
                        elif temp[0] == "m":
                            if co.ship.useFuel(200):
                                if co.ship.docked == 0:
                                    co.mineExit(temp, uni, self.clist[:])
                                else:
                                    co.sk.send("Not while docked.\n\r")
                            else:
                                co.sk.send("You don't have enough fuel.\n\r")

                        else:
                            co.sk.send("Huh?\n\r")
                        temp = ""
                        co.resetqueue()
                        
#Main loop
connections = Handler()
uni = Universe()
while 1:
    connections.run(uni)
