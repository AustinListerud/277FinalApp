// Using SDL, SDL_image, SDL_ttf, SDL_mixer standard IO, math, strings, Tennis 
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <Tennis.h>

using namespace std;

// This class wraps the SDL texture so we can easily load media and render
// Can't be in a separate file because it needs to use the same renderer pointer that main uses
class Texture
{
public:
	// Initializes variables
	Texture();

	// Deallocates memory
	~Texture();

	// Loads image at specified path
	bool loadFromFile(string path);

	// Creates image from font string
	bool loadFromRenderedText(string textureText, SDL_Color textColor, TTF_Font* font);

	// Deallocates texture
	void free();

	// Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	// Set blending
	void setBlendMode(SDL_BlendMode blending);

	// Set alpha modulation
	void setAlpha(Uint8 alpha);

	// Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	// Gets image dimensions
	int getWidth();
	int getHeight();

private:
	// The actual hardware texture
	SDL_Texture* mTexture;

	// Image dimensions
	int mWidth;
	int mHeight;
};

// Starts up SDL and creates window
bool init();

// Loads media
bool loadMedia();

// Frees media and shuts down SDL
void close();

// The window we'll be rendering to
SDL_Window* window = NULL;

// The window renderer
SDL_Renderer* renderer = NULL;

// Globally used font
TTF_Font* titleFont = NULL;
TTF_Font* scoreFont = NULL;
TTF_Font* messageFont = NULL;

// Globally used color (white)
SDL_Color textColor = { 0xFF, 0xFF, 0xFF };

// Rendered textures for title, UI message, and both paddles
Texture titleTexture, p1Texture, p2Texture, msgTexture;

// Pointers for sounds
Mix_Chunk* player1sound = NULL;
Mix_Chunk* player2sound = NULL;
Mix_Chunk* player1score = NULL;
Mix_Chunk* player2score = NULL;
Mix_Chunk* player1win = NULL;
Mix_Chunk* player2win = NULL;
Mix_Chunk* wallhitSound = NULL;

