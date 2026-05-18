#ifndef GAME
#define GAME

#include <iostream>
#include "Scene.h"
#include "SDL.h"
#include "MainScene.h"

/**
 *  @brief  游戏主控制类
**/
class Game
{
    public:
        //获取负责SDL生命周期、主循环和场景切换的唯一Game实例
        static Game& getInstance()
        {
            static Game gameInstance;
            return gameInstance;
        }

        //析构Game单例时释放场景、窗口、渲染器和SDL相关资源
        ~Game();

        //初始化SDL子系统、主窗口、主渲染器和初始菜单场景
        void initialGame();

        //启动事件处理、场景更新、场景渲染和帧率限制组成的主循环
        void runGame();

        //释放当前场景并立即接管传入的新场景
        void changeScene(Scene* scene);

        //暂存新场景请求,等待主循环安全时机再替换当前场景
        void requestSceneChange(Scene* scene);

        //清理Game持有的场景、窗口、渲染器和SDL扩展库资源
        void clearGame();

        //轮询SDL事件并交给当前场景处理键盘、窗口等输入
        void Handle(SDL_Event* event);

        //用当前帧间隔驱动当前场景的逻辑更新
        void Update(float dt);

        //清屏后渲染当前场景并把结果提交到窗口
        void Render();

        //把requestSceneChange暂存的场景切换请求应用到当前场景
        void applyPendingScene();

        //返回Game创建并持有的SDL主窗口
        SDL_Window* getWindow();

        //返回Game创建并供各场景绘制使用的SDL渲染器
        SDL_Renderer* getRenderer();

        //返回Game配置的主窗口宽度
        int getWindowWidth();

        //返回Game配置的主窗口高度
        int getWindowHeight();

        //设置菜单选择的难度偏移
        void setDifficultyOffset(float offset);

        //返回当前菜单选择的难度偏移
        float getDifficultyOffset() const;

    private:
        //私有构造函数,限制外部创建实例
        Game();

        //禁止拷贝构造,保证单例唯一
        Game(const Game& game)=delete;

        //禁止赋值操作,保证单例唯一
        Game& operator=(const Game& game)=delete;

        //控制runGame主循环是否继续执行
        bool isRunning=true;

        //当前接收事件、更新逻辑并执行渲染的场景
        Scene* currentScene=nullptr;

        //等待主循环安全时机切换的新场景
        Scene* pendingScene=nullptr;

        //SDL创建的游戏主窗口
        SDL_Window* window=nullptr;

        //SDL创建的主渲染器,供所有场景绘制使用
        SDL_Renderer* renderer=nullptr;

        //创建SDL主窗口时使用的宽度
        int windowWidth=600;

        //创建SDL主窗口时使用的高度
        int windowHeight=750;

        //菜单选择的难度偏移,EASY=-0.3,NORMAL=0,HARD=+0.4
        float difficultyOffset=0.0f;

        //主循环进行帧率限制时使用的目标帧率
        int FPS=60;

        //根据FPS换算出的单帧目标耗时
        Uint32 frameTime;

        //当前帧与上一帧之间的时间间隔,传给场景更新逻辑
        float deltaTime=0.0f;
};

#endif
