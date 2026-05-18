/**
 *  @file    MainScene.cpp
 *  @brief   游戏主场景实现
 *
 *  MainScene作为战斗编排层,负责玩家控制、子系统调度、碰撞响应和结算切换
**/

#include "MainScene.h"
#include "MenuScene.h"
#include "ResultScene.h"
#include "Game.h"
#include "Config.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <cmath>
#include <string>

MainScene::MainScene():game(Game::getInstance())
{
    std::cout<<"MainScene::构造方法."<<std::endl;
}

MainScene::~MainScene()
{
    std::cout<<"MainScene::析构方法."<<std::endl;
}

void MainScene::initialScene()
{
    //初始化随机数引擎和0到1随机分布,供敌机、粒子和道具系统复用
    std::random_device rd;
    gen=std::mt19937(rd());
    dis=std::uniform_real_distribution<float>(0.0f,1.0f);
    //加载战斗场景循环滚动背景纹理
    backgroundTexture=IMG_LoadTexture(game.getRenderer(),"assets/image/bg.png");
    if(backgroundTexture==NULL)
    {
        SDL_Log("MainScene::InitialScene()加载背景失败:%s",SDL_GetError());
    }
    //加载玩家飞机纹理并缩放为游戏内显示尺寸
    player.texture=IMG_LoadTexture(game.getRenderer(),"assets/image/Player.png");
    if(player.texture==NULL)
    {
        SDL_Log("MainScene::InitialScene()加载玩家失败:%s",SDL_GetError());
    }
    else
    {
        SDL_QueryTexture(player.texture,NULL,NULL,&player.width,&player.height);
        player.width/=4;
        player.height/=4;
    }
    //把玩家放在窗口底部中央并设置默认射击冷却
    player.position.x=game.getWindowWidth()/2.0f-player.width/2.0f;
    player.position.y=static_cast<float>(game.getWindowHeight()-player.height);
    player.coolTime=280;
    //加载玩家子弹模板纹理,后续射击时从模板复制到对象池子弹
    playerBulletTemplate.texture=IMG_LoadTexture(game.getRenderer(),"assets/image/player-bullet-1.png");
    if(playerBulletTemplate.texture==NULL)
    {
        SDL_Log("MainScene::InitialScene()加载玩家子弹失败:%s",SDL_GetError());
    }
    else
    {
        SDL_QueryTexture(playerBulletTemplate.texture,NULL,NULL,
                         &playerBulletTemplate.width,&playerBulletTemplate.height);
        playerBulletTemplate.width/=2;
        playerBulletTemplate.height/=2;
    }
    //初始化敌机、粒子和道具子系统
    enemySystem.initialize(game.getRenderer());
    particleSystem.initialize(gen,dis);
    powerUpSystem.initialize(game.getRenderer());
    //初始化游戏内UI并准备暂停菜单项
    ui.initialize(game.getRenderer(),"assets/font/VonwaonBitmap-16px.ttf",24);
    pauseMenuItems.clear();
    pauseMenuItems.push_back(MenuItem{"Continue"});
    pauseMenuItems.push_back(MenuItem{"Restart"});
    pauseMenuItems.push_back(MenuItem{"Main Menu"});
    selectedPauseIndex=0;
    //加载命中和爆炸共用的序列帧纹理
    explosionTexture=IMG_LoadTexture(game.getRenderer(),"assets/effect/explosion.png");
    if(explosionTexture==NULL)
    {
        SDL_Log("MainScene::InitialScene()加载特效失败:%s",SDL_GetError());
    }
    //加载命中、爆炸和拾取音效并设置各自音量
    hitSound=Mix_LoadWAV("assets/sound/eff5.wav");
    if(hitSound!=nullptr) Mix_VolumeChunk(hitSound,48);
    explosionSound=Mix_LoadWAV("assets/sound/explosion3.wav");
    if(explosionSound!=nullptr) Mix_VolumeChunk(explosionSound,64);
    pickupSound=Mix_LoadWAV("assets/sound/explosion1.wav");
    if(pickupSound!=nullptr) Mix_VolumeChunk(pickupSound,56);
    //加载护盾和引擎火焰视觉纹理并记录原始尺寸
    shieldTexture=IMG_LoadTexture(game.getRenderer(),"assets/image/shield.png");
    if(shieldTexture!=NULL)
    {
        SDL_QueryTexture(shieldTexture,NULL,NULL,&shieldTexW,&shieldTexH);
    }
    else { SDL_Log("加载护盾纹理失败:%s",SDL_GetError()); }
    fireTexture=IMG_LoadTexture(game.getRenderer(),"assets/image/fire.png");
    if(fireTexture!=NULL)
    {
        SDL_QueryTexture(fireTexture,NULL,NULL,&fireTexW,&fireTexH);
    }
    else { SDL_Log("加载火焰纹理失败:%s",SDL_GetError()); }
    //加载并循环播放战斗背景音乐
    bgm=Mix_LoadMUS("assets/music/03_Racing_Through_Asteroids_Loop.ogg");
    if(bgm!=nullptr)
    {
        Mix_VolumeMusic(28);
        Mix_PlayMusic(bgm,-1);
    }
}

