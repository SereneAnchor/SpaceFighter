/**
 *  @file    Config.h
 *  @brief   游戏全局可调参数集中配置
**/

#ifndef CONFIG
#define CONFIG

/**
 *  @brief  玩家基础属性
 *
 *  @param  PLAYER_HP             玩家最大生命值。
 *  @param  PLAYER_SPEED          玩家移动速度。
 *  @param  PLAYER_COOLTIME       玩家射击冷却时间,单位毫秒。
 *  @param  PLAYER_BULLET_SPEED   玩家子弹速度。
 *  @param  PLAYER_BULLET_DMG     玩家子弹伤害。
 *  @param  PLAYER_INVINCIBLE     玩家受击后的无敌时间,单位毫秒。
**/
inline constexpr int PLAYER_HP=100;
inline constexpr int PLAYER_SPEED=280;
inline constexpr int PLAYER_COOLTIME=280;
inline constexpr int PLAYER_BULLET_SPEED=900;
inline constexpr int PLAYER_BULLET_DMG=10;
inline constexpr int PLAYER_INVINCIBLE=1000;

/**
 *  @brief  敌机行为参数
 *
 *  @param  NEAR_ZONE_DIST        敌机远近区域分界距离。
 *  @param  FAR_SPEED_MUL         远区速度倍率。
 *  @param  NEAR_SPEED_MUL        近区速度倍率。
 *  @param  NEAR_COOLDOWN_MUL     近区射击冷却倍率。
 *  @param  DIFFICULTY_RAMP       每100分带来的难度增长量。
 *  @param  DIFFICULTY_MAX        动态难度上限。
 *  @param  SPEED_SCALE           难度换算到速度加成的系数。
 *  @param  COOLTIME_SCALE        难度换算到冷却缩短的系数。
**/
inline constexpr float NEAR_ZONE_DIST=250.0f;
inline constexpr float FAR_SPEED_MUL=0.8f;
inline constexpr float NEAR_SPEED_MUL=1.05f;
inline constexpr float NEAR_COOLDOWN_MUL=2.0f;
inline constexpr float DIFFICULTY_RAMP=0.3f;
inline constexpr float DIFFICULTY_MAX=2.0f;
inline constexpr float SPEED_SCALE=0.4f;
inline constexpr float COOLTIME_SCALE=0.5f;

/**
 *  @brief  敌机属性
 *
 *  @param  INSECT2_HP             enemy-1生命值。
 *  @param  INSECT2_SPEED          enemy-1移动速度。
 *  @param  INSECT2_COOLTIME       enemy-1射击冷却时间,单位毫秒。
 *  @param  INSECT2_SCORE          enemy-1击毁分数。
 *  @param  INSECT1_HP             enemy-2生命值。
 *  @param  INSECT1_SPEED          enemy-2移动速度。
 *  @param  INSECT1_COOLTIME       enemy-2射击冷却时间,单位毫秒。
 *  @param  INSECT1_SCORE          enemy-2击毁分数。
 *  @param  INSECT1_CHANCE         默认enemy-2生成概率。
 *  @param  ENEMY_COLLISION_DAMAGE 敌机本体撞击玩家造成的伤害。
**/
inline constexpr int INSECT2_HP=15;
inline constexpr int INSECT2_SPEED=220;
inline constexpr int INSECT2_COOLTIME=1050;
inline constexpr int INSECT2_SCORE=40;
inline constexpr int INSECT1_HP=25;
inline constexpr int INSECT1_SPEED=160;
inline constexpr int INSECT1_COOLTIME=1200;
inline constexpr int INSECT1_SCORE=80;
inline constexpr float INSECT1_CHANCE=0.40f;
inline constexpr int ENEMY_COLLISION_DAMAGE=20;

/**
 *  @brief  武器参数
 *
 *  @param  PLAYER_BULLET_DAMAGE  玩家子弹基础伤害。
**/
inline constexpr int PLAYER_BULLET_DAMAGE=10;

/**
 *  @brief  道具参数
 *
 *  @param  POWERUP_DROP_RATE     敌机死亡后的道具掉落概率。
 *  @param  HEAL_AMOUNT           生命道具恢复量。
 *  @param  SHIELD_DURATION       护盾持续时间,单位毫秒。
 *  @param  TIME_BUFF_DURATION    射速增益持续时间,单位毫秒。
**/
inline constexpr float POWERUP_DROP_RATE=0.22f;
inline constexpr int HEAL_AMOUNT=10;
inline constexpr int SHIELD_DURATION=3000;
inline constexpr int TIME_BUFF_DURATION=5000;

