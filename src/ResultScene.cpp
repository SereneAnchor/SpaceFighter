/**
 *  @file    ResultScene.cpp
 *  @brief   游戏结算场景实现
**/

#include "ResultScene.h"
#include "Game.h"
#include "MainScene.h"
#include "MenuScene.h"

#include <SDL.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>

namespace
{
    //去掉历史记录字段两侧多余空格
    std::string trimText(const std::string& text)
    {
        auto begin=text.find_first_not_of(' ');
        if(begin==std::string::npos)
        {
            return "";
        }
        auto end=text.find_last_not_of(' ');
        return text.substr(begin,end-begin+1);
    }
}

ResultScene::ResultScene(const ResultData& data):game(Game::getInstance()),resultData(data)
{
    std::cout<<"ResultScene::构造方法."<<std::endl;
}

ResultScene::~ResultScene()
{
    std::cout<<"ResultScene::析构方法."<<std::endl;
}

void ResultScene::initialScene()
{
    //初始化结算页字体并基于本局表现计算评级分
    ui.initialize(game.getRenderer(),"assets/font/VonwaonBitmap-16px.ttf",24);
    rankScore=resultData.score/150+resultData.wave*15+
              resultData.killCount*2+resultData.difficulty/12;
    //先读取历史最高记录,再判断本局是否刷新记录
    loadBestRecord();
    if(hasBestRecord==false||resultData.score>bestRecord.score)
    {
        bestRecord.score=resultData.score;
        bestRecord.wave=resultData.wave;
        bestRecord.rankScore=rankScore;
        isNewRecord=true;
        hasBestRecord=true;
    }
    //把本局成绩写入历史文件并加载最近记录用于结算页显示
    saveResultHistory();
    loadHistoryRecords();
}

void ResultScene::updateScene(float deltaTime)
{
    //结算页不需要按帧推进玩法逻辑
    (void)deltaTime;
}

void ResultScene::renderScene()
{
    //先清空并绘制结算页背景
    SDL_SetRenderDrawColor(game.getRenderer(),8,10,18,255);
    SDL_Rect backgroundRect={0,0,game.getWindowWidth(),game.getWindowHeight()};
    SDL_RenderFillRect(game.getRenderer(),&backgroundRect);
    SDL_SetRenderDrawColor(game.getRenderer(),0,0,0,255);
    //根据历史页开关在结算摘要和历史记录之间切换
    if(showHistory)
    {
        ui.renderHistory(game.getWindowWidth(),game.getWindowHeight(),historyRecords);
    }
    else
    {
        ResultDisplayData display;
        display.score=resultData.score;
        display.wave=resultData.wave;
        display.killCount=resultData.killCount;
        display.difficulty=resultData.difficulty;
        display.rankScore=rankScore;
        display.bestScore=bestRecord.score;
        display.bestWave=bestRecord.wave;
        display.bestRankScore=bestRecord.rankScore;
        display.isNewRecord=isNewRecord;
        ui.renderResult(game.getWindowWidth(),game.getWindowHeight(),display);
    }
}

void ResultScene::handleEvent(SDL_Event *event)
{
    //空事件直接忽略,避免访问无效输入数据
    if(event==nullptr)
    {
        return;
    }
    //只处理第一次按下的按键,避免按键连发切换场景
    if(event->type==SDL_KEYDOWN&&event->key.repeat==0)
    {
        switch(event->key.keysym.scancode)
        {
            case SDL_SCANCODE_H:
                //H键在结算摘要和历史记录之间切换
                showHistory=!showHistory;
                break;
            case SDL_SCANCODE_R:
                //R键请求切换回主游戏场景
                game.requestSceneChange(new MainScene());
                break;
            case SDL_SCANCODE_ESCAPE:
                //ESC键请求返回主菜单
                game.requestSceneChange(new MenuScene());
                break;
            default:
                break;
        }
    }
}

void ResultScene::clearScene()
{
    //释放结算页UI资源
    ui.clear();
}

