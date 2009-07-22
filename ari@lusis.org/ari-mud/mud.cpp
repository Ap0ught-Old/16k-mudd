#include <list>
#include <hash_map>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <stream.h>

#define HM	"Outskirts"
#define NFOUND	"Not found."

#define C(x)	void x(Player *p, char *a)
#define A(x,y)	if(!y) { x->c->wr("Huh?"); return; };
#define W(x)	if(!x->wiz) { x->c->wr("Huh?"); return; }
#define MP(x)	((ps.find(x) == ps.end()) ? 0 : ps[x])
#define SL(x,y)	for(x = y.front(); !y.empty(); x = y.front()) {
#define EL(y)	y.pop_front(); }

class Cnx;
class Exit;
class Player;

typedef list<Cnx *> cnlt;
typedef list<Exit *> elt;
typedef list<Player *> plt;

class Cnx {
public:
  int sck;
  Player *plyr;
  Cnx(int);
  ~Cnx();
  wr(char *);
  int selected();
};

class Player {
public:
  char n[128];
  char p[128];
  char l[128];
  int wiz;
  int hp;
  Cnx *c;
  Player(char *tn, char *tp, char *tl) { strcpy(n, tn); strcpy(p, tp); strcpy(l, tl); wiz = 0; hp = 100; };
  ~Player();
};

class Exit {
public:
  char s[128];
  char d[128];
  Exit() { };
  Exit(char *, char *);
};

class Room {
public:
  char n[128];
  char d[9999];
  elt es;
  plt ps;
  ~Room();
  Emit(char *);
};

int sock;
int maxd;

cnlt cnxs;

struct eqt : public binary_function<char *, char *, bool> {
  bool operator()(char *&x, char *&y) { return !strcasecmp(x, y); }
};

inline size_t _hs(char *s) {
  unsigned long h = 0;
  for(;*s; ++s)
    h = 5*h + toupper(*s);
  return h;
}

struct chash {
  size_t operator()(char *s) { return _hs(s); }
};

hash_map<char *, void (*)(Player *, char *), chash, eqt> cmds;
hash_map<char *, Player *, chash, eqt> ps;
hash_map<char *, Room *, chash, eqt> rs;
plt mpl;

Player::~Player() {
  if(c)
    delete c;
  rs[l]->ps.remove(this);
  ps.erase(n);
  mpl.remove(this);
}

dodeath(Player *p) {
  rs[p->l]->Emit(form("%s has died.", p->n));
  delete p;
}

C(who) {
  Cnx *c;
  cnlt cnl = cnxs;
  p->c->wr("Online users:");
  SL(c,cnl)
    p->c->wr(form("  %s", c->plyr->n));
  EL(cnl)
}

C(look) {
  Player *t;
  Exit *e;
  Room *r = rs[p->l];
  plt pl = r->ps;
  elt el = r->es;
  p->c->wr(r->n);
  p->c->wr(r->d);
  p->c->wr("Players:");
  SL(t,pl)
    if(t->c)
      p->c->wr(form("  %s", t->n));
  EL(pl)
  p->c->wr("Exits:");
  el = r->es;
  SL(e,el)
    p->c->wr(form("  %s", e->d));
  EL(el)
}

C(go) {
  Exit *e;
  Room *r = 0;
  elt el = rs[p->l]->es;
  A(p,a);
  SL(e, el)
    if(!strcasecmp(e->d, a))
      r = rs[e->d];
  EL(el)
  if(r) {
    rs[p->l]->Emit(form("%s leaves.", p->n));
    rs[p->l]->ps.remove(p);
    p->l = r->n;
    r->ps.push_front(p);
    r->Emit(form("%s arrives.", p->n));
    look(p, 0);
  } else
    p->c->wr(NFOUND);
}

C(say) {
  A(p,a);
  rs[p->l]->Emit(form("%s says \"%s\"", p->n, a));
}

C(pose) {
  A(p,a);
  rs[p->l]->Emit(form("%s %s", p->n, a));
}

