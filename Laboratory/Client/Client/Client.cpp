// Client.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#define _WINSOCK_DEPCRECATED 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "WSOCK32.lib")
#pragma comment(lib, "SHLWAPI.lib")

#include <stdio.h>
#include <fstream>
#include <string.h>
#include <winsock2.h>
#include <string>
#include <windows.h>
#include <Mswsock.h>
#include <shlwapi.h>
#include <iostream>

using namespace std;

#define PORT 1041 // port
#define SERVERADDR "127.0.0.1" //server ip

const char help[] = "\nAvailable commands :\nHelp - for open help message\nWho - for displaying information about\nExit - for stopping program\nStart Game - to launch the game\nStop server - for disconnect\n\n";
const char header[] = "ATTENTION: case of letters is important\nFor displaying available commnads enter Help\n\n";
const char pathTmp[] = "D:\\ClientReport.txt";
const char pathReport[] = "D:\\ClientReport_1.txt";
const char help_word[] = "Help\n";
const char exit_word[] = "Exit\n";
const char LaunchGame[] = "Enter coordinates\n";
char coordinates[] = "Two coordinates\n";
char Field[] = "Field was received\n";
char Statistics[] = "Get Statistics\n";
DWORD timeout = 10000;

fstream report;

bool empty_string(const std::string & s) {
	return s.empty() || s.find_first_not_of(" \t\n") == std::string::npos;
}
void DeleteSpaces() {
	string buf;
	ifstream ifs(pathTmp);
	ofstream ofs(pathReport, fstream::out | fstream::trunc);
	while (getline(ifs, buf)) {
		if (!empty_string(buf)) {
			ofs << buf << endl;
		}
	}
	ifs.close();
	ofs.close();

}
void OpenReport()
{

	report.open(pathTmp, fstream::out | fstream::trunc);
	report << "Report for working Client:\n";
	report.right;
	report << "Opertion      ";
	report << "Size      ";
	report << "Command\n";
}
void MakeRecord(bool oprt, int bytes, char*command)
{
	if (oprt)
	{
		report.right;
		report << "Send:         ";
		report << bytes;
		report << " Bytes  ";
		report << command << "\n";
	}
	else
	{
		report.right;
		report << "Receive:      ";
		report << bytes;
		report << " Bytes  ";
		report << command << "\n";
	}
}
void Stop(SOCKET &sock)
{
	report << "\nEnd of work.Succesful...";
	report.close();
	DeleteSpaces();
	closesocket(sock);
	WSACleanup();
}

int main()
{
	printf(header);
	OpenReport();
	char buff[1024] = "";
	int sendBytes, recv_bytes;

	//  1 - Winsocks library initialization
	if (WSAStartup(0x202, (WSADATA *)&buff[0]))
	{
		printf("WSAStartup error: %d\n");
			WSAGetLastError();
		return -1;
	}

	//  2 - socket opening
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)); //seting the timeout for reponse
	if (sock == INVALID_SOCKET)
	{
		printf("socket() error: %d\n", WSAGetLastError());
		report << "Socket error: %d " << WSAGetLastError();
		WSACleanup();
		return -1;
	}

	//  3 - exchange of  data packeges
	HOSTENT *hst;
	sockaddr_in dest_addr;

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);

	// detecting the node's IP address
	if (inet_addr(SERVERADDR))
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	else
		if (hst = gethostbyname(SERVERADDR))
			dest_addr.sin_addr.s_addr = ((unsigned long **)hst->h_addr_list)[0][0];
		else
		{
			printf("Unknown host: %d\n", WSAGetLastError());
			report << "Unknown host: %d\n" << WSAGetLastError();
			closesocket(sock);
			WSACleanup();
			return -1;
		}
	bool flag = false;
	while (1)
	{
		if (flag)
			cin.get();
		printf("C: ");
		//cin.clear();
		//cin.sync();
		fgets(&buff[0], sizeof(buff) - 1, stdin);

		if (!strcmp(&buff[0], exit_word))
		{
			Stop(sock);
			break;
		}
		if (!strcmp(&buff[0], help_word))
		{
			printf(help);
			flag = false;
			continue;
		}
		printf("Try to exchange packets... ");

		// Sending messages to the server
		
		sendBytes = sendto(sock, &buff[0], strlen(&buff[0]) + 1, 0, (sockaddr *)&dest_addr, sizeof(dest_addr));
		MakeRecord(true, sendBytes, buff);

		// Receive message from server
		sockaddr_in server_addr;
		int server_addr_size = sizeof(server_addr);

		recv_bytes = recvfrom(sock, &buff[0], sizeof(buff) - 1, 0, (sockaddr *)&server_addr, &server_addr_size);
		
		if (recv_bytes != SOCKET_ERROR)
		{
			printf(" // Successful\n");
		}
		else {
			if (WSAGetLastError() == WSAETIMEDOUT)
			{
				printf(" // Error: Timed out\n\n");
			}
			else
			{
				printf(" // Error\n\n");
			}
			continue;
		}
		buff[recv_bytes] = 0;
		MakeRecord(false, recv_bytes, buff);
		
		if (strcmp(buff, LaunchGame) == 0)
		{
			int y;
			char ch;
			int coord[2];
			int field[10][10];
			char buffer[1024];
			int Number_of_enemies;

			while (1)
			{
				printf(LaunchGame);
				cin >> ch >> y;
				int x = (int)(ch - 'A');
				y--;
				coord[0] = x;
				coord[1] = y;

				sendBytes = sendto(sock, (char*)&coord, sizeof(coord[0])*2, 0, (sockaddr *)&dest_addr, sizeof(dest_addr));
				MakeRecord(true, sendBytes, coordinates);

				char response[1024];

				recv_bytes = recvfrom(sock, &response[0], sizeof(buff), 0, (sockaddr *)&server_addr, &server_addr_size);
				response[recv_bytes] = 0;
				cout << response << endl;
				//cout << recv_bytes << endl;
				MakeRecord(false, recv_bytes, response);
				//cout << response << " " << recv_bytes << endl;
				
				recv_bytes = recvfrom(sock, (char*)&field, sizeof(field[0])*100, 0, (sockaddr *)&server_addr, &server_addr_size);
				MakeRecord(false, recv_bytes, Field);

				recv_bytes = recvfrom(sock, (char*)&Number_of_enemies, sizeof(int), 0, (sockaddr *)&server_addr, &server_addr_size);
				MakeRecord(false, recv_bytes, Field); // todo

				if (!Number_of_enemies)
				{
					printf("Game is end\n");
					break;
				}

				printf("Type y to proceed or q to quit\n");
				cin >> ch;
				char arh[2] = { ch, '\0' };
				sendBytes = sendto(sock, arh, sizeof(arh), 0, (sockaddr *)&dest_addr, sizeof(dest_addr));
				MakeRecord(true, sendBytes, arh);

				if (ch == 'q')
					break;

			}

			int  statistics[3]; // to do
			recv_bytes = recvfrom(sock, (char*)&statistics, sizeof(statistics[0]) * 3, 0, (sockaddr *)&server_addr, &server_addr_size);
			MakeRecord(false, recv_bytes, Statistics);

			printf("Statistics of game: Number of shots - %d, Number of hits - %d, Number of misses - %d\n", statistics[0], statistics[1], statistics[2]);

			flag = true;
		}
		else
		{
			flag = false;
			MakeRecord(false, recv_bytes, buff); // todo
			// Displaying other recieved answers
			printf("S:%s", &buff[0]);
			printf("\n");
		}
	}
}


