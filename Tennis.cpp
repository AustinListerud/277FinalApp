#include "Tennis.h"

// Paddle constructor
Paddle::Paddle(double x1, double y1, double w1, double h1)
{
	x = x1;
	y = y1;
	width = w1;
	height = h1;
	yVelocity = PADDLE_SPEED;
}

// Render paddle
void Paddle::render(SDL_Renderer* renderer)
{
	SDL_Rect PaddleRect = {this->x, this->y, this->width, this->height};
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderFillRect(renderer, &PaddleRect);
}

// Ball constructor. Randomize yVelocity 
Ball::Ball(double x1, double y1, double w1, double h1)
{
	x = x1;
	y = y1;
	width = w1;
	height = h1;
	yVelocity = rand() % 2 == 1 ? .15 * (rand() % 21) : -.15 * (rand() % 21);
	xVelocity = -4;
}

bool Ball::collides(Paddle paddle)
{
	// Do the ball and paddle have no overlap in the x plane?
	if (this->x > paddle.x + paddle.width || paddle.x > this->x + this->width)
	{
		return false;
	}

	// The y plane?
	if (this->y > paddle.y + paddle.height || paddle.y > this->y + this->height)
	{
		return false;
	}

	// If x and y planes have overlap, then there is a collision
	return true;
}

// Resets the ball to center of screen, sets its velocity to -4 to serve to player 1
void Ball::reset()
{
	this->x = SCREEN_WIDTH / 2 - this->width / 2;
	this->y = SCREEN_HEIGHT / 2 - this->height / 2;
	yVelocity = rand() % 2 == 1 ? .1 * (rand() % 21) : -.1 * (rand() % 21);
	xVelocity = -4;
}

// Renders the ball
void Ball::render(SDL_Renderer* renderer)
{
	SDL_Rect ballRect = {this->x, this->y, this->width, this->height};
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderFillRect(renderer, &ballRect);
}
