/**
 *  @file    MenuScene.cpp
 *  @brief   主菜单场景实现
**/

#include "MenuScene.h"
#include "Game.h"
#include "MainScene.h"
#include "Config.h"

#include <SDL.h>
#include <string>

namespace
{
    //把难度索引转换为菜单中显示的难度文本
    const char* difficultyName(int index)
    {
        if(index==0)
        {
            return "EASY";
        }
        if(index==2)
        {
            return "HARD";
        }
        return "NORMAL";
    }
}

MenuScene::MenuScene():game(Game::getInstance())
{
    std::cout<<"MenuScene::构造方法."<<std::endl;
}

MenuScene::~MenuScene()
{
    std::cout<<"MenuScene::析构方法."<<std::endl;
}

void MenuScene::initialScene()
{
    //初始化主菜单字体和菜单项内容
    ui.initialize(game.getRenderer(),"assets/font/VonwaonBitmap-16px.ttf",24);
    menuItems.clear();
    menuItems.push_back(MenuItem{"Start Game"});
    menuItems.push_back(MenuItem{"Difficulty: NORMAL"});
    selectedMenuIndex=0;
}

void MenuScene::updateScene(float deltaTime)
{
    //推进开始提示的闪烁计时
    blinkTimer+=deltaTime;
    if(blinkTimer>=0.5f)
    {
        blinkTimer-=0.5f;
        blinkVisible=!blinkVisible;
    }
}

void MenuScene::renderScene()
{
    //获取Game提供的主渲染器和窗口尺寸
    auto renderer=game.getRenderer();
    auto w=game.getWindowWidth();
    auto h=game.getWindowHeight();
    //先清空并绘制主菜单背景
    SDL_SetRenderDrawColor(renderer,8,12,32,255);
    SDL_RenderClear(renderer);
    //同步菜单项中的难度文本,让选择结果直接显示在主菜单上
    if(menuItems.size()>=2)
    {
        menuItems[1].label="Difficulty: "+std::string(difficultyName(difficultyIndex));
    }
    ui.renderMainMenu(w,h,menuItems,selectedMenuIndex);
}

void MenuScene::handleEvent(SDL_Event *event)
{
    //空事件直接忽略,避免访问无效输入数据
    if(event==nullptr)
    {
        return;
    }
    //只处理第一次按下的按键,避免按键连发重复切换
    if(event->type==SDL_KEYDOWN&&event->key.repeat==0)
    {
        auto key=event->key.keysym.scancode;
        if(key==SDL_SCANCODE_UP)
        {
            selectedMenuIndex=GameUI::moveSelection(selectedMenuIndex,-1,
                                                    static_cast<int>(menuItems.size()));
        }
        else if(key==SDL_SCANCODE_DOWN)
        {
            selectedMenuIndex=GameUI::moveSelection(selectedMenuIndex,1,
                                                    static_cast<int>(menuItems.size()));
        }
        else if(key==SDL_SCANCODE_LEFT&&selectedMenuIndex==1)
        {
            //在三档难度之间向左循环切换
            difficultyIndex=(difficultyIndex+2)%3;
        }
        else if(key==SDL_SCANCODE_RIGHT&&selectedMenuIndex==1)
        {
            //在三档难度之间向右循环切换
            difficultyIndex=(difficultyIndex+1)%3;
        }
        else if(key==SDL_SCANCODE_RETURN||key==SDL_SCANCODE_SPACE)
        {
            if(selectedMenuIndex==0||key==SDL_SCANCODE_SPACE)
            {
                //把当前菜单难度映射到全局难度偏移后切换到主游戏场景
                if(difficultyIndex==0)      gDifficultyOffset=-0.30f;
                else if(difficultyIndex==1) gDifficultyOffset=0.0f;
                else                        gDifficultyOffset=0.40f;
                game.requestSceneChange(new MainScene());
            }
        }
    }
}

void MenuScene::clearScene()
{
    //释放主菜单UI资源
    ui.clear();
}
