#include "Game.h"

Game::Game()
{
	window = nullptr;
	renderer = nullptr;
	isRunning = true;
	ticksCount = 0;
	Host = true;
	deltax = 0.0f;
	deltay = 0.0f;
	time(&seed); //set random seed
	srand(static_cast<unsigned int>(seed));
	CheckIfHost = true;
	Player1Wins = 0; Player2Wins = 0;

}

bool Game::CheckHost()
{
	//loop this until a packet it recieved from other player
	//the joining player will send a packet
	//the host will recieve that packet and send their own acknowledgement
	//the first to recieve a packet will be host
	char responce = 'a';

	while (1)
	{
		std::cout << "\n Are you hosting a game? Or are you joining a game? Enter 'h' or 'j' respectively: ";
		std::cin >> responce;
		std::cin.clear(); std::cin.ignore();

		if (responce == 'h')
			break;
		if (responce == 'j')
			break;
	}

	if (responce == 'h')
	{
		//this game will act as host, so wait for a packet with msg type 1

		pkt = socket->LoopReceivePacket();

		std::cout << "Recieved a packet: " << pkt->packet_data << std::endl;
		//socket->SetAddressFormatted(pkt->from_address);

		std::cout << "address is " << pkt->from_address << std::endl;

		sendData[0] = '\0';

		strcpy_s(sendData, "1");

	
		socket->SendPacket(sendData);


		return true;

	}
	else if (responce == 'j')
	{
		//this game instance will join a game, so send a packet to host address, then wait to recieve an msg type 1
		unsigned int a, b, c, d;


		std::cout << "\n Enter the ip address of the other player in this format (# # # #), (EX: 127 0 0 1): ";
		std::cin >> a >> b >> c >> d;
		std::cin.clear(); std::cin.ignore();


		socket->SetAddressRaw(a, b, c, d);

		sendData[0] = '\0';

		strcpy_s(sendData, "1");

		socket->SendPacket(sendData);

		pkt = socket->LoopReceivePacket();



		return false;
	}

}


bool Game::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("P2P Game", 250, 200, SCREENSIZEX, SCREENSIZEY, 0);
	if (!window)
	{
		SDL_Log("Unable to create window: %s", SDL_GetError());
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
	{
		SDL_Log("Failed to create renderer: &s", SDL_GetError());
		return false;
	}

	if (IMG_Init(IMG_INIT_PNG) == 0)
	{
		SDL_Log("Unable to intialize SDL_image: %s", SDL_GetError());
		return false;
	}

	if (InitSockets() == false)
	{
		std::cout << "Failed to initialize sockets!" << std::endl;
		return false;
	}


	socket = new Socket(30000); //create a new socket with port 30000, add address later


	return true;

}


