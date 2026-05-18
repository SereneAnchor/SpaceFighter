#ifndef OBJECT
#define OBJECT

#include <algorithm>
#include <SDL.h>
#include <vector>

/**
 *  @brief  玩家飞机结构体
 *
 *  @param  texture        玩家飞机纹理资源。
 *  @param  position       玩家飞机左上角位置。
 *  @param  velocity       玩家当前移动速度向量。
 *  @param  width          玩家飞机宽度。
 *  @param  height         玩家飞机高度。
 *  @param  speed          玩家移动速度。
 *  @param  health         玩家当前生命值。
 *  @param  maxHealth      玩家最大生命值。
 *  @param  isAlive        玩家是否存活。
 *  @param  score          玩家当前得分。
 *  @param  coolTime       玩家射击冷却时间,单位毫秒。
 *  @param  lastShootTime  玩家上次射击时间戳。
**/
struct Player
{
    SDL_Texture* texture=nullptr;
    SDL_FPoint position={0,0};
    SDL_FPoint velocity={0,0};
    int width=0,height=0;
    int speed=300;
    int health=100;
    int maxHealth=100;
    bool isAlive=true;
    int score=0;
    Uint32 coolTime=180;
    Uint32 lastShootTime=0;
};

/**
 *  @brief  玩家子弹结构体
 *
 *  @param  texture     玩家子弹纹理资源。
 *  @param  position    玩家子弹左上角位置。
 *  @param  direction   玩家子弹飞行方向。
 *  @param  width       玩家子弹宽度。
 *  @param  height      玩家子弹高度。
 *  @param  speed       玩家子弹飞行速度。
 *  @param  damage      玩家子弹命中伤害。
 *  @param  angle       玩家子弹渲染旋转角度。
 *  @param  isAlive     玩家子弹是否存活。
 *  @param  trail       玩家子弹拖尾位置缓存。
 *  @param  trailCount  玩家子弹当前拖尾点数量。
**/
struct PlayerBullet
{
    SDL_Texture* texture=nullptr;
    SDL_FPoint position={0,0};
    SDL_FPoint direction={0,-1};
    int width=0,height=0;
    int speed=900;
    int damage=10;
    double angle=0;
    bool isAlive=true;
    SDL_FPoint trail[4];
    int trailCount=0;
};

/**
 *  @brief  敌机结构体
 *
 *  @param  texture             敌机纹理资源。
 *  @param  position            敌机左上角位置。
 *  @param  width               敌机宽度。
 *  @param  height              敌机高度。
 *  @param  speed               敌机移动速度。
 *  @param  health              敌机当前生命值。
 *  @param  maxHealth           敌机最大生命值。
 *  @param  isAlive             敌机是否存活。
 *  @param  scoreValue          击毁该敌机获得的分数。
 *  @param  coolTime            敌机射击冷却时间,单位毫秒。
 *  @param  lastShootTime       敌机上次射击时间戳。
 *  @param  lastShootDirection  敌机上次射击方向。
 *  @param  lastShootAngle      敌机上次射击角度。
 *  @param  state               敌机当前AI状态。
 *  @param  bezierPts           敌机贝塞尔入场控制点。
 *  @param  bezierT             敌机贝塞尔入场进度。
 *  @param  bezierDuration      敌机贝塞尔入场持续时间。
**/
struct Enemy
{
    SDL_Texture* texture=nullptr;
    SDL_FPoint position={0,0};
    int width=0,height=0;
    int speed=200;
    int health=30;
    int maxHealth=30;
    bool isAlive=true;
    int scoreValue=100;
    Uint32 coolTime = 1200;
    Uint32 lastShootTime = 0;
    SDL_FPoint lastShootDirection={0,1};
    double lastShootAngle=0;

    enum State { PATROL, CHASE, ATTACK, FLEE } state = PATROL;

    SDL_FPoint bezierPts[4];
    float bezierT=1.0f;
    float bezierDuration=2.0f;
};