C(hit) {
  Player *t;
  int dd;
  A(p,a);
  t = MP(a);
  if(!t) {
    p->c->wr(NFOUND);
    return;
  }
  if(strcmp(p->l, t->l) || !t->c || t->wiz) {
    p->c->wr(NFOUND);
    return;
  }
  dd = 0.5 * (float) p->hp * (float) (rand() % 50) / 100.0;
  t->hp -= dd;
  t->c->wr(form("%s hit you for %d hp.", p->n, dd));
  p->c->wr(form("You hit %s for %d hp.", t->n, dd));
  if(t->hp <= 0)
    dodeath(t);
}

C(stat) {
  p->c->wr(form("You have %d hp.", p->hp));
}

C(wiz) {
  Player *t;
  W(p);
  A(p,a);
  t = MP(a);
  if(!t) {
    p->c->wr(NFOUND);
    return;
  }
  t->wiz = 1;
  if(t->c)
    t->c->wr("You have been wizzed.");
  p->c->wr("Wizzed.");
}

C(dewiz) {
  Player *t;
  W(p);
  A(p,a);
  t = MP(a);
  if(!t) {
    p->c->wr(NFOUND);
    return;
  }
  if(t == p) {
    t->c->wr("You can't dewiz yourself.");
    return;
  }
  t->wiz = 0;
  if(t->c)
    t->c->wr("You have been dewizzed.");
  p->c->wr("Dewizzed.");
}

C(ann) {
  Cnx *c;
  cnlt cnl = cnxs;
  W(p);
  A(p,a);
  cnl = cnxs;
  SL(c,cnl)
    c->wr(form("Announcement: %s", a));
  EL(cnl)
}

C(heal) {
  Player *t;
  W(p);
  A(p,a);
  t = MP(a);
  if(!t) {
    p->c->wr(NFOUND);
    return;
  }
  t->hp = 100;
  if(t->c)
    t->c->wr("You have been healed.");
  p->c->wr("Healed.");
}

C(kill) {
  Player *t;
  W(p);
  A(p,a);
  t = MP(a);
  if(!t) {
    p->c->wr(NFOUND);
    return;
  }
  if(t->wiz) {
    p->c->wr("You can't kill wizards.");
  }
  dodeath(t);
}

C(dig) {
  Room *r;
  Exit *e;
  W(p);
  A(p,a);
  if(rs.find(a) != rs.end()) {
    p->c->wr("Already exists.");
    return;
  }
  r = new Room;
  strcpy(r->n, a);
  r->d[0] = 0;
  rs[r->n] = r;
  e = new Exit(p->l, r->n);
  p->c->wr("Done.");
}

C(desc) {
  W(p);
  A(p,a);
  strcpy(rs[p->l]->d, a);
  p->c->wr("Description changed.");
}

C(nuke) {
  plt pl = mpl;
  Room *r;
  Player *tmp;
  W(p);
  if(!strcmp(p->l, HM)) {
    p->c->wr(form("You can't nuke %s.", HM));
    return;
  }
  pl.remove(p);
  SL(tmp,pl)
    if(!strcmp(tmp->l, p->l))
      go(tmp, HM);
  EL(pl)
  r = rs[p->l];
  go(p, HM);
  delete r;
  p->c->wr("Nuked.");
}

C(tel) {
  W(p);
  A(p,a);
  if(rs.find(a) != rs.end()) {
    rs[p->l]->Emit(form("%s leaves.", p->n));
    rs[p->l]->ps.remove(p);
    p->l = rs[a]->n;
    rs[a]->ps.push_front(p);
    rs[a]->Emit(form("%s arrives.", p->n));
    look(p, 0);
  } else
    p->c->wr(NFOUND);
}

char *splitws(char *s) {
  char *p;
  p = strpbrk(s, " \t");
  if(p)
    *p++ = 0;
  else
    return 0;
  while(*p && (*p == ' ') || (*p == '\t'))
    p++;
  if(!*p)
    p = 0;
  return p;
}

parse(Cnx *c, Player *p, char *s) {
  char *a;
  a = splitws(s);
  if(cmds.find(s) != cmds.end())
    cmds[s](p, a);
  else
    c->wr("Huh?");
}

