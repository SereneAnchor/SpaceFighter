#include <iostream>
#include <windows.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include "Game.h"


int main(int, char**)
{
    //设置控制台UTF-8编码
    SetConsoleOutputCP(CP_UTF8);

    //创建Game对象
    Game& game=Game::getInstance();

    //游戏初始化
    game.initialGame();
    game.runGame();

    return 0;
}