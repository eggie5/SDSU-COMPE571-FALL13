#ifndef BLINKTORADIO_H
#define BLINKTORADIO_H

enum {
     
	 AM_DELAYCHAINEDMSGS = 7,
     AM_BLINKTORADIOMSG = 80,
     AM_BLINKTORADIO = 6,
     TIMER_PERIOD_MILLI = 10000

};


typedef nx_struct BlinkToRadioMsg {
    nx_uint8_t msg[28];
} BlinkToRadioMsg;


#endif