cprs(Cnx *c, char *s) {
  Player *p;
  char *p1, *p2 = 0;
  p1 = splitws(s);
  if(p1)
    p2 = splitws(p1);
  if((strcmp(s, "cn") && strcmp(s, "cr")) || !p1 || !p2)
    c->wr("Invalid.");
  else {
    if(!strcmp(s, "cn")) {
      if(ps.find(p1) != ps.end())
        if(!strcmp(p2, ps[p1]->p) && !ps[p1]->c) {
          p = ps[p1];
          p->c = c;
          c->plyr = p;
          rs[p->l]->Emit(form("%s connects.", p->n));
          look(p, 0);
          return;
        }
      c->wr("Invalid password.");
    } else {
      if(ps.find(p1) != ps.end())
        c->wr("Name taken.");
      else {
        p = new Player(p1, p2, HM);
        rs[p->l]->ps.push_front(p);
        ps[p->n] = p;
        mpl.push_front(p);
        p->c = c;
        c->plyr = p;
        rs[p->l]->Emit(form("%s connects.", p->n));
        look(p, 0);
      }
    }
  }
}

Exit::Exit(char *a, char *b) {
  Exit *rt = new Exit;
  strcpy(s, a);
  strcpy(d, b);
  strcpy(rt->s, b);
  strcpy(rt->d, a);
  rs[a]->es.push_front(this);
  rs[b]->es.push_front(rt);
}

Room::~Room() {
  elt el;
  Exit *e, *e2;
  Room *r;
  SL(e,es)
    r = rs[e->d];
    el = r->es;
    SL(e2,el)
      if(!strcmp(e2->d, n)) {
        r->es.remove(e2);
        delete e2;
      }
    EL(el)
    delete e;
  EL(es)
  rs.erase(n);
}

Room::Emit(char *s) {
  Player *p;
  plt pl;
  pl = ps;
  SL(p,pl)
    if(p->c)
      p->c->wr(s);
  EL(pl)
}

Cnx::Cnx(int s) {
  sockaddr_in adr;
  int adrlen;
  adrlen = sizeof(adr);
  sck = accept(s, &adr, &adrlen);
  fcntl(sck, F_SETFL, O_NDELAY);
  if(sck >= maxd)
    maxd = sck + 1;
  plyr = 0;
  wr("-----------");
  wr("cn <n> <pw>");
  wr("cr <n> <pw>");
  wr("-----------");
}

Cnx::~Cnx() {
  cnxs.remove(this);
  close(sck);
  if(plyr) {
    rs[plyr->l]->Emit(form("%s disconnects.", plyr->n));
    plyr->c = 0;
  }
}

Cnx::wr(char *s) {
  char b[1024];
  strcpy(b, s);
  strcat(b, "\n");
  write(sck, b, strlen(b));
}

int Cnx::selected() {
  static char buf[1024];
  static char *bp = buf;
  char c;
  int got, n;
  c = 0;
  got = 0;
  while((c != '\n') && (c != '\r') && (n = read(sck, &c, 1))) {
    if(n < 0)
      return 0;
    got += n;
    *bp++ = c;
  }
  if(got <= 0)
    return 1;
  if((c == '\n') || (c == '\r')) {
    *(--bp) = '\0';
    bp = buf;
    if(*buf) {
      if(plyr)
        parse(this, plyr, buf);
      else
        cprs(this, buf);
    }
  }
  return 0;
}

int mksock(int prt) {
  int s;
  sockaddr_in server;
  int opt;
  s = socket(AF_INET, SOCK_STREAM, 0);
  opt = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(prt);
  bind(s, &server, sizeof(server));
  listen(s, 5);
  return s;
}

netloop() {
  fd_set inset;
  int fnd;
  timeval timeout;
  Cnx *c;
  cnlt cnl;

  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  FD_ZERO(&inset);
  FD_SET(sock, &inset);
  cnl = cnxs;
  SL(c,cnl)
    FD_SET(c->sck, &inset);
  EL(cnl)

  fnd = select(maxd, &inset, 0, 0, &timeout);
  if(FD_ISSET(sock, &inset)) {
    c = new Cnx(sock);
    cnxs.push_front(c);
  }
  cnl = cnxs;
  SL(c,cnl)
    if(FD_ISSET(c->sck, &inset))
      if(c->selected())
        delete c;
  EL(cnl)
}

