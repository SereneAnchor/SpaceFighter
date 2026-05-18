#include "GameUI.h"

#include <algorithm>
#include <string>

namespace
{
    //返回UI主要文本使用的白色
    SDL_Color white()
    {
        return SDL_Color{255,255,255,255};
    }

    //返回UI普通辅助文本使用的灰色
    SDL_Color gray()
    {
        return SDL_Color{180,180,180,255};
    }

    //返回UI弱提示和快捷键说明使用的暗灰色
    SDL_Color darkGray()
    {
        return SDL_Color{80,90,110,255};
    }

    //返回UI强调信息使用的青色
    SDL_Color cyan()
    {
        return SDL_Color{80,200,255,255};
    }

    //返回UI选中项和标题强调使用的黄色
    SDL_Color yellow()
    {
        return SDL_Color{255,220,80,255};
    }

    //返回UI成功状态和新纪录提示使用的绿色
    SDL_Color green()
    {
        return SDL_Color{120,255,140,255};
    }

    //返回UI危险状态和游戏结束提示使用的红色
    SDL_Color red()
    {
        return SDL_Color{255,80,80,255};
    }
}

GameUI::~GameUI()
{
    //析构时释放当前UI对象持有的字体资源
    clear();
}

bool GameUI::initialize(SDL_Renderer* rendererValue,const char* fontPath,int fontSize)
{
    //初始化前先释放旧字体,避免同一个GameUI重复初始化时泄漏
    clear();
    //保存Game创建的主渲染器引用,后续所有UI都绘制到该渲染器
    renderer=rendererValue;
    //加载菜单、HUD和结算页共用的字体资源
    font=TTF_OpenFont(fontPath,fontSize);
    if(font==nullptr)
    {
        SDL_Log("GameUI::initialize failed to load font: %s",TTF_GetError());
        return false;
    }
    return true;
}

void GameUI::clear()
{
    //释放GameUI持有的字体资源
    if(font!=nullptr)
    {
        TTF_CloseFont(font);
        font=nullptr;
    }
    //清空渲染器引用,GameUI不负责销毁Game持有的渲染器
    renderer=nullptr;
}

void GameUI::renderText(const char* text,int x,int y,SDL_Color color)
{
    //缺少字体、渲染器或文本时无法绘制
    if(font==nullptr||renderer==nullptr||text==nullptr)
    {
        return;
    }
    //把UTF-8文本用当前字体渲染为临时Surface
    auto surface=TTF_RenderUTF8_Blended(font,text,color);
    if(surface==nullptr)
    {
        return;
    }
    //把Surface转换为SDL纹理,便于提交到Game主渲染器
    auto texture=SDL_CreateTextureFromSurface(renderer,surface);
    if(texture==nullptr)
    {
        SDL_FreeSurface(surface);
        return;
    }
    //使用Surface尺寸确定文本绘制矩形
    SDL_Rect rect={x,y,surface->w,surface->h};
    //转换为纹理后立即释放临时Surface
    SDL_FreeSurface(surface);
    //把文本纹理绘制到GameUI绑定的渲染器
    SDL_RenderCopy(renderer,texture,nullptr,&rect);
    //文本纹理只服务本次绘制,绘制后立即销毁
    SDL_DestroyTexture(texture);
}

void GameUI::renderTextCentered(const char* text,int cx,int y,SDL_Color color)
{
    //先测量文本宽度,再把左上角向左偏移半个文本宽度
    int width=0;
    int height=0;
    measureText(text,width,height);
    (void)height;
    renderText(text,cx-width/2,y,color);
}

void GameUI::measureText(const char* text,int& width,int& height) const
{
    //默认返回0尺寸,避免调用方读取未初始化宽高
    width=0;
    height=0;
    //缺少字体或文本时无法测量
    if(font==nullptr||text==nullptr)
    {
        return;
    }
    //使用当前字体计算UTF-8文本实际渲染尺寸
    TTF_SizeUTF8(font,text,&width,&height);
}

void GameUI::renderPanel(int x,int y,int w,int h,SDL_Color fill,SDL_Color border)
{
    //没有绑定Game主渲染器时无法绘制面板
    if(renderer==nullptr)
    {
        return;
    }
    //开启混合模式绘制半透明面板
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    SDL_Rect rect={x,y,w,h};
    SDL_SetRenderDrawColor(renderer,fill.r,fill.g,fill.b,fill.a);
    SDL_RenderFillRect(renderer,&rect);
    //使用边框颜色绘制面板外框
    SDL_SetRenderDrawColor(renderer,border.r,border.g,border.b,border.a);
    SDL_RenderDrawRect(renderer,&rect);
    //恢复默认非混合绘制模式,避免影响后续普通绘制
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
}

