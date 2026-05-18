/**
 *  @file    ResultScene.h
 *  @brief   游戏结算场景
 *
 *  负责展示本局成绩、评级和历史最高记录,并处理重新开始与返回菜单
**/

#ifndef RESULTSCENE
#define RESULTSCENE

#include <SDL.h>
#include <string>
#include <vector>

#include "GameUI.h"
#include "Scene.h"

/**
 *  @brief  本局结算数据
 *
 *  @param  score       本局分数
 *  @param  wave        到达波次
 *  @param  killCount   击杀敌机数量
 *  @param  difficulty   结束时难度百分比
**/
struct ResultData
{
    int score=0;
    int wave=0;
    int killCount=0;
    int difficulty=100;
};

/**
 *  @brief  历史最高记录
 *
 *  @param  score       历史最高分
 *  @param  wave        历史最高分对应波次
 *  @param  rankScore   历史最高分对应评级分
**/
struct BestRecord
{
    int score=0;
    int wave=0;
    int rankScore=0;
};

class Game;

/**
 *  @brief  结算场景
 *
 *  负责显示结果、保存历史记录和处理重新开始或返回菜单输入
**/
class ResultScene:public Scene
{
    public:
        //使用本局结果创建结算场景,由MainScene结束后交给Game切换使用
        ResultScene(const ResultData& data);

        //释放结算场景持有的UI和运行时资源
        ~ResultScene();

        //初始化结算页UI并计算本局评级分
        virtual void initialScene() override;

        //结算页不推进玩法逻辑,仅保留接口以适配Scene基类
        virtual void updateScene(float deltaTime) override;

        //绘制结算页成绩、评级、历史记录和提示文本
        virtual void renderScene() override;

        //处理重新开始、返回菜单和历史记录切换输入
        virtual void handleEvent(SDL_Event* event) override;

        //释放结算页绑定的UI资源
        virtual void clearScene() override;

        //从历史文件中读取最高记录
        void loadBestRecord();

        //将本局结果追加写入历史文件
        void saveResultHistory();

        //读取最近历史记录用于结算页内展示
        void loadHistoryRecords();

        //获取当前本地时间文本
        std::string getCurrentTimeText() const;

        //将文本按指定宽度居中
        std::string centerText(const std::string& text,int width) const;

        //根据评级分返回评级文字
        const char* getRankText(int rankScore) const;

        //根据评级分返回评级颜色
        SDL_Color getRankColor(int rankScore) const;

    private:
        //游戏主控制对象
        Game& game;

        //本局结果数据
        ResultData resultData;

        //历史最高记录
        BestRecord bestRecord;

        //结算界面UI绘制工具
        GameUI ui;

        //最近历史记录
        std::vector<HistoryDisplayRecord> historyRecords;

        //本局评级分
        int rankScore=0;

        //已读取的历史记录数量
        int historyCount=0;

        //本局是否刷新最高分
        bool isNewRecord=false;

        //是否存在历史最高记录
        bool hasBestRecord=false;

        //是否显示历史记录
        bool showHistory=false;
};

#endif