hploop() {
  Player *p;
  plt pl;
  pl = mpl;
  SL(p,pl)
    if(p->hp < 100)
      p->hp++;
  EL(pl)
}

mkrm(char *n, char *d) {
  Room *r = new Room;
  strcpy(r->n, n);
  strcpy(r->d, d);
  rs[r->n] = r;
}

mkex(char *s, char *d) {
  new Exit(s, d);
}

mkpl(char *n, char *p, int w) {
  Player *t = new Player(n, p, HM);
  t->wiz = w;
  ps[t->n] = t;
  mpl.push_front(t);
  rs[t->l]->ps.push_front(t);
}

initobjs() {
  mkrm(HM, "You stand on a dirt path at the southern entrance to a small town.");
  mkrm("House", "You are inside a sod farmhouse.  The only window faces south out into the yard.  The house itself is modest, containing only a low, straw-filled bed, a large wooden chest, and a fireplace set into the north wall.");
  mkrm("Barn", "Standing in the dirt-floored barn, you smell the pungent odors of years of manure residue and are surrounded by several stalls.  Hanging on the cornerpost of each stall is riding tack, and, near the opposite wall, several leather saddles hang from the ceiling.");
  mkrm("Farmyard", "You stand in the middle of a dusty farmyard.  A house lies on the north edge and a small barn on the south.  A dirt path leads off to the east.");
  mkrm("Dirt Path", "You are walking down a narrow dirt path between a farmyard and the town.");
  mkrm("West Main Street", "You stand on the west end of the town's Main Street.  On the north side of the street are the stables, and on the south side is the town pub.");
  mkrm("Pub", "You stand inside a dusty pub.  The bartender stands behind the bar on the west side of the building, and several men sit at the bar and drink dark lagers and strong whiskeys.");
  mkrm("Stables", "You smell the scent of manure wafting up from the floor of the stables.  Against the walls various tack and saddles hang, and two walls are devoted to stalls for the horses.");
  mkrm("Main Street", "You stand in the center of a small town.  To the north lies a brick building, the town hall.  To the south is the road out of the town, with a castle visible in the distance.  Main Street continues off to the east and west.");
  mkrm("Town Hall", "You stand in the main chamber of the town hall.  Notices of public events are posted on the east wall, and a door in the west wall leads to the conference room.");
  mkrm("Conference Room", "You stand in the conference room of the town hall.  A large, long, heavy oak table dominates the room, and is surrounded by wooden chairs.  The air is dusty, and the room is dimly lit.");
  mkrm("East Main Street", "You stand in the easternmost part of the town.  Main Street stretches off to the west.  There is an apartment on the north side of the street, and a mansion to the south.");
  mkrm("Apartment Foyer", "You stand in the foyer of a small apartment house.  Stairs lead to an upper-level apartment, and a door leads from the foyer into the main-level bedroom.  A heavier wooden door with a deadbolt opens out into the street to the south.");
  mkrm("Bedroom", "The bedroom is small, and furnished with a bed, a small bookcase, and a small wardrobe.  A window opens to the west, and its view is dominated by the brick town hall.");
  mkrm("Upstairs Apartment", "The upstairs apartment is slightly more comfortable than that on the main level.  It has a separate closet, set into the west wall, and storage space in the building's attic, which can be accessed by moving a plank from the hole in the center of the ceiling, directly over the bed that dominates the room.  A window set into the north wall provides a view of the countryside, green and alive, and also lets a substantial amount of sunlight into the apartment.");
  mkrm("Mansion", "You stand in the foyer of a fairly small mansion.  There are stairs leading up into a hallway, and there is a door in the east wall leading to the dining hall.");
  mkrm("Dining Hall", "You stand in a well-lit dining hall with a large table in its center that has places for eight to sit and dine in elegance.");
  mkrm("Upstairs Hallway", "The upstairs hallway of the mansion has doors to a modest ballroom and a large bedroom.");
  mkrm("Ballroom", "The ballroom is well-lit by large windows to the south and west.  Its wooden floor is swept clean.");
  mkrm("Bedroom", "The bedroom is dimly lit through cracks in the ceiling.  A large bed dominates the room, and a faded tapestry covers the south wall.");
  mkrm("Dirt Road", "You are walking down a dirt road south of the town.  Further to the south, the road continues on, a forest lying to the west.");
  mkrm("Castle Road", "The main road continues north toward the town and south across the drawbridge into the castle, and a small path leads into the forest to the west.");
  mkrm("Forest Path", "The path is surrounded by trees and would barely be passable by a horse-drawn cart.  A cemetery lies to the south.");
  mkrm("Cemetery", "A small cemetery sits in the forest around you.  There are various crosses and tombstones in no particular pattern, many worn and leaning with age.");
  mkrm("Forest", "You are surrounded by oak trees.  A path leads to the east.");
  mkrm("Antechamber", "You stand in the castle's antechamber.  It is a small, nondescript room, with a heavy door to the great hall.");
  mkrm("Great Hall", "The castle's great hall envelops you.  Its stone walls are hung with several tapestries, and doors open to the Duke's chamber and to the throne room.  There are also steps leading down into the dungeon, on the east end of the room.");
  mkrm("Dungeon", "The dungeon is very dark and moist.  Mildew hangs in the air, and combines with the dust, making you sneeze.");
  mkrm("Duke's Chamber", "The Duke's chamber is small and contains only an elegant bed and a large wardrobe.");
  mkrm("Throne Room", "Light streams into the throne room through large holes in the roof.  The throne itself looks comfortable, and is upholstered in a deep crimson.");

  mkex("Farmyard", "House");
  mkex("Farmyard", "Barn");
  mkex("Farmyard", "Dirt Path");
  mkex("Dirt Path", "West Main Street");
  mkex("West Main Street", "Pub");
  mkex("West Main Street", "Stables");
  mkex("West Main Street", "Main Street");
  mkex("Main Street", "East Main Street");
  mkex("Main Street", "Town Hall");
  mkex("Town Hall", "Conference Room");
  mkex("East Main Street", "Apartment Foyer");
  mkex("Apartment Foyer", "Bedroom");
  mkex("Apartment Foyer", "Upstairs Apartment");
  mkex("East Main Street", "Mansion");
  mkex("Mansion", "Dining Hall");
  mkex("Mansion", "Upstairs Hallway");
  mkex("Upstairs Hallway", "Ballroom");
  mkex("Upstairs Hallway", "Bedroom");
  mkex(HM, "Main Street");
  mkex(HM, "Dirt Road");
  mkex("Dirt Road", "Castle Road");
  mkex("Castle Road", "Forest Path");
  mkex("Forest Path", "Cemetery");
  mkex("Forest Path", "Forest");
  mkex("Castle Road", "Antechamber");
  mkex("Antechamber", "Great Hall");
  mkex("Great Hall", "Dungeon");
  mkex("Great Hall", "Duke's Chamber");
  mkex("Great Hall", "Throne Room");

  mkpl("God", "Ari", 1);
}

struct cmd {
  char *n;
  void (*f)(Player *, char *);
};

cmd ctab[] = {
  {"who", who},
  {"look", look},
  {"go", go},
  {"say", say},
  {"pose", pose},
  {"hit", hit},
  {"stat", stat},
  {"wiz", wiz},
  {"dewiz", dewiz},
  {"ann", ann},
  {"heal", heal},
  {"kill", kill},
  {"dig", dig},
  {"desc", desc},
  {"nuke", nuke},
  {"tel", tel},
  {0, 0}
};

initcmd() {
  int i = 0;
  for(; ctab[i].n; i++)
    cmds[ctab[i].n] = ctab[i].f;
}

main(int ac, char *av[]) {
  if(ac > 1)
    sock = mksock(atoi(av[1]));
  else
    sock = mksock(4201);
  maxd = sock + 1;
  initobjs();
  initcmd();
  while(1) {
    netloop();
    hploop();
  }
}

