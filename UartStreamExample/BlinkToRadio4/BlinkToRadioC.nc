#include <Serial.h>
#include <Timer.h>
#include "BlinkToRadio.h"

module BlinkToRadioC {
   uses interface Boot;
   uses interface Leds;
   uses interface Timer<TMilli> as Timer0; 
   

   uses interface AMPacket;
   uses interface Packet  as RadioPacket;
   uses interface AMSend as AMRadioSend;
   uses interface Receive as RadioReceive;
   uses interface SplitControl as AMRadioControl;
   
   uses interface SendBytePacket;
   uses interface UartStream;   
   uses interface SplitControl as AMSerialControl;
   uses interface Receive as AMSerialReceive;
   uses interface AMSend  as AMSerialSend;
   uses interface Packet  as SerialPacket;
   
 
 
}

implementation {
   uint16_t   counter = 0;
   bool  busy = FALSE;
   bool  sbusy = FALSE;
   message_t   pkt;
   message_t   serialpkt;


   

   event  void Boot.booted()
      {
       call AMSerialControl.start();
       }
  


   event  void AMRadioControl.startDone(error_t err) {
       if (err == SUCCESS)  {
       // Timer should not start before Radio is initialized and ready
       call Timer0.startOneShot(TIMER_PERIOD_MILLI);
       }
       else {
             call AMRadioControl.start();
            }
    }


    event  void AMRadioControl.stopDone(error_t err) {
	   call AMRadioControl.start();
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
 

event void Timer0.fired()
       {
       uint8_t    *mydata;
       uint8_t    *sreceivedData;
       uint8_t    i;
     


 	mydata = (uint8_t*)  malloc(5);
	sreceivedData = (uint8_t*)  malloc(5);

	call UartStream.receive(sreceivedData,5);
   
     if (!sbusy)  
          {
          //call SendBytePacket.startSend(0x44);
          *(mydata) = 0x41;
          *(mydata+1) = 0x42;
          *(mydata+2) = 0x43;
          *(mydata+3) = 0x45;
          *(mydata+4) = '\n';
           
    	  call UartStream.send(mydata, 5);
	    
         
	  }

 
   }


event void AMRadioSend.sendDone(message_t* msg, error_t error) {
 // if(&pkt == msg)  {
                    busy = FALSE;
//                   }
}



  event void AMSerialSend.sendDone(message_t* bufPtr, error_t error) {
  
  sbusy = FALSE;
  }

  
  event message_t* AMSerialReceive.receive(message_t* msg, void* payload, uint8_t len) {


  }


async event uint8_t SendBytePacket.nextByte() 
{

return(0x42);


//call SendBytePacket.completeSend();

}


async event void SendBytePacket.sendCompleted(error_t error) {




}





async event void UartStream.receivedByte(uint8_t byte) 
{
 //Signals the receipt of a byte. 


}

async event void UartStream.receiveDone(uint8_t *buf, uint16_t len, error_t error) 
{
//Signal completion of receiving a stream. 

       uint8_t    *mydata;
       uint8_t    i;

       mydata = (uint8_t*)  malloc(5);


     if (!sbusy)  
          {
          //call SendBytePacket.startSend(0x44);
          *(mydata) = *(buf);
          *(mydata+1) = *(buf+1);
          *(mydata+2) = *(buf+2);
          *(mydata+3) = *(buf+3);
          *(mydata+4) = *(buf+4);

    	  call UartStream.send(mydata, 5);
	    
         
	  }

	call UartStream.receive(mydata,5);




}

async event void UartStream.sendDone(uint8_t *buf, uint16_t len, error_t error) 
{//Signal completion of sending a stream. 



}

 event message_t* RadioReceive.receive(message_t* msg, void* payload, uint8_t len) {
 if (len == sizeof(BlinkToRadioMsg)) {
     message_t  *myret = msg;			//Save a pointer to the message 
     uint8_t    *mydata;
     uint8_t    i;
     
     BlinkToRadioMsg* btrmsg =(BlinkToRadioMsg*) payload;
     call Leds.set(btrmsg->counter);
   
     mydata = (uint8_t*)  malloc(5);
   
     if (!sbusy)  
          {
          BlinkToRadioMsg* btrpkt = (BlinkToRadioMsg*) (call SerialPacket.getPayload(&serialpkt, NULL));
          btrpkt->nodeid = 0xEEEE;
          btrpkt->counter = btrmsg->counter;
          btrpkt->messagecount = counter;
          btrpkt->thatfield = 3;
          btrpkt->sourceaddress = call AMPacket.source(msg);
          
          call AMPacket.setDestination(&serialpkt,0xCCCC);
          call AMPacket.setSource(&serialpkt,0xBB);
          call AMPacket.setType(&serialpkt,0xCC);

          
          //call SendBytePacket.startSend(0x44);
          *mydata = 5;
          *(mydata) = 0x01;
          *(mydata+1) = 0x01;
          *(mydata+2) = 0x02;
          *(mydata+3) = 0x03;
          *(mydata+4) = 0x04;
           
    	  call UartStream.send(mydata, 5);
		   
          //if (call AMSerialSend.send(AM_BROADCAST_ADDR, &serialpkt, sizeof(BlinkToRadioMsg)) == SUCCESS) {
          //if (call AMSerialSend.send(AM_BROADCAST_ADDR, &myret, len) == SUCCESS) {
          
          
         // call Leds.set(btrmsg->counter);
         // sbusy = TRUE;
         // }
         }


}
return msg;
}

}
