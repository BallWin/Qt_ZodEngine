#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "qzod_dnclientserver_global.h"
#include "socket_handler.h"



#ifdef Q_OS_WIN  //  _WIN32 if windows
#include <windows.h>		//win for Sleep(1000)
#include <direct.h>		//win
#else
#include <sys/types.h>		//lin
#include <sys/socket.h>		//lin
#include <netinet/in.h>		//lin
#include <netdb.h>		//lin
#include <sys/stat.h>		//lin
#include <sys/ioctl.h>		//lin
#include <arpa/inet.h>
#endif



class QZOD_DNCLIENTSERVERSHARED_EXPORT ClientSocket
{
	public:
		ClientSocket();
		
		int Start(const char *address_ = "localhost", int port_ = 8000);
		void SetEventList(list<Event*> *event_list_);
		int Process();
		
		int SendMessage(int pack_id, const char *data, int size);
		int SendMessageAscii(int pack_id, const char *data);
		
		SocketHandler* GetHandler();
		void ClearConnection();
	private:
		int CreateSocket();
		int Connect();
		
		string address;
		int port;
		int s;
		
		SocketHandler* shandler;
		list<Event*> *event_list;
};

#endif
