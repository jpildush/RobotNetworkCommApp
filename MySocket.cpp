#include "MySocket.h";

/*
A constructor that configures the socket and connection types, sets the IP Address
and Port Number and dynamically allocates memory for the Buffer. Note that the
constructor should put servers in conditions to either accept connections (if TCP),
or to receive messages (if UDP).
*/
MySocket::MySocket(SocketType socketType, std::string IPAddress, unsigned int port, ConnectionType connectType, unsigned int size)
{
	// Set SocketType and ConnectionType
	this->mySocket = socketType;
	this->connectionType = connectType;

	// Set IP and Port
	this->IPAddr = IPAddress;
	this->Port = port;

	// Check if size is more than 0, if not set to DEFAULT_SIZE of 128
	if (size > 0) this->MaxSize = size;
	else this->MaxSize = DEFAULT_SIZE;

	// Dynamically allocates memory for the Buffer
	this->Buffer = new char[MaxSize];
	// Set bTCPConnect to default false
	this->bTCPConnect = false;

	// Start WSA
	if ((WSAStartup(MAKEWORD(2, 2), &this->wsadata)) != 0) {
		std::cout << "Could not start DLLs" << std::endl;
		std::cin.get();
		exit(0);
	}

	// Check Connection Type
	if (this->connectionType == ConnectionType::TCP) {

		// If TCP CLIENT
		if (this->mySocket == SocketType::CLIENT) {
			// Initialize ConnectionSocket
			this->ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (ConnectionSocket == INVALID_SOCKET) {
				WSACleanup();
				std::cout << "Could not initialize socket" << std::endl;
				std::cin.get();
				exit(0);
			}
		}

		// If TCP SERVER
		if (this->mySocket == SocketType::SERVER) {
			// Initialize WelcomeSocket
			this->WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (WelcomeSocket == INVALID_SOCKET) {
				WSACleanup();
				std::cout << "Could not initialize socket" << std::endl;
				std::cin.get();
				exit(0);
			}

			// Bind WelcomeSocket
			SvrAddr.sin_family = AF_INET; //Address family type internet
			SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
			SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address
			if ((bind(this->WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
				closesocket(this->WelcomeSocket);
				WSACleanup();
				std::cout << "Could not bind to the socket" << std::endl;
				std::cin.get();
				exit(0);
			}

			// Call the function to start listening and accepting
			this->ConnectTCP();
		}
	}
	// IF UDP SERVER
	else if (this->connectionType == ConnectionType::UDP) {
		// Initialize ConnectionClient for either UDP Client or Server 
		this->ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (ConnectionSocket == INVALID_SOCKET) {
			WSACleanup();
			std::cout << "Could not initialize socket" << std::endl;
			std::cin.get();
			exit(0);
		}

		// If UDP SERVER
		if (this->mySocket == SocketType::SERVER) {

			// Bind ConnectionSocket
			SvrAddr.sin_family = AF_INET; //Address family type internet
			SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
			SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address
			if ((bind(this->ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
				closesocket(this->ConnectionSocket);
				WSACleanup();
				std::cout << "Could not bind to the socket" << std::endl;
				std::cin.get();
				exit(0);
			}
		}
	}

}

// A destructor that cleans up all dynamically allocated memory space 
MySocket::~MySocket()
{
	// Delete dynamically allocated Buffer
	delete Buffer;

	if (this->mySocket == SocketType::SERVER && this->connectionType == ConnectionType::TCP) {
		closesocket(this->ConnectionSocket); //closes connection socket
		closesocket(this->WelcomeSocket); //closes welcome socket
	}
	else 
		closesocket(this->ConnectionSocket); //closes connection socket

	if (this->connectionType == ConnectionType::UDP)
		WSACleanup(); //frees Winsock DLL resource
}

// Used to establish a TCP/IP socket connection. 
void MySocket::ConnectTCP()
{
	// Try to connect to Server | keep waiting until connection established
	if (this->mySocket == SocketType::CLIENT && this->connectionType == ConnectionType::TCP && !this->bTCPConnect)
	{
		std::cout << "Trying to connect to the server" << std::endl;
		bTCPConnect = false;
		SvrAddr.sin_family = AF_INET; //Address family type internet
		SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
		SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address
		while (!bTCPConnect) {
			if ((connect(this->ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
			else {
				std::cout << "Connection Established" << std::endl;
				bTCPConnect = true;
			}
		}
	}
	// Start accepting Connections
	else if (this->mySocket == SocketType::SERVER && this->connectionType == ConnectionType::TCP)
	{
		// Start Listening
		if (listen(this->WelcomeSocket, 1) == SOCKET_ERROR) {
			closesocket(this->WelcomeSocket);
			WSACleanup();
			std::cout << "Could not listen to the provided socket." << std::endl;
			std::cin.get();
			exit(0);
		}
		else {
			std::cout << "Waiting for client connection" << std::endl;
		}

		// Loop until connected
		while (!this->bTCPConnect)
		{
			if ((this->ConnectionSocket = accept(this->WelcomeSocket, NULL, NULL)) == SOCKET_ERROR) {
				closesocket(this->WelcomeSocket);
				WSACleanup();
				std::cout << "Could not accept incoming connection" << std::endl;
				std::cin.get();
				exit(0);
			}
			else {
				std::cout << "Connection Accepted" << std::endl;
				this->bTCPConnect = true;
			}
		}
	}
}

// Used to disconnect an established TCP / IP socket connection.
void MySocket::DisconnectTCP()
{
	if (this->connectionType == ConnectionType::TCP) {
		closesocket(this->ConnectionSocket);
		this->bTCPConnect = false;
		if (this->mySocket == SocketType::CLIENT)
		{
			this->ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (ConnectionSocket == INVALID_SOCKET) {
				WSACleanup();
				std::cout << "Could not initialize socket" << std::endl;
				std::cin.get();
				exit(0);
			}
		}
	}
}

/*
Used to transmit a block of RAW data, specified by the starting
memory address and number of bytes, over the socket. This function
should work with both TCP and UDP.
*/
void MySocket::SendData(const char * data, int size)
{
	if (this->connectionType == ConnectionType::TCP && this->bTCPConnect)
		send(this->ConnectionSocket, data, size, 0);
	else if (this->connectionType == ConnectionType::UDP)
	{
		SvrAddr.sin_family = AF_INET; //Address family type internet
		SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
		SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address

		sendto(this->ConnectionSocket, data, size, 0, (sockaddr *)&SvrAddr, sizeof(SvrAddr));
	}
}

/*
Used to receive the last block of RAW data stored in the internal MySocket Buffer.
After getting the received message into Buffer, this function will transfer its contents
to the provided memory address and return the total number of bytes written.
This function should work with both TCP and UDP.
*/
int MySocket::GetData(char * data)
{
	memset(this->Buffer, 0, MaxSize);
	int count = 0;
	if (this->connectionType == ConnectionType::TCP && this->bTCPConnect) {
		count = recv(this->ConnectionSocket, this->Buffer, this->MaxSize - 1, 0);
	}
	else if (this->connectionType == ConnectionType::UDP) {
		SvrAddr.sin_family = AF_INET; //Address family type internet
		SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
		SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address

		int addr_len = sizeof(SvrAddr);
		count = recvfrom(this->ConnectionSocket, this->Buffer, this->MaxSize - 1, 0, (sockaddr *)&SvrAddr, &addr_len);
	}


	if (count > 0 && count < MaxSize)
		memcpy(data, Buffer, count);
	else 
		count = -1;

	return count;
}

// Returns the IP address configured within the MySocket object 
std::string MySocket::GetIPAddr()
{
	return this->IPAddr;
}

// Changes the default IP address within the MySocket object 
void MySocket::SetIPAddr(std::string IPAddress)
{
	if (!this->bTCPConnect)
	{
		this->IPAddr = IPAddress;
		if (this->connectionType == ConnectionType::TCP && this->mySocket == SocketType::SERVER)
		{
			closesocket(this->WelcomeSocket);

			this->WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (WelcomeSocket == INVALID_SOCKET) {
				WSACleanup();
				std::cout << "Could not initialize socket" << std::endl;
				std::cin.get();
				exit(0);
			}

			SvrAddr.sin_family = AF_INET; //Address family type internet
			SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
			SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address
			if ((bind(this->WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
				closesocket(this->WelcomeSocket);
				WSACleanup();
				std::cout << "Could not bind to the socket" << std::endl;
				std::cin.get();
				exit(0);
			}
		}
		else if (this->connectionType == ConnectionType::UDP && this->mySocket == SocketType::SERVER)
		{
			closesocket(this->ConnectionSocket);

			this->ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (ConnectionSocket == INVALID_SOCKET) {
				WSACleanup();
				std::cout << "Could not initialize socket" << std::endl;
				std::cin.get();
				exit(0);
			}

			SvrAddr.sin_family = AF_INET; //Address family type internet
			SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
			SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address
			if ((bind(this->ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
				closesocket(this->ConnectionSocket);
				WSACleanup();
				std::cout << "Could not bind to the socket" << std::endl;
				std::cin.get();
				exit(0);
			}
		}
	}
	else
		std::cout << "A connection has already been established, cannot change ip address." << std::endl;
}

// Changes the default Port number within the MySocket object 
void MySocket::SetPort(int port)
{
	if (!this->bTCPConnect)
	{
		this->Port = port;
		if (this->connectionType == ConnectionType::TCP && this->mySocket == SocketType::SERVER)
		{
			closesocket(this->WelcomeSocket);

			this->WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (WelcomeSocket == INVALID_SOCKET) {
				WSACleanup();
				std::cout << "Could not initialize socket" << std::endl;
				std::cin.get();
				exit(0);
			}

			SvrAddr.sin_family = AF_INET; //Address family type internet
			SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
			SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address
			if ((bind(this->WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
				closesocket(this->WelcomeSocket);
				WSACleanup();
				std::cout << "Could not bind to the socket" << std::endl;
				std::cin.get();
				exit(0);
			}
		}
		else if (this->connectionType == ConnectionType::UDP && this->mySocket == SocketType::SERVER)
		{
			closesocket(this->ConnectionSocket);

			this->ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (ConnectionSocket == INVALID_SOCKET) {
				WSACleanup();
				std::cout << "Could not initialize socket" << std::endl;
				std::cin.get();
				exit(0);
			}

			SvrAddr.sin_family = AF_INET; //Address family type internet
			SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
			SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address
			if ((bind(this->ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
				closesocket(this->ConnectionSocket);
				WSACleanup();
				std::cout << "Could not bind to the socket" << std::endl;
				std::cin.get();
				exit(0);
			}
		}
	}
	else
		std::cout << "A connection has already been established, cannot change port." << std::endl;
}

// Returns the Port number configured within the MySocket object 
int MySocket::GetPort()
{
	return this->Port;
}

// Returns the default SocketType the MySocket object is configured as 
SocketType MySocket::GetType()
{
	return this->mySocket;
}

// Changes the default SocketType within the MySocket object 
void MySocket::SetType(SocketType socketType)
{
	if (!this->bTCPConnect)
	{
		this->mySocket = socketType;
		if (this->connectionType == ConnectionType::TCP)
		{
			closesocket(this->WelcomeSocket);
			closesocket(this->ConnectionSocket);

			if (this->mySocket == SocketType::CLIENT) {
				this->ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (ConnectionSocket == INVALID_SOCKET) {
					WSACleanup();
					std::cout << "Could not initialize socket" << std::endl;
					std::cin.get();
					exit(0);
				}
			}

			if (this->mySocket == SocketType::SERVER) {
				this->WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (WelcomeSocket == INVALID_SOCKET) {
					WSACleanup();
					std::cout << "Could not initialize socket" << std::endl;
					std::cin.get();
					exit(0);
				}

				SvrAddr.sin_family = AF_INET; //Address family type internet
				SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
				SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address
				if ((bind(this->WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
					closesocket(this->WelcomeSocket);
					WSACleanup();
					std::cout << "Could not bind to the socket" << std::endl;
					std::cin.get();
					exit(0);
				}
			}
		}
		else if (this->connectionType == ConnectionType::UDP)
		{
			closesocket(this->WelcomeSocket);
			closesocket(this->ConnectionSocket);

			this->ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (ConnectionSocket == INVALID_SOCKET) {
				WSACleanup();
				std::cout << "Could not initialize socket" << std::endl;
				std::cin.get();
				exit(0);
			}

			if (this->mySocket == SocketType::SERVER) {
				SvrAddr.sin_family = AF_INET; //Address family type internet
				SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
				SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address
				if ((bind(this->ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
					closesocket(this->ConnectionSocket);
					WSACleanup();
					std::cout << "Could not bind to the socket" << std::endl;
					std::cin.get();
					exit(0);
				}
			}
		}
	}
}