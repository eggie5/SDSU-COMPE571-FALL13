#include <Timer.h>
#include <Serial.h>
#include "BlinkToRadio.h"
#include "HomeWork3.h"

configuration HomeWork3AppC {

}

implementation {
   components MainC;
   components LedsC;
   components HomeWork3C as App;
   components new TimerMilliC() as Timer0;  // We only one timer since a node is either root 
											// or sensor node. In either case there is only one timer
   
   // For Radio Communication 
   components ActiveMessageC;
   components new AMSenderC(AM_RADIOMESSAGE);
   components new AMReceiverC(AM_RADIOMESSAGE);
   
   // For Serial Communication 
   components new SerialAMSenderC(AM_RADIOMESSAGE);
   components SerialActiveMessageC;
   components new SerialAMReceiverC(AM_RADIOMESSAGE);
   
   
   
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
   App.AMSerialReceive -> SerialAMReceiverC;

}