void ResultScene::loadBestRecord()
{
    //先清空历史读取状态,再从历史文件恢复最高记录
    hasBestRecord=false;
    historyCount=0;
    std::ifstream historyFile("ResultHistory.txt");
    if(historyFile.is_open())
    {
        //优先读取新版历史文件,按行解析最新的结算记录
        std::string line;
        int score=0;
        int wave=0;
        int currentRankScore=0;
        bool hasScore=false;
        while(std::getline(historyFile,line))
        {
            if(line.empty()||line.find("No |")!=std::string::npos
               ||line.find("Game No")!=std::string::npos)
            {
                continue;
            }
            if(line.find('|')!=std::string::npos)
            {
                score=0;
                wave=0;
                currentRankScore=0;
                std::string noText,timeText,scoreText,waveText,killsText;
                std::string difficultyText,rankScoreText,rankText,bestText;
                std::stringstream lineStream(line);
                std::getline(lineStream,noText,'|');
                std::getline(lineStream,timeText,'|');
                std::getline(lineStream,scoreText,'|');
                std::getline(lineStream,waveText,'|');
                std::getline(lineStream,killsText,'|');
                std::getline(lineStream,difficultyText,'|');
                std::getline(lineStream,rankScoreText,'|');
                std::getline(lineStream,rankText,'|');
                std::getline(lineStream,bestText,'|');
                std::stringstream(scoreText)>>score;
                std::stringstream(waveText)>>wave;
                std::stringstream(rankScoreText)>>currentRankScore;
                historyCount++;
                if(hasBestRecord==false||score>bestRecord.score)
                {
                    //用历史文件中的高分记录更新当前最高成绩
                    bestRecord.score=score;
                    bestRecord.wave=wave;
                    bestRecord.rankScore=currentRankScore;
                    hasBestRecord=true;
                }
                continue;
            }
            std::stringstream recordStream(line);
            std::string label;
            recordStream>>label;
            if(label=="RecordStart")
            {
                score=0;
                wave=0;
                currentRankScore=0;
                hasScore=false;
            }
            else if(label=="Score:")
            {
                recordStream>>score;
                hasScore=true;
            }
            else if(label=="Wave:")
            {
                recordStream>>wave;
            }
            else if(label=="RankScore:")
            {
                recordStream>>currentRankScore;
            }
            else if(label=="RecordEnd")
            {
                historyCount++;
                if(hasScore&&(hasBestRecord==false||score>bestRecord.score))
                {
                    //记录结束时把当前记录和历史最高分比较
                    bestRecord.score=score;
                    bestRecord.wave=wave;
                    bestRecord.rankScore=currentRankScore;
                    hasBestRecord=true;
                }
                hasScore=false;
            }
        }
        if(hasScore&&(hasBestRecord==false||score>bestRecord.score))
        {
            historyCount++;
            //文件尾部若还有未闭合记录,也参与最高分比较
            bestRecord.score=score;
            bestRecord.wave=wave;
            bestRecord.rankScore=currentRankScore;
            hasBestRecord=true;
        }
        if(hasBestRecord)
        {
            return;
        }
    }
    std::ifstream file("best_record.txt");
    if(file.is_open()==false)
    {
        return;
    }
    //兼容旧版最高分文件格式,按不同标记继续解析
    std::string label;
    file>>label;
    if(label.empty())
    {
        return;
    }
    if(label[0]>='0'&&label[0]<='9')
    {
        //旧格式直接写入分数、波次和评级分
        bestRecord.score=std::stoi(label);
        file>>bestRecord.wave>>bestRecord.rankScore;
        hasBestRecord=true;
        return;
    }
    file.clear();
    file.seekg(0);
    while(file>>label)
    {
        if(label=="BestScore:")
        {
            file>>bestRecord.score;
        }
        else if(label=="BestWave:")
        {
            file>>bestRecord.wave;
        }
        else if(label=="BestRankScore:")
        {
            file>>bestRecord.rankScore;
        }
        else
        {
            std::string text;
            file>>text;
        }
    }
    if(bestRecord.score>0||bestRecord.wave>0||bestRecord.rankScore>0)
    {
        hasBestRecord=true;
    }
}

