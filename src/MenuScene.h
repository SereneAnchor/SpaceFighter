/**
 *  @file    MenuScene.h
 *  @brief   主菜单场景
**/

#ifndef MENUSCENE
#define MENUSCENE

#include <SDL.h>
#include <vector>

#include "GameUI.h"
#include "Scene.h"

class Game;

/**
 *  @brief  主菜单场景
 *
 *  显示标题、难度选择和开始提示
**/
class MenuScene:public Scene
{
    public:
        //构造主菜单场景
        MenuScene();

        //析构主菜单场景
        ~MenuScene();

        //加载菜单字体和初始状态
        virtual void initialScene() override;

        //更新菜单闪烁提示
        virtual void updateScene(float deltaTime) override;

        //渲染菜单界面
        virtual void renderScene() override;

        //处理难度选择和开始游戏输入
        virtual void handleEvent(SDL_Event* event) override;

        //释放菜单字体资源
        virtual void clearScene() override;

    private:
        //游戏主控制对象
        Game& game;

        //菜单UI绘制工具
        GameUI ui;

        //主菜单项
        std::vector<MenuItem> menuItems;

        //当前选中的菜单项
        int selectedMenuIndex=0;

        //当前难度索引,0=EASY,1=NORMAL,2=HARD
        int difficultyIndex=1;

        //闪烁计时器
        float blinkTimer=0;

        //闪烁文字是否可见
        bool blinkVisible=true;
};

#endif
