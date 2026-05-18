/**
 *  @file    EnemySystem.cpp
 *  @brief   敌机系统实现
**/

#include "EnemySystem.h"
#include "Object.h"
#include "Config.h"
#include <SDL_image.h>
#include <cmath>

namespace
{
    constexpr float ATTACK_DRIFT_SPEED=42.0f;
}

void EnemySystem::initialize(SDL_Renderer* renderer)
{
    //用Game主渲染器加载enemy-1轻型敌机纹理到轻型敌机模板
    insect2Template.texture=IMG_LoadTexture(renderer,"assets/image/enemy-1.png");
    if(insect2Template.texture==NULL)
    {
        SDL_Log("EnemySystem::Initialize()加载enemy-1失败:%s",SDL_GetError());
    }
    else
    {
        SDL_QueryTexture(insect2Template.texture,NULL,NULL,
                         &insect2Template.width,&insect2Template.height);
        insect2Template.width/=5;
        insect2Template.height/=5;
    }
    //把Config中的轻型敌机参数写入enemy-1模板
    insect2Template.health=INSECT2_HP;
    insect2Template.maxHealth=INSECT2_HP;
    insect2Template.speed=INSECT2_SPEED;
    insect2Template.coolTime=INSECT2_COOLTIME;
    insect2Template.scoreValue=INSECT2_SCORE;
    //用Game主渲染器加载enemy-2重型敌机纹理到重型敌机模板
    insect1Template.texture=IMG_LoadTexture(renderer,"assets/image/enemy-2.png");
    if(insect1Template.texture==NULL)
    {
        SDL_Log("EnemySystem::Initialize()加载enemy-2失败:%s",SDL_GetError());
    }
    else
    {
        SDL_QueryTexture(insect1Template.texture,NULL,NULL,
                         &insect1Template.width,&insect1Template.height);
        insect1Template.width/=3;
        insect1Template.height/=3;
    }
    //把Config中的重型敌机参数写入enemy-2模板
    insect1Template.health=INSECT1_HP;
    insect1Template.maxHealth=INSECT1_HP;
    insect1Template.speed=INSECT1_SPEED;
    insect1Template.coolTime=INSECT1_COOLTIME;
    insect1Template.scoreValue=INSECT1_SCORE;
    //加载轻型敌机射击时复制使用的子弹模板纹理
    insect2BulletTemplate.texture=IMG_LoadTexture(renderer,"assets/image/enemy-bullet-1.png");
    if(insect2BulletTemplate.texture==NULL)
    {
        SDL_Log("EnemySystem::Initialize()加载enemy-bullet-1失败:%s",SDL_GetError());
    }
    else
    {
        SDL_QueryTexture(insect2BulletTemplate.texture,NULL,NULL,
                         &insect2BulletTemplate.width,&insect2BulletTemplate.height);
        insect2BulletTemplate.width/=2;
        insect2BulletTemplate.height/=2;
    }
    //加载重型敌机射击时复制使用的子弹模板纹理
    insect1BulletTemplate.texture=IMG_LoadTexture(renderer,"assets/image/enemy-bullet-2.png");
    if(insect1BulletTemplate.texture==NULL)
    {
        SDL_Log("EnemySystem::Initialize()加载enemy-bullet-2失败:%s",SDL_GetError());
    }
    else
    {
        SDL_QueryTexture(insect1BulletTemplate.texture,NULL,NULL,
                         &insect1BulletTemplate.width,&insect1BulletTemplate.height);
        insect1BulletTemplate.width/=2;
        insect1BulletTemplate.height/=2;
    }
}

void EnemySystem::generate(float difficulty,std::mt19937& gen,
                           std::uniform_real_distribution<float>& dis,int windowW,
                           float insect1Chance,bool forceSpawn)
{
    //根据传入概率在轻型和重型敌机之间选择本次生成类型
    auto spawnType=EnemySpawnType::Light;
    if(dis(gen)<insect1Chance)
    {
        spawnType=EnemySpawnType::Heavy;
    }
    //把选出的类型交给generateByType完成对象池获取和列表加入
    generateByType(spawnType,difficulty,gen,dis,windowW,forceSpawn);
}

