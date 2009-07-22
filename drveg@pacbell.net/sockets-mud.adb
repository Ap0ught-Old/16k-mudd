--------------------------------------------------------------------------------
-- Copyright(C) 2000 David Kristola
-- This file is an extension of the AdaSockets library, available from
-- http://www-inf.enst.fr/ANC/
-- I release it under the Ada Community License
-- http://www.pogner.demon.co.uk/components/bc/ACL
--------------------------------------------------------------------------------

with Ada.Characters.Latin_1;

with Sockets.Thin;
with Sockets.Naming;
with Sockets.Constants;

package body Sockets.MUD is

   use Ada.Characters.Latin_1, Interfaces.C;
   use Sockets, Sockets.Thin, Sockets.Constants;

   -----------------------------------------------------------------------------
   procedure Set_Nonblocking(Socket : Socket_FD'Class) is

      O_NONBLOCK : constant := 4; -- not in Sockets.Constants

      Res : Interfaces.C.int;

   begin
      Res := C_Fcntl(Socket.FD, F_SETFL, O_NONBLOCK);
   end Set_Nonblocking;


   -----------------------------------------------------------------------------
   procedure Create
      (Connector :    out Connector_Type;
       Port      : in     Positive) is

   begin
      Socket(Connector.Socket, AF_INET, SOCK_STREAM);
      Setsockopt(Connector.Socket, SOL_SOCKET, SO_REUSEADDR, 1);
      Bind(Connector.Socket, Port);
      Listen(Connector.Socket, 5);
      Set_Nonblocking(Connector.Socket);
   end Create;


   -----------------------------------------------------------------------------
   procedure Destroy
      (Connector : in out Connector_Type) is

   begin
      Shutdown(Connector.Socket);
   end Destroy;


   -----------------------------------------------------------------------------
   procedure Accept_Socket
      (Socket     : in     Socket_FD;
       New_Socket :    out Socket_FD;
       IP_Address :    out Sockets.Thin.In_Addr;
       Successful :    out Boolean) is

      Sin  : aliased Sockaddr_In;
      Size : aliased int := Sin'SIZE / 8;
      Code : int;

   begin
      Code := C_Accept(Socket.FD, Sin'ADDRESS, Size'ACCESS);
      if Code = Failure then
         IP_Address := (others => 0);
         Successful := False;
      else
         New_Socket := (FD => Code);
         IP_Address := Sin.Sin_Addr;
         Successful := True;
      end if;
   end Accept_Socket;


   -----------------------------------------------------------------------------
   function Link_Pending
      (Connector : in     Connector_Type) return Boolean is

      Handle : Handle_Type renames Connector.Handle;

   begin
      if Connector.Pend then
         return True; -- didn't accept it yet
      else
         Accept_Socket(Handle.Ptr.Socket, Handle.Ptr.Link, Handle.Ptr.Addr, Handle.Ptr.Pend);
         return Handle.Ptr.Pend;
      end if;
   end Link_Pending;


   -----------------------------------------------------------------------------
   procedure Get_Link
      (Connector : in out Connector_Type;
       Link      :    out Link_Type) is

   begin
      Link.FD := Connector.Link.FD;
      Link.Addr := Connector.Addr;
      Set_Nonblocking(Link);
      Connector.Pend := False;
   end Get_Link;


   -----------------------------------------------------------------------------
   procedure Destroy
      (Link : in out Link_Type) is

   begin
      Shutdown(Sockets.Socket_FD(Link));
   end Destroy;


   -----------------------------------------------------------------------------
   procedure Status
     (Link   : in     Link_Type;
      Closed :    out Boolean;
      Has_Cr :    out Boolean;
      Has_Lf :    out Boolean;
      Count  :    out Natural) is

      Msg_Peek  : constant int := 2;

      Buffer    : String(1 .. 1024);
      Addr      : aliased In_Addr;
      Addrlen   : aliased int := Addr'SIZE / 8;
      Msg_Count : int;

   begin
      Msg_Count := C_Recvfrom (Link.FD, Buffer'ADDRESS, Buffer'LENGTH, Msg_Peek,
                    Addr'ADDRESS, Addrlen'ACCESS);
      Closed := (Msg_Count = 0);
      if Msg_Count < 0 then
         Count := 0;
      else
         Count := Integer(Msg_Count);
      end if;

      Has_Cr := False;
      Has_Lf := False;

      if Msg_Count > 0 then
         for I in Buffer'FIRST .. Positive(Msg_Count) loop
            if Buffer(I) = LF then
               Has_Lf := True;
            elsif Buffer(I) = CR then
               Has_Cr := True;
            end if;
         end loop;
      end if;
   end Status;


end Sockets.MUD;