void MainScene::updateScene(float deltaTime)
{
    //暂停时跳过战斗逻辑更新,保留当前画面状态
    if(isPaused)
    {
        return;
    }
    //根据分数计算基础动态难度
    float scoreFactor=1.0f+gDifficultyOffset+player.score/800.0f*DIFFICULTY_RAMP;
    //根据玩家命中率修正难度,命中样本不足时按默认命中率处理
    float accuracy=0.5f;
    if(playerShotsFired>=DDA_ACCURACY_MIN_SHOTS)
    {
        accuracy=static_cast<float>(playerHitsLanded)/playerShotsFired;
    }
    float accuracyMod=(accuracy-0.5f)*DDA_ACCURACY_WEIGHT*2.0f;
    //根据近期受伤量降低难度,避免连续受伤时压力继续升高
    float damageMod=recentDamageTaken*DDA_DAMAGE_WEIGHT;
    if(damageMod>DDA_DAMAGE_CAP) damageMod=DDA_DAMAGE_CAP;
    //合成目标难度并限制在配置范围内
    targetDifficulty=scoreFactor+accuracyMod-damageMod;
    if(targetDifficulty<1.0f) targetDifficulty=1.0f;
    if(targetDifficulty>DIFFICULTY_MAX) targetDifficulty=DIFFICULTY_MAX;
    //平滑追赶目标难度,避免敌机节奏突然跳变
    difficulty+=(targetDifficulty-difficulty)*deltaTime*DDA_SMOOTH_SPEED;
    //推进伤害统计窗口,到期后清空近期受伤量
    damageWindowTimer-=deltaTime;
    if(damageWindowTimer<=0)
    {
        recentDamageTaken=0;
        damageWindowTimer=DDA_DAMAGE_WINDOW;
    }
    //推进背景滚动位置
    updateBackground(deltaTime);
    if(player.isAlive)
    {
        //玩家存活时处理输入、子弹、波次、敌机和碰撞
        keyboardControl(deltaTime);
        updatePlayerBullet(deltaTime);
        waveSystem.update(deltaTime,enemySystem,difficulty,gen,dis,game.getWindowWidth());
        enemySystem.update(deltaTime,player,game.getWindowWidth(),game.getWindowHeight());
        enemySystem.updateBullets(deltaTime,game.getWindowWidth(),game.getWindowHeight());
        checkPlayerBulletHitEnemy();
        checkEnemyBulletHitPlayer();
        checkEnemyHitPlayer();
    }
    //更新特效、粒子和道具系统
    updateEffect();
    particleSystem.update(deltaTime);
    powerUpSystem.update(deltaTime,game.getWindowHeight(),player);
    //玩家拾取道具成功时播放拾取音效
    if(powerUpSystem.checkPickup(player,explosionTexture,effectPool,effectList))
    {
        if(pickupSound!=nullptr) Mix_PlayChannel(-1,pickupSound,0);
    }
    //统一处理碰撞产生的事件,包括特效、粒子、音效和计分
    dispatchEvents();
    //玩家死亡后延迟进入结算场景,保留爆炸反馈时间
    if(resultScenePending)
    {
        resultSceneTimer-=deltaTime;
        if(resultSceneTimer<=0)
        {
            enterResultScene();
            return;
        }
    }
}

