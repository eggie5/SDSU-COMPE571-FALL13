#include <Timer.h>
#include <Serial.h>
#include "BlinkToRadio.h"

configuration BlinkToRadioAppC {

}

implementation {
   components MainC;
   components LedsC;
   components SerialP;
   components PlatformSerialC;
   components BlinkToRadioC as App;
   components new TimerMilliC() as Timer0;
   components ActiveMessageC;
   components new AMSenderC(AM_BLINKTORADIO);
   components new AMReceiverC(AM_BLINKTORADIO);
   components new SerialAMSenderC(AM_BLINKTORADIOMSG);
   components SerialActiveMessageC;
   
   App.Boot ->MainC;
   App.Leds ->LedsC;
   App.Timer0 -> Timer0;
   App.RadioPacket ->AMSenderC;
   App.AMPacket -> AMSenderC;
   App.AMRadioSend -> AMSenderC;
   App.AMRadioControl -> ActiveMessageC;
   App.RadioReceive -> AMReceiverC;
   App.AMSerialSend -> SerialAMSenderC;
   App.AMSerialControl -> SerialActiveMessageC;
   App.SerialPacket -> SerialAMSenderC;
   App.SendBytePacket -> SerialP;	
   App.UartStream -> PlatformSerialC;
   App.UartStream -> PlatformSerialC;
}