void EnemySystem::generateByType(EnemySpawnType type,float difficulty,std::mt19937& gen,
                                 std::uniform_real_distribution<float>& dis,int windowW,
                                 bool forceSpawn)
{
    //随机刷怪模式按难度概率生成,波次系统调用时可通过forceSpawn强制生成
    if(forceSpawn==false&&dis(gen)>1.0f/(120.0f*difficulty))
    {
        return;
    }
    //根据生成类型选择要复制的敌机模板
    Enemy* templatePtr=&insect2Template;
    if(type==EnemySpawnType::Heavy||type==EnemySpawnType::Elite)
    {
        templatePtr=&insect1Template;
    }
    //从敌机对象池获取可用对象并复制模板属性
    Enemy* enemy=enemyPool.acquire();
    *enemy=*templatePtr;
    //把敌机放到屏幕上方随机横向位置,等待进入战场
    enemy->position.x=dis(gen)*(windowW-enemy->width);
    enemy->position.y=-static_cast<float>(enemy->height);
    //重型敌机使用贝塞尔曲线从屏幕上方滑入战场
    if(templatePtr==&insect1Template)
    {
        enemy->bezierT=0;
        enemy->bezierDuration=2.0f+dis(gen)*0.5f;
        float x0=enemy->position.x;
        float y0=enemy->position.y;
        float x3=dis(gen)*(windowW-enemy->width);
        float y3=80.0f+dis(gen)*120.0f;
        float midY=(y0+y3)*0.5f;
        float swing=120.0f+dis(gen)*80.0f;
        enemy->bezierPts[0]={x0,y0};
        enemy->bezierPts[1]={x0-swing*0.6f,midY};
        enemy->bezierPts[2]={x3+swing*0.6f,midY};
        enemy->bezierPts[3]={x3,y3};
    }
    //根据当前动态难度调整敌机速度和射击冷却
    enemy->speed=static_cast<int>(templatePtr->speed*(1.0f+(difficulty-1.0f)*SPEED_SCALE));
    enemy->coolTime=static_cast<Uint32>(templatePtr->coolTime/(1.0f+(difficulty-1.0f)*COOLTIME_SCALE));
    enemy->lastShootTime=SDL_GetTicks()-enemy->coolTime;
    //把初始化完成的敌机加入活跃列表,后续由update和render处理
    enemyList.push_back(enemy);
}

