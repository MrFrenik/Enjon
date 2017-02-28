#include "Game.h"

#include <stdio.h>

Game::Game()
{

}

Game::~Game()
{

}

void Game::Initialize()
{

}

void Game::Update(Enjon::f32 dt)
{
	printf("dt: %f\n", dt);
}

void Game::Shutdown()
{
	printf("Shutting down game...\n");
}