void GameUI::renderHealthBar(int x,int y,int w,int h,int value,int maxValue)
{
    //没有绑定Game主渲染器时无法绘制血条
    if(renderer==nullptr)
    {
        return;
    }
    //先绘制血条背景和边框
    SDL_Rect back={x,y,w,h};
    SDL_SetRenderDrawColor(renderer,35,35,42,255);
    SDL_RenderFillRect(renderer,&back);
    SDL_SetRenderDrawColor(renderer,120,120,130,255);
    SDL_RenderDrawRect(renderer,&back);
    //最大值无效时只保留背景,避免除零
    if(maxValue<=0)
    {
        return;
    }
    //把当前值限制在合法范围内,避免血条越界
    int clampedValue=std::max(0,std::min(value,maxValue));
    int pct=clampedValue*100/maxValue;
    //根据生命百分比选择血条颜色
    if(pct>=80)
    {
        SDL_SetRenderDrawColor(renderer,70,220,90,255);
    }
    else if(pct>=30)
    {
        SDL_SetRenderDrawColor(renderer,230,210,70,255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer,230,70,70,255);
    }
    //按生命比例计算前景宽度并绘制填充部分
    int fillWidth=clampedValue*w/maxValue;
    SDL_Rect front={x+2,y+2,fillWidth>4?fillWidth-4:0,h-4};
    SDL_RenderFillRect(renderer,&front);
}

void GameUI::renderMenuList(const std::vector<MenuItem>& items,int selectedIndex,
                            int centerX,int startY,int lineHeight)
{
    //逐项绘制MenuScene或暂停菜单传入的菜单文本
    for(int i=0;i<static_cast<int>(items.size());i++)
    {
        std::string label=items[static_cast<size_t>(i)].label;
        SDL_Color color=gray();
        //当前选中项添加尖括号并使用高亮颜色
        if(i==selectedIndex)
        {
            label="> "+label+" <";
            color=yellow();
        }
        renderTextCentered(label.c_str(),centerX,startY+i*lineHeight,color);
    }
}

void GameUI::renderMainMenu(int windowWidth,int windowHeight,
                            const std::vector<MenuItem>& items,int selectedIndex)
{
    //以窗口中心作为主菜单所有文本的横向基准
    int centerX=windowWidth/2;
    //绘制游戏标题
    renderTextCentered("SPACE FIGHTER",centerX,static_cast<int>(windowHeight*0.26f),cyan());
    //绘制MenuScene维护的菜单项和当前选中项
    renderMenuList(items,selectedIndex,centerX,static_cast<int>(windowHeight*0.46f),42);
    //绘制主菜单操作提示
    renderTextCentered("UP / DOWN Select    LEFT / RIGHT Change Difficulty",
                       centerX,static_cast<int>(windowHeight*0.68f),darkGray());
    renderTextCentered("ENTER / SPACE  Start Game",
                       centerX,static_cast<int>(windowHeight*0.74f),gray());
    renderTextCentered("W A S D Move    J Shoot    P Pause",
                       centerX,static_cast<int>(windowHeight*0.84f),darkGray());
}

void GameUI::renderPauseMenu(int windowWidth,int windowHeight,
                             const std::vector<MenuItem>& items,int selectedIndex)
{
    //没有绑定Game主渲染器时无法绘制暂停菜单
    if(renderer==nullptr)
    {
        return;
    }
    //绘制覆盖当前游戏画面的半透明黑色遮罩
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,0,0,0,160);
    SDL_Rect overlay={0,0,windowWidth,windowHeight};
    SDL_RenderFillRect(renderer,&overlay);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
    //计算暂停面板在窗口中央的位置
    int panelW=320;
    int panelH=230;
    int panelX=windowWidth/2-panelW/2;
    int panelY=windowHeight/2-panelH/2;
    //绘制暂停面板、标题、菜单项和快捷键提示
    renderPanel(panelX,panelY,panelW,panelH,
                SDL_Color{8,12,28,220},SDL_Color{80,140,190,180});
    renderTextCentered("PAUSED",windowWidth/2,panelY+34,white());
    renderMenuList(items,selectedIndex,windowWidth/2,panelY+90,34);
    renderTextCentered("P Continue    R Restart    ESC Menu",
                       windowWidth/2,panelY+panelH-38,darkGray());
}

