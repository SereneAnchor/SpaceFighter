/**
 *  @file    ParticleSystem.h
 *  @brief   粒子系统
 *
 *  负责生成、更新和渲染爆炸、命中火花和引擎尾迹粒子
**/

#ifndef PARTICLESYSTEM
#define PARTICLESYSTEM

#include <list>
#include <random>
#include <SDL.h>
#include "Object.h"

/**
 *  @brief  粒子系统
 *
 *  使用对象池复用粒子对象,减少频繁new/delete
**/
class ParticleSystem
{
    public:
        //初始化随机数工具引用,供后续粒子发射函数复用
        void initialize(std::mt19937& gen,std::uniform_real_distribution<float>& dis);

        //在指定中心点生成爆炸粒子
        void emitExplosion(SDL_FPoint center,int minCount,int maxCount);

        //在指定中心点生成命中火花
        void emitHitSparks(SDL_FPoint center,int minCount,int maxCount);

        //在玩家尾部喷口位置生成引擎尾迹
        void emitEngineTrail(SDL_FPoint center);

        //更新所有粒子的生命周期、位置和透明度
        void update(float deltaTime);

        //把所有存活粒子绘制到Game主渲染器
        void render(SDL_Renderer* renderer);

        //清理所有粒子并归还对象池
        void clear();

        //获取粒子列表
        std::list<Particle*>& getList()
        {
            return particleList;
        }

        //获取粒子对象池
        ObjectPool<Particle>& getPool()
        {
            return particlePool;
        }

    private:
        //存活粒子列表
        std::list<Particle*> particleList;

        //粒子对象池
        ObjectPool<Particle> particlePool;

        //上次生成尾迹时间
        Uint32 lastEngineTrailTime=0;

        //随机数引擎指针
        std::mt19937* genPtr=nullptr;

        //随机分布指针
        std::uniform_real_distribution<float>* disPtr=nullptr;
};

#endif