void EnemySystem::update(float deltaTime,const Player& player,int windowW,int windowH)
{
    //缓存当前时间和玩家中心点,供AI状态和射击冷却计算使用
    auto currentTime=SDL_GetTicks();
    auto playerCX=player.position.x+player.width/2;
    auto playerCY=player.position.y+player.height/2;
    //逐个更新活跃敌机列表中的敌机
    for(auto it=enemyList.begin();it!=enemyList.end();)
    {
        auto enemy=*it;
        auto enemyCX=enemy->position.x+enemy->width/2;
        auto enemyCY=enemy->position.y+enemy->height/2;
        auto dx=playerCX-enemyCX;
        auto dy=playerCY-enemyCY;
        auto dist=std::sqrt(dx*dx+dy*dy);
        auto speed=enemy->speed;
        //贝塞尔入场未完成时只更新曲线位置,暂不执行FSM行为
        if(enemy->bezierT<1.0f)
        {
            enemy->bezierT+=deltaTime/enemy->bezierDuration;
            if(enemy->bezierT>=1.0f)
            {
                enemy->bezierT=1.0f;
            }
            float t=enemy->bezierT;
            float u=1.0f-t;
            auto& pt=enemy->bezierPts;
            enemy->position.x=u*u*u*pt[0].x+3*u*u*t*pt[1].x+3*u*t*t*pt[2].x+t*t*t*pt[3].x;
            enemy->position.y=u*u*u*pt[0].y+3*u*u*t*pt[1].y+3*u*t*t*pt[2].y+t*t*t*pt[3].y;
        }
        else
        {
            //根据敌机和玩家距离以及敌机血量切换FSM状态
            float hpRatio=static_cast<float>(enemy->health)/enemy->maxHealth;
            switch(enemy->state)
            {
                case Enemy::PATROL:
                    if(dist<=FSM_PATROL_CHASE_DIST)
                    {
                        enemy->state=Enemy::CHASE;
                    }
                    break;
                case Enemy::CHASE:
                    if(hpRatio<FSM_FLEE_HP)
                    {
                        enemy->state=Enemy::FLEE;
                    }
                    else if(dist<=FSM_CHASE_ATTACK_DIST)
                    {
                        enemy->state=Enemy::ATTACK;
                    }
                    else if(dist>FSM_CHASE_PATROL_DIST)
                    {
                        enemy->state=Enemy::PATROL;
                    }
                    break;
                case Enemy::ATTACK:
                    if(hpRatio<FSM_FLEE_HP)
                    {
                        enemy->state=Enemy::FLEE;
                    }
                    else if(dist>FSM_ATTACK_CHASE_DIST)
                    {
                        enemy->state=Enemy::CHASE;
                    }
                    break;
                case Enemy::FLEE:
                    if(dist>FSM_FLEE_PATROL_DIST)
                    {
                        enemy->state=Enemy::PATROL;
                    }
                    break;
            }
            //根据当前FSM状态决定敌机本帧的移动方式
            switch(enemy->state)
            {
                case Enemy::PATROL:
                {
                    enemy->position.y+=deltaTime*speed*0.45f;
                    float phase=currentTime*0.0012f+enemy->position.x*0.007f;
                    enemy->position.x+=std::sin(phase)*deltaTime*55.0f;
                    break;
                }
                case Enemy::CHASE:
                    enemy->position.y+=deltaTime*speed*0.55f;
                    if(std::fabs(dx)>4.0f)
                    {
                        enemy->position.x+=(dx>0?1.0f:-1.0f)*deltaTime*speed*0.40f;
                    }
                    break;
                case Enemy::ATTACK:
                {
                    enemy->position.y+=deltaTime*speed*0.50f;
                    if(std::fabs(dx)>16.0f)
                    {
                        enemy->position.x+=(dx>0?1.0f:-1.0f)*deltaTime*speed*0.22f;
                    }
                    float phase=currentTime*0.002f+enemy->position.y*0.01f;
                    enemy->position.x+=std::sin(phase)*deltaTime*ATTACK_DRIFT_SPEED;
                    break;
                }
                case Enemy::FLEE:
                    enemy->position.y+=deltaTime*speed*0.75f;
                    if(std::fabs(dx)>4.0f)
                    {
                        enemy->position.x+=(dx>0?-1.0f:1.0f)*deltaTime*speed*FSM_FLEE_SPEED_MUL;
                    }
                    break;
            }
        }
        //把敌机横向位置限制在窗口范围内
        if(enemy->position.x<0)
        {
            enemy->position.x=0;
        }
        if(enemy->position.x>windowW-enemy->width)
        {
            enemy->position.x=static_cast<float>(windowW-enemy->width);
        }
        //敌机离开窗口后清除其子弹并归还到敌机对象池
        if(enemy->position.y<-200.0f||enemy->position.y>windowH)
        {
            clearBullet(enemy);
            enemyPool.release(enemy);
            it=enemyList.erase(it);
        }
        else
        {
            //只有完成入场且不处于逃跑状态的敌机才允许尝试射击
            bool canShoot=(enemy->bezierT>=1.0f&&enemy->position.y>0);
            Uint32 effectiveCool=enemy->coolTime;
            if(canShoot)
            {
                switch(enemy->state)
                {
                    case Enemy::PATROL:
                        effectiveCool=static_cast<Uint32>(enemy->coolTime*FSM_PATROL_COOL_MUL);
                        break;
                    case Enemy::FLEE:
                        canShoot=false;
                        break;
                    case Enemy::ATTACK:
                        effectiveCool=static_cast<Uint32>(enemy->coolTime*FSM_ATTACK_COOL_MUL);
                        break;
                    default:
                        break;
                }
            }
            //达到状态冷却时间且同屏子弹未超限时创建敌机子弹
            if(canShoot&&enemyBulletList.size()<ENEMY_BULLET_LIMIT
               &&currentTime-enemy->lastShootTime>effectiveCool)
            {
                shootBullet(enemy,player);
                enemy->lastShootTime=currentTime;
            }
            ++it;
        }
    }
}