void MainScene::renderScene()
{
    //按从远到近的顺序绘制背景、实体、特效和HUD
    renderBackground();
    renderPlayerBullet();
    if(player.isAlive)
    {
        auto currentTime=SDL_GetTicks();
        //在玩家飞机下方绘制带透明度脉冲的引擎火焰
        if(fireTexture!=nullptr&&fireTexW>0)
        {
            float scale=player.width/static_cast<float>(fireTexW)*0.55f;
            int flameW=static_cast<int>(fireTexW*scale);
            int flameH=static_cast<int>(fireTexH*scale);
            SDL_Rect flameRect={static_cast<int>(player.position.x+player.width/2.0f-flameW/2.0f),
                                static_cast<int>(player.position.y+player.height*0.85f),
                                flameW,flameH};
            auto pulse=static_cast<Uint8>(200+55*std::sin(currentTime*0.008f));
            SDL_SetTextureAlphaMod(fireTexture,pulse);
            SDL_RenderCopy(game.getRenderer(),fireTexture,NULL,&flameRect);
            SDL_SetTextureAlphaMod(fireTexture,255);
        }
        SDL_Rect playerRect={static_cast<int>(player.position.x),
                             static_cast<int>(player.position.y),
                             player.width,player.height};
        //玩家处于护盾状态时让飞机纹理偏蓝
        if(powerUpSystem.shieldEndTime!=0&&currentTime<powerUpSystem.shieldEndTime)
        {
            SDL_SetTextureColorMod(player.texture,100,180,255);
        }
        //玩家受击后的短时间内用红色闪烁提示无敌状态
        else if(lastHurtTime!=0&&currentTime-lastHurtTime<180)
        {
            SDL_SetTextureColorMod(player.texture,255,80,80);
        }
        SDL_RenderCopy(game.getRenderer(),player.texture,NULL,&playerRect);
        SDL_SetTextureColorMod(player.texture,255,255,255);
        //玩家护盾有效时在飞机上方叠加护盾光环
        if(powerUpSystem.shieldEndTime!=0&&currentTime<powerUpSystem.shieldEndTime
           &&shieldTexture!=nullptr&&shieldTexW>0)
        {
            float sScale=player.width/static_cast<float>(shieldTexW)*1.8f;
            int shieldW=static_cast<int>(shieldTexW*sScale);
            int shieldH=static_cast<int>(shieldTexH*sScale);
            SDL_Rect shieldRect={static_cast<int>(player.position.x+player.width/2.0f-shieldW/2.0f),
                                 static_cast<int>(player.position.y+player.height/2.0f-shieldH/2.0f),
                                 shieldW,shieldH};
            auto alpha=static_cast<Uint8>(140+115*std::sin(currentTime*0.006f));
            SDL_SetTextureAlphaMod(shieldTexture,alpha);
            SDL_RenderCopy(game.getRenderer(),shieldTexture,NULL,&shieldRect);
            SDL_SetTextureAlphaMod(shieldTexture,255);
        }
    }
    //绘制敌机、敌机子弹、特效、道具、粒子和HUD
    enemySystem.render(game.getRenderer());
    enemySystem.renderBullets(game.getRenderer());
    renderEffect();
    powerUpSystem.render(game.getRenderer());
    particleSystem.render(game.getRenderer());
    renderGameInfo();
}

void MainScene::handleEvent(SDL_Event *event)
{
    //空事件直接忽略,避免访问无效输入数据
    if(event==nullptr)
    {
        return;
    }
    //只处理第一次按下的键盘事件,避免按键连发重复执行菜单操作
    if(event->type==SDL_KEYDOWN&&event->key.repeat==0)
    {
        auto key=event->key.keysym.scancode;
        if(isPaused&&player.isAlive)
        {
            //暂停菜单打开时优先处理菜单选择和确认
            if(key==SDL_SCANCODE_UP)
            {
                selectedPauseIndex=GameUI::moveSelection(selectedPauseIndex,-1,
                                                         static_cast<int>(pauseMenuItems.size()));
                return;
            }
            if(key==SDL_SCANCODE_DOWN)
            {
                selectedPauseIndex=GameUI::moveSelection(selectedPauseIndex,1,
                                                         static_cast<int>(pauseMenuItems.size()));
                return;
            }
            if(key==SDL_SCANCODE_RETURN)
            {
                if(selectedPauseIndex==0)
                {
                    isPaused=false;
                }
                else if(selectedPauseIndex==1)
                {
                    game.requestSceneChange(new MainScene());
                }
                else if(selectedPauseIndex==2)
                {
                    game.requestSceneChange(new MenuScene());
                }
                return;
            }
        }
        switch(key)
        {
            case SDL_SCANCODE_P:
                //P键在玩家存活时切换暂停状态
                if(player.isAlive)
                {
                    isPaused=!isPaused;
                    selectedPauseIndex=0;
                }
                break;
            case SDL_SCANCODE_R:
                //R键在死亡或暂停时请求重开主游戏场景
                if(player.isAlive==false||isPaused)
                {
                    game.requestSceneChange(new MainScene());
                }
                break;
            case SDL_SCANCODE_ESCAPE:
                //ESC键请求返回主菜单场景
                game.requestSceneChange(new MenuScene());
                break;
            default:
                break;
        }
    }
}

