#ifndef GAMEUI
#define GAMEUI

#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_ttf.h>

/**
 *  @brief  菜单项数据
 *
 *  @param  label  菜单项显示文本
**/
struct MenuItem
{
    std::string label;
};

/**
 *  @brief  主玩法HUD显示数据
 *
 *  @param  health             玩家当前生命值
 *  @param  maxHealth          玩家最大生命值
 *  @param  score              玩家当前分数
 *  @param  difficultyPercent  当前动态难度百分比
 *  @param  weaponLevel        玩家当前武器等级
 *  @param  wave               当前波次编号
 *  @param  waveTextTimer      WAVE提示剩余显示时间
 *  @param  waveCompleteTimer  WAVE COMPLETE提示剩余显示时间
 *  @param  pickupNotice       道具拾取提示文本
 *  @param  showPickupNotice   是否显示道具拾取提示
 *  @param  showGameOver       是否显示游戏结束提示
**/
struct HudData
{
    int health=0;
    int maxHealth=0;
    int score=0;
    int difficultyPercent=100;
    int weaponLevel=1;
    int wave=0;
    float waveTextTimer=0.0f;
    float waveCompleteTimer=0.0f;
    std::string pickupNotice;
    bool showPickupNotice=false;
    bool showGameOver=false;
};

/**
 *  @brief  结算页显示数据
 *
 *  @param  score          本局最终分数
 *  @param  wave           本局到达波次
 *  @param  killCount      本局击杀敌机数量
 *  @param  difficulty     本局结束时难度百分比
 *  @param  rankScore      本局评分分数
 *  @param  bestScore      历史最高分
 *  @param  bestWave       历史最高分对应波次
 *  @param  bestRankScore  历史最高分对应评分分数
 *  @param  isNewRecord    本局是否刷新最高分
**/
struct ResultDisplayData
{
    int score=0;
    int wave=0;
    int killCount=0;
    int difficulty=100;
    int rankScore=0;
    int bestScore=0;
    int bestWave=0;
    int bestRankScore=0;
    bool isNewRecord=false;
};

/**
 *  @brief  历史记录页单行显示数据
 *
 *  @param  no          历史记录序号
 *  @param  score       历史记录分数文本
 *  @param  wave        历史记录波次文本
 *  @param  kills       历史记录击杀数文本
 *  @param  difficulty  历史记录难度文本
 *  @param  rank        历史记录评级文本
**/
struct HistoryDisplayRecord
{
    std::string no;
    std::string score;
    std::string wave;
    std::string kills;
    std::string difficulty;
    std::string rank;
};

/**
 *  @brief  游戏UI绘制工具类
**/
class GameUI
{
public:
    GameUI()=default;

    //析构时释放GameUI持有的字体资源
    ~GameUI();

    //绑定Game创建的SDL渲染器并加载指定字体文件
    bool initialize(SDL_Renderer* renderer,const char* fontPath,int fontSize);

    //释放GameUI持有的字体资源并解除渲染器引用
    void clear();

    //把指定文本绘制到GameUI绑定渲染器的指定左上角位置
    void renderText(const char* text,int x,int y,SDL_Color color);

    //把指定文本按中心点横向居中后绘制到GameUI绑定渲染器
    void renderTextCentered(const char* text,int cx,int y,SDL_Color color);

    //测量指定文本使用当前字体渲染后的宽高
    void measureText(const char* text,int& width,int& height) const;

    //在GameUI绑定渲染器上绘制半透明面板和边框
    void renderPanel(int x,int y,int w,int h,SDL_Color fill,SDL_Color border);

    //根据当前值和最大值绘制HUD生命条
    void renderHealthBar(int x,int y,int w,int h,int value,int maxValue);

    //绘制菜单项列表并用高亮样式标记当前选中项
    void renderMenuList(const std::vector<MenuItem>& items,int selectedIndex,
                        int centerX,int startY,int lineHeight);

    //绘制MenuScene传入的主菜单项、难度提示和操作提示
    void renderMainMenu(int windowWidth,int windowHeight,
                        const std::vector<MenuItem>& items,int selectedIndex);

    //绘制MainScene暂停状态下的遮罩、面板和暂停菜单项
    void renderPauseMenu(int windowWidth,int windowHeight,
                         const std::vector<MenuItem>& items,int selectedIndex);

    //绘制MainScene提供的玩家状态、分数、波次和提示信息
    void renderHud(int windowWidth,int windowHeight,const HudData& data);

    //绘制ResultScene提供的本局结算和历史最佳记录
    void renderResult(int windowWidth,int windowHeight,const ResultDisplayData& data);

    //绘制ResultScene读取并整理后的最近历史记录
    void renderHistory(int windowWidth,int windowHeight,
                       const std::vector<HistoryDisplayRecord>& records);

    //根据方向在菜单项数量范围内循环移动选中索引
    static int moveSelection(int selectedIndex,int direction,int itemCount);

    //根据评分分数返回结算页显示的评级文本
    static const char* getRankText(int rankScore);

    //根据评分分数返回结算页和历史页使用的评级颜色
    static SDL_Color getRankColor(int rankScore);

private:
    //Game创建并传入的SDL渲染器,GameUI只使用不拥有
    SDL_Renderer* renderer=nullptr;

    //GameUI加载并持有的字体资源
    TTF_Font* font=nullptr;
};

#endif
