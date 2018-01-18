#ifndef Pkt_Def_H
#define Pkt_Def_H

#include <stdio.h>
#include <iostream>
#include <iomanip>

// The following constant integer definitions
const int FORWARD = 1;
const int BACKWARD = 2; 
const int RIGHT = 3;
const int LEFT = 4;
const int UP = 5;
const int DOWN = 6;
const int OPEN = 7;
const int CLOSE = 8;
// Represents the size of the Header in bytes (must be calculated by hand) 
const int HEADERSIZE = 6;

enum CmdType
{
	DRIVE, STATUS, SLEEP, ARM, CLAW, ACK, EMPTY
};

struct Header
{
	int PktCount;
	unsigned char Drive : 1;
	unsigned char Status : 1;
	unsigned char Sleep : 1;
	unsigned char Arm : 1;
	unsigned char Claw : 1;
	unsigned char Ack : 1;
	unsigned char Padding : 2;
	unsigned char Length;
};

struct MotorBody
{
	char Direction;
	uint8_t Duration; // An 1-byte unsigned int
};

class PktDef
{
private:
	// Private structure to define CmdPacket
	struct CmdPacket
	{
		Header header;
		char * data;
		char CRC;
	}
	packet;
	// A char *RawBuffer that will store all data in PktDef in a serialized form 
	// that can be used to transmit it over TCP / IP
	char * RawBuffer;
public:
	PktDef();
	PktDef(char *);
	void SetCmd(CmdType);
	void SetBodyData(char *, int);
	void SetPktCount(int);
	CmdType GetCmd();
	bool GetAck();
	int GetLength();
	char * GetBodyData();
	int GetPktCount();
	bool CheckCRC(char *, int);
	void CalcCRC();
	char * GenPacket();
};

#endif Pkt_Def_H