void EnemySystem::render(SDL_Renderer* renderer)
{
    //遍历活跃敌机列表,把每架敌机绘制到Game主渲染器
    for(auto enemy:enemyList)
    {
        SDL_Rect enemyRect={static_cast<int>(enemy->position.x),
                            static_cast<int>(enemy->position.y),
                            enemy->width,enemy->height};
        SDL_RenderCopy(renderer,enemy->texture,NULL,&enemyRect);
    }
}

void EnemySystem::shootBullet(Enemy* enemy,const Player& player)
{
    //忽略空敌机指针,避免访问无效敌机数据
    if(enemy==nullptr)
    {
        return;
    }
    //根据敌机当前纹理判断使用轻型还是重型子弹模板
    bool isInsect1=(enemy->texture==insect1Template.texture);
    EnemyBullet* bt=isInsect1?&insect1BulletTemplate:&insect2BulletTemplate;
    //计算敌机中心点和玩家短时间后的预测中心点
    auto enemyCX=enemy->position.x+enemy->width/2.0f;
    auto enemyCY=enemy->position.y+enemy->height/2.0f;
    float predictTime=isInsect1?0.35f:0.25f;
    auto playerCX=player.position.x+player.width/2.0f+player.velocity.x*predictTime;
    auto playerCY=player.position.y+player.height/2.0f+player.velocity.y*predictTime;
    auto dirX=playerCX-enemyCX;
    auto dirY=playerCY-enemyCY;
    //只在玩家位于敌机下方时更新射击方向,避免敌机向上开火
    if(dirY>=0)
    {
        auto length=std::sqrt(dirX*dirX+dirY*dirY);
        if(length==0)
        {
            enemy->lastShootDirection=SDL_FPoint{0,1};
        }
        else
        {
            enemy->lastShootDirection=SDL_FPoint{dirX/length,dirY/length};
        }
        enemy->lastShootAngle=std::atan2(enemy->lastShootDirection.x,
                                         enemy->lastShootDirection.y)*180.0/3.14159265358979323846;
    }
    //重型敌机发射双发子弹,轻型敌机发射单发子弹
    int bulletCount=isInsect1?2:1;
    float offsets[2]={-8.0f,8.0f};
    for(int i=0;i<bulletCount;i++)
    {
        //从敌机子弹对象池获取可用子弹并复制模板数据
        auto bullet=bulletPool.acquire();
        *bullet=*bt;
        bullet->position.x=enemyCX-bullet->width/2.0f;
        bullet->position.y=enemyCY-bullet->height/2.0f;
        //重型敌机对子弹方向添加左右角度偏移形成双发弹道
        if(isInsect1)
        {
            float baseRad=static_cast<float>(enemy->lastShootAngle*3.14159265358979323846/180.0);
            float rad=baseRad+offsets[i]*3.14159265358979323846f/180.0f;
            bullet->direction={std::sin(rad),std::cos(rad)};
            bullet->angle=enemy->lastShootAngle+offsets[i];
        }
        else
        {
            bullet->direction=enemy->lastShootDirection;
            bullet->angle=enemy->lastShootAngle;
        }
        //记录发射该子弹的敌机,并把子弹加入活跃子弹列表
        bullet->owner=enemy;
        enemyBulletList.push_back(bullet);
    }
}

