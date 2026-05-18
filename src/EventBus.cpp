/**
 *  @file    EventBus.cpp
 *  @brief   事件总线实现 —— 简单的环形读写队列
**/

#include "EventBus.h"

void EventBus::publish(EventType type,SDL_FPoint position,int value)
{
    //把碰撞检测或玩法逻辑传入的信息组装成统一事件数据
    GameEvent event;
    event.type=type;
    event.position=position;
    event.value=value;
    //追加到内部事件队列末尾,等待MainScene后续统一消费
    queue.push_back(event);
}

bool EventBus::poll(GameEvent& event)
{
    //当前读取位置已经越过队列末尾时表示没有待处理事件
    if(readIndex>=static_cast<int>(queue.size()))
    {
        return false;
    }
    //把当前事件复制给调用方,通常由MainScene::dispatchEvents继续处理
    event=queue[readIndex];
    //推进读取位置,下次poll读取下一个事件
    readIndex++;
    return true;
}

void EventBus::clear()
{
    //清空内部事件队列,丢弃尚未被MainScene消费的事件
    queue.clear();
    //重置读取位置,保证下一次发布后从队首重新读取
    readIndex=0;
}
