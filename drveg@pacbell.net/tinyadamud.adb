--------------------------------------------------------------------------------
-- Copyright(C) 2000 David Kristola
-- released under the The Ada Community License
-- http://www.pogner.demon.co.uk/components/bc/ACL
--
-- Tinyadamud is my entry into Erwin S. Andreasen's 16K MUD competition
-- http://www.andreasen.org/16k.shtml
--
-- Ada (ISO/IEC 8652:1995) was desinged to be readable, and therefore is at a
-- disadvantage when it comes to the smallest byte count of the source code.
-- I have used the "use" clause and many shortened type and variable names.
-- At the end, even with minimal functionality, the code was still too large.
-- I considered replacing "_Type" with something shorter, but instead commented
-- out the ability to save characters (avatars).  With no descriptions
-- associated with avatars, there was little point to saving them anyway.
--
-- Ada does not have any built-in socket support, so this program was built
-- against AdaSockets (http://www-inf.enst.fr/ANC/).  AdaSockets version 0.1.3
-- did not have non-blocking socket operations, so i extended it in the
-- Sockets.MUD package.  Since this is not part of the library distribution,
-- it is included in this release.
--
-- While the standard Ada library is extensive, it does not contain anything
-- similar to the Standard Template Library.  The Booch Components are
-- available from
-- http://www.pogner.demon.co.uk/components/bc/
-- However, use of this library (and many others) is not allowed under the
-- terms of the 16K MUD competition.
--
-- This program was developed using GNAT 3.11 (a free Ada compiler built on
-- gcc, and provided by Ada Core Technologies) under MachTen 4.1.1 (a UNIX
-- environment that runs as an application on Macs from Tenon Intersystems)
-- running on a Power Macintosh 8500 running MacOS 8.5.1.
-- Ada Core Technologies: http://www.gnat.com/
-- Tenon Intersystems: http://www.tenon.com/
--
-- In this distrabution, you should find the following source code files:
-- tinyadamud.adb (the main program)
-- signals.ads (package specification for handling the broken pipe signal)
-- signals.adb (package body)
-- sockets-mud.ads (package spec for Sockets.MUD)
-- sockets-mud.adb (package body)
--
-- Note: signals.adb (package body of Signals) contains GNAT specific code
-- for connecting an interrupt handler to the SIGPIP UNIX signal.  Other
-- implementations of Ada may use a different mechanism.  System.OS_Interface
-- is implementation defined.
--
--------------------------------------------------------------------------------

with Signals;
with Sockets; use Sockets;
with Sockets.MUD; use Sockets.MUD;
with Ada.Strings.Unbounded; use Ada.Strings.Unbounded;
with Ada.Characters.Latin_1;
with Text_IO; use Text_IO;
with Unchecked_Deallocation;

procedure Tinyadamud is

   package L renames Ada.Characters.Latin_1;

   Terminate_Program : Boolean := False;

   subtype U_Type is Unbounded_String;

   -- Conversion operators to go back and forth between strings and unbounded strings.
   function "+"(S : String) return U_Type renames To_Unbounded_String;
   function "+"(U : U_Type) return String renames To_String;

   -- A few string and character constants to make life a little easier.
   Bold : constant String := L.ESC & "[1m";
   Unbold : constant String := L.ESC & "[0m";
   Q : constant Character := '"';


   -- This type provides descriptions of variable length by using a linked
   -- list of unbounded strings.
   --
   type Description_Type;
   type DP_Type is access Description_Type;
   type Description_Type is
      record
         Line : U_Type;
         Next : DP_Type;
      end record;

   -- Avatar is used instead of Character so that it can be clearly
   -- shortened to "A".  This type provides a linked list of avatars.
   --
   type Avatar_Type;
   type AP_Type is access Avatar_Type; -- Avatar_Pointer_Type
   type Avatar_Type is
      record
         Name     : U_Type;
         Playing  : Boolean := False;
         Password : U_Type;
         Next     : AP_Type;
      end record;

   -- A linked list of all Avatars (characters).
   Avatar_List : AP_Type;

   --Write_Avatars : Boolean := False; -- Flag to indicate that the avatar database needs to be updated.


   -- This type is used to track connections.  Connection variables are
   -- often shortened to "C".  To avoid the need for robust iterators,
   -- the boolean flag G (Good) is used to indicate good/bad connections.
   -- The game state associated with this particular connection/player
   -- is encoded by a pointer to a parser routine.
   --
   type Connection_Type;
   type CP_Type is access Connection_Type; -- Connection_Pointer_Type

   -- A pointer type for the game parser associated with a connection.
   -- Parser procedures work on a connection (C) and parse the input
   -- string (S).
   --
   type Parser_Pointer_Type is access procedure(C : in out CP_Type; S : in String);

   type Room_Type;
   type RP_Type is access Room_Type; -- Room_Pointer_Type

   type Connection_Type is
      record
         G      : Boolean;
         Socket : Link_Type;
         Parser : Parser_Pointer_Type;
         Avatar : AP_Type;
         R      : RP_Type;
         Next   : CP_Type;
      end record;

   -- Connection_List
   C_List : CP_Type;

   -- The port number should come from a configuration file or command line,
   -- but that would take extra code.
   --
   Port_Number : constant := 6565;
   Port : Connector_Type;


   -- Direction type
   type D_Type is (V, N, S, E, W, U, D); -- V for void, a bad direction.
   subtype VD_Type is D_Type range N..D; -- Valid_Direction_Type
   type Direction_Array_Type is array (VD_Type) of RP_Type;

   type Room_Type is
      record
         Name  : U_Type;
         Desc  : DP_Type;
         Exits : Direction_Array_Type;
         Next  : RP_Type;
      end record;

   Write_Rooms : Boolean := False; -- Flag to indicate that the room database needs to be updated.

   Opposite : constant array (VD_Type) of VD_Type :=
      (S, N, W, E, D, U);


   -- Commands understood by the main game parser.  SHUTDOWN
   -- is not listed in the on-line help because it causes the
   -- program to close all connections and terminate.
   --
   type Command_Type is
      (SAY,
       HELP,
       QUIT,
       LOOK,
       ROOMS,
       MAKE_ROOM,
       MAKE_DOOR,
       TELEPORT,
       N, S, E, W, U, D, -- move in various directions
       --PASSWORD,
       SHUTDOWN);

   -- The first room in this list is special, it is the room into which
   -- characters fist enter the game.
   --
   Room_List : RP_Type;

   Name_Prompt : constant String := "Please enter your character name:";

   -- The two database file names.  The extension "tam" is for tiny ada mud.
   -- RDB is where rooms are stored, ADB is where avatars are stored.
   --
   RDB : constant String := "room.tam";
   ADB : constant String := "avatar.tam";

   procedure Free is new Unchecked_Deallocation(Connection_Type, CP_Type);

   -----------------------------------------------------------------------------
   -- Return the first substring of S (deliniated by a space).
   --
   function First(S : String) return String is
   begin
      for I in S'RANGE loop
         if S(I) = ' ' then
            return S(S'FIRST..I-1);
         end if;
      end loop;
      return S; -- no blanks, return the whole string.
   end First;

   -----------------------------------------------------------------------------
   -- Return the second part of S.
   --
   function Rest(S : String) return String is
      B : Boolean := False; -- found a blank
   begin
      for I in S'RANGE loop
         if S(I) = ' ' then
            B := True;
         elsif B then
            return S(I..S'LAST);
         end if;
      end loop;
      return ""; -- there was no second part to S
   end Rest;

   -----------------------------------------------------------------------------
   -- This translates the string S into a command.  (For those non-Ada people
   -- looking at this code, 'IMAGE returns a string representing the member of
   -- the enumerated type, in upper case).
   --
   function Command(S : String) return Command_Type is
   begin
      for C in Command_Type loop -- Loop over all of the commands
         if S = Command_Type'IMAGE(C) then -- See if S is this command
            return C;
         end if;
      end loop;
      return SAY; -- If S is none of the commands, return SAY
   end Command;

   -----------------------------------------------------------------------------
   -- This routine returns the name of the avatar assoiciated with a connection,
   -- with the Bold and Unbold escape sequences around it.
   --
   function Name(C : CP_Type) return String is
   begin
      return Bold&(+C.Avatar.Name)&Unbold;
   end Name;

   -----------------------------------------------------------------------------
   -- Send S to the player connected via C, if the connection is good.
   --
   procedure Say(C : CP_Type; S : String) is
   begin
      if C.G then
         Put_Line(C.Socket, S);
      end if;
   end Say;

   -----------------------------------------------------------------------------
   -- Send S to every connected player (with a good connection).
   --
   procedure Broadcast(S : String) is
      C : CP_Type := C_List;
   begin
      while C /= null loop
         Say(C, S);
         C := C.Next;
      end loop;
   end Broadcast;

   -----------------------------------------------------------------------------
   -- Send S to every player with an avatar in room R.
   --
   procedure Say(R : RP_Type; S : String) is
      C : CP_Type := C_List;
   begin
      while C /= null loop
         if C.R = R then
            Say(C, S);
         end if;
         C := C.Next;
      end loop;
   end Say;

   -----------------------------------------------------------------------------
   -- Send room information to C.
   --
   procedure Look(C : CP_Type) is
      Cur : CP_Type := C_List;
      D : DP_Type := C.R.Desc;
   begin
      Say(C, "Room: "&(+C.R.Name));
      while D /= null loop
         Say(C, +D.Line);
         D := D.Next;
      end loop;
      Say(C, "");
      Say(C, "Exits:");
      for D in VD_Type loop
         if C.R.Exits(D) /= null then
            Say(C, " "&VD_Type'IMAGE(D)&" "&(+C.R.Exits(D).Name));
         end if;
      end loop;
      Say(C, "Avatars:");
      while Cur /= null loop
         if Cur.G and Cur.R = C.R then
            Say(C, Name(Cur));
         end if;
         Cur := Cur.Next;
      end loop;
      New_Line(C.Socket);
   end Look;

   -----------------------------------------------------------------------------
   -- Move the avatar connected to C from Old_R to New_R.  D specifies the
   -- direction the avatar moves.  D is of D_Type, not the valid direction
   -- type, so it includes V (void).  If D is set to V, the avatar is
   -- teleported from one location to the other.
   --
   procedure Move
      (C : in out CP_Type;
       D : D_Type;
       Old_R, New_R : RP_Type) is
   begin
      C.R := null; -- by setting this to null, C will not be sent the exit notice.
      if D = V then
         if Old_R /= null then
            Say(Old_R, Name(C)&" has vanished from the room.");
         end if;
         if New_R /= null then
            Say(C, "You arrive in "&(+New_R.Name));
            Say(New_R, Name(C)&" has appeared in the room.");
            C.R := New_R;
            Look(C);
         end if;
      else
         if Old_R /= null then
            Say(Old_R, Name(C)&" has exited "&D_Type'IMAGE(D));
         end if;
         if New_R /= null then
            Say(C, "You enter "&(+New_R.Name));
            Say(New_R, Name(C)&" has entered from "&D_Type'IMAGE(Opposite(D)));
            C.R := New_R;
            Look(C);
         end if;
      end if;
   end Move;

   -----------------------------------------------------------------------------
   procedure Move
      (C : in out CP_Type;
       D : D_Type) is
   begin
      if C.R.Exits(D) = null then
         Say(C, "Sorry, you can't go that way.");
      else
         Move(C, D, C.R, C.R.Exits(D));
      end if;
   end Move;

   -----------------------------------------------------------------------------
   function Find_Room(S : String) return RP_Type is
      R : RP_Type := Room_List;
   begin
      while R /= null and then S /= R.Name loop
         R := R.Next;
      end loop;
      return R;
   end Find_Room;

   -----------------------------------------------------------------------------
   function Find_Avatar(S : String) return AP_Type is
      A : AP_Type := Avatar_List;
   begin
      while A /= null and then S /= A.Name loop
         A := A.Next;
      end loop;
      return A;
   end Find_Avatar;

   -----------------------------------------------------------------------------
   procedure Make_A_Door(C : in out CP_Type; S : in String) is
      D : VD_Type;
      R : RP_Type;
   begin
      D := VD_Type'VALUE(First(S)); -- Could raise an exception.
      R := Find_Room(Rest(S));
      if R = null then
         Say(C, Q&Rest(S)&Q&" is not a valid room.");
      elsif C.R.Exits(D) /= null then
         Say(C, "This room already has a door there.");
      elsif R.Exits(Opposite(D)) /= null then
         Say(C, "That room already has a door there.");
      else
         Say(C, "You have constructed a door going "&
            VD_Type'IMAGE(D)&" from here to "&Q&(+R.Name)&Q);
         C.R.Exits(D) := R; -- Link this room to that room.
         R.Exits(Opposite(D)) := C.R; -- And link the other one back.
         Write_Rooms := True;
      end if;
   exception
      when others =>
         Say(C, Q&First(S)&Q&" is not a valid direction.");
   end Make_A_Door;

   -----------------------------------------------------------------------------
   -- This routine does not remove the connection record from the connection
   -- list, it flags the connection as being bad (C.G = False).  This routine
   -- may be called while some other routine is iterating through a list of
   -- connections.  Since we do not have robust iterators, we use this flag
   -- and scrub the list of bad connections when no other routines are using
   -- the list.
   --
   procedure Close_Connection(C : in out CP_Type) is
   begin
      C.G := False;
      if C.Avatar /= null then
         C.Avatar.Playing := False;
         Move(C, V, C.R, null);
      end if;
      Destroy(C.Socket);
   end Close_Connection;

   -----------------------------------------------------------------------------
   -- Turn S into an unbounded string and append it to the end of D.
   --
   procedure Append(D : in out DP_Type; S : String) is
      L : DP_Type;
   begin
      if D = null then -- S is being appened to an empty description list.
         D := new Description_Type'(+S, null);
      else
         L := D;
         while L.Next /= null loop
            L := L.Next;
         end loop;
         L.Next := new Description_Type'(+S, null);
      end if;
   end Append;

   -- Create a prototype for this routine.  It will be defined below, but it
   -- needs to see Game_Parser, and Game_Parser needs to see this one.
   --
   procedure Desc_Parser(C : in out CP_Type; S : in String);

   -----------------------------------------------------------------------------
   -- The main game parser.
   --
   procedure Game_Parser(C : in out CP_Type; S : in String) is
      R : RP_Type;
   begin
      case Command(First(S)) is
         when SAY =>
            Say(C.R, Name(C)&" says: "&Q&S&Q);

         when HELP =>
            Say(C, "QUIT to exit game.");
            Say(C, "ROOMS lists game rooms, TELEPORT <room name> will send you there.");
            Say(C, "LOOK displays a room, it's exits, and the avatars in it.");
            Say(C, "N, S, E, W, U, and D are direction and motion commands");
            --Say(C, "PASSWORD <password> sets an avatar password, and causes that character to be saved.");
            Say(C, "MAKE_ROOM <room name> creates a new room.");
            Say(C, "MAKE_DOOR <direction> <room name> links two rooms.");
            --Say(C, "");

         when QUIT =>
            Say(C, "Goodbye!");
            Close_Connection(C);

         when ROOMS =>
            R := Room_List;
            Say(C, "Room list:");
            while R /= null loop
               Say(C, +R.Name);
               R := R.Next;
            end loop;

         when LOOK =>
            Look(C);

         when MAKE_ROOM =>
            -- if the room name is not "" and room does not already exist...
            if Rest(S) /= "" and then Find_Room(Rest(S)) = null then
               R := new Room_Type;
               R.Name := +Rest(S);
               R.Next := Room_List.Next; -- link the new room in AFTER the first one.
               Room_List.Next := R;
               Say(C, "You have just created room '"&Rest(S)&"'.");
               Move(C, V, C.R, R); -- teleport to new room
               C.Parser := Desc_Parser'ACCESS;
               Say(C, "Enter a room description, enter '.' on it's own line to end.");
            end if;

         when MAKE_DOOR =>
            Make_A_Door(C, Rest(S));

         when TELEPORT =>
            if Find_Room(Rest(S)) /= null then
               Move(C, V, C.R, Find_Room(Rest(S)));
            else
               Put_Line("Sorry, no such place as '"&Rest(S)&"'.");
            end if;

         when N..D => -- any direction
            Move(C, VD_Type'VALUE(S));

         --when PASSWORD =>
         --   if Rest(S) = "" then
         --      Say(C, "Sorry, passwords can't be blank.");
         --   else
         --      C.Avatar.Password := +Rest(S);
         --      Say(C, "Your password has been set.");
         --      Write_Avatars := True;
         --   end if;

         when SHUTDOWN =>
            Terminate_Program := True;
            Broadcast("Server going down now.");

      end case;
   exception
      when others =>
         Say(C, "Input error, try again.");
   end Game_Parser;

   -- Create a prototype for this routine.  It will be defined below, but it
   -- needs to see Get_Password, and Get_Password needs to see this one.
   --
   --procedure Get_Name(C : in out CP_Type; S : in String);

   -----------------------------------------------------------------------------
   --procedure Get_Password(C : in out CP_Type; S : in String) is
   --begin
   --   if S = C.Avatar.Password then
   --      C.Avatar.Playing := True;
   --      C.Parser := Game_Parser'ACCESS;
   --      Move(C, V, null, Room_List);
   --   else
   --      Say(C, "Incorrect password.  Please start over.");
   --      Say(C, Name_Prompt);
   --      C.Parser := Get_Name'ACCESS;
   --   end if;
   --end Get_Password;

   -----------------------------------------------------------------------------
   procedure Get_Name(C : in out CP_Type; S : in String) is
   begin
      if S = "" then
         Say(C, Name_Prompt);
      else
         C.Avatar := Find_Avatar(S);
         if C.Avatar = null then -- Name S does not exist yet, create a new avatar.
            C.Avatar := new Avatar_Type;
            C.Avatar.Next := Avatar_List;
            Avatar_List := C.Avatar;
            C.Avatar.Name := +S;
            C.Parser := Game_Parser'ACCESS;
            C.Avatar.Playing := True;
            Move(C, V, null, Room_List);
         elsif C.Avatar.Playing then -- Oooops, that avatar is currently playing.
            Say(C, Name(C)&" is already playing the game.");
            C.Avatar := null;
            Say(C, Name_Prompt);
         --elsif C.Avatar.Password /= "" then -- That avatar has a password, check it.
         --   C.Parser := Get_Password'ACCESS;
         --   Say(C, "Enter password:");
         else -- Name is in list, but Avatar does not have a password.
            C.Parser := Game_Parser'ACCESS;
            C.Avatar.Playing := True;
            Move(C, V, null, Room_List);
         end if;
      end if;
   end Get_Name;

   -----------------------------------------------------------------------------
   procedure Desc_Parser(C : in out CP_Type; S : in String) is
   begin
      if S = "." then -- we are done.
         C.Parser := Game_Parser'ACCESS;
         Say(C, "Description completed.");
         Write_Rooms := True;
      else
         Append(C.R.Desc, S);
      end if;
   end Desc_Parser;

   -----------------------------------------------------------------------------
   -- By past experience, i know that some telnet applications like to send
   -- various protocols to the server.  Responding to them never seemed to
   -- be productive.  This routine will strip out telnet chatter and other
   -- unwanted characters.
   --
   function Filter(S : String) return String is
      N : String(1..S'LENGTH);
      J : Positive := N'FIRST;
   begin
      for I in S'RANGE loop
         -- If character I is a nice, safe character...
         if S(I) in ' ' .. '~' then
            N(J) := S(I);
            J := J + 1;
         end if;
      end loop;
      return N(1..J-1);
   end Filter;


   -----------------------------------------------------------------------------
   -- Loop through the connection list and process all inputs.
   --
   procedure Check_For_Inputs is

      Closed, Cr, Lf : Boolean;
      Count  : Integer;

      C : CP_Type := C_List;

   begin
      while C /= null loop
         if C.G then -- Only check good connections
            begin
               Status(C.Socket, Closed, Cr, Lf, Count);
               if Closed then
                  Close_Connection(C);
               elsif Cr and Lf then
                  -- We want both a carriage return and a line feed
                  -- before processing an input line.  Note that
                  -- there may be some delay between the bulk of the
                  -- line and the CR/LF characters.  This has caused
                  -- problems in the past.  Since non-blocking socket
                  -- flow control is being used, Get_Line does the
                  -- wrong thing if it can't find an LF in the input
                  -- stream.
                  --
                  C.Parser.all(C, Filter(Get_Line(C.Socket)));
               end if;
            exception
               when others => null;
            end;
         end if;
         C := C.Next;
      end loop;
   end Check_For_Inputs;


   -----------------------------------------------------------------------------
   -- This is a two pass process.  The first pass reads in all of the room
   -- information for each room, but does not link the exits.  The second
   -- pass resolves these links.
   --
   procedure Read_Room_Database is
      File : File_Type;
      R : RP_Type;
      B : String(1..256); -- an input buffer
      L : Positive; -- the lenght of data read into the buffer
   begin
      Open(File, In_File, RDB);
      while not End_Of_File(File) loop
         if R = null then -- reading first room
            R := new Room_Type;
            Room_List := R;
         else
            R.Next := new Room_Type;
            R := R.Next;
         end if;
         Get_Line(File, B, L);
         R.Name := +B(1..L);
         Get_Line(File, B, L);
         while B(1..L) /= "." loop
            Append(R.Desc, B(1..L));
            Get_Line(File, B, L);
         end loop;
         for D in VD_Type loop
            Skip_Line(File);
         end loop;
      end loop;
      Reset(File);
      R := Room_List;
      while R /= null loop
         Get_Line(File, B, L);
         while B(1..L) /= "." loop
            Get_Line(File, B, L);
         end loop;
         for D in VD_Type loop
            Get_Line(File, B, L);
            R.Exits(D) := Find_Room(B(1..L));
         end loop;
         R := R.Next;
      end loop;
      Close(File);
      Write_Rooms := False;
   exception
      when others => -- There is no room database.
         if Is_Open(File) then
            Put_Line("Error reading "&RDB&" at line "&Positive_Count'IMAGE(Line(File)));
            Close(File);
         end if;
   end Read_Room_Database;


   -----------------------------------------------------------------------------
   procedure Write_Room_Database is
      File : File_Type;
      R : RP_Type := Room_List;
      D : DP_Type;
   begin
      -- If the database file does not exist, Open causes an exception.  In
      -- that case, use Create to create a brand new file.
      --
      begin
         Open(File, Out_File, RDB);
      exception
         when others =>
            Create(File, Out_File, RDB);
      end;

      while R /= null loop
         Put_Line(File, +R.Name);
         D := R.Desc;
         while D /= null loop
            Put_Line(File, +D.Line);
            D := D.Next;
         end loop;
         Put_Line(File, ".");
         for D in VD_Type loop
            if R.Exits(D) = null then
               Put_Line(File, "null");
            else
               Put_Line(File, +R.Exits(D).Name);
            end if;
         end loop;
         R := R.Next;
      end loop;
      Close(File);
      Write_Rooms := False;
   end Write_Room_Database;

   -----------------------------------------------------------------------------
   --procedure Read_Avatar_Database is
   --   File : File_Type;
   --   A : AP_Type;   
   --   B : String(1..256); -- an input buffer
   --   L : Positive; -- the lenght of data read into the buffer
   --begin
   --   Open(File, In_File, ADB);
   --   while not End_Of_File(File) loop
   --      if A = null then -- reading first avatar
   --         A := new Avatar_Type;
   --         Avatar_List := A;
   --      else
   --         A.Next := new Avatar_Type;
   --         A := A.Next;
   --      end if;
   --      Get_Line(File, B, L);
   --      A.Name := +B(1..L);
   --      Get_Line(File, B, L);
   --      A.Password := +B(1..L);
   --      A.Playing := False;
   --   end loop;
   --   Close(File);
   --   Write_Avatars := False;
   --exception
   --   when others => -- There is no room database.
   --      if Is_Open(File) then
   --         Put_Line("Error reading "&ADB&" at line "&Positive_Count'IMAGE(Line(File)));
   --         Close(File);
   --      end if;
   --end Read_Avatar_Database;

   -----------------------------------------------------------------------------
   --procedure Write_Avatar_Database is
   --   File : File_Type;
   --   A : AP_Type := Avatar_List;
   --begin
   --   -- If the database file does not exist, Open causes an exception.  In
   --   -- that case, use Create to create a brand new file.
   --   --
   --   begin
   --      Open(File, Out_File, ADB);
   --   exception
   --      when others =>
   --         Create(File, Out_File, ADB);
   --   end;
   --   while A /= null loop
   --      Put_Line(File, +A.Name);
   --      Put_Line(File, +A.Password);
   --      A := A.Next;
   --   end loop;
   --   Close(File);
   --   Write_Avatars := False;
   --end Write_Avatar_Database;


   -----------------------------------------------------------------------------
   -- In order to avoid difficult problems with robust iterators, dropped
   -- connections are flagged, and scrubbed from the list after the game
   -- loop processing.
   --
   procedure Scrub_C_List is
      C, T : CP_Type;
   begin
      while C_List /= null and then not C_List.G loop
         T := C_List;
         C_List := C_List.Next;
         Free(T);
      end loop;
      if C_List /= null then
         C := C_List;
         while C.Next /= null loop
            if C.Next.G then
               C := C.Next;
            else
               T := C.Next;
               C.Next := C.Next.Next;
               Free(T);
            end if;
         end loop;
      end if;
   end Scrub_C_List;

   -----------------------------------------------------------------------------
   procedure Establish_Connection is
      C : CP_Type;
   begin
      C := new Connection_Type;
      Get_Link(Port, C.Socket);
      C.G := True;

      -- Default to the new name parser.
      C.Parser := Get_Name'ACCESS;
      C.Next := C_List;
      C_List := C;

      Say(C, "Welcome to Tinyadamud!");
      Say(C, "Commands are all upper case, like HELP.");
      Say(C, "");
      Say(C, Name_Prompt);
   end Establish_Connection;

begin
   -- If a connection is broken and not detected, a Broken Pipe signal
   -- (interrupt) will be generated if data is sent to that connection.
   --
   Signals.Attach;

   --Read_Avatar_Database;
   Read_Room_Database;

   -- There may not actually be a room database, or there may have been
   -- an error.  Check to see if there are any rooms in the list, and
   -- if not, create one so that there is always a room in the game.
   --
   if Room_List = null then
      Room_List := new Room_Type'(+"Green Room", null, (others => null), null);
   end if;

   Create(Port, Port_Number);
   while not Terminate_Program loop
      if Link_Pending(Port) then
         Establish_Connection;
      end if;
      Check_For_Inputs;
      Scrub_C_List;
      if Write_Rooms then Write_Room_Database; end if;
      --if Write_Avatars then Write_Avatar_Database; end if;
      delay 0.1;
   end loop;

   Destroy(Port);
end Tinyadamud;
