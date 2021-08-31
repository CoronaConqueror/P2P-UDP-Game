#pragma once
#include "SDL.h"
#include "SDL_image.h"
#include <string>
#include "Network.h"
#include <iostream>
#include <cstdlib>
#include <time.h>


const float SCREENSIZEX = 800.0f;
const float SCREENSIZEY = 600.0f;
const float PLAYERSPEED = 30.0f;
const float PLAYERSIZE = 100.0f;
const float FOODSIZE = 20.0f;

//player size 120,120
//food 40,40

class Sprite;

class Game
{
public:

	Game();
	bool Initialize();
	void RunLoop();
	void Shutdown();

	SDL_Texture* GetTexture(const std::string& fileName);

	bool InitSockets()
	{
		WSADATA WsaData;
		return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
	}

	void ShutDownSockets()
	{
		WSACleanup();
	}

	Sprite* Background;
	Sprite* Player1;
	Sprite* Player2;
	Sprite* Food;

	float deltax;
	float deltay;

	//these track the food counter if over 6 food is collected, then that player wins and the counters are reset
	int Player1Food;
	int Player2Food;
	int Player1Wins; //counts number of wins during a game
	int Player2Wins;

	time_t seed;


	//this will house the data sent over packets, clear it every time you send one
	char sendData[256];


	SDL_Rect* GetRandomFoodPosition()
	{
		SDL_Rect* box = new SDL_Rect();
		
		
		box->x = rand() % static_cast<int>((SCREENSIZEX - 55.0f));
		box->y = rand() % static_cast<int>((SCREENSIZEY - 66.0f));

		return box;
	}

	//initial function to join a game, or start hosting a game, return true if hosting, return false if joining
	bool CheckHost();


	




private:

	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();

	SDL_Window* window;

	SDL_Renderer* renderer;
	
	
	bool isRunning;

	int ticksCount;

	bool Host; // if true, then they control player1 and act as host for the connection
				//if false, then they control player2 and "depend" on the host for the game data and decisions

	//bool for checking to see if host/join state is required
	//true if need to check, false if you dont
	bool CheckIfHost;


	//for all socket operations, add the sending address into this class
	Socket* socket;


	

	//holds the return value of a packet check
	RecPacket* pkt;


};



class Sprite
{
public:

	Sprite();
	~Sprite();

	Sprite(int x, int y, int w, int h)
	{
		box = new SDL_Rect();
		box->x = x; box->y = y; box->w = w; box->h = h;
		texture = nullptr;
	}

	void SetTexture(SDL_Texture* tex)
	{
		texture = tex;
	}

	SDL_Texture* GetTexture()
	{
		return texture;
	}

	void UpdatePosition(int x, int y)
	{
		box->x = x; box->y = y;
	}

	SDL_Rect* GetPosition()
	{
		return box;
	}

private:
	SDL_Texture* texture;
	SDL_Rect* box;
};



