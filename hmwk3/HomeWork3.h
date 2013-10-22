#ifndef FIRSTHOMEWORK_H
#define FIRSTHOMEWORK_H

enum {
     
     AM_RADIOMESSAGE = 6,				
     TIMER_PERIOD_INVITATION = 2000,		// Invitation message interval
	 TIMER_PERIOD_SENSORDATA = 5000			// Sensor data reporting interval 

};


typedef nx_struct ControlMsg {
    nx_uint16_t  destID;
    nx_uint16_t  sourceID;
	nx_uint8_t  packetType;  
	nx_uint8_t  packetID; 
} ControlMsg;

typedef nx_struct DataMsg {
    nx_uint16_t  destID;
    nx_uint16_t  sourceID;
	nx_uint8_t  packetType;  
	nx_uint8_t  packetID;
	nx_uint16_t	sensorID;
	nx_uint16_t	readingTime;
	nx_uint16_t	valueReported;

} DataMsg;



#endif


