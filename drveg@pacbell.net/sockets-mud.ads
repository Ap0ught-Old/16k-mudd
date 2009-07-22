--------------------------------------------------------------------------------
-- Copyright(C) 2000 David Kristola
-- This file is an extension of the AdaSockets library, available from
-- http://www-inf.enst.fr/ANC/
-- I release it under the Ada Community License
-- http://www.pogner.demon.co.uk/components/bc/ACL
--------------------------------------------------------------------------------

with Sockets.Thin;

package Sockets.MUD is

   type Connector_Type is limited private;

   type Link_Type is new Socket_FD with private;


   procedure Create
      (Connector :    out Connector_Type;
       Port      : in     Positive);

   procedure Destroy
      (Connector : in out Connector_Type);

   function Link_Pending
      (Connector : in     Connector_Type) return Boolean;

   procedure Get_Link
      (Connector : in out Connector_Type;
       Link      :    out Link_Type);

   procedure Destroy
      (Link : in out Link_Type);

   procedure Status
      (Link   : in     Link_Type;
       Closed :    out Boolean;
       Has_CR :    out Boolean;
       Has_LF :    out Boolean;
       Count  :    out Natural);

private

   type Handle_Type (Ptr : access Connector_Type) is limited null record;

   type Connector_Type is limited
      record
         Handle : Handle_Type(Connector_Type'ACCESS);
         Socket : Socket_FD;
         Link   : Socket_FD;
         Pend   : Boolean := False;
         Addr   : Sockets.Thin.In_Addr;
      end record;

   type Link_Type is new Socket_FD with
      record
         Addr : Sockets.Thin.In_Addr;
      end record;

end Sockets.MUD;
