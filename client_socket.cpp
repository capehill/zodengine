#include "client_socket.h"
#include "common.h"

using namespace COMMON;

ClientSocket::ClientSocket()
{
	s = -1;
	shandler = NULL;
	event_list = NULL;
}

int ClientSocket::SendMessage(int pack_id, const char *data, int size)
{
	if(shandler) return shandler->SendMessage(pack_id, data, size);
	else return 0;
}

int ClientSocket::SendMessageAscii(int pack_id, const char *data)
{
	if(shandler) return shandler->SendMessageAscii(pack_id, data);
	else return 0;
}

void ClientSocket::SetEventList(list<Event*> *event_list_)
{
	event_list = event_list_;
}

int ClientSocket::Start(const char *address_, int port_)
{
	address = address_;
	port = port_;
	
	if(!CreateSocket()) return 0;
	
	if(!Connect()) return 0;
	
	return 1;
}

void ClientSocket::ClearConnection()
{
	if(shandler)
	{
		delete shandler;
		shandler = NULL;
	}

	s = -1;
}

int ClientSocket::Process()
{
	char *message;
	int size;
	int pack_id;
	if(!shandler) return 0;
	
	//not connected, free it up
	if(!shandler->Connected())
	{
		ClearConnection();
		
		event_list->push_back(new Event(OTHER_EVENT, DISCONNECT_EVENT, 0, NULL, 0));
	}
	else if(shandler->DoRecv())
	{
		while(shandler->DoFastProcess(&message, &size, &pack_id))
			event_list->push_back(new Event(TCP_EVENT, pack_id, 0, message, size));
		shandler->ResetFastProcess();

		/*
		while(shandler->DoProcess(&message, &size, &pack_id))
		{
			event_list->push_back(new Event(TCP_EVENT, pack_id, 0, message, size));
 			//printf("ClientSocket::Process:got packet id:%d\n", pack_id);
		}*/
	}

	/*
	while(shandler->PacketAvailable() && shandler->GetPacket(&message, &size, &pack_id))
		event_list->push_back(new Event(TCP_EVENT, pack_id, 0, message, size));	
	*/

	return 1;
}

SocketHandler* ClientSocket::GetHandler()
{
	return shandler;
}

int ClientSocket::Connect()
{
	struct sockaddr_in c_in;

/* TODO: currently only local game works. Also network needs little endian conversions so there is work todo if real netplay is desired
	struct hostent * host;
	
	host = gethostbyname(address.c_str());
	
	if(!host) 
	{
		printf("could not resolve host '%s'\n", address.c_str());
		return 0;
	}
*/	

	memset(&c_in, 0, sizeof(c_in));
	
#ifdef __amigaos4__

	// Because it looks like if we try to connect to server before it's up, our connect() fails with EINVAL?
	extern int serverSocketReady;

	while (serverSocketReady == 0)
	{
		uni_pause(10);
	}

	printf("server socket ready\n");
	
	c_in.sin_addr.s_addr = inet_addr("127.0.0.1");

	#warning TODO
#else

	memcpy((char*)&c_in.sin_addr, (char*)host->h_addr_list[0], host->h_length);	

#endif
	
	c_in.sin_family = AF_INET;
	c_in.sin_port = htons(port);
		
	while(connect(s, (struct sockaddr *)&c_in, sizeof(c_in)) < 0)
	{
		static double first_failure = current_time();

		printf("Reconnecting to port %d, socket %d (errno %d)\n", port, s, errno);
		
		uni_pause(1000);
		
		if(current_time() - first_failure > 30.0)
		{
			printf("could not connect to '%s' (errno %d)\n", address.c_str(), errno);
			return 0;
		}
	}
	
	shandler = new SocketHandler(s,c_in);
	
	event_list->push_back(new Event(OTHER_EVENT, CONNECT_EVENT, 0, NULL, 0));
	
	return 1;
}

int ClientSocket::CreateSocket()
{
	if(s != -1) return 1;
	
#ifdef _WIN32
	WSADATA wsaData;
	
	WSAStartup(0x202,&wsaData);
#endif
	
	if((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		printf("ClientSocket::CreateSocket:error in socket creation\n");
		return 0;
	}
	
	return 1;
}
