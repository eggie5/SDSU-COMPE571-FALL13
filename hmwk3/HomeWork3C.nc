#include <Timer.h>
#include "BlinkToRadio.h"

module HomeWork3C {
   uses interface Boot;
   uses interface Leds;
   uses interface Timer<TMilli> as Timer0; 

   uses interface AMPacket;
   uses interface Packet  as RadioPacket;
   uses interface AMSend as AMRadioSend;
   uses interface Receive as RadioReceive;
   uses interface SplitControl as AMRadioControl;
   
   
   uses interface SplitControl as AMSerialControl;
   uses interface Receive as AMSerialReceive;
   uses interface AMSend  as AMSerialSend;
   uses interface Packet  as SerialPacket;
  
 
}

implementation {
   uint16_t   datacounter = 0;
   uint16_t	  invcounter = 0;
   bool  radiobusy = FALSE;
   bool  serialbusy = FALSE;
   message_t   radiopkt;
   message_t   serialpkt;
   bool  isRoot  = TRUE ;	// Set this to FALSE if this will be an end node rather than a root node
							//
   bool  isRegistered = FALSE;
   

   event  void Boot.booted()
      {
       call AMSerialControl.start();
       }
  

   
   event  void AMSerialControl.startDone(error_t err) {
       if (err == SUCCESS)  {
             call AMRadioControl.start();   // Start the Radio control after serial control is completed
											//
       }
       else {
             call AMSerialControl.start();
            }
    }


 event void AMSerialControl.stopDone(error_t err) {
    call AMSerialControl.start();
  }



   event  void AMRadioControl.startDone(error_t err) {
       if (err == SUCCESS)  {
       // Timer should not start before Radio is initialized and ready
       if (isRoot) {// At this point we should start the timer only if this is a root node. 
					// The Sensor Data Timer for the end node should be started after it completes registration
					call Timer0.startPeriodic(TIMER_PERIOD_INVITATION);
					}
       }
       else {
             call AMRadioControl.start();
            }
    }


    event  void AMRadioControl.stopDone(error_t err) {
	   call AMRadioControl.start();
    }

 

event void Timer0.fired()
       {
}


event void AMRadioSend.sendDone(message_t* msg, error_t error) {
	radiobusy = FALSE;
}



  event void AMSerialSend.sendDone(message_t* bufPtr, error_t error) {
  serialbusy = FALSE;
  }
  
event message_t* AMSerialReceive.receive(message_t* msg, void* payload, uint8_t len) {
// Read from serial port and send the packet over to radio port


		// Assuming the serial packet is read the following lines example how a radio packet can be constructed and send out
		DataMsg* datapkt = (DataMsg*) (call RadioPacket.getPayload(&radiopkt, NULL));
		datapkt->destID = 0xFFFF;  // All invitation messages are to be broadcasted
		datapkt->sourceID = TOS_NODE_ID; // Please make sure that this is OK and satisfies the address requirements 
											// Otherwise modify code to the spec

		datapkt->packetType = 0x01;
		datapkt->packetID = datacounter;
		datapkt->sensorID = 0x01;		// I only substituted some numbers here.
		datapkt->readingTime = 0x01;
		datapkt->valueReported = datacounter;

		if (call AMRadioSend.send(AM_BROADCAST_ADDR, &radiopkt, sizeof(DataMsg)) == SUCCESS) 
				{
				radiobusy = TRUE;
				datacounter++;          
				}
	

  }


event message_t* RadioReceive.receive(message_t* msg, void* payload, uint8_t len) {
//read messagew and send over serial port 

if (len == sizeof(BlinkToRadioMsg)) {
     message_t  *myret = msg;			//Save a pointer to the message 
     BlinkToRadioMsg* btrmsg =(BlinkToRadioMsg*) payload;
     if (!serialbusy)  
          {
          BlinkToRadioMsg* btrpkt = (BlinkToRadioMsg*) (call SerialPacket.getPayload(&serialpkt, NULL));
          btrpkt->nodeid = 0xEEEE;




          if (call AMSerialSend.send(AM_BROADCAST_ADDR, &serialpkt, sizeof(BlinkToRadioMsg)) == SUCCESS) {
          serialbusy = TRUE;
          }


         }


}

return msg;

}     
}
