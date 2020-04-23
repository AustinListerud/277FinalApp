#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <time.h>

// Global variables for screen dimensions and paddle speed
const int SCREEN_WIDTH = 864;
const int SCREEN_HEIGHT = 486;
const double PADDLE_SPEED = SCREEN_HEIGHT / 30;

// Paddle class contains paddle data and function to render it
class Paddle
{
public:

	//Position, dimensions, velocity of paddle
	double x = -1, y = -1, width = 10, height = 40, yVelocity;

	// Ctor with member variable parameters
	Paddle(double x1, double y1, double w1, double h1);
	void render(SDL_Renderer*); 
};

// Ball class contains ball data and functions that reset, handle collision detection, and render the ball
class Ball
{
public:

	// Position, dimensions of ball
	double x = -1, y = -1, width = 10, height = 10;

	// yVelocity will be randomly 200 or -200, xVelocity will be plus or minus 80 to 100
	double yVelocity;
	double xVelocity;

	Ball(double x1, double y1, double w1, double h1);

	bool collides(Paddle);
	void reset();
	void render(SDL_Renderer*);
};