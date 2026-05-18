/**
 *  @file    WaveSystem.cpp
 *  @brief   波次系统实现
 *
 *  状态机:
 *    REST(2.5s,显示WAVE X) → 逐个生成敌机 → 全部生成完毕
 *    → 等待敌机清场 → REST(下一波)
 *
 *  波次公式: 敌机数 = 3 + wave*2
 *  敌机类型通过波次权重随机选择,后期重型敌机最高50%
**/

#include "WaveSystem.h"
#include "Config.h"
#include "WeightedRandom.h"

#include <vector>

void WaveSystem::reset()
{
    //重置波次编号、待生成敌机数量和生成倒计时
    currentWave=0;
    enemiesToSpawn=0;
    spawnTimer=0;
    //重置波间休息状态、休息倒计时和HUD提示计时
    inRest=true;
    restTimer=WAVE_REST_TIME;
    waveTextTimer=0;
    completeTimer=0;
    //标记当前没有正在生成或等待清场的波次
    waveActive=false;
}

void WaveSystem::update(float deltaTime,EnemySystem& enemySystem,float difficulty,
                        std::mt19937& gen,std::uniform_real_distribution<float>& dis,int windowW)
{
    if(inRest)
    {
        //处于波间休息时只推进休息倒计时
        restTimer-=deltaTime;
        if(restTimer<=0)
        {
            //休息结束后进入下一波
            currentWave++;
            //根据波次计算本波需要生成的敌机数量
            enemiesToSpawn=BASE_ENEMY_COUNT+currentWave*ENEMY_PER_WAVE;
            if(enemiesToSpawn>18) 
            {
                //限制单波最大生成数量,避免后期敌机过多
                enemiesToSpawn=18; 
            }
            //随着波次增加逐渐缩短敌机生成间隔
            spawnInterval=SPAWN_INTERVAL-currentWave*0.02f;
            if(spawnInterval<0.18f)
            {
                spawnInterval=0.18f;
            }            
            //让新波开始后立即生成第一架敌机
            spawnTimer=0;
            //离开休息状态并标记当前波次进行中
            inRest=false;
            waveActive=true;
            //通知HUD显示WAVE提示
            waveTextTimer=2.0f;
        }
    }
    else if(enemiesToSpawn>0)
    {
        //当前波次仍有敌机待生成时推进生成倒计时
        spawnTimer-=deltaTime;
        if(spawnTimer<=0)
        {
            //按当前波次权重选择敌机类型
            auto spawnType=chooseEnemySpawnType(gen);
            //把实际生成交给EnemySystem,WaveSystem只控制节奏和类型
            enemySystem.generateByType(spawnType,difficulty,gen,dis,windowW,true);
            //记录本波剩余待生成敌机数量
            enemiesToSpawn--;
            //重置下一次敌机生成倒计时
            spawnTimer=spawnInterval;
        }
    }
    else if(waveActive)
    {
        //本波敌机全部生成完毕后等待EnemySystem清空敌机和子弹
        if(enemySystem.getList().empty()&&enemySystem.getBulletList().empty())
        {
            //敌机和敌机子弹都清空后进入下一轮波间休息
            waveActive=false;
            inRest=true;
            restTimer=WAVE_REST_TIME;
            //通知HUD显示WAVE COMPLETE提示
            completeTimer=WAVE_TEXT_TIME;
        }
    }
    //推进HUD的WAVE提示倒计时
    if(waveTextTimer>0)
    {
        waveTextTimer-=deltaTime;
        if(waveTextTimer<0) 
        {
            waveTextTimer=0;
        }        
    }
    //推进HUD的WAVE COMPLETE提示倒计时
    if(completeTimer>0)
    {
        completeTimer-=deltaTime;
        if(completeTimer<0) 
        {
            completeTimer=0;
        }
    }
}

EnemySpawnType WaveSystem::chooseEnemySpawnType(std::mt19937& gen) const
{
    //默认只生成轻型敌机,后续波次逐渐提高重型敌机权重
    float lightWeight=100.0f;
    float heavyWeight=0.0f;
    //第9波后轻型和重型敌机各占一半权重
    if(currentWave>=9)
    {
        lightWeight=50.0f;
        heavyWeight=50.0f;
    }
    //第7波后重型敌机权重提高到40%
    else if(currentWave>=7)
    {
        lightWeight=60.0f;
        heavyWeight=40.0f;
    }
    //第5波后重型敌机权重提高到30%
    else if(currentWave>=5)
    {
        lightWeight=70.0f;
        heavyWeight=30.0f;
    }
    //第3波后开始少量生成重型敌机
    else if(currentWave>=3)
    {
        lightWeight=85.0f;
        heavyWeight=15.0f;
    }
    //Elite为未来第三类敌机预留,当前权重保持为0
    std::vector<WeightedChoice<EnemySpawnType>> choices={
        {EnemySpawnType::Light,lightWeight},
        {EnemySpawnType::Heavy,heavyWeight},
        {EnemySpawnType::Elite,0.0f}
    };
    //使用通用加权随机工具选择本次要生成的敌机类型
    return chooseWeighted(choices,gen,EnemySpawnType::Light);
}