void MainScene::clearScene()
{
    //释放背景纹理
    if(backgroundTexture!=nullptr)
    {
        SDL_DestroyTexture(backgroundTexture);
        backgroundTexture=nullptr;
    }
    //停止并释放背景音乐
    if(bgm!=nullptr)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(bgm);
        bgm=nullptr;
    }
    //停止所有音效通道并释放场景持有的音效资源
    Mix_HaltChannel(-1);
    if(hitSound!=nullptr)      { Mix_FreeChunk(hitSound);      hitSound=nullptr;      }
    if(explosionSound!=nullptr){ Mix_FreeChunk(explosionSound);explosionSound=nullptr;}
    if(pickupSound!=nullptr)   { Mix_FreeChunk(pickupSound);   pickupSound=nullptr;   }
    //释放游戏内UI资源
    ui.clear();
    //释放特效纹理并归还活跃特效对象
    if(explosionTexture!=nullptr)
    {
        SDL_DestroyTexture(explosionTexture);
        explosionTexture=nullptr;
    }
    for(auto& effect:effectList)
    {
        if(effect!=nullptr) { effectPool.release(effect); effect=nullptr; }
    }
    effectList.clear();
    //释放玩家飞机纹理
    if(player.texture!=nullptr)
    {
        SDL_DestroyTexture(player.texture);
        player.texture=nullptr;
    }
    //释放玩家子弹模板纹理并归还活跃子弹对象
    if(playerBulletTemplate.texture!=nullptr)
    {
        SDL_DestroyTexture(playerBulletTemplate.texture);
        playerBulletTemplate.texture=nullptr;
    }
    for(auto& bullet:bulletList)
    {
        if(bullet!=nullptr) { playerBulletPool.release(bullet); bullet=nullptr; }
    }
    bulletList.clear();
    //释放护盾和引擎火焰视觉纹理
    if(shieldTexture!=nullptr){ SDL_DestroyTexture(shieldTexture); shieldTexture=nullptr; }
    if(fireTexture!=nullptr)  { SDL_DestroyTexture(fireTexture);   fireTexture=nullptr;   }
    //清理敌机、粒子和道具子系统持有的资源
    enemySystem.clear();
    particleSystem.clear();
    powerUpSystem.clear();
}

void MainScene::updateBackground(float deltaTime)
{
    //按背景速度推进滚动位置,超过一屏后循环回绕
    backgroundY+=deltaTime*backgroundSpeed;
    if(backgroundY>=game.getWindowHeight())
    {
        backgroundY-=game.getWindowHeight();
    }
}

void MainScene::renderBackground()
{
    //没有背景纹理时跳过绘制
    if(backgroundTexture==nullptr) return;
    //绘制两张首尾相接的背景图形成纵向循环滚动
    SDL_Rect rectA={0,static_cast<int>(backgroundY),
                    game.getWindowWidth(),game.getWindowHeight()};
    SDL_Rect rectB={0,static_cast<int>(backgroundY)-game.getWindowHeight(),
                    game.getWindowWidth(),game.getWindowHeight()};
    SDL_RenderCopy(game.getRenderer(),backgroundTexture,NULL,&rectA);
    SDL_RenderCopy(game.getRenderer(),backgroundTexture,NULL,&rectB);
}

void MainScene::keyboardControl(float deltaTime)
{
    //读取当前键盘状态并缓存移动前位置用于计算玩家速度
    auto keyState=SDL_GetKeyboardState(NULL);
    SDL_FPoint oldPosition=player.position;
    //根据WASD输入更新玩家飞机位置
    if(keyState[SDL_SCANCODE_W]) player.position.y-=deltaTime*player.speed;
    if(keyState[SDL_SCANCODE_S]) player.position.y+=deltaTime*player.speed;
    if(keyState[SDL_SCANCODE_A]) player.position.x-=deltaTime*player.speed;
    if(keyState[SDL_SCANCODE_D]) player.position.x+=deltaTime*player.speed;
    //把玩家位置限制在窗口范围内
    if(player.position.x<0) player.position.x=0;
    if(player.position.x>game.getWindowWidth()-player.width)
        player.position.x=static_cast<float>(game.getWindowWidth()-player.width);
    if(player.position.y<0) player.position.y=0;
    if(player.position.y>game.getWindowHeight()-player.height)
        player.position.y=static_cast<float>(game.getWindowHeight()-player.height);
    if(deltaTime>0)
    {
        player.velocity.x=(player.position.x-oldPosition.x)/deltaTime;
        player.velocity.y=(player.position.y-oldPosition.y)/deltaTime;
    }
    else
    {
        player.velocity={0,0};
    }
    //J键按住且冷却完成时发射玩家子弹
    auto currentTime=SDL_GetTicks();
    if(keyState[SDL_SCANCODE_J])
    {
        if(currentTime-player.lastShootTime>static_cast<Uint32>(player.coolTime))
        {
            shootPlayerBullet();
            player.lastShootTime=currentTime;
        }
    }
}

