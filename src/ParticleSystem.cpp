/**
 *  @file    ParticleSystem.cpp
 *  @brief   粒子子系统实现
 *
 *  粒子生命周期: 发射(设置初速/颜色/大小) → 每帧运动+衰减 → 生命归零归还池
 *  渲染: SDL_SetRenderDrawColor + SDL_RenderFillRectF,无纹理依赖
**/

#include "ParticleSystem.h"
#include <cmath>

void ParticleSystem::initialize(std::mt19937& gen,std::uniform_real_distribution<float>& dis)
{
    //缓存外部传入的随机数工具,供粒子生成时复用
    genPtr=&gen;
    disPtr=&dis;
}

void ParticleSystem::emitExplosion(SDL_FPoint center,int minCount,int maxCount)
{
    //随机确定本次爆炸粒子总数,再逐个从对象池取出粒子
    auto count=static_cast<int>(minCount+(*disPtr)(*genPtr)*(maxCount-minCount+1));
    for(int i=0;i<count;i++)
    {
        auto particle=particlePool.acquire();
        particle->position=center;
        //为每个粒子生成360度随机方向
        auto angle=(*disPtr)(*genPtr)*2.0f*3.14159265358979323846f;
        //为每个粒子生成爆炸初速度
        auto speed=(*disPtr)(*genPtr)*300.0f+80.0f;
        particle->velocity={std::cos(angle)*speed,std::sin(angle)*speed};
        //设置爆炸粒子的生命时长
        particle->maxLifetime=0.3f+(*disPtr)(*genPtr)*0.5f;
        particle->lifetime=particle->maxLifetime;
        //设置爆炸粒子的尺寸
        particle->size=3.0f+(*disPtr)(*genPtr)*5.0f;
        //设置爆炸粒子的颜色基调
        particle->r=255;
        particle->g=static_cast<Uint8>(180+(*disPtr)(*genPtr)*75);
        particle->b=0;
        particle->a=255;
        particle->isAlive=true;
        particleList.push_back(particle);
    }
}

void ParticleSystem::emitHitSparks(SDL_FPoint center,int minCount,int maxCount)
{
    //随机确定本次命中火花数量,再逐个从对象池取出粒子
    auto count=static_cast<int>(minCount+(*disPtr)(*genPtr)*(maxCount-minCount+1));
    for(int i=0;i<count;i++)
    {
        auto particle=particlePool.acquire();
        particle->position=center;
        //为每个粒子生成360度随机方向
        auto angle=(*disPtr)(*genPtr)*2.0f*3.14159265358979323846f;
        //为每个粒子生成命中火花初速度
        auto speed=(*disPtr)(*genPtr)*150.0f+40.0f;
        particle->velocity={std::cos(angle)*speed,std::sin(angle)*speed};
        //设置命中火花的生命时长
        particle->maxLifetime=0.1f+(*disPtr)(*genPtr)*0.2f;
        particle->lifetime=particle->maxLifetime;
        //设置命中火花的尺寸
        particle->size=2.0f+(*disPtr)(*genPtr)*3.0f;
        //设置命中火花的颜色基调
        particle->r=255;
        particle->g=static_cast<Uint8>(200+(*disPtr)(*genPtr)*55);
        particle->b=static_cast<Uint8>((*disPtr)(*genPtr)*100);
        particle->a=255;
        particle->isAlive=true;
        particleList.push_back(particle);
    }
}

void ParticleSystem::emitEngineTrail(SDL_FPoint center)
{
    //从对象池取出一个尾迹粒子并写入喷口附近的随机偏移
    auto particle=particlePool.acquire();
    particle->position={center.x+(*disPtr)(*genPtr)*6.0f-3.0f,center.y};
    //让尾迹向下飘散,同时保留轻微横向漂移
    particle->velocity={(*disPtr)(*genPtr)*20.0f-10.0f,(*disPtr)(*genPtr)*40.0f+30.0f};
    //设置尾迹粒子的生命时长
    particle->maxLifetime=0.15f+(*disPtr)(*genPtr)*0.15f;
    particle->lifetime=particle->maxLifetime;
    //设置尾迹粒子的尺寸
    particle->size=2.0f+(*disPtr)(*genPtr)*3.0f;
    //设置尾迹粒子的颜色基调
    particle->r=static_cast<Uint8>(30+(*disPtr)(*genPtr)*60);
    particle->g=static_cast<Uint8>(120+(*disPtr)(*genPtr)*80);
    particle->b=static_cast<Uint8>(200+(*disPtr)(*genPtr)*55);
    particle->a=220;
    particle->isAlive=true;
    particleList.push_back(particle);
}

void ParticleSystem::update(float deltaTime)
{
    //遍历所有活跃粒子,推进生命周期和位置
    for(auto it=particleList.begin();it!=particleList.end();)
    {
        auto particle=*it;
        if(particle==nullptr)
        {
            it=particleList.erase(it);
            continue;
        }
        //推进粒子剩余生命时间
        particle->lifetime-=deltaTime;
        if(particle->lifetime<=0)
        {
            //生命结束后归还对象池并移出活跃列表
            particle->isAlive=false;
            particlePool.release(particle);
            it=particleList.erase(it);
        }
        else
        {
            //更新粒子位置并模拟空气阻力
            particle->position.x+=particle->velocity.x*deltaTime;
            particle->position.y+=particle->velocity.y*deltaTime;
            particle->velocity.x*=0.98f;
            particle->velocity.y*=0.98f;
            //透明度按剩余生命比例渐隐
            float ratio=particle->lifetime/particle->maxLifetime;
            particle->a=static_cast<Uint8>(255*ratio);
            ++it;
        }
    }
}

void ParticleSystem::render(SDL_Renderer* renderer)
{
    //遍历活跃粒子,用彩色矩形绘制到Game主渲染器
    for(auto particle:particleList)
    {
        if(particle==nullptr||particle->isAlive==false)
        {
            continue;
        }
        SDL_SetRenderDrawColor(renderer,particle->r,particle->g,particle->b,particle->a);
        SDL_FRect rect={particle->position.x-particle->size/2,
                        particle->position.y-particle->size/2,
                        particle->size,particle->size};
        SDL_RenderFillRectF(renderer,&rect);
    }
    //恢复默认黑色绘制状态
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
}

void ParticleSystem::clear()
{
    //把仍在场景中的粒子全部归还到对象池
    for(auto& particle:particleList)
    {
        if(particle!=nullptr)
        {
            particlePool.release(particle);
            particle=nullptr;
        }
    }
    particleList.clear();
}
