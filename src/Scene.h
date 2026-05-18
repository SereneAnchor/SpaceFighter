#ifndef SCENE
#define SCENE

#include <SDL.h>

/**
 *  @brief  场景基类
 *
 *  定义菜单场景、主游戏场景和结算场景等所有场景都必须实现的生命周期接口
**/
class Scene
{
    public:
        //构造场景基类对象,供各具体场景继承时复用
        Scene()=default;

        //通过虚析构保证删除场景基类指针时会进入子类析构流程
        virtual ~Scene()=default;

        //初始化当前场景需要的纹理、音频、UI和运行时状态
        virtual void initialScene()=0;

        //由Game主循环按帧调用,推进当前场景的玩法或菜单逻辑
        virtual void updateScene(float deltaTime)=0;

        //由Game主循环调用,把当前场景的可见内容绘制到主渲染器
        virtual void renderScene()=0;

        //接收Game分发的SDL事件,处理移动、选择、暂停等输入
        virtual void handleEvent(SDL_Event* event)=0;

        //在场景切换或游戏退出前释放当前场景持有的资源
        virtual void clearScene()=0;
};

#endif
