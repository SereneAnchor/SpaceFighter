/**
 *  @file    PowerUpSystem.cpp
 *  @brief   道具子系统实现
 *
 *  道具生命周期: 敌机死亡→概率生成→下落→玩家碰触拾取→增益生效
 *  增益到期自动恢复: 护盾消失 / 射速恢复原始值
**/

#include "PowerUpSystem.h"
#include "Object.h"
#include "Config.h"
#include <SDL_image.h>
#include <string>

void PowerUpSystem::initialize(SDL_Renderer* renderer)
{
    //用Game主渲染器加载三种道具纹理,供后续掉落对象复用
    lifeTexture=IMG_LoadTexture(renderer,"assets/image/bonus_life.png");
    if(lifeTexture==NULL)
    {
        SDL_Log("PowerUpSystem::Initialize()加载生命道具失败:%s",SDL_GetError());
    }
    shieldTexture=IMG_LoadTexture(renderer,"assets/image/bonus_shield.png");
    if(shieldTexture==NULL)
    {
        SDL_Log("PowerUpSystem::Initialize()加载护盾道具失败:%s",SDL_GetError());
    }
    timeTexture=IMG_LoadTexture(renderer,"assets/image/bonus_time.png");
    if(timeTexture==NULL)
    {
        SDL_Log("PowerUpSystem::Initialize()加载时间道具失败:%s",SDL_GetError());
    }
}

void PowerUpSystem::spawn(SDL_FPoint position,std::mt19937& gen,
                          std::uniform_real_distribution<float>& dis)
{
    //按配置概率决定敌机死亡位置是否掉落道具
    if(dis(gen)>POWERUP_DROP_RATE)
    {
        return;
    }
    //从道具对象池获取可用对象,随后写入本次掉落属性
    auto powerUp=powerUpPool.acquire();
    //按权重在生命、护盾和射速道具之间随机选择
    auto roll=dis(gen);
    if(roll<0.4f)
    {
        powerUp->type=PowerUpType::LIFE;
        powerUp->texture=lifeTexture;
    }
    else if(roll<0.7f)
    {
        powerUp->type=PowerUpType::SHIELD;
        powerUp->texture=shieldTexture;
    }
    else
    {
        powerUp->type=PowerUpType::TIME;
        powerUp->texture=timeTexture;
    }
    //设置道具尺寸、初始位置、下落速度和存活状态
    powerUp->width=24;
    powerUp->height=24;
    powerUp->position.x=position.x-powerUp->width/2;
    powerUp->position.y=position.y;
    powerUp->speed=120;
    powerUp->isAlive=true;
    powerUpList.push_back(powerUp);
}

void PowerUpSystem::update(float deltaTime,int windowH,Player& player)
{
    //缓存当前时间,用于判断护盾和射速增益是否到期
    auto currentTime=SDL_GetTicks();
    //遍历活跃道具列表,推进下落位置并回收离开屏幕的道具
    for(auto it=powerUpList.begin();it!=powerUpList.end();)
    {
        auto powerUp=*it;
        if(powerUp==nullptr)
        {
            it=powerUpList.erase(it);
            continue;
        }
        powerUp->position.y+=deltaTime*powerUp->speed;
        if(powerUp->position.y>windowH)
        {
            powerUpPool.release(powerUp);
            it=powerUpList.erase(it);
        }
        else
        {
            ++it;
        }
    }
    //护盾到期后清空结束时间,碰撞逻辑会据此恢复普通受伤规则
    if(shieldEndTime!=0&&currentTime>=shieldEndTime)
    {
        shieldEndTime=0;
    }
    //射速增益到期后恢复玩家原始射击冷却
    if(timeBuffEndTime!=0&&currentTime>=timeBuffEndTime)
    {
        timeBuffEndTime=0;
        player.coolTime=playerOriginalCoolTime;
        playerOriginalCoolTime=0;
    }
    //推进拾取提示倒计时,结束后清空HUD显示文本
    if(pickupNoticeTimer>0)
    {
        pickupNoticeTimer-=deltaTime;
        if(pickupNoticeTimer<0)
        {
            pickupNoticeTimer=0;
            pickupNotice.clear();
        }
    }
}

void PowerUpSystem::render(SDL_Renderer* renderer)
{
    //遍历活跃道具列表,把仍有纹理的道具绘制到主渲染器
    for(auto powerUp:powerUpList)
    {
        if(powerUp==nullptr||powerUp->texture==nullptr)
        {
            continue;
        }
        SDL_Rect rect={static_cast<int>(powerUp->position.x),
                       static_cast<int>(powerUp->position.y),
                       powerUp->width,powerUp->height};
        SDL_RenderCopy(renderer,powerUp->texture,NULL,&rect);
    }
}

