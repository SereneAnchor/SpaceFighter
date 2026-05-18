#include "Game.h"
#include "MenuScene.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

Game::Game()
{
    //输出Game单例创建日志,用于观察程序启动阶段的对象生命周期
    std::cout<<"Game::构造方法."<<std::endl;
}

Game::~Game()
{
    //释放Game持有的场景、窗口、渲染器和SDL相关资源
    clearGame();
    //输出Game单例释放日志,用于观察程序退出阶段的对象生命周期
    std::cout<<"Game::析构方法."<<std::endl; 
}

void Game::initialGame()
{
    //根据目标帧率计算主循环每帧至少需要占用的毫秒数
    frameTime=static_cast<Uint32>(1000.0f/FPS);
    //关闭输入法候选窗口,游戏中只处理键盘按键
    SDL_SetHint("SDL_IME_SHOW_UI","0");
    //初始化SDL核心模块,为窗口、输入、音频等子系统做准备
    if(SDL_Init(SDL_INIT_EVERYTHING)!=0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,"SDL_Init Error:%s",SDL_GetError());
        //SDL核心模块初始化失败后停止主循环,避免后续访问无效资源
        isRunning=false;
        return;
    }
    //关闭文本输入,避免中文输入法影响游戏按键
    SDL_StopTextInput();
    //创建Game持有的SDL主窗口,后续所有场景都渲染到这个窗口
    window=SDL_CreateWindow("SDL2 Game",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
                            windowWidth,windowHeight,SDL_WINDOW_SHOWN);
    //主窗口创建失败时停止主循环,避免继续创建依赖窗口的渲染器
    if(window==nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,"SDL_CreateWindow Error:%s",SDL_GetError());
        isRunning=false;
        return;
    }
    //基于主窗口创建硬件加速并启用垂直同步的渲染器,供所有场景绘制使用
    renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    //主渲染器创建失败时停止主循环,避免场景加载纹理时访问空渲染器
    if(renderer==nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,"SDL_CreateRenderer Error:%s",SDL_GetError());
        isRunning=false;
        return;
    }
    //初始化SDL_image并启用PNG加载支持,供场景加载图片资源
    if(IMG_Init(IMG_INIT_PNG)!=IMG_INIT_PNG)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,"IMG_Init Error:%s",SDL_GetError());
        isRunning=false;
        return;    
    }
    //初始化SDL_mixer音频播放环境,供场景播放BGM和音效
    if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048)!=0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,"Mix_OpenAudio Error:%s",Mix_GetError());
        isRunning=false;
        return;
    }
    //初始化SDL_ttf字体渲染环境,供UI绘制文本
    if(TTF_Init()!=0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,"TTF_Init Error:%s",TTF_GetError());
        isRunning=false;
        return;
    }
    //创建初始菜单场景并交给Game管理
    currentScene=new MenuScene();
    currentScene->initialScene();
}

void Game::runGame()
{
    //记录上一帧时间,用于计算帧间隔
    auto lastFrameTime=SDL_GetTicks();
    while (isRunning)
    {
        //记录当前帧开始时间
        auto frameStart=SDL_GetTicks();
        //计算当前帧与上一帧之间的时间间隔
        deltaTime=(frameStart-lastFrameTime)/1000.0f;
        lastFrameTime=frameStart;
        //限制deltaTime最大值,避免窗口卡顿导致移动距离过大
        if(deltaTime>0.05f)
        {
            deltaTime=0.05f;
        }
        //处理本帧积累的SDL事件,并把输入继续交给当前场景
        SDL_Event event;
        Handle(&event);  
        //应用事件处理阶段产生的延迟场景切换请求
        applyPendingScene();
        //用deltaTime更新当前场景的玩法或菜单逻辑
        Update(deltaTime);      
        //应用更新阶段产生的延迟场景切换请求
        applyPendingScene();
        //让当前场景绘制本帧画面并提交到窗口
        Render();
        //记录当前帧结束时间并计算本帧耗时
        auto frameEnd=SDL_GetTicks();
        auto frameDuration=frameEnd-frameStart;
        //当前帧耗时不足目标帧长时等待剩余时间
        if(frameDuration<frameTime)
        {
            SDL_Delay(frameTime-frameDuration);
        }
    }
}

void Game::requestSceneChange(Scene *scene)
{
    //忽略空场景请求,避免后续切换到无效场景
    if(scene==nullptr)
    {
        return;
    }
    //若已有等待切换的场景,先释放旧请求以保留最新切换意图
    if(pendingScene!=nullptr)
    {
        delete pendingScene;
    }
    //保存新的待切换场景,稍后由主循环调用applyPendingScene应用
    pendingScene=scene;
}

