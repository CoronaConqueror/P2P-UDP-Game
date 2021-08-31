#pragma once
#include <WinSock2.h>
#pragma comment(lib, "wsock32.lib")

//a struct for all the data recieved from a packet
struct RecPacket
{
	unsigned char packet_data[256]; //holds the data recieved from a packet
	unsigned int from_address; //holds the address of the packet, in correct format
	unsigned int from_port; //holds the port, in correct format
};






//handles the ip address formatting
class Address
{
public:

	Address()
	{}
	~Address()
	{}


	//sets up the address into packet sending format, send each number seperated by commas, and end with port
	Address(unsigned int a,
		unsigned int b,
		unsigned int c,
		unsigned int d,
		unsigned short port)
	{

		//SETTING UP ADDRESS FORMAT __________________________________
		unsigned int ADDress = (a << 24) | (b << 16) | (c << 8) | d;


		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(ADDress);
		addr.sin_port = htons(port);


	}

	//holds the address in correct format for sending packets
	sockaddr_in addr;

};




//handles all socket operations, the port I will use is 30000
class Socket
{
public:

	Socket();
	~Socket();

	Socket(unsigned short prt)
	{
		port = prt;

		handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (handle <= 0)
		{
			printf("no socket created\n");
		}


		//binding a socket to a port

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons((unsigned short)port);

		if (bind(handle, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
		{
			printf("failed to bind socket\n");
		}



		//192.168.1.67 is the local IP FOR THIS MACHINE

		//setting a socket to non-blocking mode
		DWORD nonBlocking = 1;
		if (ioctlsocket(handle, FIONBIO, &nonBlocking) != 0)
		{
			printf("failed to set non-blocking\n");
		}



	}


	void SetAddressRaw(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
	{
		
		Address* ADD = new Address(a, b, c, d, port);
		
		addr = *ADD;
	}

	void SetAddressFormatted(unsigned int ADD)
	{
		addr.addr.sin_addr.s_addr = htonl(ADD);
	}



	//sends a packet to another ip address, pass a const char for data and the address in correct format
	void SendPacket(const char sendingdata[])
	{
		

		//sending data
		int sent_bytes = sendto(handle, (const char*)sendingdata, sizeof(sendingdata) + 10, 0, (sockaddr*)&addr, sizeof(sockaddr_in));

		if (sent_bytes != sizeof(sendingdata) + 10)
		{
			printf("failed to send packet\n");
		}

	}


	//loops until a packet is recieved
	RecPacket* LoopReceivePacket()
	{
		RecPacket* Recieved = new RecPacket();

		unsigned int max_packet_size = sizeof(Recieved->packet_data);

		typedef int socklen_t;

		sockaddr_in from;

		socklen_t fromLength = sizeof(from);

		int bytes = recvfrom(handle, (char*)Recieved->packet_data, max_packet_size, 0, (sockaddr*)&from, &fromLength);

		while (bytes <= 0) //loops until a packet is recieved
		{
			//no packet yet
			bytes = recvfrom(handle, (char*)Recieved->packet_data, max_packet_size, 0, (sockaddr*)&from, &fromLength);
		}
		

		addr.addr.sin_family = AF_INET;
		addr.addr.sin_addr.s_addr = from.sin_addr.s_addr;
		addr.addr.sin_port = from.sin_port;


		Recieved->from_address = ntohl(from.sin_addr.s_addr);
		Recieved->from_port = ntohs(from.sin_port);


		return Recieved;

	}

	//only checks once
	RecPacket* ReceivePacket()
	{
		RecPacket* Recieved = new RecPacket();

		unsigned int max_packet_size = sizeof(Recieved->packet_data);

		typedef int socklen_t;

		sockaddr_in from;

		socklen_t fromLength = sizeof(from);

		int bytes = recvfrom(handle, (char*)Recieved->packet_data, max_packet_size, 0, (sockaddr*)&from, &fromLength);

		if(bytes <= 0) //no packet, send nullptr
		{
			//no packet
			return nullptr;
		}

		Recieved->from_address = ntohl(from.sin_addr.s_addr);
		Recieved->from_port = ntohs(from.sin_port);

		
		return Recieved;



	}


private:
	//holds the port for this socket
	unsigned short port;

	//holds the integer for the handle of the socket
	int handle;

	//holds the address for sending packets
	Address addr;


};




// packet type 1 = "1" - link acknowledgement

// packet type 2 = "2 xposition yposition" - other player position, need to change xposition from string to integer to float

// packet type 3 = "3 xposition yposition" - position of new food, need to change xposition from string to integer to float

// packet type 4 = "4" - this is for setting the other player into host/join checking state