void Game::RunLoop()
{
	while (isRunning)
	{

		//add hosting and joining logic here, when someone leaves the other must go to a restart state

		if (CheckIfHost == true)
		{
			if (CheckHost() == true)
			{
				//this game instance will act as host, sending its data to player2, player controls Player1
				Host = true;



				SDL_Texture* tex = GetTexture("images/Background.png");

				Background = new Sprite(0, 0, 800, 600);
				Background->SetTexture(tex);

				tex = GetTexture("images/Player1.png");

				Player1 = new Sprite(250, 200, PLAYERSIZE, PLAYERSIZE);
				Player1->SetTexture(tex);



				tex = GetTexture("images/Player2.png");

				Player2 = new Sprite(650, 200, PLAYERSIZE, PLAYERSIZE);
				Player2->SetTexture(tex);


				tex = GetTexture("images/Food.png");

				Food = new Sprite(GetRandomFoodPosition()->x, GetRandomFoodPosition()->y, FOODSIZE, FOODSIZE);
				Food->SetTexture(tex);


				sendData[0] = '\0';

				strcpy_s(sendData, "3 ");


				char s1[10], s2[10];

				_itoa_s(Food->GetPosition()->x, s1, 10);
				_itoa_s(Food->GetPosition()->y, s2, 10);

				strcat_s(sendData, s1);
				strcat_s(sendData, " ");
				strcat_s(sendData, s2);

				//std::cout << sendData << std::endl;


				socket->SendPacket(sendData);




				CheckIfHost = false;
			}
			else
			{
				//this game instance will act as a joined player, recieving critical data from host, and sending its location , player controls player 2
				Host = false;



				SDL_Texture* tex = GetTexture("images/Background.png");

				Background = new Sprite(0, 0, 800, 600);
				Background->SetTexture(tex);

				tex = GetTexture("images/Player1.png");

				Player1 = new Sprite(250, 200, PLAYERSIZE, PLAYERSIZE);
				Player1->SetTexture(tex);


				tex = GetTexture("images/Player2.png");

				Player2 = new Sprite(50, 50, PLAYERSIZE, PLAYERSIZE);
				Player2->SetTexture(tex);



				tex = GetTexture("images/Food.png");

				Food = new Sprite(-100, -100, FOODSIZE, FOODSIZE);
				Food->SetTexture(tex);




				CheckIfHost = false;
			}
		}
	
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::Shutdown()
{
	//add code here to send a final packet to other player to send them to host/join state

	sendData[0] = '\0';

	strcpy_s(sendData, "4");


	socket->SendPacket(sendData);



	ShutDownSockets();

	IMG_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


SDL_Texture* Game::GetTexture(const std::string& fileName)
{


	SDL_Texture* tex = nullptr;

	//load from file
	SDL_Surface* surf = IMG_Load(fileName.c_str());
	if (!surf)
	{
		SDL_Log("Failed to load texture file %s", fileName.c_str());
		return nullptr;
	}

	//create texture from surface
	tex = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
	if (!tex)
	{
		SDL_Log("Failed to convert surface to texture for &s", fileName.c_str());
		return nullptr;
	}

	return tex;
}


void Game::ProcessInput()
{

	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			isRunning = false;
			break;
		
		}
	}


	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE])
	{
		isRunning = false;
	}

}