/**
 *  @brief  波次参数
 *
 *  @param  WAVE_REST_TIME        波间休息时间。
 *  @param  WAVE_TEXT_TIME        波次提示显示时间。
 *  @param  SPAWN_INTERVAL        敌机生成间隔。
 *  @param  BASE_ENEMY_COUNT      基础敌机数量。
 *  @param  ENEMY_PER_WAVE        每波增加的敌机数量。
**/
inline constexpr float WAVE_REST_TIME=2.5f;
inline constexpr float WAVE_TEXT_TIME=2.0f;
inline constexpr float SPAWN_INTERVAL=0.55f;
inline constexpr int BASE_ENEMY_COUNT=2;
inline constexpr int ENEMY_PER_WAVE=1;

/**
 *  @brief  贝塞尔入场参数
 *
 *  @param  BEZIER_MIN_TIME       最短入场时间。
 *  @param  BEZIER_SWING_MIN      最小摆动幅度。
**/
inline constexpr float BEZIER_MIN_TIME=2.0f;
inline constexpr float BEZIER_SWING_MIN=120.0f;

/**
 *  @brief  FSM状态机参数
 *
 *  @param  FSM_PATROL_CHASE_DIST 巡逻转追击距离。
 *  @param  FSM_CHASE_ATTACK_DIST 追击转攻击距离。
 *  @param  FSM_CHASE_PATROL_DIST 追击转巡逻距离。
 *  @param  FSM_ATTACK_CHASE_DIST 攻击转追击距离。
 *  @param  FSM_FLEE_PATROL_DIST  逃跑转巡逻距离。
 *  @param  FSM_FLEE_HP           触发逃跑的生命比例。
 *  @param  FSM_ATTACK_COOL_MUL   攻击状态冷却倍率。
 *  @param  FSM_PATROL_COOL_MUL   巡逻状态冷却倍率。
 *  @param  FSM_FLEE_SPEED_MUL    逃跑状态速度倍率。
 *  @param  ENEMY_BULLET_LIMIT    敌机子弹同屏上限。
**/
inline constexpr float FSM_PATROL_CHASE_DIST=250.0f;
inline constexpr float FSM_CHASE_ATTACK_DIST=120.0f;
inline constexpr float FSM_CHASE_PATROL_DIST=340.0f;
inline constexpr float FSM_ATTACK_CHASE_DIST=180.0f;
inline constexpr float FSM_FLEE_PATROL_DIST=360.0f;
inline constexpr float FSM_FLEE_HP=0.3f;
inline constexpr float FSM_ATTACK_COOL_MUL=0.55f;
inline constexpr float FSM_PATROL_COOL_MUL=1.8f;
inline constexpr float FSM_FLEE_SPEED_MUL=1.25f;
inline constexpr int ENEMY_BULLET_LIMIT=16;

/**
 *  @brief  DDA动态难度参数
 *
 *  @param  DDA_SMOOTH_SPEED        难度平滑速度。
 *  @param  DDA_ACCURACY_WEIGHT     命中率影响权重。
 *  @param  DDA_DAMAGE_WEIGHT       受伤影响权重。
 *  @param  DDA_DAMAGE_CAP          受伤修正上限。
 *  @param  DDA_DAMAGE_WINDOW       受伤统计窗口。
 *  @param  DDA_ACCURACY_MIN_SHOTS  命中率生效的最低射击数。
**/
inline constexpr float DDA_SMOOTH_SPEED=0.6f;
inline constexpr float DDA_ACCURACY_WEIGHT=0.35f;
inline constexpr float DDA_DAMAGE_WEIGHT=0.02f;
inline constexpr float DDA_DAMAGE_CAP=0.35f;
inline constexpr float DDA_DAMAGE_WINDOW=4.0f;
inline constexpr int DDA_ACCURACY_MIN_SHOTS=20;

/**
 *  @brief  菜单难度偏移
 *
 *  @param  gDifficultyOffset  菜单选择的难度偏移值,EASY=-0.3,NORMAL=0,HARD=+0.4。
**/
inline float gDifficultyOffset=0.0f;

#endif
