#ifndef BLINKTORADIO_H
#define BLINKTORADIO_H

enum {
     
	 AM_DELAYCHAINEDMSGS = 7,
     AM_BLINKTORADIOMSG = 80,
     AM_BLINKTORADIO = 6,
     TIMER_PERIOD_MILLI = 10000

};


typedef nx_struct BlinkToRadioMsg {
    nx_uint16_t  nodeid;
    nx_uint16_t  counter;  // nx prefix eliminates to manually address endiannes
	nx_uint16_t  messagecount;
	nx_uint16_t  thatfield;
	nx_uint16_t  sourceaddress;
} BlinkToRadioMsg;


#endif