// Texture constructor
Texture::Texture()
{
	// Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

// Destructor
Texture::~Texture()
{
	// Deallocate
	free();
}

// This function loads files into textures
bool Texture::loadFromFile(string path)
{
	// Get rid of preexisting texture
	free();

	// The final texture
	SDL_Texture* newTexture = NULL;

	// Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		// Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		// Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			// Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		// Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	// Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

// This function loads text into a texture of the chosen font
bool Texture::loadFromRenderedText(string textureText, SDL_Color textColor, TTF_Font* font)
{
	// Get rid of preexisting texture
	free();

	// Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
	if (textSurface == NULL)
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else
	{
		// Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			// Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		// Get rid of old surface
		SDL_FreeSurface(textSurface);
	}

	// Return success
	return mTexture != NULL;
}

// Frees memory when done
void Texture::free()
{
	// Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}


void Texture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	// Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void Texture::setBlendMode(SDL_BlendMode blending)
{
	// Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void Texture::setAlpha(Uint8 alpha)
{
	// Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void Texture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	// Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	// Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	// Render to screen
	SDL_RenderCopyEx(renderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int Texture::getWidth()
{
	return mWidth;
}

int Texture::getHeight()
{
	return mHeight;
}

// Initializes the framework, the window, and the renderer
bool init()
{
	// Initialization flag
	bool success = true;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		// Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		// Create window
		window = SDL_CreateWindow("Bumper Tennis", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			// Create vsynced renderer for window
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				// Initialize renderer color
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

				// Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}

				// Initialize SDL_ttf
				if (TTF_Init() == -1)
				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}

				// Initialize SDL_Mixer
				if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
				{
					printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

// Loads the silkscreen retro font and sets it to the start screen
bool loadMedia()
{
	// Loading success flag
	bool success = true;

	// Open the fonts
	titleFont = TTF_OpenFont("slkscr.ttf", 28);
	scoreFont = TTF_OpenFont("slkscr.ttf", 70);
	messageFont = TTF_OpenFont("slkscr.ttf", 21);

	if (!titleFont || !scoreFont || !messageFont)
	{
		printf("Failed to load silkscreen font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false;
	}
	else
	{
		// Load title and initial score into texture
		if (!titleTexture.loadFromRenderedText("Bumper Tennis", textColor, titleFont) 			
			|| !msgTexture.loadFromRenderedText("by Austin Listerud. Press Enter to begin.", textColor, messageFont))
		{
			printf("Failed to render text texture!\n");
			success = false;
		}
	}

	//Load sound effects
	player1sound = Mix_LoadWAV("player1sound.wav");
	if (player1sound == NULL)
	{
		printf("Failed to load player 1 sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	player2sound = Mix_LoadWAV("player2sound.wav");
	if (player2sound == NULL)
	{
		printf("Failed to load player 2 sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	player1score = Mix_LoadWAV("player1score.wav");
	if (player1score == NULL)
	{
		printf("Failed to load player 1 score sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	player2score = Mix_LoadWAV("player2score.wav");
	if (player2score == NULL)
	{
		printf("Failed to load player 2 score sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	player1win = Mix_LoadWAV("player1win.wav");
	if (player1win == NULL)
	{
		printf("Failed to load player 1 win sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	player2win = Mix_LoadWAV("player2win.wav");
	if (player2win == NULL)
	{
		printf("Failed to load player 2 win sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	wallhitSound = Mix_LoadWAV("wallHit.wav");
	if (wallhitSound == NULL)
	{
		printf("Failed to load wall hit effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	return success;
}

// Frees memory when the program closes
void close()
{
	// Free textures
	titleTexture.free();
	p1Texture.free();
	p2Texture.free();

	// Free fonts
	TTF_CloseFont(titleFont);
	TTF_CloseFont(scoreFont);
	titleFont = scoreFont = NULL;

	// Free sounds and set pointers to null
	Mix_FreeChunk(player1sound);
	player1sound = NULL;
	Mix_FreeChunk(player2sound);
	player2sound = NULL;
	Mix_FreeChunk(player1score);
	player1score = NULL;
	Mix_FreeChunk(player2score);
	player2score = NULL;
	Mix_FreeChunk(player1win);
	player1win = NULL;
	Mix_FreeChunk(player2win);
	player2win = NULL;
	Mix_FreeChunk(wallhitSound);
	wallhitSound = NULL;

	// Destroy window	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

	// Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	Mix_Quit();
	SDL_Quit();
}


int main(int argc, char* args[])
{
	// Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		// Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			// Initialize player scores, serving player (always 1 after adding AI), and winning player 
			int player1Score = 9;
			int player2Score = 0;
			int servingPlayer = 1;
			int winningPlayer = -1; 

			// Seed RNG, initialize paddles, the AI's velocity, the ball, flags and accumulator for random game events
			srand(time(NULL));
			Paddle player1(10, 40, 10, 40);
			Paddle player2(SCREEN_WIDTH - 20, SCREEN_HEIGHT - 80, 10, 40);
			player2.yVelocity /= 6;
			Ball ball(SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2 - 5, 10, 10);
			bool sevenFlag = false;
			bool zigzagFlag = false;
			int zigzagTot = 0;

			// Variable holds the game state, initialized to start
			string gameState = "start";

			// Main loop flag
			bool quit = false;

			// Event handler
			SDL_Event event;

			// While application is running
			while (!quit)
			{
				// Handle events on queue
				while (SDL_PollEvent(&event) != 0)
				{
					// User requests quit
					if (event.type == SDL_QUIT)
					{
						quit = true;
					}

					// User presses a key
					else if (event.type == SDL_KEYDOWN)
					{   // User presses either enter/return key, change the game state
						if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
						{
							if (gameState == "start")
							{
								gameState = "serve";
							}
							else if (gameState == "serve")
							{
								gameState = "play";
							}
							else if (gameState == "done")
							{
								gameState = "serve";
								ball.reset();
								player1Score = 0;
								player2Score = 0;
								servingPlayer = 1;
								sevenFlag = false;
								player1.height = 40;
								player2.height = 40;
							}
						}
						// Bal is in play
						if (gameState == "play")
						{
							if (event.key.keysym.sym == SDLK_s)
							{
								player1.y = fmin(SCREEN_HEIGHT - player1.height, player1.y + player1.yVelocity);
							}
							else if (event.key.keysym.sym == SDLK_w)
							{
								player1.y = fmax(0, player1.y - player1.yVelocity);
							}
						}
					}

				}

				// Update game even if no keydown
				if (gameState == "play")
				{   // If there is no zig zag serves, the ball keeps moving normally
					if (!zigzagFlag)
					{
						ball.x += ball.xVelocity;
						ball.y += ball.yVelocity;
					}
					else
					{   // After moving in its direction 20 times, it reverses, giving a zigzag pattern
						if (zigzagTot < 20 * ball.height)
						{
							ball.x += ball.xVelocity;
							ball.y += 2 * ball.yVelocity;
							zigzagTot += ball.height;
						}
						else
						{
							ball.yVelocity *= -1;
							zigzagTot = 0;
						}
					}

					if (ball.collides(player1))
					{
						// Play sound
						Mix_PlayChannel(-1, player1sound, 0);

						// Turn off zigzag serve if player1 hits it
						zigzagFlag = false;

						// Move ball on collision in front of paddle. Several collisions happen when the ball hits the top of the paddle
						ball.x = player1.x + player1.width;

						// So the velocity doesn't overflow past the maximum double, otherwise increase its speed
						if (ball.xVelocity > DBL_MAX)
						{
							ball.xVelocity = -DBL_MAX;
						}
						else
						{
							ball.xVelocity *= -1.05;
						}

						// Randomize yVelocity of ball and reverse its direction when collision occurs
						if (ball.yVelocity < 0)
						{
							ball.yVelocity = -.1 * (rand() % 21);
						}
						else
						{
							ball.yVelocity = .1 * (rand() % 21);
						}


					}
					if (ball.collides(player2))
					{
						// Play sound
						Mix_PlayChannel(-1, player2sound, 0);

						ball.x = player2.x - player2.width;

						// Player 2 will sometimes serve the ball in a zigzag and speed it up after player1 scores 3 
						if (player1Score > 2 && !(rand() % 5))
						{
							zigzagFlag = true;
							ball.xVelocity *= 1.5;
							ball.yVelocity *= 1.5;
						}
						// So the velocity doesn't overflow
						if (ball.xVelocity > DBL_MAX)
						{
							ball.xVelocity = -DBL_MAX;
						}
						else
						{
							ball.xVelocity *= -1.05;
						}

						if (ball.yVelocity < 0)
						{
							ball.yVelocity = -.1 * (rand() % 21);
						}
						else
						{
							ball.yVelocity = .1 * (rand() % 21);
						}

						// Player2 gets a random velocity divided by it to make the AI have a variable skill. Not too good or bad.
						player2.yVelocity = PADDLE_SPEED / (6 + rand() % 5);
					}

					// If the ball hits the top of screen, reverse its direction
					if (ball.y <= 0)
					{
						// Play sound
						Mix_PlayChannel(-1, wallhitSound, 0);

						ball.y = 0;
						ball.yVelocity *= -1;
						
					}

					// If the ball hits the bottom of screen, reverse its direction
					if (ball.y >= SCREEN_HEIGHT - ball.height)
					{
						// Play sound
						Mix_PlayChannel(-1, wallhitSound, 0);

						ball.y = SCREEN_HEIGHT - ball.height;
						ball.yVelocity *= -1;
					}

					// Player1 scores
					if (ball.x >= SCREEN_WIDTH)
					{
						// Play sound
						Mix_PlayChannel(-1, player1score, 0);

						player1Score += 1;
						ball.reset();
					}

					// Player2 scores. Turn off zigzag flag
					if (ball.x <= 0)
					{
						// Play sound
						Mix_PlayChannel(-1, player2score, 0);

						player2Score += 1;
						zigzagFlag = false;
						ball.reset();
					}

					if (player1Score == 10)
					{
						// Play sound
						Mix_PlayChannel(-1, player1win, 0);

						winningPlayer = 1;
						gameState = "done";
					}
					else if (player2Score == 10)
					{
						// Play sound
						Mix_PlayChannel(-1, player2win, 0);

						winningPlayer = 2;
						gameState = "done";
					}

					//  I for player 2
					if (ball.xVelocity > 0 /*&& ((player2.y > ball.y + ball.height)
						^ (player2.y + player2.height < ball.y))*/)
					{
						/*if (ball.x < player2.x - 2 * ball.width)
						{
							if (player2.y + player2.height > ball.y + ball.height)
							{
								player2.y = fmax(0, player2.y - player2.yVelocity);
							}
							else if (player2.y + player2.height < ball.y + ball.height)
							{
								player2.y = fmin(SCREEN_HEIGHT - player2.height, player2.y + player2.yVelocity);
							}
						}
						else
						{*/
							// Player2 is trying to hit midpoint of paddle with midpoint of ball
							if (player2.y + player2.height / 2 > ball.y + ball.height / 2)
							{
								player2.y = fmax(0, player2.y - player2.yVelocity);
							}
							else if (player2.y + player2.height / 2 < ball.y + ball.height / 2)
							{
								player2.y = fmin(SCREEN_HEIGHT - player2.height, player2.y + player2.yVelocity);
							}
						// }


						// Randomly reverse at midpoint of screen when score hits 6
						if (player1Score > 5 && ball.x - ball.width / 2 > SCREEN_WIDTH / 2  
							&& ball.x < SCREEN_WIDTH / 2 + ball.width && !(rand() % 7))
						{
							
							ball.xVelocity *= -1.05;
						}

						// When score hits 8, player1 gets smaller and player 2 gets bigger.
						// Flag ensures this block of code executes only once
						if (!sevenFlag && player1Score > 7)
						{
							sevenFlag == true;
							player1.height = 25;
							player2.height = 60;
						}

					
					}


				}
					
			
				
				// Clear screen
				SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
				SDL_RenderClear(renderer);

				// Set UI message. loadMedia() set message already if gameState == "start"
				if (gameState == "serve")
				{
					string tmpMessage = "Player 1's serve. Press Enter.";
					msgTexture.loadFromRenderedText(tmpMessage, textColor, messageFont);
				}
				else if (gameState == "done")
				{
					string tmpMessage = "Player " + to_string(winningPlayer) + " wins! Press Enter to restart";
					msgTexture.loadFromRenderedText(tmpMessage, textColor, messageFont);
				}
				// Display UI message, e.g. "Press enter" or "Player 1 wins!" No message during play.
				if (gameState != "play")
				{
					msgTexture.render(SCREEN_WIDTH / 2 - msgTexture.getWidth() / 2, 80);
				}

				// Render title of game
				titleTexture.render((SCREEN_WIDTH - titleTexture.getWidth()) / 2, (SCREEN_HEIGHT - titleTexture.getHeight()) / 20);
				
				// Display score by loading text into texture and rendering it
				p1Texture.loadFromRenderedText(to_string(player1Score), textColor, scoreFont);
				p2Texture.loadFromRenderedText(to_string(player2Score), textColor, scoreFont);

				p1Texture.render(SCREEN_WIDTH / 2  - 100, (SCREEN_HEIGHT - p1Texture.getHeight()) / 3);
				p2Texture.render(SCREEN_WIDTH / 2 + 100 - p2Texture.getWidth(), (SCREEN_HEIGHT - p2Texture.getHeight()) / 3);

				// Render ball and paddles
				ball.render(renderer);
				player1.render(renderer);
				player2.render(renderer);

				// Update screen
				SDL_RenderPresent(renderer);
			}
		}
	}

	// Free resources and close SDL
	close();

	return 0;
}