void MainScene::shootPlayerBullet()
{
    if(weaponLevel<=1)
    {
        //一级武器发射单发直线子弹
        createPlayerBullet(0,{0,-1},0);
    }
    else if(weaponLevel==2)
    {
        //二级武器发射左右并排双发子弹
        createPlayerBullet(-player.width*0.18f,{0,-1},0);
        createPlayerBullet(player.width*0.18f,{0,-1},0);
    }
    else
    {
        //三级武器发射中间直射加两侧散射子弹
        createPlayerBullet(0,{0,-1},0);
        createPlayerBullet(-player.width*0.24f,{-0.18f,-0.98f},-10);
        createPlayerBullet(player.width*0.24f,{0.18f,-0.98f},10);
    }
    //首次射击时懒加载激光音效,后续直接复用
    static Mix_Chunk* laserSound=nullptr;
    if(laserSound==nullptr) laserSound=Mix_LoadWAV("assets/sound/laser_shoot4.wav");
    if(laserSound!=nullptr) Mix_PlayChannel(-1,laserSound,0);
}

void MainScene::createPlayerBullet(float offsetX,SDL_FPoint direction,double angle)
{
    //从玩家子弹对象池获取可用子弹并复制模板属性
    auto bullet=playerBulletPool.acquire();
    *bullet=playerBulletTemplate;
    //根据玩家当前位置和偏移设置子弹初始位置、方向和角度
    bullet->position.x=player.position.x+player.width/2.0f-bullet->width/2.0f+offsetX;
    bullet->position.y=player.position.y;
    bullet->direction=direction;
    bullet->angle=angle;
    bullet->isAlive=true;
    bullet->trailCount=0;
    bulletList.push_back(bullet);
    playerShotsFired++;
}

