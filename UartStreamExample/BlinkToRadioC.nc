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

		BlinkToRadioMsg* btrpkt = (BlinkToRadioMsg*) (call SerialPacket.getPayload(&serialpkt, NULL));
		
        mydata = (uint8_t*)  malloc(28);

        

        for(i=0; i<28; i++)
        {
            btrpkt->msg[i] = buf[i];
        }
        call AMRadioSend.send(AM_BROADCAST_ADDR, &serialpkt, sizeof(BlinkToRadioMsg))
        call UartStream.receive(mydata,28); //restarts serial


    }

    async event void UartStream.sendDone(uint8_t *buf, uint16_t len, error_t error)
    {   //Signal completion of sending a stream.

    }

    event message_t* RadioReceive.receive(message_t* msg, void* payload, uint8_t len)
    {

        uint8_t    *mydata;
        uint8_t    i;

        BlinkToRadioMsg* btrmsg =(BlinkToRadioMsg*) payload;



        mydata = (uint8_t*)  malloc(28);

        for(i=0; i<28; i++)
        {
            mydata[i]=btrmsg->msg[i];
        }

        call UartStream.send(mydata, 28);



        return msg;
    }

}