void Game::changeScene(Scene *scene)
{
    //避免空场景切换导致后续生命周期调用崩溃
    if(scene==nullptr)
    {
        return;
    }
    //切换前先调用当前场景的清理逻辑并释放场景对象
    if(currentScene!=nullptr)
    {
        currentScene->clearScene();
        delete currentScene;
    }
    //接管传入的新场景并执行其资源初始化
    currentScene=scene;
    currentScene->initialScene();
}

void Game::applyPendingScene()
{
    //没有待切换场景时直接返回,保持当前场景继续运行
    if(pendingScene==nullptr)
    {
        return;
    }
    //取出待切换场景并清空等待槽位,避免切换过程中重复释放
    auto nextScene=pendingScene;
    pendingScene=nullptr;
    //把暂存的新场景替换为当前场景
    changeScene(nextScene);
}

void Game::clearGame()
{
    //释放尚未应用的待切换场景,避免退出时泄漏场景对象
    if(pendingScene!=nullptr)
    {
        delete pendingScene;
        pendingScene=nullptr;
    }
    //清理并释放当前正在运行的场景
    if(currentScene!=nullptr)
    {
        currentScene->clearScene();
        delete currentScene;
        currentScene=nullptr;
    }
    //关闭SDL_ttf字体模块,释放字体渲染相关全局资源
    if(TTF_WasInit()!=0)
    {
        TTF_Quit();
    }
    //关闭SDL_mixer音频模块,释放音频设备和混音器资源
    Mix_CloseAudio();
    Mix_Quit();
    //关闭SDL_image图片模块,释放图片加载扩展资源
    IMG_Quit();
    //销毁Game创建的主渲染器
    if(renderer!=nullptr)
    {
        SDL_DestroyRenderer(renderer);
        renderer=nullptr;
    }
    //销毁Game创建的主窗口
    if(window!=nullptr)
    {
        SDL_DestroyWindow(window);
        window=nullptr;
    }
    //关闭SDL核心模块,完成SDL全局清理
    if(SDL_WasInit(0)!=0)
    {
        SDL_Quit();
    }
}

void Game::Handle(SDL_Event *event)
{
    //持续取出SDL事件队列中本帧积累的窗口和输入事件
    while(SDL_PollEvent(event))
    {
        //收到窗口关闭事件时停止Game主循环
        if(event->type==SDL_QUIT)
        {
            isRunning=false;
        }
        //窗口重新获得焦点时关闭文本输入,避免输入法候选框干扰游戏按键
        if(event->type==SDL_WINDOWEVENT&&event->window.event==SDL_WINDOWEVENT_FOCUS_GAINED)
        {
            SDL_StopTextInput();
        }
        //把SDL事件交给当前场景处理菜单选择、移动、射击等输入
        if(currentScene!=nullptr)
        {
            currentScene->handleEvent(event);
        }
    }
}

void Game::Update(float dt)
{
    //当前场景为空时跳过更新,避免初始化失败或清理后的空指针访问
    if(currentScene==nullptr)
    {
        return;
    }
    //把本帧时间间隔传给当前场景,驱动场景内部逻辑更新
    currentScene->updateScene(dt);
}

void Game::Render()
{
    //清空Game主渲染器中的上一帧画面
    SDL_RenderClear(renderer);
    //当前场景为空时只提交清屏结果,避免渲染阶段访问空指针
    if(currentScene==nullptr)
    {
        SDL_RenderPresent(renderer);
        return;
    }
    //调用当前场景把背景、角色、UI等内容绘制到主渲染器
    currentScene->renderScene();
    //把主渲染器中的绘制结果提交到SDL主窗口
    SDL_RenderPresent(renderer);
}

SDL_Window *Game::getWindow()
{
    //返回Game持有的SDL主窗口指针
    return window;
}

SDL_Renderer *Game::getRenderer()
{
    //返回Game持有的SDL主渲染器指针
    return renderer;
}

int Game::getWindowWidth()
{
    //返回Game创建主窗口时使用的宽度
    return windowWidth;
}

int Game::getWindowHeight()
{
    //返回Game创建主窗口时使用的高度
    return windowHeight;
}

void Game::setDifficultyOffset(float offset)
{
    //保存菜单选择的难度偏移,供后续主游戏场景读取
    difficultyOffset=offset;
}

float Game::getDifficultyOffset() const
{
    //返回当前难度偏移,避免场景之间依赖全局可变状态
    return difficultyOffset;
}
