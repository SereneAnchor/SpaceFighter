/**
 *  @file    PowerUpSystem.h
 *  @brief   道具系统
 *
 *  管理生命、护盾、射速道具的掉落、拾取、增益效果和提示文本
**/

#ifndef POWERUPSYSTEM
#define POWERUPSYSTEM

#include <list>
#include <random>
#include <string>
#include <SDL.h>
#include "Object.h"

struct Player;

/**
 *  @brief  道具系统
 *
 *  负责从敌机死亡位置生成道具,并在玩家拾取后应用对应增益
**/
class PowerUpSystem
{
    public:
        //加载生命、护盾和射速道具纹理
        void initialize(SDL_Renderer* renderer);

        //在敌机死亡位置按掉落概率生成一个随机类型道具
        void spawn(SDL_FPoint position,std::mt19937& gen,
                   std::uniform_real_distribution<float>& dis);

        //更新道具下落位置,并处理护盾和射速增益到期恢复
        void update(float deltaTime,int windowH,Player& player);

        //把所有未拾取道具绘制到Game主渲染器
        void render(SDL_Renderer* renderer);

        //检测玩家和道具碰撞,拾取后应用效果并创建拾取特效
        bool checkPickup(Player& player,SDL_Texture* explosionTex,
                         ObjectPool<Effect>& effectPool,std::list<Effect*>& effectList);

        //归还活跃道具并释放道具纹理资源
        void clear();

        //护盾结束时间,0表示未激活
        Uint32 shieldEndTime=0;

        //射速增益结束时间,0表示未激活
        Uint32 timeBuffEndTime=0;

        //玩家原始射击冷却
        int playerOriginalCoolTime=0;

        //道具首次提示标记
        bool powerUpNotified[3]={false,false,false};

        //拾取提示剩余显示时间
        float pickupNoticeTimer=0;

        //拾取提示文本
        std::string pickupNotice;

    private:
        //生命道具纹理
        SDL_Texture* lifeTexture=nullptr;

        //护盾道具纹理
        SDL_Texture* shieldTexture=nullptr;

        //射速道具纹理
        SDL_Texture* timeTexture=nullptr;

        //活跃道具列表
        std::list<PowerUp*> powerUpList;

        //道具对象池
        ObjectPool<PowerUp> powerUpPool;
};

#endif
