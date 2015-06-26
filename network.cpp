#include "SDLheaders.h"
#include "network.h"
#include "Const.h"
#include <iostream>
#include <sstream>
#include <thread>
#include "game.h"
#include "g_Game.h"

#ifdef __APPLE__
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#elif defined(WIN32)
	#include "curl/curl.h"
	#include <winsock2.h>
#endif

/// Refer to: http://gpwiki.org/index.php/SDL:Tutorial:Using_SDL_net
///           http://www.libsdl.org/projects/SDL_net/docs/SDL_net.html
///           http://beej.us/guide/bgnet/

// Contents of the web page are put into this global variable.
std::string pageSource;

// This is the callback function that is called by curl_easy_perform(curl)
size_t handle_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int numbytes = size*nmemb;
    // The data is not null-terminated, so get the last character, and replace
    // it with '\0'.
    char lastchar = *((char *) ptr + numbytes - 1);
    *((char *) ptr + numbytes - 1) = '\0';
    pageSource.append((char *)ptr);
    pageSource.append(1,lastchar);
    *((char *) ptr + numbytes - 1) = lastchar;  // Might not be necessary.
    return size*nmemb;
}

// src: http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

// src: http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

int C_Network::init()
{
    port = 8592;
	isServer = false;
	receivedlevelFromPeer = false;
    if(SDLNet_Init()==-1)
    {
        std::clog<<"ERROR: SDLNet_Init: "<<SDLNet_GetError()<<"\n";
        return false;
    }
	getThisIP();
    return true;
}

C_NetGame* C_Network::getNextAvailableGame()
{
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if(curl)
	{
        std::string pageURL = "http://www.campbelljc.com/sokoban/games.txt";
        						
		curl_easy_setopt(curl, CURLOPT_URL, pageURL.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_data);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{
			std::cout<<"curl_easy_perform() failed: "<<curl_easy_strerror(res)<<"\n";
			return NULL;
		}
		else
		{
            std::cout << "PS:" << pageSource << std::endl << std::flush;
			
			std::vector<std::string> lines = split(pageSource, '\n');
			for (int count = 0; count < lines.size(); count ++)
			{
				if (lines.at(count) == "" || lines.at(count).find(",") == std::string::npos) continue;
				
				std::cout<<"CUR LINE:"<<lines.at(count)<<std::endl;
				
				std::string ipaddr;
				ipaddr.assign(lines.at(count), lines.at(count).find("G:")+2, std::string::npos);
				ipaddr.erase(ipaddr.find(","));
				
				std::string diffstr;
				diffstr.assign(lines.at(count), lines.at(count).find("D:")+2, lines.at(count).length()-1);
				
				int diff = atoi(diffstr.c_str());
				
				std::cout<<"ip: <"<<ipaddr<<"> diff: <"<<diffstr<<">\n";
				//sscanf(lines.at(count).c_str(), "%s-%d", ipaddr, &diff);
				
				C_NetGame game(ipaddr, diff);
				// if IP == IP...
				
				bool same = false;
				for (int gameCount = 0; gameCount < games.size(); gameCount ++)
				{
					if (games.at(gameCount).ipString == game.ipString && games.at(gameCount).difficulty == game.difficulty)
					{
						same = true;
						break;
					}
				}
				if (!same)
				{
					games.push_back(game);
					std::cout<<"diff: "<< game.difficulty;
					return &games.back();
				}
			}
		}
		curl_easy_cleanup(curl);
	}
	return NULL;
}

bool C_Network::hostGame(int difficulty)
{
	getThisIP();
	
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if(curl)
	{
        std::string pageURL = "http://www.campbelljc.com/sokoban/host.php?diff="+toString(difficulty);
        						
		curl_easy_setopt(curl, CURLOPT_URL, pageURL.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_data);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{
			std::cout<<"curl_easy_perform() failed2: "<<curl_easy_strerror(res)<<"\n";
			return false;
		}
		else
		{
            std::cout << pageSource << std::endl << std::flush;
		}
		curl_easy_cleanup(curl);
		
		if (!startServer())
		{
			std::cout<<"Couldn't start server!\n";
			return false;
		}
		
		waitForClients(1);
		
		receivedlevelFromPeer = false;
		
		return true;
	}
	else return false;
}

void C_Network::removeGameFromLobby()
{
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if(curl)
	{
        std::string pageURL = "http://www.campbelljc.com/sokoban/remove.php";
        						
		curl_easy_setopt(curl, CURLOPT_URL, pageURL.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_data);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{
			std::cout<<"curl_easy_perform() failed3: "<<curl_easy_strerror(res)<<"\n";
			return;
		}
		else
		{
            std::cout << "removed game from lobby. src:" << pageSource <<"\n";
		}
		curl_easy_cleanup(curl);
	}
}