/**
 *  @brief  敌机子弹结构体
 *
 *  @param  texture     敌机子弹纹理资源。
 *  @param  position    敌机子弹左上角位置。
 *  @param  direction   敌机子弹飞行方向。
 *  @param  width       敌机子弹宽度。
 *  @param  height      敌机子弹高度。
 *  @param  speed       敌机子弹飞行速度。
 *  @param  damage      敌机子弹命中伤害。
 *  @param  angle       敌机子弹渲染旋转角度。
 *  @param  owner       发射该子弹的敌机。
 *  @param  isAlive     敌机子弹是否存活。
 *  @param  trail       敌机子弹拖尾位置缓存。
 *  @param  trailCount  敌机子弹当前拖尾点数量。
**/
struct EnemyBullet
{
    SDL_Texture* texture=nullptr;
    SDL_FPoint position={0,0};
    SDL_FPoint direction = {0, 0};
    int width=0,height=0;
    int speed=240;
    int damage=8;
    double angle=0;
    Enemy* owner=nullptr;
    bool isAlive=true;
    SDL_FPoint trail[4];
    int trailCount=0;
};

/**
 *  @brief  游戏特效结构体
 *
 *  @param  texture        特效序列帧纹理资源。
 *  @param  position       特效左上角位置。
 *  @param  width          特效渲染宽度。
 *  @param  height         特效渲染高度。
 *  @param  frameWidth     单帧纹理宽度。
 *  @param  frameHeight    单帧纹理高度。
 *  @param  currentFrame   当前播放帧索引。
 *  @param  totalFrame     特效总帧数。
 *  @param  frameTime      每帧持续时间,单位毫秒。
 *  @param  lastFrameTime  上次切换帧的时间戳。
 *  @param  isAlive        特效是否仍在播放。
**/
struct Effect
{
    SDL_Texture* texture=nullptr;
    SDL_FPoint position={0,0};
    int width=0,height=0;
    int frameWidth=32,frameHeight=32;
    int currentFrame=0;
    int totalFrame=9;
    Uint32 frameTime=50;
    Uint32 lastFrameTime=0;
    bool isAlive=true;
};

/**
 *  @brief  道具类型
 *
 *  @param  LIFE    生命恢复道具。
 *  @param  SHIELD  护盾道具。
 *  @param  TIME    射速增益道具。
**/
enum class PowerUpType
{
    LIFE,
    SHIELD,
    TIME
};

/**
 *  @brief  道具结构体
 *
 *  @param  texture   道具纹理资源。
 *  @param  position  道具左上角位置。
 *  @param  width     道具宽度。
 *  @param  height    道具高度。
 *  @param  speed     道具下落速度。
 *  @param  type      道具类型。
 *  @param  isAlive   道具是否仍可拾取。
**/
struct PowerUp
{
    SDL_Texture* texture=nullptr;
    SDL_FPoint position={0,0};
    int width=0,height=0;
    float speed=120;
    PowerUpType type=PowerUpType::LIFE;
    bool isAlive=true;
};

/**
 *  @brief  粒子结构体
 *
 *  @param  position     粒子当前位置
 *  @param  velocity     粒子当前速度向量
 *  @param  lifetime     粒子剩余生命周期
 *  @param  maxLifetime  粒子最大生命周期
 *  @param  r            粒子红色通道
 *  @param  g            粒子绿色通道
 *  @param  b            粒子蓝色通道
 *  @param  a            粒子透明度通道
 *  @param  size         粒子渲染尺寸
 *  @param  isAlive      粒子是否存活
**/
struct Particle
{
    SDL_FPoint position={0,0};
    SDL_FPoint velocity={0,0};
    float lifetime=0;
    float maxLifetime=1.0f;
    Uint8 r=255,g=255,b=255,a=255;
    float size=4;
    bool isAlive=false;
};

/**
 *  @brief  对象池模板类
**/
template<typename T>
class ObjectPool
{
    public:
        //析构对象池并释放全部已创建对象
        ~ObjectPool()
        {
            clear();
        }

        //获取一个可用对象,优先复用空闲对象,没有空闲对象时创建新对象
        T* acquire()
        {
            if(available.empty())
            {
                T* obj=new T();
                all.push_back(obj);
                return obj;
            }
            T* obj=available.back();
            available.pop_back();
            return obj;
        }

        //将对象归还到对象池的空闲列表
        void release(T* obj)
        {
            if(obj==nullptr)
            {
                return;
            }
            //防止同一对象被重复归还,重复指针进入空闲列表后,后续可能被多次acquire,导致多个逻辑对象共享同一块内存
            if(std::find(available.begin(),available.end(),obj)!=available.end())
            {
                return;
            }
            available.push_back(obj);
        }

        //清空对象池,释放全部已创建对象并清空记录
        void clear()
        {
            for(auto obj:all)
            {
                delete obj;
            }
            all.clear();
            available.clear();
        }

    private:
        //对象池创建过的全部对象
        std::vector<T*> all;

        //当前可复用的空闲对象
        std::vector<T*> available;
};

#endif
