// Server.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#define _WINSOCK_DEPCRECATED 
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <fstream>
#include <iomanip>
#include <windows.h>
#include <Mswsock.h>
#include <string>
#include <iostream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <shlwapi.h>
#include <vector>
#include <stdlib.h> 
#include <time.h>

using namespace std;

#define PORT 1041

const char pathTmp[] = "D:\\ServerReport.txt";
const char pathReport[] = "D:\\ServerReport_1.txt";
const char who[] = "Who\n";
const char stop[] = "Stop server\n";
const char LaunchGame[] = "Start Game\n";
char message[] = "Enter coordinates\n";
char coordinates[] = "Two coordinates\n";
char hit[] = "Hit\n";
char miss[] = "Miss\n";
char Field[] = "Field was sent\n";
char quit[] = "Type y or q\n";
char Statistics[] = "Statistics was sent\n";
char GameIsEnd[] = "Game is end\n";
fstream report;

void GenerateField(int pole[10][10]);
void PrintField(int field[10][10]);

bool empty_string(const std::string & s) 
{
	return s.empty() || s.find_first_not_of(" \t\n") == std::string::npos;
}

void DeleteSpaces() 
{
	string buf;
	ifstream ifs(pathTmp);
	ofstream ofs(pathReport, fstream::out | fstream::trunc);
	while (getline(ifs, buf)) {
		if (!empty_string(buf)) {
			ofs << buf << std::endl;
		}
	}
	ifs.close();
	ofs.close();

}