void ResultScene::saveResultHistory()
{
    //先检查历史文件是否已有表头,避免重复写入
    std::ifstream checkFile("ResultHistory.txt");
    bool needHeader=true;
    if(checkFile.is_open())
    {
        std::string firstLine;
        std::getline(checkFile,firstLine);
        needHeader=firstLine.find("Game No")==std::string::npos;
    }
    std::ofstream file("ResultHistory.txt",std::ios::app);
    if(file.is_open()==false)
    {
        return;
    }
    //按固定列宽写入新一条结算记录
    int noWidth=9;
    int timeWidth=21;
    int scoreWidth=13;
    int waveWidth=14;
    int killWidth=16;
    int difficultyWidth=18;
    int rankScoreWidth=12;
    int rankWidth=12;
    int recordWidth=15;
    if(needHeader)
    {
        file<<centerText("Game No",noWidth)<<"| "
            <<centerText("Played Time",timeWidth)<<"| "
            <<centerText("Final Score",scoreWidth)<<"| "
            <<centerText("Reached Wave",waveWidth)<<"| "
            <<centerText("Enemies Killed",killWidth)<<"| "
            <<centerText("Final Difficulty",difficultyWidth)<<"| "
            <<centerText("Rank Score",rankScoreWidth)<<"| "
            <<centerText("Final Rank",rankWidth)<<"| "
            <<centerText("Record Status",recordWidth)<<"\n";
    }
    file<<centerText(std::to_string(historyCount+1),noWidth)<<"| "
        <<centerText(getCurrentTimeText(),timeWidth)<<"| "
        <<centerText(std::to_string(resultData.score),scoreWidth)<<"| "
        <<centerText(std::to_string(resultData.wave),waveWidth)<<"| "
        <<centerText(std::to_string(resultData.killCount),killWidth)<<"| "
        <<centerText(std::to_string(resultData.difficulty)+"%",difficultyWidth)<<"| "
        <<centerText(std::to_string(rankScore),rankScoreWidth)<<"| "
        <<centerText(getRankText(rankScore),rankWidth)<<"| "
        <<centerText(isNewRecord?"New Best":"Normal",recordWidth)<<"\n";
}

void ResultScene::loadHistoryRecords()
{
    //重新加载最近历史记录,供结算页切换到历史视图时使用
    historyRecords.clear();
    std::ifstream file("ResultHistory.txt");
    if(file.is_open()==false)
    {
        return;
    }
    std::string line;
    while(std::getline(file,line))
    {
        if(line.empty()||line.find('|')==std::string::npos||
           line.find("Game No")!=std::string::npos)
        {
            continue;
        }
        std::stringstream stream(line);
        std::string timeText;
        std::string rankScoreText;
        std::string bestText;
        HistoryDisplayRecord record;
        std::getline(stream,record.no,'|');
        std::getline(stream,timeText,'|');
        std::getline(stream,record.score,'|');
        std::getline(stream,record.wave,'|');
        std::getline(stream,record.kills,'|');
        std::getline(stream,record.difficulty,'|');
        std::getline(stream,rankScoreText,'|');
        std::getline(stream,record.rank,'|');
        std::getline(stream,bestText,'|');
        record.no=trimText(record.no);
        record.score=trimText(record.score);
        record.wave=trimText(record.wave);
        record.kills=trimText(record.kills);
        record.difficulty=trimText(record.difficulty);
        record.rank=trimText(record.rank);
        historyRecords.push_back(record);
    }
    //最多只保留最近十条记录,避免结算页列表过长
    constexpr size_t maxRows=10;
    if(historyRecords.size()>maxRows)
    {
        historyRecords.erase(historyRecords.begin(),historyRecords.end()-maxRows);
    }
}

std::string ResultScene::getCurrentTimeText() const
{
    //读取当前本地时间并转换为固定显示文本
    std::time_t currentTime=std::time(nullptr);
    std::tm timeInfo{};
#if defined(_WIN32)
    localtime_s(&timeInfo,&currentTime);
#else
    localtime_r(&currentTime,&timeInfo);
#endif
    std::ostringstream timeStream;
    timeStream<<std::put_time(&timeInfo,"%Y-%m-%d %H:%M:%S");
    return timeStream.str();
}

std::string ResultScene::centerText(const std::string& text,int width) const
{
    //文本宽度超过列宽时直接返回原文,避免截断历史表头
    if(static_cast<int>(text.size())>=width)
    {
        return text;
    }
    int left=(width-static_cast<int>(text.size()))/2;
    int right=width-static_cast<int>(text.size())-left;
    return std::string(left,' ')+text+std::string(right,' ');
}

const char* ResultScene::getRankText(int score) const
{
    //根据评级分返回结算页显示的评级字母
    if(score>=180)
    {
        return "S";
    }
    if(score>=130)
    {
        return "A";
    }
    if(score>=85)
    {
        return "B";
    }
    return "C";
}

SDL_Color ResultScene::getRankColor(int score) const
{
    //根据评级分返回结算页使用的评级颜色
    if(score>=180)
    {
        return SDL_Color{255,220,80,255};
    }
    if(score>=130)
    {
        return SDL_Color{90,210,255,255};
    }
    if(score>=85)
    {
        return SDL_Color{120,255,140,255};
    }
    return SDL_Color{160,160,160,255};
}