void C_Network::getThisIP()
{
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if(curl)
	{
        std::string pageURL = "http://www.campbelljc.com/sokoban/myip.php";
        						
		curl_easy_setopt(curl, CURLOPT_URL, pageURL.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_data);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{
			std::cout<<"curl_easy_perform() failed4: "<<curl_easy_strerror(res)<<"\n";
			return;
		}
		else
		{
            std::cout << pageSource << std::endl << std::flush;
			
			thisIP.port = 8592;
			thisIP.host = inet_addr(pageSource.c_str());
		}
		curl_easy_cleanup(curl);
	}
}

bool C_Network::startServer()
{
    if (SDLNet_ResolveHost(&ip, NULL, port) < 0)
	{
		std::clog<<"SDLNet_ResolveHost: "<<SDLNet_GetError()<<"\n";
		return false;
	}
	if (!(sd = SDLNet_TCP_Open(&ip)))
	{
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
	isServer = true;
	return true;
}

bool C_Network::joinGame(C_NetGame* game)
{
	std::cout<<"We are trying to connect to: " <<game->ipString<<"\n";
	receivedlevelFromPeer = false;
	return startClient(game->ipString.c_str());
}

bool C_Network::startClient(const char* host = "127.0.0.1")
{
    isServer = false;
	/* Resolve the host we are connecting to */
	if (SDLNet_ResolveHost(&ip, host, port) < 0)
	{
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		return false;
	}

	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(sd = SDLNet_TCP_Open(&ip)))
	{
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		return false;
	}
	
	std::cout<<"client started\n";
	
	return true;
}

