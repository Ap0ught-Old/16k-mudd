--------------------------------------------------------------------------------
-- Copyright(C) 2000 David Kristola
-- I release this trivial file to the Public Domain
--------------------------------------------------------------------------------

with System.OS_Interface;
with System.Interrupts;

package body Signals is

   protected Handler is
      procedure Broken_Pipe;
      pragma Interrupt_Handler(Broken_Pipe);
   end Handler;

   protected body Handler is
      procedure Broken_Pipe is
      begin
         null; -- ignore it.
      end Broken_Pipe;
   end Handler;

   procedure Attach is
   begin
      System.Interrupts.Attach_Handler
        (New_Handler => Handler.Broken_Pipe'ACCESS,
         Interrupt   => System.OS_Interface.SIGPIPE);
   end Attach;

end Signals;