void Game::UpdateGame()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), ticksCount + 16))
	{ //enforce 60 fps
	}

	// /1000.0f
	float deltaTime = (SDL_GetTicks() - ticksCount) / 1000.0f;

	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}
	ticksCount = SDL_GetTicks();

	
	if (Host == true) //player is hosting and controls player 1
	{


		//recieve packets, transform the data, and update game based on the packet type


		while (1) //loops until a nullptr is recieved by Recieve packet
		{
			pkt = socket->ReceivePacket();

			if (pkt == nullptr)
			{
				break;
			}
			else
			{
				//process data based on message type
				char type = ' ';
				int i = 2;

				type = *pkt->packet_data;
				
				//std::cout << pkt->packet_data << std::endl;

				if (type == '1') // acknowledgement packet, do nothing with it
				{
					break;
				}

				if (type == '2') //player position packet, update other players position, in this case player2
				{
					char xChar[3] = "\0";
					char yChar[3] = "\0";

					xChar[0] = pkt->packet_data[i]; i++;

					if (pkt->packet_data[i] == ' ')
					{
						i++;

						yChar[0] = pkt->packet_data[i]; i++;
						yChar[1] = pkt->packet_data[i]; i++;
						yChar[2] = pkt->packet_data[i];

						Player2->UpdatePosition(atoi(xChar), atoi(yChar));
						break;
					}
					
					xChar[1] = pkt->packet_data[i]; i++;

					if (pkt->packet_data[i] == ' ')
					{
						i++;

						yChar[0] = pkt->packet_data[i]; i++;
						yChar[1] = pkt->packet_data[i]; i++;
						yChar[2] = pkt->packet_data[i];

						Player2->UpdatePosition(atoi(xChar), atoi(yChar));
						break;
					}
				
					xChar[2] = pkt->packet_data[i]; i++;
					i++;

					yChar[0] = pkt->packet_data[i]; i++;
					yChar[1] = pkt->packet_data[i]; i++;
					yChar[2] = pkt->packet_data[i];

					Player2->UpdatePosition(atoi(xChar), atoi(yChar));
					break;

				}

				if (type == '3') // food position packet, means the other player collected food, increment other players food counter, and update food position
				{
					Player2Food++;

					std::cout << "Player 2 Food: " << Player2Food << std::endl;



					char xChar[3] = "\0";
					char yChar[3] = "\0";

					xChar[0] = pkt->packet_data[i]; i++;

					if (pkt->packet_data[i] == ' ')
					{
						i++;

						yChar[0] = pkt->packet_data[i]; i++;
						yChar[1] = pkt->packet_data[i]; i++;
						yChar[2] = pkt->packet_data[i];

						Food->UpdatePosition(atoi(xChar), atoi(yChar));
						break;
					}
					


					xChar[1] = pkt->packet_data[i]; i++;

					if (pkt->packet_data[i] == ' ')
					{
						i++;

						yChar[0] = pkt->packet_data[i]; i++;
						yChar[1] = pkt->packet_data[i]; i++;
						yChar[2] = pkt->packet_data[i];

						Food->UpdatePosition(atoi(xChar), atoi(yChar));
						break;
					}

					

					xChar[2] = pkt->packet_data[i]; i++;
					i++;

					yChar[0] = pkt->packet_data[i]; i++;
					yChar[1] = pkt->packet_data[i]; i++;
					yChar[2] = pkt->packet_data[i];



					Food->UpdatePosition(atoi(xChar), atoi(yChar));


					break;
				}

				if (type == '4') // means the other player quit their game, set this game into the check Host/join state, reset all counters
				{
					CheckIfHost = true;

					Player1Food = 0;
					Player2Food = 0;
					Player1Wins = 0;
					Player2Wins = 0;


					break;
				}


			}



		}










		SDL_Rect* currPos = Player1->GetPosition();



		const Uint8* state = SDL_GetKeyboardState(NULL);
		if (state[SDL_SCANCODE_W])
		{
			//w is pressed
			deltay -= PLAYERSPEED * deltaTime;

		}
		else if (state[SDL_SCANCODE_A])
		{
			//a is pressed
			deltax -= PLAYERSPEED * deltaTime;
		}
		else if (state[SDL_SCANCODE_S])
		{
			//s is pressed
			deltay += PLAYERSPEED * deltaTime;

		}
		else if (state[SDL_SCANCODE_D])
		{
			//d is pressed
			deltax += PLAYERSPEED * deltaTime;
		}

		Player1->UpdatePosition(currPos->x + deltax, currPos->y + deltay);


		if (Player1->GetPosition()->x + Player1->GetPosition()->w > SCREENSIZEX)
		{
			//hit right boundry
			Player1->UpdatePosition(SCREENSIZEX - Player1->GetPosition()->w, Player1->GetPosition()->y);
			deltax = 0.0f;
		}

		if (Player1->GetPosition()->y + Player1->GetPosition()->h > SCREENSIZEY)
		{
			//hit bottom boundry
			Player1->UpdatePosition(Player1->GetPosition()->x, SCREENSIZEY - Player1->GetPosition()->h);
			deltay = 0.0f;
		}

		if (Player1->GetPosition()->x < 0)
		{
			//hit left boundry
			Player1->UpdatePosition(0, Player1->GetPosition()->y);
			deltax = 0.0f;
		}

		if (Player1->GetPosition()->y < 0)
		{
			//hit top boundry
			Player1->UpdatePosition(Player1->GetPosition()->x, 0);
			deltay = 0.0f;
		}



		//food collision logic

		while (SDL_HasIntersection(Player1->GetPosition(), Food->GetPosition()) == true)
		{
			//player has collided with the food, increment P1 counter, Change food position, and send new position to Player 2

			Player1Food++;

			std::cout << "Player 1 Food: " << Player1Food << std::endl;

			Food->UpdatePosition(GetRandomFoodPosition()->x, GetRandomFoodPosition()->y);

			//ADD code here to send a packet with the position to Player 2
			

			
			sendData[0] = '\0';

			strcpy_s(sendData, "3 ");


			char s1[10], s2[10];

			_itoa_s(Food->GetPosition()->x, s1, 10);
			_itoa_s(Food->GetPosition()->y, s2, 10);

			strcat_s(sendData, s1);
			strcat_s(sendData, " ");
			strcat_s(sendData, s2);

			//std::cout << sendData << std::endl;

			
			socket->SendPacket(sendData);



			break;
		}





		//send Host's position to player 2

		sendData[0] = '\0';

		strcpy_s(sendData, "2 ");


		char s1[10], s2[10];

		_itoa_s(Player1->GetPosition()->x, s1, 10);
		_itoa_s(Player1->GetPosition()->y, s2, 10);

		strcat_s(sendData, s1);
		strcat_s(sendData, " ");
		strcat_s(sendData, s2);

		//std::cout << sendData << std::endl;

		socket->SendPacket(sendData);






	}
	else //player is joining a game, so they control Player 2 and Player 1 sends their location and all other data
	{



		while (1) //loops until a nullptr is recieved by Recieve packet
		{
			pkt = socket->ReceivePacket();


			if (pkt == nullptr)
			{
				break;
			}
			else
			{
				//process data based on message type
				char type = ' ';
				int i = 2;

				type = *pkt->packet_data;



				if (type == '1') // acknowledgement packet, do nothing with it
				{
					break;
				}

				if (type == '2') //player position packet, update other players position, in this case player1
				{


					char xChar[3] = "\0";
					char yChar[3] = "\0";

					xChar[0] = pkt->packet_data[i]; i++;

					if (pkt->packet_data[i] == ' ')
					{
						i++;

						yChar[0] = pkt->packet_data[i]; i++;
						yChar[1] = pkt->packet_data[i]; i++;
						yChar[2] = pkt->packet_data[i];

						Player1->UpdatePosition(atoi(xChar), atoi(yChar));


						break;
					}


					xChar[1] = pkt->packet_data[i]; i++;

					if (pkt->packet_data[i] == ' ')
					{
						i++;

						yChar[0] = pkt->packet_data[i]; i++;
						yChar[1] = pkt->packet_data[i]; i++;
						yChar[2] = pkt->packet_data[i];

						Player1->UpdatePosition(atoi(xChar), atoi(yChar));


						break;
					}

					xChar[2] = pkt->packet_data[i]; i++;
					i++;

					yChar[0] = pkt->packet_data[i]; i++;
					yChar[1] = pkt->packet_data[i]; i++;
					yChar[2] = pkt->packet_data[i];


					Player1->UpdatePosition(atoi(xChar), atoi(yChar));


					break;
				}

				if (type == '3') // food position packet, means the other player collected food, increment other players food counter, and update food position
				{
					Player1Food++;

					std::cout << "Player 1 Food: " << Player1Food << std::endl;

					char xChar[3] = "\0";
					char yChar[3] = "\0";

					xChar[0] = pkt->packet_data[i]; i++;

					if (pkt->packet_data[i] == ' ')
					{
						i++;

						yChar[0] = pkt->packet_data[i]; i++;
						yChar[1] = pkt->packet_data[i]; i++;
						yChar[2] = pkt->packet_data[i];

						Food->UpdatePosition(atoi(xChar), atoi(yChar));


						break;
					}


					xChar[1] = pkt->packet_data[i]; i++;

					if (pkt->packet_data[i] == ' ')
					{
						i++;

						yChar[0] = pkt->packet_data[i]; i++;
						yChar[1] = pkt->packet_data[i]; i++;
						yChar[2] = pkt->packet_data[i];

						Food->UpdatePosition(atoi(xChar), atoi(yChar));


						break;
					}

					xChar[2] = pkt->packet_data[i]; i++;
					i++;

					yChar[0] = pkt->packet_data[i]; i++;
					yChar[1] = pkt->packet_data[i]; i++;
					yChar[2] = pkt->packet_data[i];


					Food->UpdatePosition(atoi(xChar), atoi(yChar));


					break;
				}

				if (type == '4') // means the other player quit their game, set this game into the check Host/join state, reset all counters
				{
					CheckIfHost = true;

					Player1Food = 0;
					Player2Food = 0;
					Player1Wins = 0;
					Player2Wins = 0;

					break;
				}


			}



		}























		SDL_Rect* currPos = Player2->GetPosition();



		const Uint8* state = SDL_GetKeyboardState(NULL);
		if (state[SDL_SCANCODE_W])
		{
			//w is pressed
			deltay -= PLAYERSPEED * deltaTime;

		}
		else if (state[SDL_SCANCODE_A])
		{
			//a is pressed
			deltax -= PLAYERSPEED * deltaTime;
		}
		else if (state[SDL_SCANCODE_S])
		{
			//s is pressed
			deltay += PLAYERSPEED * deltaTime;

		}
		else if (state[SDL_SCANCODE_D])
		{
			//d is pressed
			deltax += PLAYERSPEED * deltaTime;
		}

		Player2->UpdatePosition(currPos->x + deltax, currPos->y + deltay);


		if (Player2->GetPosition()->x + Player2->GetPosition()->w > SCREENSIZEX)
		{
			//hit right boundry
			Player2->UpdatePosition(SCREENSIZEX - Player2->GetPosition()->w, Player2->GetPosition()->y);
			deltax = 0.0f;
		}

		if (Player2->GetPosition()->y + Player2->GetPosition()->h > SCREENSIZEY)
		{
			//hit bottom boundry
			Player2->UpdatePosition(Player2->GetPosition()->x, SCREENSIZEY - Player2->GetPosition()->h);
			deltay = 0.0f;
		}

		if (Player2->GetPosition()->x < 0)
		{
			//hit left boundry
			Player2->UpdatePosition(0, Player2->GetPosition()->y);
			deltax = 0.0f;
		}

		if (Player2->GetPosition()->y < 0)
		{
			//hit top boundry
			Player2->UpdatePosition(Player2->GetPosition()->x, 0);
			deltay = 0.0f;
		}



		//food collision logic

		while (SDL_HasIntersection(Player2->GetPosition(), Food->GetPosition()) == true)
		{
			//player has collided with the food, increment P1 counter, Change food position, and send new position to other player

			Player2Food++;


			std::cout << "Player 2 Food: " << Player2Food << std::endl;

		

			Food->UpdatePosition(GetRandomFoodPosition()->x, GetRandomFoodPosition()->y);

			//ADD code here to send a packet with the position to Player 1


			sendData[0] = '\0';

			strcpy_s(sendData, "3 ");


			char s1[10], s2[10];

			_itoa_s(Food->GetPosition()->x, s1, 10);
			_itoa_s(Food->GetPosition()->y, s2, 10);

			strcat_s(sendData, s1);
			strcat_s(sendData, " ");
			strcat_s(sendData, s2);

			//std::cout << sendData << std::endl;


			socket->SendPacket(sendData);


			break;
		}




		//send joined player sends position to player1

		sendData[0] = '\0';

		strcpy_s(sendData, "2 ");


		char s1[10], s2[10];

		_itoa_s(Player2->GetPosition()->x, s1, 10);
		_itoa_s(Player2->GetPosition()->y, s2, 10);

		strcat_s(sendData, s1);
		strcat_s(sendData, " ");
		strcat_s(sendData, s2);

		// "2 100 100"

		//std::cout << sendData << std::endl;

		socket->SendPacket(sendData);






	}








	//check for victory condition

	if (Player1Food > 9)
	{
		//player 1 wins, reset all food counters, and display a message in console stating victory
		Player1Food = 0; Player2Food = 0;
		Player1Wins++;

		std::cout << "Player 1 wins this match!!  The score is now: " << Player1Wins << "-" << Player2Wins << std::endl << " A new match has begun!" << std::endl;

	}


	if (Player2Food > 9)
	{
		//player 2 wins, reset all food counters, increment win counter, and display a message in console stating victory
		Player1Food = 0; Player2Food = 0;
		Player2Wins++;

		std::cout << "Player 2 wins this match!! The score is now: " << Player1Wins << "-" << Player2Wins << std::endl << " A new match has begun!" << std::endl;

	}






}
void Game::GenerateOutput()
{

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_RenderCopy(renderer, Background->GetTexture(), NULL, Background->GetPosition());
	SDL_RenderCopy(renderer, Player1->GetTexture(), NULL, Player1->GetPosition());

	//if player2 is connected render them
	SDL_RenderCopy(renderer, Player2->GetTexture(), NULL, Player2->GetPosition());


	//also render the food
	SDL_RenderCopy(renderer, Food->GetTexture(), NULL, Food->GetPosition());

	SDL_RenderPresent(renderer);

}

