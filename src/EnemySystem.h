/**
 *  @file    EnemySystem.h
 *  @brief   敌机系统: 管理敌机生成、AI移动、射击、子弹更新和资源清理
**/

#ifndef ENEMYSYSTEM
#define ENEMYSYSTEM

#include <list>
#include <random>
#include <SDL.h>
#include "Object.h"

struct Player;

/**
 *  @brief  敌机生成类型
 *
 *  @param  Light  轻型敌机
 *  @param  Heavy  重型敌机
 *  @param  Elite  精英敌机
**/
enum class EnemySpawnType
{
    Light,
    Heavy,
    Elite
};

/**
 *  @brief  敌机系统
**/
class EnemySystem
{
    public:
        //用Game提供的渲染器加载敌机和敌机子弹模板纹理
        void initialize(SDL_Renderer* renderer);

        //按随机概率选择敌机类型,再尝试生成到活跃敌机列表
        void generate(float difficulty,std::mt19937& gen,
                      std::uniform_real_distribution<float>& dis,int windowW,
                      float insect1Chance=0.40f,bool forceSpawn=false);

        //按指定类型从对象池获取敌机,初始化后加入活跃敌机列表
        void generateByType(EnemySpawnType type,float difficulty,std::mt19937& gen,
                            std::uniform_real_distribution<float>& dis,int windowW,
                            bool forceSpawn=false);

        //根据玩家位置更新活跃敌机的AI状态、移动、越界回收和射击
        void update(float deltaTime,const Player& player,int windowW,int windowH);

        //把活跃敌机列表中的敌机绘制到传入的SDL渲染器
        void render(SDL_Renderer* renderer);

        //让指定敌机根据玩家预测位置创建敌机子弹并加入子弹列表
        void shootBullet(Enemy* enemy,const Player& player);

        //更新活跃敌机子弹的位置、拖尾和越界回收
        void updateBullets(float deltaTime,int windowW,int windowH);

        //把活跃敌机子弹列表中的子弹和拖尾绘制到传入渲染器
        void renderBullets(SDL_Renderer* renderer);

        //从活跃子弹列表中回收指定敌机发射出的所有子弹
        void clearBullet(Enemy* enemy);

        //回收活跃敌机和子弹,并释放敌机系统持有的模板纹理
        void clear();

        //返回活跃敌机列表,供MainScene碰撞检测和清理死亡敌机
        std::list<Enemy*>& getList()
        {
            return enemyList;
        }

        //返回活跃敌机子弹列表,供MainScene检测玩家受击
        std::list<EnemyBullet*>& getBulletList()
        {
            return enemyBulletList;
        }

        //返回敌机对象池,供MainScene在击毁敌机后归还对象
        ObjectPool<Enemy>& getPool()
        {
            return enemyPool;
        }

        //返回敌机子弹对象池,供MainScene在子弹命中后归还对象
        ObjectPool<EnemyBullet>& getBulletPool()
        {
            return bulletPool;
        }

    private:
        //enemy-1轻型敌机模板,生成轻型敌机时复制该数据
        Enemy insect2Template;

        //enemy-2重型敌机模板,生成重型敌机时复制该数据
        Enemy insect1Template;

        //enemy-1轻型敌机子弹模板,轻型敌机射击时复制该数据
        EnemyBullet insect2BulletTemplate;

        //enemy-2重型敌机子弹模板,重型敌机射击时复制该数据
        EnemyBullet insect1BulletTemplate;

        //保存当前场景中正在移动、渲染和碰撞检测的敌机
        std::list<Enemy*> enemyList;

        //复用敌机对象,减少频繁创建和销毁敌机的开销
        ObjectPool<Enemy> enemyPool;

        //保存当前场景中正在移动、渲染和碰撞检测的敌机子弹
        std::list<EnemyBullet*> enemyBulletList;

        //复用敌机子弹对象,减少敌机连续射击时的分配开销
        ObjectPool<EnemyBullet> bulletPool;
};

#endif
