#include <iostream>
#include "MySocket.h"


int main()
{
	MySocket ClientSocket(SocketType::CLIENT, "127.0.0.1", 5000, ConnectionType::TCP, 100);

	std::string Pkt = "I love BTN415";

	ClientSocket.ConnectTCP();
	ClientSocket.SendData(Pkt.c_str(), strlen(Pkt.c_str()) + 1);

	char buff[100];
	memset(buff, 0, 100);
	int RxSize = ClientSocket.GetData(buff);

	std::cout << "Msg = " << buff << ", Bytes = " << RxSize << std::endl;

	ClientSocket.DisconnectTCP();

	return 1;
}