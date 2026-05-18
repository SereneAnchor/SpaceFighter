/**
 *  @file    WaveSystem.h
 *  @brief   波次系统: 控制敌机分批生成、波间休息和敌机类型权重
**/

#ifndef WAVESYSTEM
#define WAVESYSTEM

#include <random>

#include "EnemySystem.h"

/**
 *  @brief  主场景使用的敌机波次调度系统
**/
class WaveSystem
{
    public:
        //把波次流程恢复到第一波开始前的休息状态
        void reset();

        //由MainScene每帧调用,根据计时器驱动EnemySystem生成敌机并判断清场
        void update(float deltaTime,EnemySystem& enemySystem,float difficulty,
                    std::mt19937& gen,std::uniform_real_distribution<float>& dis,int windowW);

        //返回当前波次编号,供HUD和结算页显示
        int getWave() const
        {
            return currentWave;
        }

        //返回波次系统当前是否正在等待下一波开始
        bool isResting() const
        {
            return inRest;
        }

        //返回WAVE提示剩余显示时间,供HUD决定是否绘制波次开始提示
        float getWaveTextTimer() const
        {
            return waveTextTimer;
        }

        //返回WAVE COMPLETE提示剩余显示时间,供HUD决定是否绘制清场提示
        float getCompleteTimer() const
        {
            return completeTimer;
        }

    private:
        //当前已经开始的波次编号,0表示尚未开始第一波
        int currentWave=0;

        //当前波次还需要交给EnemySystem生成的敌机数量
        int enemiesToSpawn=0;

        //距离下一次生成敌机的倒计时
        float spawnTimer=0;

        //当前波次内两次敌机生成之间的间隔
        float spawnInterval=0.55f;

        //波次系统是否处于波间休息状态
        bool inRest=true;

        //波间休息剩余时间
        float restTimer=2.0f;

        //HUD显示WAVE提示的剩余时间
        float waveTextTimer=0;

        //HUD显示WAVE COMPLETE提示的剩余时间
        float completeTimer=0;

        //当前波次是否仍在生成或等待清场
        bool waveActive=false;

        //根据当前波次使用权重随机选择要生成的敌机类型
        EnemySpawnType chooseEnemySpawnType(std::mt19937& gen) const;
};

#endif