void OpenReport()
{

	report.open(pathTmp, fstream::out | fstream::trunc);
	report << "Report for working server:\n";
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
	OpenReport();
	//  connecting library
	WSADATA WSA_SOCKET;
	if (WSAStartup(0x202, &WSA_SOCKET))
	{
		printf("WSAStartup error: %d\n", WSAGetLastError());
		return -1;
	}

	// socket creation
	SOCKET sock;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("Socket() error: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	// binding a socket to a local address
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	//local_addr.sin_addr.s_addr = inet_addr("10.1.3.206");
	local_addr.sin_port = htons(PORT);
	//cout << inet_ntoa(local_addr.sin_addr) << " " << ntohs(local_addr.sin_port) << endl;
	if (bind(sock, (sockaddr *)&local_addr, sizeof(local_addr)))
	{
		printf("bind error: %d\n", WSAGetLastError());
		report << "Bind error: " << WSAGetLastError() << "\nServer stopped.\n";
		closesocket(sock);
		WSACleanup();
		return -1;
	}
	// processing packets sent by customer
	char buff[1024];
	while (1)
	{
		sockaddr_in client_addr;
		int client_addr_size = sizeof(client_addr);
		int bsize = recvfrom(sock, &buff[0], sizeof(buff) - 1, 0, (sockaddr *)&client_addr, &client_addr_size);
		if (bsize == SOCKET_ERROR)
		{
			printf("recvfrom() error: %d\n", WSAGetLastError());
			report << "Receiving error:" << WSAGetLastError() << "\n";
		}

		buff[bsize] = 0;
		MakeRecord(false, bsize, buff);

		HOSTENT *hst;
		hst = gethostbyaddr((char *)&client_addr.sin_addr, 4, AF_INET);
		printf("+%s [%s:%d] new DATAGRAM!\n", (hst) ? hst->h_name : "Unknown host", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		printf("C=>S:%s\n", &buff[0]);

		if (strcmp(buff, stop) == 0)//повертає 0 якщо buff = stop
		{
			char result[] = "Result Succesful\n";
			int bytes = sendto(sock, &result[0], strlen(result) + 1, 0, (sockaddr *)&client_addr, sizeof(client_addr));
			MakeRecord(true, bytes, result);
			Stop(sock);
			break;
		}
		//command who
		if (strcmp(buff, who) == 0)
		{
			char result[] = "Sergey Mykytchyn, variant 16, Sea Battle\n";
			int bytes = sendto(sock, &result[0], strlen(result) + 1, 0, (sockaddr *)&client_addr, sizeof(client_addr));
			MakeRecord(true, bytes, result);
		}
		else if (strcmp(buff, LaunchGame) == 0) {

			printf("Game Started\n");

			int Number_of_shots = 0;
			int Number_of_hits = 0;
			int Number_of_misses = 0;
			int Number_of_enemies = 20;
			int coord[2];
			int field[10][10];
			int bytes, bsize;

			GenerateField(field);
			PrintField(field);

			bytes = sendto(sock, &message[0], strlen(message) + 1, 0, (sockaddr *)&client_addr, sizeof(client_addr));
			MakeRecord(true, bytes, message);

			while (Number_of_enemies)
			{

				bsize = recvfrom(sock, (char *)&coord, sizeof(coord[0])*2, 0, (sockaddr *)&client_addr, &client_addr_size);
				MakeRecord(false, bsize, coordinates);
				Number_of_shots++;

				char th = coord[0] + 'A';

				printf("Coordinates entered %c %d \n", th, coord[1]+1);

				if (field[coord[0]][coord[1]] == 1)
				{
					cout << hit;
					bytes = sendto(sock, &hit[0], strlen(hit) + 1, 0, (sockaddr *)&client_addr, sizeof(client_addr));
					MakeRecord(true, bytes, hit);
					Number_of_hits++;
					Number_of_enemies--;
					field[coord[0]][coord[1]] = 2;
					//cout << "here 1\n";
				}
				else 
				{
					cout << miss;
					bytes = sendto(sock, &miss[0], strlen(miss) + 1, 0, (sockaddr *)&client_addr, sizeof(client_addr));
					//cout << "Bytes sent miss " << bytes << endl;
					MakeRecord(true, bytes, miss);
					field[coord[0]][coord[1]] = 8;
					Number_of_misses++;
					//cout << "here 2\n";
				}

				PrintField(field);
				printf("\n");

			    bytes = sendto(sock, (char *)&field, sizeof(field[0])*100, 0, (sockaddr *)&client_addr, sizeof(client_addr));
				MakeRecord(true, bytes, Field);

				bytes = sendto(sock, (char *)&Number_of_enemies, sizeof(int), 0, (sockaddr *)&client_addr, sizeof(client_addr));
				MakeRecord(true, bytes, Field); // todo

				if (!Number_of_enemies)
					break;

				char arh[2];

				bsize = recvfrom(sock, arh, sizeof(arh), 0, (sockaddr *)&client_addr, &client_addr_size); // todo
				MakeRecord(false, bsize, arh);
				cout << arh << endl;
				cout << bsize << endl;

				if (arh[0]=='q')
				{
					break;
				}
			}

			printf("Game End\n");

			int statistics[3] = { Number_of_shots, Number_of_hits, Number_of_misses };
			printf("Statistics of game: Number of shots - %d, Number of hits - %d, Number of misses - %d\n", Number_of_shots, Number_of_hits, Number_of_misses);

			bytes = sendto(sock, (char *)&statistics, sizeof(statistics[0]) * 3, 0, (sockaddr *)&client_addr, sizeof(client_addr));
			MakeRecord(true, bytes, Statistics);

		}else{

			char error[] = "Error : wrong command!\n";
			int sendBytes = sendto(sock, &error[0], strlen(error) + 1, 0, (sockaddr *)&client_addr, sizeof(client_addr));
			MakeRecord(true, sendBytes, error);
		}
	}
}

void GenerateField(int pole[10][10])
{
	int i, j, f, k, r;
	int rand_chislo, rand_chislo1, ts, s;
	srand(time(NULL));

	for (i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			pole[i][j] = 0;

	do
	{
		f = 0;
		rand_chislo = 0 + rand() % 10;
		rand_chislo1 = 0 + rand() % 10;

		ts = 1 + rand() % 4;

		if (ts == 1) { for (i = 0; i < 3; i++) if (rand_chislo == i)f = 1; }
		if (ts == 2) { for (i = 7; i < 10; i++) if (rand_chislo == i)f = 1; }
		if (ts == 3) { for (i = 0; i < 3; i++) if (rand_chislo1 == i)f = 1; }
		if (ts == 4) { for (i = 7; i < 10; i++) if (rand_chislo1 == i)f = 1; }

	} while (f == 1);

	switch (ts)
	{
	case 1: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo - 1][rand_chislo1] = 1;
		pole[rand_chislo - 2][rand_chislo1] = 1; pole[rand_chislo - 3][rand_chislo1] = 1; break;
	case 2: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo + 1][rand_chislo1] = 1;
		pole[rand_chislo + 2][rand_chislo1] = 1; pole[rand_chislo + 3][rand_chislo1] = 1; break;
	case 3: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo][rand_chislo1 - 1] = 1;
		pole[rand_chislo][rand_chislo1 - 2] = 1; pole[rand_chislo][rand_chislo1 - 3] = 1; break;
	case 4: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo][rand_chislo1 + 1] = 1;
		pole[rand_chislo][rand_chislo1 + 2] = 1; pole[rand_chislo][rand_chislo1 + 3] = 1; break;
	}

	for (k = 0; k < 3; k++)
	{
		do {
			f = 0;
			rand_chislo = 0 + rand() % 10;
			rand_chislo1 = 0 + rand() % 10;

			ts = 1 + rand() % 4;

			if (ts == 1) {
				if (rand_chislo != 0) {
					for (i = rand_chislo - 2; i < rand_chislo + 2; i++)
						for (j = rand_chislo1 - 1; j <= rand_chislo1 + 1; j++)
						{
							if (pole[i][j] != 0) { f = 1; }
						}
				}
				else f = 1;
			}
			if (ts == 2) {
				if (rand_chislo != 9)
				{
					for (i = rand_chislo - 1; i <= rand_chislo + 2; i++)
						for (j = rand_chislo1 - 1; j <= rand_chislo1 + 1; j++)
						{
							if (pole[i][j] != 0) { f = 1; }
						}
				}
				else f = 1;
			}
			if (ts == 3)
			{
				if (rand_chislo1 != 0)
				{
					for (i = rand_chislo - 1; i <= rand_chislo + 1; i++)
						for (j = rand_chislo1 - 2; j <= rand_chislo1 + 1; j++)
						{
							if (pole[i][j] != 0) { f = 1; }
						}
				}
				else f = 1;
			}
			if (ts == 4)
			{
				if (rand_chislo1 != 9)
				{
					for (i = rand_chislo - 1; i <= rand_chislo + 1; i++)
						for (j = rand_chislo1 - 1; j <= rand_chislo1 + 2; j++)
						{
							if (pole[i][j] != 0) { f = 1; }
						}
				}
				else f = 1;
			}
		} while (f == 1);
		switch (ts)
		{
		case 1: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo - 1][rand_chislo1] = 1; break;
		case 2: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo + 1][rand_chislo1] = 1; break;
		case 3: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo][rand_chislo1 - 1] = 1; break;
		case 4: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo][rand_chislo1 + 1] = 1; break;
		}


	}

	for (k = 0; k < 2; k++)
	{
		do
		{
			f = 0;
			rand_chislo = 0 + rand() % 10;
			rand_chislo1 = 0 + rand() % 10;

			ts = 1 + rand() % 4;

			if (ts == 1)
			{
				if (rand_chislo != 0 && rand_chislo != 1)
				{
					for (i = rand_chislo - 3; i < rand_chislo + 2; i++)
						for (j = rand_chislo1 - 1; j <= rand_chislo1 + 1; j++)
						{
							if (pole[i][j] != 0) { f = 1; }
						}
				}
				else f = 1;
			}
			if (ts == 2)
			{
				if (rand_chislo != 9 && rand_chislo != 8)
				{
					for (i = rand_chislo - 1; i <= rand_chislo + 3; i++)
						for (j = rand_chislo1 - 1; j <= rand_chislo1 + 1; j++)
						{
							if (pole[i][j] != 0) { f = 1; }
						}
				}
				else f = 1;
			}
			if (ts == 3)
			{
				if (rand_chislo1 != 0 && rand_chislo1 != 1)
				{
					for (i = rand_chislo - 1; i <= rand_chislo + 1; i++)
						for (j = rand_chislo1 - 3; j <= rand_chislo1 + 1; j++)
						{
							if (pole[i][j] != 0) { f = 1; }
						}
				}
				else f = 1;
			}
			if (ts == 4)
			{
				if (rand_chislo1 != 9 && rand_chislo1 != 8)
				{
					for (i = rand_chislo - 1; i <= rand_chislo + 1; i++)
						for (j = rand_chislo1 - 1; j <= rand_chislo1 + 3; j++)
						{
							if (pole[i][j] != 0) { f = 1; }
						}
				}
				else f = 1;
			}
		} while (f == 1);

		switch (ts)
		{
		case 1: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo - 1][rand_chislo1] = 1; pole[rand_chislo - 2][rand_chislo1] = 1; break;
		case 2: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo + 1][rand_chislo1] = 1; pole[rand_chislo + 2][rand_chislo1] = 1; break;
		case 3: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo][rand_chislo1 - 1] = 1; pole[rand_chislo][rand_chislo1 - 2] = 1; break;
		case 4: pole[rand_chislo][rand_chislo1] = 1; pole[rand_chislo][rand_chislo1 + 1] = 1; pole[rand_chislo][rand_chislo1 + 2] = 1; break;
		}
	}

	for (i = 0; i < 4; i++)
	{
		do
		{
			rand_chislo = 0 + rand() % 10;
			rand_chislo1 = 0 + rand() % 10;
		} while (pole[rand_chislo][rand_chislo1] != 0 || pole[rand_chislo + 1][rand_chislo1] != 0 || pole[rand_chislo - 1][rand_chislo1] != 0 || pole[rand_chislo][rand_chislo1 + 1] != 0 || pole[rand_chislo][rand_chislo1 - 1] != 0 || pole[rand_chislo + 1][rand_chislo1 + 1] != 0 || pole[rand_chislo - 1][rand_chislo1 - 1] != 0 || pole[rand_chislo + 1][rand_chislo1 - 1] != 0 || pole[rand_chislo - 1][rand_chislo1 + 1] != 0);

		pole[rand_chislo][rand_chislo1] = 1;
	}
}

void PrintField(int field[10][10])
{
	printf("  ");
	for (int i = 1; i <= 10; i++)
		printf("%d ", i);
	printf("\n");
	char ch;
	for (int i = 0; i < 10; i++)
	{
		ch = 'A' + i;
		printf("%c ", ch);
		for (int j = 0; j < 10; j++)
			printf("%d ", field[i][j]);
		printf("\n");
	}
}