/**
Parameters: num - number of clients to wait for
**/
void C_Network::waitForClients(int num = 1)
{
    TCPsocket client;
    while (this->csd.size() < num)
    {
		/* This check the sd if there is a pending connection.
		* If there is one, accept that, and open a new socket for communicating */
		if ((client = SDLNet_TCP_Accept(sd)))
		{
			/* Now we can communicate with the client using csd socket
			* sd will remain opened waiting other connections */

			/* Get the remote address */
			if ((remoteIP = SDLNet_TCP_GetPeerAddress(client)))
			{
				/* Print the address, converting in the host format */
				printf("Host connected: %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
				csd.push_back(client);
			}
			else
			{
				fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
			}
		}
    }
	
	std::cout <<"all clients connected!\n";
	removeGameFromLobby();
}

void C_Network::sendLevelToPeer()
{ // ref http://www.libsdl.org/projects/SDL_net/docs/demos/tcpclient.c
	char message[1024];
	message[0] = MSG_LEVEL;

	int msgcount = 1;
    for (int i = 0; i < g_Game.width; i++)
    {
        for (int j = 0; j < g_Game.height; j++)
        {
			if (g_Game.getBoardPtr()->getEntityPtr(j, i) == 0 && g_Game.getBoardPtr()->getEntityPtrG(j, i) == 0) message[msgcount] = ' ';
			else if (g_Game.getBoardPtr()->getEntityPtr(j, i) == 0) message[msgcount] = g_Game.getBoardPtr()->getEntityPtrG(j, i)->getChar();
			else message[msgcount] = g_Game.getBoardPtr()->getEntityPtr(j, i)->getChar();
			
			msgcount ++;
		}
		message[msgcount] = '\n';
		msgcount ++;
	}
	
	std::cout<<"----snd\n";
	for (int i = 0; i < 1 + (g_Game.width*g_Game.height)+g_Game.width; i ++) std::cout<<message[i];
	std::cout<<"----\n";
	
	
	int len = 1 + (g_Game.width*g_Game.height)+g_Game.width;
	printf("Sending %d: %s\n", len, message);
	
	int result;
	
	if (isServer)
	{ // send on csd[0]
		result = SDLNet_TCP_Send(csd[0],message,len);
	}
	else
	{ // send on sd
		result = SDLNet_TCP_Send(sd,message,len);
	}
	std::cout<<"sending "<<result<<"b\n";
	
	if (result < len)
	{
		printf("SDLNet_TCP_Send: %s\n",SDLNet_GetError());
	}
}

void C_Network::broadcastMoves()
{
	while(g_Game.getState() != STATE_ENDSCREEN || g_Game.movesToBroadcast.size() > 0)
	{
		if (g_Game.movesToBroadcast.size() > 0)
		{
			C_Move move = g_Game.movesToBroadcast.front();
			g_Game.movesToBroadcast.pop();
			
			char message[8];
			message[0] = (char)(((int)'0')+MSG_MOVE);
			message[1] = (char)(((int)'0')+move.pos.x);
			message[2] = (char)(((int)'0')+move.pos.y);
			message[3] = (char)(((int)'0')+move.x_);
			if (move.x_ == -1) message[3] = 'i'; // hack
			message[4] = (char)(((int)'0')+move.y_);
			if (move.y_ == -1) message[4] = 'I';
	
//			std::cout<<"----snd\n";
//			for (int i = 0; i < 8; i ++) std::cout<<message[i];
//			std::cout<<"----\n";
	
			int result;
	
			if (isServer)
			{ // send on csd[0]
				result = SDLNet_TCP_Send(csd[0],message,8);
			}
			else
			{ // send on sd
				result = SDLNet_TCP_Send(sd,message,8);
			}
			std::cout<<"bcast move,sending "<<result<<"b\n";
	
			if (result < 8)
			{
				printf("SDLNet_TCP_Send: %s\n",SDLNet_GetError());
			}
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
	}
	std::cout<<"STOPPING BCAST OF MOVES!\n";
}

/*void C_Network::broadcastGameOver()
{
	char message[8];
	message[0] = (char)(((int)'0')+MSG_GAMEOVER);
	
	int result;
	
	if (isServer)
	{ // send on csd[0]
		result = SDLNet_TCP_Send(csd[0],message,8);
	}
	else
	{ // send on sd
		result = SDLNet_TCP_Send(sd,message,8);
	}
	std::cout<<"bcast gover, sending "<<result<<"b\n";
	
	if (result < 8)
	{
		printf("SDLNet_TCP_Send: %s\n",SDLNet_GetError());
	}
}*/

void C_Network::receiveLevelFromPeer()
{
	TCPsocket* sock;
	if (isServer) sock = &csd[0];
	else sock = &sd;

	char message[1024];
	while(true)
	{
		std::cout<<"-";
		if (SDLNet_TCP_Recv(*sock, message, 1024) > 0)
		{
			break;
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
	}
	
	if (message[0] != MSG_LEVEL)
	{
		std::cout<<"some weird msg received when waiting for level\n";
		return receiveLevelFromPeer();
	}
	
	std::cout<<"RECEIVED!\n";
	
	for(int count = 0; count < 1023; count ++) message[count] = message[count+1];
	
	g_Game.loadOpponentBoardFromMessage(message);
	receivedlevelFromPeer = true;
}

void C_Network::receiveMoves()
{
	TCPsocket* sock;
	if (isServer) sock = &csd[0];
	else sock = &sd;

	char message[8];
//	for (int i = 0; i < 8; i ++) message[i] = 0;
	
	while(g_Game.getState() != STATE_ENDSCREEN)
	{
		std::cout<<"-"<<g_Game.oppGameFinished<<","<<g_Game.done<<"-";
		if (SDLNet_TCP_Recv(*sock, message, 8) > 0)
		{
			int msg[5];
			for (int i = 0; i < 5; i++)
			{
				if ( (i == 3 && message[i] == 'i') || (i == 4 && message[i] == 'I') ) msg[i] = -1;
				else msg[i] = message[i] - '0';
	//			std::cout<<"message["<<i<<"] = "<<msg[i]<<"\n";
			}
			
///			bool corrupt = false;
//			for (int i = 0; i < 5; i++) if (msg[i] == -1) corrupt = true;
//			if (corrupt) { std::cout<<"corrupt\n"; continue; }
			
			switch(msg[0])
			{
				case MSG_MOVE:
				{
					C_Coord posToMove(msg[1], msg[2]);
					int x_ = msg[3];
					int y_ = msg[4];
					
					g_Game.getOpponentBoardPtr()->getEntityPtr(posToMove.x, posToMove.y)->move(x_, y_);
					
					break;
				}
			/*	case MSG_GAMEOVER:
				{
					g_Game.oppGameFinished = true;
					std::cout<<"Opp game finished\n";
					
					return;
				}*/
			}
		}
//		for (int i = 0; i < 8; i ++) message[i] = '0';
		std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
		std::cout<<"state:"<<g_Game.getState()<<"\n";
	}
	std::cout<<"END RCV MOVES (BOTH GAMES DONE)\n";
}

/*void C_Network::parseMessages()
{
    if (isServer)
    {
        //Make sure all clients get a chance to have their message get through
        static int last = 0;
        for (int i = last, j = 0; j < csd.size(); i = (i+1)%csd.size(), j++)
        {
            if (SDLNet_TCP_Recv(csd[i], buffer, 512) > 0)
            {
                //Process one message at a time
                last = i;
                return;
            }
        }
    }
    else
    {
        if (SDLNet_TCP_Recv(sd, buffer, 512) > 0)
        {

        }
    }
}*/

void C_Network::cleanUp()
{
    SDLNet_Quit();
}
