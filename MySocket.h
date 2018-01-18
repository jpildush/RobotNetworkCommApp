#ifndef	MYSOCKET_H
#define MYSOCKET_H

#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

// Enumeration of type SocketType that contains{ CLIENT, SERVER }
enum SocketType { CLIENT, SERVER };

// Enumeration of type ConnectionType that contains{ TCP, UDP }
enum ConnectionType { TCP, UDP };

// Constant integer that defines the DEFAULT_SIZE of the buffer space
const int DEFAULT_SIZE = 128;

class MySocket
{
private:
	// To dynamically allocate RAW buffer space for communication activities 
	char * Buffer;
	// Configured as a TCP/IP Server 
	SOCKET WelcomeSocket;
	// Used for client/server communications (both TCP and UDP) 
	SOCKET ConnectionSocket;
	// To store connection information 
	sockaddr_in SvrAddr;
	// To hold the type of socket the MySocket object is initialized to
	SocketType mySocket;
	// To hold the IPv4 IP Address string 
	std::string IPAddr;
	// To hold the port number to be used 
	int Port;
	// To define the Transport Layer protocol being used (TCP/UDP)
	ConnectionType connectionType;
	// To flag to determine if a connection has been established or not 
	bool bTCPConnect;
	// To store the maximum number of bytes the buffer is allocated to. 
	int MaxSize;
	// WSADATA
	WSADATA wsadata;
public:
	/*  
	A constructor that configures the socket and connection types, sets the IP Address 
	and Port Number and dynamically allocates memory for the Buffer. Note that the 
	constructor should put servers in conditions to either accept connections (if TCP), 
	or to receive messages (if UDP). 
	*/
	MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int = DEFAULT_SIZE);

	// A destructor that cleans up all dynamically allocated memory space 
	~MySocket();

	// Used to establish a TCP/IP socket connection. 
	void ConnectTCP();
	// Used to disconnect an established TCP / IP socket connection.
	void DisconnectTCP();

	/*
	Used to transmit a block of RAW data, specified by the starting 
	memory address and number of bytes, over the socket. This function 
	should work with both TCP and UDP. 
	*/
	void SendData(const char*, int);

	/*
	Used to receive the last block of RAW data stored in the internal MySocket Buffer.   
	After getting the received message into Buffer, this function will transfer its contents 
	to the provided memory address and return the total number of bytes written.  
	This function should work with both TCP and UDP. 
	*/
	int GetData(char*);

	// Returns the IP address configured within the MySocket object 
	std::string GetIPAddr();
	// Changes the default IP address within the MySocket object 
	void SetIPAddr(std::string);

	// Changes the default Port number within the MySocket object 
	void SetPort(int);
	// Returns the Port number configured within the MySocket object 
	int GetPort();

	// Returns the default SocketType the MySocket object is configured as 
	SocketType GetType();
	// Changes the default SocketType within the MySocket object 
	void SetType(SocketType);
};

#endif MYSOCKET_H