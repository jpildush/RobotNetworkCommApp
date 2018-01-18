#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <thread>
#include "MySocket.h"
#include "Pkt_Def.h"

bool ExeComplete = false;

void CommandThread(std::string ip, unsigned int port)
{
	MySocket commandSocket(SocketType::CLIENT, ip, port, ConnectionType::TCP);
	commandSocket.ConnectTCP();

	while (!ExeComplete)
	{
		try
		{
			std::cout << ("Enter the command number.\n1. Drive\n2. Arm\n3. Claw\n4. Sleep") << std::endl << std::endl;
			int user = 0;
			std::cin >> user;

			PktDef pkt;
			MotorBody body;

			switch (user)
			{
			case 1:
				pkt.SetCmd(DRIVE);

				std::cout << ("Enter the command number for DRIVE.\n1. Forward\n2. Backward\n3. Right\n4. Left") << std::endl << std::endl;
				std::cin >> user;
				switch (user)
				{
				case 1:
					body.Direction = FORWARD;
					break;
				case 2:
					body.Direction = BACKWARD;
					break;
				case 3:
					body.Direction = RIGHT;
					break;
				case 4:
					body.Direction = LEFT;
					break;
				}
				std::cout << ("Enter the duration : ");
				std::cin >> user;
				body.Duration = user;
				pkt.SetBodyData((char *)&body, 2);
				break;
			case 2:
				pkt.SetCmd(ARM);
				std::cout << ("Enter the command number for ARM.\n1. Up\n2. Down") << std::endl << std::endl;
				std::cin >> user;
				switch (user)
				{
				case 1:
					body.Direction = UP;
					break;
				case 2:
					body.Direction = DOWN;
					break;
				}
				body.Duration = 0;
				pkt.SetBodyData((char *)&body, 2);
				break;
			case 3:
				pkt.SetCmd(CLAW);
				std::cout << ("Enter the command number for CLAW.\n1. Open\n2. Close") << std::endl << std::endl;
				std::cin >> user;
				switch (user)
				{
				case 1:
					body.Direction = OPEN;
					break;
				case 2:
					body.Direction = CLOSE;
					break;
				}
				body.Duration = 0;
				pkt.SetBodyData((char *)&body, 2);
				break;
			case 4:
				pkt.SetCmd(SLEEP);
				break;
			}

			pkt.SetPktCount(pkt.GetPktCount() + 1);
			pkt.CalcCRC();

			char * buffer = new char[DEFAULT_SIZE];
			commandSocket.SendData(pkt.GenPacket(), pkt.GetLength());

			if (commandSocket.GetData(buffer) < 0)
				throw new std::exception("Data has not been Received properly!");
			else if (pkt.CheckCRC(buffer, buffer[5]))
			{
				PktDef newPacket(buffer);
				if (newPacket.GetCmd() == EMPTY)
					throw new std::exception("Negative Acknowledgement Received!");
				else if (pkt.GetCmd() != newPacket.GetCmd())
					throw new std::exception("Command sent is Unequal to Command Received!");
				else if (!newPacket.GetAck())
					throw new std::exception("Ack flag has not been set!");
				else if (newPacket.GetCmd() == SLEEP)
				{
					commandSocket.DisconnectTCP();
					ExeComplete = true;
				}
			}

		}
		catch (std::exception e)
		{
			std::printf(e.what());
		}
	}
}

void TelemetryThread(std::string ip, unsigned int port)
{
	MySocket telemetrySocket(SocketType::CLIENT, ip, port, ConnectionType::TCP);
	telemetrySocket.ConnectTCP();

	while (!ExeComplete)
	{
		try
		{
			char * buffer = new char[DEFAULT_SIZE];
			if (telemetrySocket.GetData(buffer) > 0)
			{
				PktDef pkt(buffer);
				if (pkt.CheckCRC(buffer, buffer[5]) && pkt.GetCmd() == STATUS)
				{
					buffer = pkt.GenPacket();
					std::cout << "RAW Data Bytes: ";

					for (int x = 0; x < (int)pkt.GetLength(); x++)
						std::cout << std::hex << (unsigned int)*(buffer++) << ", ";
					std::cout << std::endl << std::noshowbase << std::dec;

					buffer = pkt.GetBodyData();
					short tempData;
					memcpy(&tempData, &buffer[0], 2);
					std::cout << "Sonar Sensor Data: " << tempData << ",  ";

					memcpy(&tempData, &buffer[2], 2);
					std::cout << "Arm Position Data: " << tempData << std::endl;

					std::cout << "Status Bits: " << std::endl;

					char * charData = &buffer[4];
					std::cout << "Drive Flag: " << ((*charData & 0x01) ? "1" : "0") << std::endl;
					std::cout << "Arm Status: " << ((*charData >> 1 & 0x01) ? "Arm is Up" :
						((*charData >> 2 & 0x01) ? "Arm is Down" : "")) << std::endl;
					std::cout << "Claw Status: " << ((*charData >> 3 & 0x01) ? "Claw is Open" :
						((*charData >> 4 & 0x01) ? "Claw is Closed" : "")) << std::endl << std::endl;
				}
			}
		}
		catch (std::exception e)
		{
			std::printf(e.what());
		}
	}
}

int main(int argc, char* argv[]) {
	int commandPort, telemetryPort;
	std::string ip;
	
	if (argc == 4)
	{
		ip = argv[1];
		commandPort = (int)argv[2];
		telemetryPort = (int)argv[3];
	}
	else
	{
		ip = "127.0.0.1";
		commandPort = 27000;
		telemetryPort = 27501;
	}

	std::thread(CommandThread, ip, commandPort).detach();
	std::thread(TelemetryThread, ip, telemetryPort).detach();
	while (!ExeComplete) { }
	
	exit(0);
}