bool PowerUpSystem::checkPickup(Player& player,SDL_Texture* explosionTex,
                                ObjectPool<Effect>& effectPool,std::list<Effect*>& effectList)
{
    //缓存当前时间,用于延长护盾和射速增益持续时间
    auto currentTime=SDL_GetTicks();
    for(auto it=powerUpList.begin();it!=powerUpList.end();)
    {
        auto powerUp=*it;
        if(powerUp==nullptr||powerUp->isAlive==false)
        {
            it=powerUpList.erase(it);
            continue;
        }
        //用AABB检测玩家飞机是否碰到当前道具
        if(player.position.x<powerUp->position.x+powerUp->width&&
           player.position.x+player.width>powerUp->position.x&&
           player.position.y<powerUp->position.y+powerUp->height&&
           player.position.y+player.height>powerUp->position.y)
        {
            //首次拾取该类型道具时开启HUD拾取提示
            auto typeIndex=static_cast<int>(powerUp->type);
            if(powerUpNotified[typeIndex]==false)
            {
                powerUpNotified[typeIndex]=true;
                pickupNoticeTimer=2.0f;
            }
            switch(powerUp->type)
            {
                case PowerUpType::LIFE:
                    //生命道具恢复玩家血量,但不超过最大生命值
                    player.health+=HEAL_AMOUNT;
                    if(player.health>player.maxHealth)
                    {
                        player.health=player.maxHealth;
                    }
                    if(pickupNoticeTimer==2.0f) 
                    {
                        pickupNotice="LIFE  +"+std::to_string(HEAL_AMOUNT)+" HP";
                    }                    
                    break;
                case PowerUpType::SHIELD:
                    //护盾道具刷新护盾结束时间,由受伤逻辑读取该状态
                    shieldEndTime=currentTime+SHIELD_DURATION;
                    if(pickupNoticeTimer==2.0f) 
                    {
                        pickupNotice="SHIELD  "+std::to_string(SHIELD_DURATION/1000)+"s";
                    }                    
                    break;
                case PowerUpType::TIME:
                    //射速道具保存玩家原始冷却,并临时缩短射击间隔
                    if(timeBuffEndTime==0)
                    {
                        playerOriginalCoolTime=player.coolTime;
                    }
                    player.coolTime=playerOriginalCoolTime/2;
                    timeBuffEndTime=currentTime+TIME_BUFF_DURATION;
                    if(pickupNoticeTimer==2.0f) 
                    {
                        pickupNotice="RAPID FIRE  "+std::to_string(TIME_BUFF_DURATION/1000)+"s";
                    }
                    break;
            }
            //拾取成功时从特效对象池取出特效并加入活跃特效列表
            if(explosionTex!=nullptr)
            {
                auto effect=effectPool.acquire();
                //设置拾取特效纹理、位置、帧动画参数和存活状态
                effect->texture=explosionTex;
                effect->width=32;
                effect->height=32;
                effect->position.x=powerUp->position.x+powerUp->width/2-16;
                effect->position.y=powerUp->position.y+powerUp->height/2-16;
                effect->totalFrame=4;
                effect->frameTime=35;
                effect->currentFrame=0;
                effect->lastFrameTime=SDL_GetTicks();
                effect->isAlive=true;
                effectList.push_back(effect);
            }
            //道具生效后归还对象池并从活跃列表移除
            powerUpPool.release(powerUp);
            it=powerUpList.erase(it);
            return true;
        }
        else
        {
            ++it;
        }
    }
    return false;
}

void PowerUpSystem::clear()
{
    //把仍在场景中的道具全部归还到对象池
    for(auto& powerUp:powerUpList)
    {
        if(powerUp!=nullptr)
        {
            powerUpPool.release(powerUp);
            powerUp=nullptr;
        }
    }
    powerUpList.clear();
    //释放生命、护盾和射速道具纹理资源
    if(lifeTexture!=nullptr)   
    { 
        SDL_DestroyTexture(lifeTexture);   
        lifeTexture=nullptr;   
    }
    if(shieldTexture!=nullptr) 
    { 
        SDL_DestroyTexture(shieldTexture); 
        shieldTexture=nullptr; 
    }
    if(timeTexture!=nullptr)   
    { 
        SDL_DestroyTexture(timeTexture);   
        timeTexture=nullptr;   
    }
    //重置三种道具的首次提示标记
    for(int i=0;i<3;i++) 
    {
        powerUpNotified[i]=false;
    }
    //清空当前拾取提示状态
    pickupNoticeTimer=0;
    pickupNotice.clear();
}
