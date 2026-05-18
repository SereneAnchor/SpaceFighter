/**
 *  @file    MainScene.h
 *  @brief   游戏主场景
 *
 *  负责战斗流程、玩家控制、碰撞、HUD、特效和结算切换
**/

#ifndef MAINSCENE
#define MAINSCENE

#include <iostream>
#include <list>
#include <random>
#include <vector>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include "Scene.h"
#include "Object.h"
#include "EnemySystem.h"
#include "ParticleSystem.h"
#include "PowerUpSystem.h"
#include "WaveSystem.h"
#include "EventBus.h"
#include "GameUI.h"

class Game;

/**
 *  @brief  主游戏场景
 *
 *  调度敌机、粒子、道具、波次和事件系统,并处理主要游戏逻辑
**/
class MainScene:public Scene
{
    public:
        //构造主场景并绑定Game单例
        MainScene();

        //析构主场景
        ~MainScene();

        //初始化玩家、敌机、字体、音效和背景资源
        virtual void initialScene() override;

        //更新战斗逻辑和各子系统
        virtual void updateScene(float deltaTime) override;

        //渲染背景、角色、子弹、特效和HUD
        virtual void renderScene() override;

        //处理暂停、重开和返回菜单输入
        virtual void handleEvent(SDL_Event* event) override;

        //释放主场景资源
        virtual void clearScene() override;

        //更新滚动背景位置
        void updateBackground(float deltaTime);

        //渲染循环滚动背景
        void renderBackground();

        //处理玩家移动和射击输入
        void keyboardControl(float deltaTime);

        //根据武器等级发射玩家子弹
        void shootPlayerBullet();

        //从对象池创建一颗玩家子弹
        void createPlayerBullet(float offsetX,SDL_FPoint direction,double angle);

        //更新玩家子弹位置和越界回收
        void updatePlayerBullet(float deltaTime);

        //渲染玩家子弹和拖尾
        void renderPlayerBullet();

        //检测两个矩形对象是否碰撞
        bool checkCollision(SDL_FPoint posA,int wA,int hA,
                            SDL_FPoint posB,int wB,int hB);

        //检测玩家子弹命中敌机
        void checkPlayerBulletHitEnemy();

        //检测敌机子弹命中玩家
        void checkEnemyBulletHitPlayer();

        //检测敌机本体撞击玩家
        void checkEnemyHitPlayer();

        //分发事件总线中的游戏事件
        void dispatchEvents();

        //渲染游戏信息栏
        void renderGameInfo();

        //创建命中特效
        void createHitEffect(SDL_FPoint position,int width,int height);

        //创建爆炸特效
        void createExplosionEffect(SDL_FPoint position,int width,int height);

        //更新特效动画帧
        void updateEffect();

        //渲染特效动画
        void renderEffect();

        //进入结算场景
        void enterResultScene();

    private:
        //游戏主控制对象
        Game& game;

        //玩家飞机数据
        Player player;

        //敌机系统
        EnemySystem enemySystem;

        //粒子系统
        ParticleSystem particleSystem;

        //道具系统
        PowerUpSystem powerUpSystem;

        //波次系统
        WaveSystem waveSystem;

        //事件总线
        EventBus eventBus;

        //游戏中UI绘制工具
        GameUI ui;

        //暂停菜单项
        std::vector<MenuItem> pauseMenuItems;

        //游戏是否暂停
        bool isPaused=false;

        //当前选中的暂停菜单项
        int selectedPauseIndex=0;

        //玩家受击无敌时间
        Uint32 invincibleTime=1000;

        //玩家上次受击时间
        Uint32 lastHurtTime=0;

        //当前动态难度
        float difficulty=1.0f;

        //目标动态难度
        float targetDifficulty=1.0f;

        //玩家累计发射次数
        int playerShotsFired=0;

        //玩家累计命中次数
        int playerHitsLanded=0;

        //伤害窗口内累计受伤
        int recentDamageTaken=0;

        //伤害统计窗口倒计时
        float damageWindowTimer=0;

        //玩家击杀数量
        int killCount=0;

        //当前武器等级
        int weaponLevel=1;

        //最大武器等级
        int maxWeaponLevel=3;

        //每次升级所需击杀数
        int killsPerWeaponLevel=8;

        //是否等待进入结算场景
        bool resultScenePending=false;

        //进入结算场景倒计时
        float resultSceneTimer=0;

        //背景纹理
        SDL_Texture* backgroundTexture=nullptr;

        //背景滚动位置
        float backgroundY=0;

        //背景滚动速度
        int backgroundSpeed=80;

        //命中音效
        Mix_Chunk* hitSound=nullptr;

        //爆炸音效
        Mix_Chunk* explosionSound=nullptr;

        //拾取音效
        Mix_Chunk* pickupSound=nullptr;

        //背景音乐
        Mix_Music* bgm=nullptr;

        //护盾视觉纹理
        SDL_Texture* shieldTexture=nullptr;

        //引擎火焰纹理
        SDL_Texture* fireTexture=nullptr;

        //护盾纹理尺寸
        int shieldTexW=0,shieldTexH=0;

        //火焰纹理尺寸
        int fireTexW=0,fireTexH=0;

        //爆炸和命中特效纹理
        SDL_Texture* explosionTexture=nullptr;

        //活跃特效列表
        std::list<Effect*> effectList;

        //特效对象池
        ObjectPool<Effect> effectPool;

        //玩家子弹模板
        PlayerBullet playerBulletTemplate;

        //活跃玩家子弹列表
        std::list<PlayerBullet*> bulletList;

        //玩家子弹对象池
        ObjectPool<PlayerBullet> playerBulletPool;

        //随机数引擎
        std::mt19937 gen;

        //0到1随机分布
        std::uniform_real_distribution<float> dis;
};

#endif