void MainScene::updatePlayerBullet(float deltaTime)
{
    //设置子弹越界回收边距,允许拖尾稍微飞出屏幕后再消失
    int margin=35;
    for(auto it=bulletList.begin();it!=bulletList.end();)
    {
        auto bullet=*it;
        //把拖尾历史位置向后移动,为本帧位置腾出首位
        for(int i=3;i>0;i--)
        {
            bullet->trail[i]=bullet->trail[i-1];
        }
        if(bullet->trailCount<4)
        {
            bullet->trailCount++;
        }
        bullet->trail[0]=bullet->position;
        //根据子弹方向、速度和帧间隔更新位置
        bullet->position.x+=bullet->direction.x*deltaTime*bullet->speed;
        bullet->position.y+=bullet->direction.y*deltaTime*bullet->speed;
        //子弹离开窗口范围后归还对象池
        if(bullet->position.y+margin<0
           ||bullet->position.x<-margin
           ||bullet->position.x>game.getWindowWidth()+margin)
        {
            playerBulletPool.release(bullet);
            it=bulletList.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void MainScene::renderPlayerBullet()
{
    //遍历玩家子弹列表,先绘制拖尾再绘制子弹本体
    for(auto bullet:bulletList)
    {
        //按历史位置绘制逐渐变淡的拖尾
        for(int i=0;i<bullet->trailCount;i++)
        {
            auto alpha=static_cast<Uint8>(255*(bullet->trailCount-i)
                                         /(bullet->trailCount+1)*0.4f);
            SDL_SetTextureAlphaMod(bullet->texture,alpha);
            SDL_Rect trailRect={static_cast<int>(bullet->trail[i].x),
                              static_cast<int>(bullet->trail[i].y),
                              bullet->width,bullet->height};
            SDL_RenderCopyEx(game.getRenderer(),bullet->texture,NULL,&trailRect,
                             bullet->angle,NULL,SDL_FLIP_NONE);
        }
        //恢复不透明度后绘制当前子弹本体
        SDL_SetTextureAlphaMod(bullet->texture,255);
        SDL_Rect bulletRect={static_cast<int>(bullet->position.x),
                            static_cast<int>(bullet->position.y),
                            bullet->width,bullet->height};
        SDL_RenderCopyEx(game.getRenderer(),bullet->texture,NULL,&bulletRect,
                         bullet->angle,NULL,SDL_FLIP_NONE);
    }
}

bool MainScene::checkCollision(SDL_FPoint posA,int wA,int hA,
                               SDL_FPoint posB,int wB,int hB)
{
    //使用AABB判断两个矩形范围是否相交
    return posA.x<posB.x+wB&&
           posA.x+wA>posB.x&&
           posA.y<posB.y+hB&&
           posA.y+hA>posB.y;
}

void MainScene::checkPlayerBulletHitEnemy()
{
    //遍历玩家子弹并检测是否命中任意活跃敌机
    for(auto bulletIt=bulletList.begin();bulletIt!=bulletList.end();)
    {
        auto bullet=*bulletIt;
        if(bullet==nullptr)
        {
            bulletIt=bulletList.erase(bulletIt);
            continue;
        }
        for(auto enemy:enemySystem.getList())
        {
            if(enemy==nullptr||enemy->isAlive==false) continue;
            if(checkCollision(bullet->position,bullet->width,bullet->height,
                              enemy->position,enemy->width,enemy->height))
            {
                //子弹命中后记录命中次数并扣除敌机生命值
                bullet->isAlive=false;
                playerHitsLanded++;
                enemy->health-=bullet->damage;
                SDL_FPoint hitPos={bullet->position.x+bullet->width/2.0f,
                                   bullet->position.y+bullet->height/2.0f};
                if(enemy->health<=0)
                {
                    //敌机死亡时发布击杀事件,由事件分发统一处理计分和反馈
                    enemy->health=0;
                    enemy->isAlive=false;
                    SDL_FPoint enemyCenter={enemy->position.x+enemy->width/2.0f,
                                            enemy->position.y+enemy->height/2.0f};
                    eventBus.publish(EventType::EnemyKilled,enemyCenter,
                                     enemy->scoreValue);
                }
                else
                {
                    //敌机未死亡时发布命中事件,用于生成火花和音效
                    eventBus.publish(EventType::EnemyHit,hitPos);
                }
                break;
            }
        }
        if(bullet->isAlive==false)
        {
            playerBulletPool.release(bullet);
            bulletIt=bulletList.erase(bulletIt);
        }
        else
        {
            ++bulletIt;
        }
    }
    //把死亡敌机的子弹清理后归还敌机对象池
    for(auto enemyIt=enemySystem.getList().begin();
        enemyIt!=enemySystem.getList().end();)
    {
        auto enemy=*enemyIt;
        if(enemy==nullptr||enemy->isAlive==false)
        {
            enemySystem.clearBullet(enemy);
            enemySystem.getPool().release(enemy);
            enemyIt=enemySystem.getList().erase(enemyIt);
        }
        else
        {
            ++enemyIt;
        }
    }
}

void MainScene::checkEnemyBulletHitPlayer()
{
    //玩家死亡后不再检测敌机子弹伤害
    if(player.isAlive==false) return;
    //缓存当前时间,用于判断护盾和受击无敌时间
    auto currentTime=SDL_GetTicks();
    //遍历敌机子弹列表,检测是否命中玩家飞机
    for(auto bulletIt=enemySystem.getBulletList().begin();
        bulletIt!=enemySystem.getBulletList().end();)
    {
        auto bullet=*bulletIt;
        if(bullet==nullptr)
        {
            bulletIt=enemySystem.getBulletList().erase(bulletIt);
            continue;
        }
        if(checkCollision(bullet->position,bullet->width,bullet->height,
                          player.position,player.width,player.height))
        {
            bullet->isAlive=false;
            //护盾有效时吸收子弹,只发布命中特效事件
            if(powerUpSystem.shieldEndTime!=0&&currentTime<powerUpSystem.shieldEndTime)
            {
                SDL_FPoint pPos={player.position.x+player.width/2.0f,
                                 player.position.y+player.height/2.0f};
                eventBus.publish(EventType::EnemyHit,pPos);
            }
            else if(lastHurtTime==0||currentTime-lastHurtTime>invincibleTime)
            {
                //无护盾且无敌时间结束时扣除玩家生命值
                lastHurtTime=currentTime;
                recentDamageTaken+=bullet->damage;
                damageWindowTimer=DDA_DAMAGE_WINDOW;
                SDL_FPoint pPos={player.position.x+player.width/2.0f,
                                 player.position.y+player.height/2.0f};
                if(player.health-bullet->damage<=0)
                {
                    //玩家生命归零时发布死亡事件并取消暂停状态
                    player.health=0;
                    player.isAlive=false;
                    isPaused=false;
                    eventBus.publish(EventType::PlayerDied,pPos);
                }
                else
                {
                    //玩家仍存活时发布受伤事件,由事件分发生成反馈
                    player.health-=bullet->damage;
                    eventBus.publish(EventType::PlayerHit,pPos,bullet->damage);
                }
            }
        }
        if(bullet->isAlive==false)
        {
            //被消耗的敌机子弹归还对象池
            enemySystem.getBulletPool().release(bullet);
            bulletIt=enemySystem.getBulletList().erase(bulletIt);
        }
        else
        {
            ++bulletIt;
        }
    }
}

void MainScene::checkEnemyHitPlayer()
{
    //玩家死亡后不再检测敌机本体撞击
    if(player.isAlive==false)
    {
        return;
    }
    //缓存当前时间,用于判断护盾和受击无敌时间
    auto currentTime=SDL_GetTicks();
    for(auto enemyIt=enemySystem.getList().begin();
        enemyIt!=enemySystem.getList().end();)
    {
        auto enemy=*enemyIt;
        if(enemy==nullptr||enemy->isAlive==false)
        {
            ++enemyIt;
            continue;
        }
        if(checkCollision(enemy->position,enemy->width,enemy->height,
                          player.position,player.width,player.height)==false)
        {
            ++enemyIt;
            continue;
        }
        SDL_FPoint enemyCenter={enemy->position.x+enemy->width/2.0f,
                                enemy->position.y+enemy->height/2.0f};
        SDL_FPoint playerCenter={player.position.x+player.width/2.0f,
                                 player.position.y+player.height/2.0f};
        //敌机本体撞到玩家后立即销毁敌机并清除其子弹
        enemy->isAlive=false;
        enemySystem.clearBullet(enemy);
        eventBus.publish(EventType::EnemyHit,enemyCenter);
        if(powerUpSystem.shieldEndTime!=0&&currentTime<powerUpSystem.shieldEndTime)
        {
            //护盾有效时只生成玩家位置的命中特效,不扣除生命
            eventBus.publish(EventType::EnemyHit,playerCenter);
        }
        else if(lastHurtTime==0||currentTime-lastHurtTime>invincibleTime)
        {
            //无护盾且无敌时间结束时按撞击伤害扣血
            lastHurtTime=currentTime;
            recentDamageTaken+=ENEMY_COLLISION_DAMAGE;
            damageWindowTimer=DDA_DAMAGE_WINDOW;
            if(player.health-ENEMY_COLLISION_DAMAGE<=0)
            {
                //撞击导致玩家死亡时发布死亡事件
                player.health=0;
                player.isAlive=false;
                isPaused=false;
                eventBus.publish(EventType::PlayerDied,playerCenter);
            }
            else
            {
                //玩家仍存活时发布玩家受伤事件
                player.health-=ENEMY_COLLISION_DAMAGE;
                eventBus.publish(EventType::PlayerHit,playerCenter,ENEMY_COLLISION_DAMAGE);
            }
        }
        //被撞毁的敌机归还对象池并移出活跃列表
        enemySystem.getPool().release(enemy);
        enemyIt=enemySystem.getList().erase(enemyIt);
    }
}

void MainScene::dispatchEvents()
{
    //持续从事件总线取出碰撞事件并统一分发反馈逻辑
    GameEvent event;
    while(eventBus.poll(event))
    {
        switch(event.type)
        {
            case EventType::EnemyKilled:
                //敌机死亡时生成爆炸反馈、尝试掉落道具并增加分数
                createExplosionEffect(event.position,32,32);
                particleSystem.emitExplosion(event.position,10,18);
                powerUpSystem.spawn(event.position,gen,dis);
                killCount++;
                //根据累计击杀数提升玩家武器等级
                weaponLevel=killCount/killsPerWeaponLevel+1;
                if(weaponLevel>maxWeaponLevel)
                {
                    weaponLevel=maxWeaponLevel;
                }
                player.score+=event.value;
                if(explosionSound!=nullptr) Mix_PlayChannel(-1,explosionSound,0);
                break;
            case EventType::EnemyHit:
                //敌机被命中或护盾吸收伤害时生成命中反馈
                createHitEffect(event.position,8,8);
                particleSystem.emitHitSparks(event.position,3,6);
                if(hitSound!=nullptr) Mix_PlayChannel(-1,hitSound,0);
                break;
            case EventType::PlayerHit:
                //玩家受伤时生成命中特效、火花和音效
                createHitEffect(event.position,player.width,player.height);
                particleSystem.emitHitSparks(event.position,5,10);
                if(hitSound!=nullptr) Mix_PlayChannel(-1,hitSound,0);
                break;
            case EventType::PlayerDied:
                //玩家死亡时生成爆炸反馈并启动结算延迟
                createExplosionEffect(event.position,player.width,player.height);
                particleSystem.emitExplosion(event.position,20,30);
                if(explosionSound!=nullptr) Mix_PlayChannel(-1,explosionSound,0);
                if(resultScenePending==false)
                {
                    resultScenePending=true;
                    resultSceneTimer=1.0f;
                }
                break;
            default:
                break;
        }
    }
}

void MainScene::renderGameInfo()
{
    //把主场景状态整理为HUD数据结构,交给GameUI统一绘制
    HudData hud;
    hud.health=player.health;
    hud.maxHealth=player.maxHealth;
    hud.score=player.score;
    hud.difficultyPercent=static_cast<int>(difficulty*100);
    hud.weaponLevel=weaponLevel;
    hud.wave=waveSystem.getWave();
    hud.waveTextTimer=waveSystem.getWaveTextTimer();
    hud.waveCompleteTimer=waveSystem.getCompleteTimer();
    hud.pickupNotice=powerUpSystem.pickupNotice;
    hud.showPickupNotice=powerUpSystem.pickupNoticeTimer>0;
    hud.showGameOver=player.isAlive==false;
    ui.renderHud(game.getWindowWidth(),game.getWindowHeight(),hud);
    //暂停状态下叠加暂停菜单
    if(isPaused&&player.isAlive)
    {
        ui.renderPauseMenu(game.getWindowWidth(),game.getWindowHeight(),
                           pauseMenuItems,selectedPauseIndex);
    }
}

void MainScene::createHitEffect(SDL_FPoint position,int width,int height)
{
    //没有特效纹理时跳过命中特效创建
    if(explosionTexture==nullptr) return;
    //从特效对象池取出对象并设置小尺寸命中特效
    auto effect=effectPool.acquire();
    effect->texture=explosionTexture;
    effect->width=32;  effect->height=32;
    effect->position.x=position.x+width/2.0f-16.0f;
    effect->position.y=position.y+height/2.0f-16.0f;
    effect->totalFrame=4;
    effect->frameTime=35;
    effect->currentFrame=0;
    effect->lastFrameTime=SDL_GetTicks();
    effect->isAlive=true;
    effectList.push_back(effect);
}

void MainScene::createExplosionEffect(SDL_FPoint position,int width,int height)
{
    //没有特效纹理时跳过爆炸特效创建
    if(explosionTexture==nullptr) return;
    //从特效对象池取出对象并设置大尺寸爆炸特效
    auto effect=effectPool.acquire();
    effect->texture=explosionTexture;
    effect->width=72;  effect->height=72;
    effect->position.x=position.x+width/2.0f-36.0f;
    effect->position.y=position.y+height/2.0f-36.0f;
    effect->totalFrame=9;
    effect->frameTime=45;
    effect->currentFrame=0;
    effect->lastFrameTime=SDL_GetTicks();
    effect->isAlive=true;
    effectList.push_back(effect);
}

void MainScene::updateEffect()
{
    //遍历活跃特效,按帧时间推进序列帧动画
    auto currentTime=SDL_GetTicks();
    for(auto it=effectList.begin();it!=effectList.end();)
    {
        auto effect=*it;
        if(effect==nullptr) { it=effectList.erase(it); continue; }
        if(currentTime-effect->lastFrameTime>effect->frameTime)
        {
            effect->currentFrame++;
            effect->lastFrameTime=currentTime;
            if(effect->currentFrame>=effect->totalFrame)
            {
                effect->isAlive=false;
            }
        }
        if(effect->isAlive==false)
        {
            //播放结束的特效归还对象池
            effectPool.release(effect);
            it=effectList.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void MainScene::renderEffect()
{
    //遍历活跃特效,按当前帧裁剪精灵图并绘制到主渲染器
    for(auto effect:effectList)
    {
        if(effect==nullptr||effect->texture==nullptr) continue;
        SDL_Rect src={effect->currentFrame*effect->frameWidth,0,
                      effect->frameWidth,effect->frameHeight};
        SDL_Rect dst={static_cast<int>(effect->position.x),
                      static_cast<int>(effect->position.y),
                      effect->width,effect->height};
        SDL_RenderCopy(game.getRenderer(),effect->texture,&src,&dst);
    }
}

void MainScene::enterResultScene()
{
    //把本局成绩整理为结算数据并请求切换到结算场景
    ResultData data;
    data.score=player.score;
    data.wave=waveSystem.getWave();
    data.killCount=killCount;
    data.difficulty=static_cast<int>(difficulty*100);
    game.requestSceneChange(new ResultScene(data));
}