void GameUI::renderHud(int windowWidth,int windowHeight,const HudData& data)
{
    //没有绑定Game主渲染器时无法绘制HUD
    if(renderer==nullptr)
    {
        return;
    }
    //绘制MainScene游戏画面顶部的HUD背景条
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    SDL_Rect hudRect={0,0,windowWidth,64};
    SDL_SetRenderDrawColor(renderer,6,10,20,185);
    SDL_RenderFillRect(renderer,&hudRect);
    SDL_SetRenderDrawColor(renderer,80,120,170,120);
    SDL_RenderDrawLine(renderer,0,63,windowWidth,63);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
    //绘制玩家生命值和生命条
    renderText("HP",14,8,gray());
    renderHealthBar(48,14,132,14,data.health,data.maxHealth);
    std::string hpText=std::to_string(data.health)+" / "+std::to_string(data.maxHealth);
    renderText(hpText.c_str(),188,8,white());
    //绘制MainScene传入的分数、难度和武器等级
    renderText("SCORE",360,8,gray());
    renderText(std::to_string(data.score).c_str(),452,8,white());
    renderText("DIFF",14,36,gray());
    std::string diffText=std::to_string(data.difficultyPercent)+"%";
    renderText(diffText.c_str(),82,36,yellow());
    renderText("WEAPON",190,36,gray());
    std::string weaponText="LV"+std::to_string(data.weaponLevel);
    renderText(weaponText.c_str(),300,36,cyan());
    //当前波次有效时把波次信息绘制到HUD右侧
    if(data.wave>0)
    {
        std::string waveText="Wave:"+std::to_string(data.wave);
        int width=0;
        int height=0;
        measureText(waveText.c_str(),width,height);
        (void)height;
        renderText(waveText.c_str(),windowWidth-width-14,36,gray());
    }
    //波次开始时绘制MainScene传入的WAVE居中提示
    if(data.waveTextTimer>0)
    {
        std::string bigWave="WAVE "+std::to_string(data.wave);
        renderTextCentered(bigWave.c_str(),windowWidth/2,windowHeight/2-40,yellow());
    }
    //波次完成时绘制MainScene传入的完成提示
    if(data.waveCompleteTimer>0)
    {
        std::string completeText="WAVE "+std::to_string(data.wave)+" COMPLETE";
        renderTextCentered(completeText.c_str(),windowWidth/2,windowHeight/2-40,green());
    }
    //道具系统提供拾取提示时在画面中部显示
    if(data.showPickupNotice)
    {
        renderTextCentered(data.pickupNotice.c_str(),windowWidth/2,windowHeight/2+50,cyan());
    }
    //玩家死亡后绘制Game Over提示和结果页前的快捷键提示
    if(data.showGameOver)
    {
        renderTextCentered("GAME OVER",windowWidth/2,windowHeight/2-24,red());
        renderTextCentered("R - Restart  ESC - Menu",windowWidth/2,windowHeight/2+8,gray());
    }
}

void GameUI::renderResult(int windowWidth,int windowHeight,const ResultDisplayData& data)
{
    //以窗口中心作为结算页文本的横向基准
    int centerX=windowWidth/2;
    int y=68;
    int lineHeight=28;
    //绘制结算页标题
    renderTextCentered("GAME RESULT",centerX,y,yellow());
    y+=34;
    //刷新最高分时显示新纪录提示
    if(data.isNewRecord)
    {
        renderTextCentered("NEW RECORD",centerX,y,green());
        y+=30;
    }
    //根据ResultScene计算出的评分分数绘制评级
    renderTextCentered(("Rank: "+std::string(getRankText(data.rankScore))).c_str(),
                       centerX,y,getRankColor(data.rankScore));
    y+=lineHeight+10;
    //绘制ResultScene传入的本局成绩
    renderTextCentered(("Score: "+std::to_string(data.score)).c_str(),centerX,y,white());
    renderTextCentered(("Wave: "+std::to_string(data.wave)).c_str(),centerX,y+lineHeight,white());
    renderTextCentered(("Kills: "+std::to_string(data.killCount)).c_str(),centerX,y+lineHeight*2,white());
    renderTextCentered(("Difficulty: "+std::to_string(data.difficulty)+"%").c_str(),
                       centerX,y+lineHeight*3,cyan());
    //绘制ResultScene读取到的历史最佳成绩
    int bestY=y+lineHeight*5;
    renderTextCentered(("Best Score: "+std::to_string(data.bestScore)).c_str(),
                       centerX,bestY,gray());
    renderTextCentered(("Best Wave: "+std::to_string(data.bestWave)).c_str(),
                       centerX,bestY+lineHeight,gray());
    renderTextCentered(("Best Rank: "+std::string(getRankText(data.bestRankScore))).c_str(),
                       centerX,bestY+lineHeight*2,getRankColor(data.bestRankScore));
    //绘制结算页快捷操作提示
    renderTextCentered("H History",centerX,windowHeight-158,cyan());
    renderTextCentered("R Restart",centerX,windowHeight-122,white());
    renderTextCentered("ESC Menu",centerX,windowHeight-86,gray());
}