void EnemySystem::updateBullets(float deltaTime,int windowW,int windowH)
{
    //设置子弹越界回收边距,允许子弹稍微飞出屏幕后再回收
    int margin=10;
    for(auto it=enemyBulletList.begin();it!=enemyBulletList.end();)
    {
        auto bullet=*it;
        //把历史位置向后移动,为本帧拖尾记录腾出首位
        for(int i=3;i>0;i--)
        {
            bullet->trail[i]=bullet->trail[i-1];
        }
        if(bullet->trailCount<4)
        {
            bullet->trailCount++;
        }
        bullet->trail[0]=bullet->position;
        //根据子弹方向、速度和帧间隔更新子弹位置
        bullet->position.x+=bullet->direction.x*deltaTime*bullet->speed;
        bullet->position.y+=bullet->direction.y*deltaTime*bullet->speed;
        //子弹离开窗口范围后从活跃列表移除并归还对象池
        if(bullet->position.y>windowH+margin||bullet->position.y<-margin
           ||bullet->position.x<-margin||bullet->position.x>windowW+margin)
        {
            bulletPool.release(bullet);
            it=enemyBulletList.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void EnemySystem::renderBullets(SDL_Renderer* renderer)
{
    for(auto bullet:enemyBulletList)
    {
        //先根据子弹历史位置绘制逐渐变淡的拖尾
        for(int i=0;i<bullet->trailCount;i++)
        {
            auto alpha=static_cast<Uint8>(255*(bullet->trailCount-i)/(bullet->trailCount+1)*0.4f);
            SDL_SetTextureAlphaMod(bullet->texture,alpha);
            SDL_Rect trailRect={static_cast<int>(bullet->trail[i].x),
                              static_cast<int>(bullet->trail[i].y),
                              bullet->width,bullet->height};
            SDL_RenderCopyEx(renderer,bullet->texture,NULL,&trailRect,
                             bullet->angle,NULL,SDL_FLIP_NONE);
        }
        //恢复纹理不透明度后绘制当前子弹本体
        SDL_SetTextureAlphaMod(bullet->texture,255);
        SDL_Rect bulletRect={static_cast<int>(bullet->position.x),
                            static_cast<int>(bullet->position.y),
                            bullet->width,bullet->height};
        SDL_RenderCopyEx(renderer,bullet->texture,NULL,&bulletRect,
                         bullet->angle,NULL,SDL_FLIP_NONE);
    }
}

void EnemySystem::clearBullet(Enemy* enemy)
{
    //忽略空敌机指针,避免清理不存在的归属关系
    if(enemy==nullptr)
    {
        return;
    }
    //遍历活跃子弹列表,查找并清除owner指向该敌机的子弹
    for(auto it=enemyBulletList.begin();it!=enemyBulletList.end();)
    {
        auto bullet=*it;
        if(bullet==nullptr)
        {
            it=enemyBulletList.erase(it);
            continue;
        }
        if(bullet->owner==enemy)
        {
            //断开子弹与敌机的归属关系并归还到子弹对象池
            bullet->owner=nullptr;
            bulletPool.release(bullet);
            it=enemyBulletList.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void EnemySystem::clear()
{
    //把仍在场景中的敌机子弹全部归还到子弹对象池
    for(auto& bullet:enemyBulletList)
    {
        if(bullet!=nullptr)
        {
            bulletPool.release(bullet);
            bullet=nullptr;
        }
    }
    enemyBulletList.clear();
    //释放轻型敌机子弹模板持有的纹理资源
    if(insect2BulletTemplate.texture!=nullptr)
    {
        SDL_DestroyTexture(insect2BulletTemplate.texture);
        insect2BulletTemplate.texture=nullptr;
    }
    //释放重型敌机子弹模板持有的纹理资源
    if(insect1BulletTemplate.texture!=nullptr)
    {
        SDL_DestroyTexture(insect1BulletTemplate.texture);
        insect1BulletTemplate.texture=nullptr;
    }
    //把仍在场景中的敌机全部归还到敌机对象池
    for(auto& enemy:enemyList)
    {
        if(enemy!=nullptr)
        {
            enemyPool.release(enemy);
            enemy=nullptr;
        }
    }
    enemyList.clear();
    //释放重型敌机模板持有的纹理资源
    if(insect1Template.texture!=nullptr)
    {
        SDL_DestroyTexture(insect1Template.texture);
        insect1Template.texture=nullptr;
    }
    //释放轻型敌机模板持有的纹理资源
    if(insect2Template.texture!=nullptr)
    {
        SDL_DestroyTexture(insect2Template.texture);
        insect2Template.texture=nullptr;
    }
}
