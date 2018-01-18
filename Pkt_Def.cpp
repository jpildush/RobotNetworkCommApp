#include "Pkt_Def.h"


// A default constructor that places the PktDef object in a safe state
PktDef::PktDef() 
{
	// Set all Header information set to zero 
	packet.header.PktCount = 0;
	packet.header.Drive = 0;
	packet.header.Status = 0;
	packet.header.Sleep = 0;
	packet.header.Arm = 0;
	packet.header.Claw = 0;
	packet.header.Ack = 0;
	packet.header.Padding = 0;
	packet.header.Length = HEADERSIZE + sizeof(char);

	// Set data pointer to nullptr
	packet.data = nullptr;

	// CRC set to zero
	packet.CRC = 0;
}

// An overloaded constructor that takes a RAW data buffer
PktDef::PktDef(char * dataBuffer)
{
	// Populate PktCount and Command Flags
	int * ptrInt = (int *)&dataBuffer[0];
	packet.header.PktCount = *ptrInt++;
	packet.header.Drive = (char)(*ptrInt) & 0x01;
	packet.header.Status = (char)(*ptrInt >> 1) & 0x01;
	packet.header.Sleep = (char)(*ptrInt >> 2) & 0x01;
	packet.header.Arm = (char)(*ptrInt >> 3) & 0x01;
	packet.header.Claw = (char)(*ptrInt >> 4) & 0x01;
	packet.header.Ack = (char)(*ptrInt >> 5) & 0x01;
	packet.header.Padding = 0;

	// Populate length in Header
	char * ptrChar = &dataBuffer[5];
	packet.header.Length = (*ptrChar++);
	
	// Populate the packet bodydata
	packet.data = new char[packet.header.Length - HEADERSIZE - sizeof(char)];
	for (int i = 0; i < packet.header.Length - HEADERSIZE - sizeof(char); i++)
		packet.data[i] = *ptrChar++;

	// Populate the CRC
	packet.CRC = (*ptrChar);
}

// A set function that sets the packets command flag based on the CmdType
void PktDef::SetCmd(CmdType ct)
{
	switch (ct)
	{
	case DRIVE:
		packet.header.Drive = 1;
		packet.header.Status = 0;
		packet.header.Sleep = 0;
		packet.header.Arm = 0;
		packet.header.Claw = 0;
		packet.header.Ack = 0;
		break;
	case STATUS:
		packet.header.Status = 1;
		break;
	case SLEEP:
		packet.header.Drive = 0;
		packet.header.Status = 0;
		packet.header.Sleep = 1;
		packet.header.Arm = 0;
		packet.header.Claw = 0;
		packet.header.Ack = 0;

		delete packet.data;
		packet.data = nullptr;
		packet.header.Length = HEADERSIZE + sizeof(char);
		break;
	case ARM:
		packet.header.Drive = 0;
		packet.header.Status = 0;
		packet.header.Sleep = 0;
		packet.header.Arm = 1;
		packet.header.Claw = 0;
		packet.header.Ack = 0;
		break;
	case CLAW:
		packet.header.Drive = 0;
		packet.header.Status = 0;
		packet.header.Sleep = 0;
		packet.header.Arm = 0;
		packet.header.Claw = 1;
		packet.header.Ack = 0;
		break;
	case ACK:
		packet.header.Ack = 1;
		break;
	}
}

// A set function that takes a pointer to a RAW data buffer and the size of the buffer in bytes. 
void PktDef::SetBodyData(char * data, int size)
{
	// Set the new packet length with body data
	packet.header.Length = HEADERSIZE + size + sizeof(char);

	// Allocate amount of space for data
	packet.data = new char[size];

	// Copies the provided data
	for (int i = 0; i < size; i++) 
		packet.data[i] = *data++;
}

// Set function that sets the objects PktCount header variable
void PktDef::SetPktCount(int pc)
{
	packet.header.PktCount = pc;
}
// A query function that returns the CmdType based on the set command flag bit
CmdType PktDef::GetCmd()
{
	if (packet.header.Drive & 0x01) return DRIVE;
	else if (packet.header.Status & 0x01) return STATUS;
	else if (packet.header.Sleep & 0x01) return SLEEP;
	else if (packet.header.Arm & 0x01) return ARM;
	else if (packet.header.Claw & 0x01) return CLAW;
	else if (packet.header.Ack & 0x01) return ACK;
	else
		return EMPTY;
}

// A query function that returns True / False based on the Ack flag in the header
bool PktDef::GetAck()
{
	return packet.header.Ack;
}

// A query function that returns the packet length in bytes 
int PktDef::GetLength()
{
	return packet.header.Length;
}

// A query function that returns a pointer to the objects Body field
char * PktDef::GetBodyData()
{
	return packet.data;
}

// A query function that returns the PktCount value 
int PktDef::GetPktCount()
{
	return packet.header.PktCount;
}

/* 
	A function that takes a pointer to a RAW data buffer, the size of the buffer 
	in bytes, and calculates the CRC.If the calculated CRC matches the CRC of the 
	packet in the buffer the function returns TRUE, otherwise FALSE.
*/
bool PktDef::CheckCRC(char * data, int size)
{
	// Set count variable
	int count = 0;

	// Count the CRC of data
	for (int i = 0; i < size - sizeof(char); i++) 
		for (int j = 0; j < 8; j++)
			count += ((data[i] >> j) & 0x01);	

	// Compare the CRC of the data to the counted CRC
	return (count == data[size - sizeof(char)]) ? true : false;
}

// A function that calculates the CRC and sets the objects packet CRC parameter
void PktDef::CalcCRC()
{
	// Set count variable
	int count = 0;
	
	// Count the Header
	char * ptr = (char*)&packet.header;
	for (int i = 0; i < HEADERSIZE; i++) 
		for (int j = 0; j < 8; j++)
			count += ((ptr[i] >> j) & 0x01);


	// If body data isn't empty
	if (packet.data != nullptr)
	{
		// Count the body data
		ptr = packet.data;
		for (int i = 0; i < (packet.header.Length - HEADERSIZE - sizeof(char)); i++)
			for (int j = 0; j < 8; j++)
				count += ((ptr[i] >> j) & 0x01);
	}

	// Set the CRC count
	packet.CRC = count;
}

/*
	A function that allocates the private RawBuffer and transfers the contents 
	from the objects member variables into a RAW data packet (RawBuffer) for
	transmission. The address of the allocated RawBuffer is returned.
*/
char * PktDef::GenPacket()
{
	// Allocate the RawBuffer
	RawBuffer = new char[packet.header.Length];

	// Transfer header data into RawBuffer
	char * ptr = (char*)&packet.header;
	for (int i = 0; i < HEADERSIZE; i++)
		RawBuffer[i] = ptr[i];

	// If body data isn't empty
	if (packet.data != nullptr)
	{
		// Transfer body data into RawBuffer
		ptr = (char*)packet.data;
		for (int i = 0; i < (packet.header.Length - HEADERSIZE - sizeof(char)); i++)
			RawBuffer[HEADERSIZE + i] = ptr[i];
	}

	// Transfer the CRC into RawBuffer
	RawBuffer[packet.header.Length - sizeof(char)] = packet.CRC;

	// Return RawBuffer
	return RawBuffer;
}

