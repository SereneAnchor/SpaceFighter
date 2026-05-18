/**
 *  @file    EventBus.h
 *  @brief   游戏事件总线: 用队列解耦碰撞检测和效果响应
**/

#ifndef EVENTBUS
#define EVENTBUS

#include <SDL.h>
#include <vector>

/**
 *  @brief  游戏事件类型
 *
 *  @param  EnemyKilled    敌机死亡,value为得分
 *  @param  EnemyHit       敌机被击中
 *  @param  PlayerHit      玩家被击中,value为伤害
 *  @param  PlayerDied     玩家死亡
 *  @param  PowerUpPicked  玩家拾取道具
**/
enum class EventType
{
    EnemyKilled,
    EnemyHit,
    PlayerHit,
    PlayerDied,
    PowerUpPicked
};

/**
 *  @brief  游戏事件数据
 *
 *  @param  type      事件类型
 *  @param  position  事件发生位置
 *  @param  value     事件附加数值
**/
struct GameEvent
{
    EventType type=EventType::EnemyKilled;
    SDL_FPoint position={0,0};
    int value=0;
};

/**
 *  @brief  主场景内部使用的游戏事件队列
**/
class EventBus
{
    public:
        //将碰撞、死亡或拾取产生的游戏事件追加到内部事件队列
        void publish(EventType type,SDL_FPoint position={0,0},int value=0);

        //从内部事件队列取出一个事件,供MainScene统一分发处理
        bool poll(GameEvent& event);

        //清空尚未被MainScene消费的事件并重置读取位置
        void clear();

        //获取内部事件队列当前保存的事件总数
        int count() const
        {
            return static_cast<int>(queue.size());
        }

    private:
        //保存碰撞检测和玩法逻辑发布的游戏事件
        std::vector<GameEvent> queue;

        //MainScene下一次poll时要读取的事件下标
        int readIndex=0;
};

#endif