void GameUI::renderHistory(int windowWidth,int windowHeight,
                           const std::vector<HistoryDisplayRecord>& records)
{
    //以窗口中心作为历史页标题和底部提示的横向基准
    int centerX=windowWidth/2;
    renderTextCentered("HISTORY",centerX,62,yellow());
    //最多显示最近8条历史记录
    int visibleRows=std::min(static_cast<int>(records.size()),8);
    //根据历史记录数量计算面板高度
    int panelX=46;
    int panelY=104;
    int panelW=windowWidth-92;
    int panelH=records.empty()?190:82+visibleRows*34;
    renderPanel(panelX,panelY,panelW,panelH,
                SDL_Color{8,12,28,190},SDL_Color{70,110,150,160});
    //没有历史记录时绘制空状态提示
    if(records.empty())
    {
        renderTextCentered("No history records",centerX,panelY+84,gray());
    }
    else
    {
        //计算历史记录表头和列位置
        int headerY=panelY+24;
        int rowY=panelY+64;
        int noX=62;
        int scoreX=118;
        int waveX=214;
        int killsX=300;
        int diffX=392;
        int rankX=470;
        //绘制历史记录表头
        renderText("#",noX,headerY,cyan());
        renderText("SCORE",scoreX,headerY,cyan());
        renderText("WAVE",waveX,headerY,cyan());
        renderText("KILLS",killsX,headerY,cyan());
        renderText("DIFF",diffX,headerY,cyan());
        renderText("RANK",rankX,headerY,cyan());
        //绘制表头和数据行之间的分隔线
        if(renderer!=nullptr)
        {
            SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer,80,120,170,115);
            SDL_RenderDrawLine(renderer,panelX+18,headerY+32,panelX+panelW-18,headerY+32);
            SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
        }
        //从最新记录开始绘制最近的历史成绩
        for(int i=0;i<visibleRows;i++)
        {
            const auto& record=records[records.size()-1-static_cast<size_t>(i)];
            int y=rowY+i*34;
            SDL_Color rowColor=(i==0)?white():gray();
            //绘制本行历史记录的文本列
            renderText(record.no.c_str(),noX,y,rowColor);
            renderText(record.score.c_str(),scoreX,y,rowColor);
            renderText(record.wave.c_str(),waveX,y,rowColor);
            renderText(record.kills.c_str(),killsX,y,rowColor);
            renderText(record.difficulty.c_str(),diffX,y,rowColor);
            //把历史记录中的评级文本转换为颜色选择所需的评分区间
            int rankValue=0;
            if(record.rank=="S")
            {
                rankValue=180;
            }
            else if(record.rank=="A")
            {
                rankValue=130;
            }
            else if(record.rank=="B")
            {
                rankValue=85;
            }
            renderText(record.rank.c_str(),rankX,y,getRankColor(rankValue));
        }
    }
    //绘制历史页快捷操作提示
    renderTextCentered("H Result    R Restart    ESC Menu",
                       centerX,windowHeight-74,gray());
}

int GameUI::moveSelection(int selectedIndex,int direction,int itemCount)
{
    //没有可选菜单项时固定返回0
    if(itemCount<=0)
    {
        return 0;
    }
    //用取模实现菜单选择在首尾之间循环移动
    return (selectedIndex+direction+itemCount)%itemCount;
}

const char* GameUI::getRankText(int rankScore)
{
    //根据ResultScene计算出的评分分数映射到显示评级
    if(rankScore>=180)
    {
        return "S";
    }
    if(rankScore>=130)
    {
        return "A";
    }
    if(rankScore>=85)
    {
        return "B";
    }
    return "C";
}

SDL_Color GameUI::getRankColor(int rankScore)
{
    //根据评分分数映射到结算页和历史页使用的评级颜色
    if(rankScore>=180)
    {
        return SDL_Color{255,220,80,255};
    }
    if(rankScore>=130)
    {
        return SDL_Color{90,210,255,255};
    }
    if(rankScore>=85)
    {
        return SDL_Color{120,255,140,255};
    }
    return SDL_Color{160,160,160,255};
}
