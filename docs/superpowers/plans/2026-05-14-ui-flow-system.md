# UI Flow System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a compact `GameUI` module and use it to unify the main menu, pause menu, gameplay HUD, result summary, and in-result history view.

**Architecture:** Keep the existing `Game` and `Scene` architecture. Add one UI helper module, `GameUI`, that owns font rendering helpers and reusable UI drawing functions while scenes keep state, input decisions, and scene transitions.

**Tech Stack:** C++17, SDL2, SDL2_ttf, SDL2_image, SDL2_mixer, CMake.

---

## File Structure

- Create: `src/GameUI.h`
  - Declares `GameUI`, simple menu/history data structs, and helper methods for text, panels, health bars, menus, HUD, results, and history.
- Create: `src/GameUI.cpp`
  - Implements rendering helpers and compact UI drawing methods.
- Modify: `src/MenuScene.h`
  - Replace direct font ownership with `GameUI`, add selected menu state.
- Modify: `src/MenuScene.cpp`
  - Use `GameUI` for menu rendering and keyboard selection while preserving difficulty offset behavior.
- Modify: `src/MainScene.h`
  - Add `GameUI` member and selected pause menu state.
- Modify: `src/MainScene.cpp`
  - Initialize/clear UI, route pause menu input, and delegate HUD/pause/game-over rendering to `GameUI`.
- Modify: `src/ResultScene.h`
  - Add `GameUI`, history view state, and parsed history record storage.
- Modify: `src/ResultScene.cpp`
  - Render result/history through `GameUI`, add `H` toggle, parse recent history records.
- Modify: `CMakeLists.txt`
  - Include `src/GameUI.cpp` in the executable target.

## Task 1: Add `GameUI` Module

**Files:**
- Create: `src/GameUI.h`
- Create: `src/GameUI.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Add `GameUI.h`**

Create `src/GameUI.h` with:

```cpp
#ifndef GAMEUI
#define GAMEUI

#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_ttf.h>

struct MenuItem
{
    std::string label;
};

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

struct HistoryDisplayRecord
{
    std::string no;
    std::string score;
    std::string wave;
    std::string kills;
    std::string difficulty;
    std::string rank;
};

class GameUI
{
public:
    GameUI()=default;
    ~GameUI();

    bool initialize(SDL_Renderer* renderer,const char* fontPath,int fontSize);
    void clear();

    void renderText(const char* text,int x,int y,SDL_Color color);
    void renderTextCentered(const char* text,int cx,int y,SDL_Color color);
    void measureText(const char* text,int& width,int& height) const;

    void renderPanel(int x,int y,int w,int h,SDL_Color fill,SDL_Color border);
    void renderHealthBar(int x,int y,int w,int h,int value,int maxValue);
    void renderMenuList(const std::vector<MenuItem>& items,int selectedIndex,int centerX,int startY,int lineHeight);
    void renderMainMenu(int windowWidth,int windowHeight,const std::vector<MenuItem>& items,int selectedIndex);
    void renderPauseMenu(int windowWidth,int windowHeight,const std::vector<MenuItem>& items,int selectedIndex);
    void renderHud(int windowWidth,int windowHeight,const HudData& data);
    void renderResult(int windowWidth,int windowHeight,const ResultDisplayData& data);
    void renderHistory(int windowWidth,int windowHeight,const std::vector<HistoryDisplayRecord>& records);

    static int moveSelection(int selectedIndex,int direction,int itemCount);
    static const char* getRankText(int rankScore);
    static SDL_Color getRankColor(int rankScore);

private:
    SDL_Renderer* renderer=nullptr;
    TTF_Font* font=nullptr;
};

#endif
```

- [ ] **Step 2: Add `GameUI.cpp`**

Create `src/GameUI.cpp` with implementations for:

```cpp
#include "GameUI.h"

#include <algorithm>

GameUI::~GameUI()
{
    clear();
}

bool GameUI::initialize(SDL_Renderer* rendererValue,const char* fontPath,int fontSize)
{
    clear();
    renderer=rendererValue;
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
    if(font!=nullptr)
    {
        TTF_CloseFont(font);
        font=nullptr;
    }
    renderer=nullptr;
}
```

Implement the rest of `GameUI.cpp` with the following exact behavior:

- `renderText`: return early if `font`, `renderer`, or `text` is null; create a blended UTF-8 surface, create a texture, render it at `{x,y,surface->w,surface->h}`, then free the surface and destroy the texture.
- `renderTextCentered`: call `measureText`, subtract half the measured width from `cx`, and call `renderText`.
- `measureText`: set width and height to zero when `font` or `text` is null; otherwise call `TTF_SizeUTF8`.
- `renderPanel`: set blend mode to `SDL_BLENDMODE_BLEND`, fill the rect with the supplied fill color, draw the border with the supplied border color, then restore `SDL_BLENDMODE_NONE`.
- `renderHealthBar`: draw a dark background, draw a gray border, clamp `value` to `[0,maxValue]`, choose green/yellow/red based on percent, and fill the inner bar.
- `renderMenuList`: draw every item centered; draw selected item in yellow and unselected items in gray; prefix selected item with `> ` and suffix it with ` <`.
- `renderMainMenu`: draw `SPACE FIGHTER`, draw the menu list, then draw concise controls text.
- `renderPauseMenu`: draw a translucent full-screen overlay, draw a centered panel, draw `PAUSED`, then draw the menu list.
- `renderHud`: draw the top HUD strip, health bar, HP text, score, difficulty, weapon level, wave text, wave prompt, wave complete prompt, pickup notice, and game-over prompt.
- `renderResult`: draw result title, optional `NEW RECORD`, rank, score, wave, kills, difficulty, best score, best wave, best rank, and footer controls.
- `renderHistory`: draw `HISTORY`, render up to the supplied records, show `No history records` when the vector is empty, and draw footer controls.
- `moveSelection`: return zero when `itemCount <= 0`; otherwise wrap selection with `(selectedIndex + direction + itemCount) % itemCount`.
- `getRankText` and `getRankColor`: use the same S/A/B/C thresholds as `ResultScene`.

Use these constants inside the `.cpp` file:

```cpp
namespace
{
    constexpr int kMenuLineHeight=38;
    constexpr int kHudHeight=64;
    constexpr int kHistoryMaxRows=10;

    SDL_Color white() { return SDL_Color{255,255,255,255}; }
    SDL_Color gray() { return SDL_Color{180,180,180,255}; }
    SDL_Color cyan() { return SDL_Color{80,200,255,255}; }
    SDL_Color yellow() { return SDL_Color{255,220,80,255}; }
}
```

- [ ] **Step 3: Add CMake source**

Open `CMakeLists.txt`, find the executable source list, and add:

```cmake
src/GameUI.cpp
```

Expected result: the project target compiles `GameUI.cpp`.

- [ ] **Step 4: Build after adding the module**

Run:

```powershell
cmake --build build
```

Expected: build may fail if the local build directory is not configured, but it should not fail because of missing `GameUI` symbols once the file is included.

## Task 2: Convert Main Menu to `GameUI`

**Files:**
- Modify: `src/MenuScene.h`
- Modify: `src/MenuScene.cpp`

- [ ] **Step 1: Update `MenuScene.h`**

Add:

```cpp
#include <vector>
#include <string>
#include "GameUI.h"
```

Replace `TTF_Font* titleFont` and `TTF_Font* infoFont` members with:

```cpp
GameUI ui;
int selectedMenuIndex=0;
std::vector<MenuItem> menuItems;
```

Remove the `renderText(...)` declaration from `MenuScene`, because text rendering moves into `GameUI`.

- [ ] **Step 2: Initialize menu UI**

In `MenuScene::initialScene()`, replace the two font loads with:

```cpp
ui.initialize(game.getRenderer(),"assets/font/VonwaonBitmap-16px.ttf",24);
menuItems.clear();
menuItems.push_back(MenuItem{"Start Game"});
menuItems.push_back(MenuItem{"Difficulty: NORMAL"});
selectedMenuIndex=0;
```

- [ ] **Step 3: Update difficulty label**

Add a local helper in `MenuScene.cpp`:

```cpp
namespace
{
    const char* difficultyName(int index)
    {
        if(index==0) return "EASY";
        if(index==2) return "HARD";
        return "NORMAL";
    }
}
```

Before rendering, set:

```cpp
menuItems[1].label="Difficulty: "+std::string(difficultyName(difficultyIndex));
```

- [ ] **Step 4: Render through `GameUI`**

Replace `MenuScene::renderScene()` content with:

```cpp
auto renderer=game.getRenderer();
SDL_SetRenderDrawColor(renderer,8,12,32,255);
SDL_RenderClear(renderer);

menuItems[1].label="Difficulty: "+std::string(difficultyName(difficultyIndex));
ui.renderMainMenu(game.getWindowWidth(),game.getWindowHeight(),menuItems,selectedMenuIndex);
```

- [ ] **Step 5: Update menu input**

In `MenuScene::handleEvent()`, keep the `SDL_KEYDOWN` guard and implement:

```cpp
auto key=event->key.keysym.scancode;
if(key==SDL_SCANCODE_UP)
{
    selectedMenuIndex=GameUI::moveSelection(selectedMenuIndex,-1,static_cast<int>(menuItems.size()));
}
else if(key==SDL_SCANCODE_DOWN)
{
    selectedMenuIndex=GameUI::moveSelection(selectedMenuIndex,1,static_cast<int>(menuItems.size()));
}
else if(key==SDL_SCANCODE_LEFT&&selectedMenuIndex==1)
{
    difficultyIndex=(difficultyIndex+2)%3;
}
else if(key==SDL_SCANCODE_RIGHT&&selectedMenuIndex==1)
{
    difficultyIndex=(difficultyIndex+1)%3;
}
else if(key==SDL_SCANCODE_RETURN||key==SDL_SCANCODE_SPACE)
{
    if(selectedMenuIndex==0||key==SDL_SCANCODE_SPACE)
    {
        if(difficultyIndex==0)      gDifficultyOffset=-0.30f;
        else if(difficultyIndex==1) gDifficultyOffset=0.0f;
        else                        gDifficultyOffset=0.40f;
        game.requestSceneChange(new MainScene());
    }
}
```

- [ ] **Step 6: Clear UI**

In `MenuScene::clearScene()`:

```cpp
ui.clear();
```

- [ ] **Step 7: Build**

Run:

```powershell
cmake --build build
```

Expected: menu scene compiles without direct `TTF_Font*` members.

## Task 3: Upgrade Pause Menu and HUD Rendering

**Files:**
- Modify: `src/MainScene.h`
- Modify: `src/MainScene.cpp`

- [ ] **Step 1: Update `MainScene.h`**

Add:

```cpp
#include <vector>
#include "GameUI.h"
```

Add private members:

```cpp
GameUI ui;
std::vector<MenuItem> pauseMenuItems;
int selectedPauseIndex=0;
```

Remove `TTF_Font* font=nullptr;` after replacing all `font` usage with `GameUI`.

- [ ] **Step 2: Initialize UI and pause menu**

In `MainScene::initialScene()`, replace the font load block with:

```cpp
ui.initialize(game.getRenderer(),"assets/font/VonwaonBitmap-16px.ttf",24);
pauseMenuItems.clear();
pauseMenuItems.push_back(MenuItem{"Continue"});
pauseMenuItems.push_back(MenuItem{"Restart"});
pauseMenuItems.push_back(MenuItem{"Main Menu"});
selectedPauseIndex=0;
```

- [ ] **Step 3: Update pause input**

In `MainScene::handleEvent()`, while handling `SDL_KEYDOWN`, add paused menu navigation:

```cpp
if(isPaused&&player.isAlive)
{
    if(event->key.keysym.scancode==SDL_SCANCODE_UP)
    {
        selectedPauseIndex=GameUI::moveSelection(selectedPauseIndex,-1,static_cast<int>(pauseMenuItems.size()));
        return;
    }
    if(event->key.keysym.scancode==SDL_SCANCODE_DOWN)
    {
        selectedPauseIndex=GameUI::moveSelection(selectedPauseIndex,1,static_cast<int>(pauseMenuItems.size()));
        return;
    }
    if(event->key.keysym.scancode==SDL_SCANCODE_RETURN)
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
```

Keep existing shortcut behavior:

```cpp
case SDL_SCANCODE_P:
case SDL_SCANCODE_R:
case SDL_SCANCODE_ESCAPE:
```

- [ ] **Step 4: Replace HUD body with `GameUI` calls**

Replace `MainScene::renderGameInfo()` with:

```cpp
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

if(isPaused&&player.isAlive)
{
    ui.renderPauseMenu(game.getWindowWidth(),game.getWindowHeight(),
                       pauseMenuItems,selectedPauseIndex);
}
```

- [ ] **Step 5: Remove old text helpers**

Remove or stop using:

```cpp
void MainScene::renderText(...)
void MainScene::renderTextCentered(...)
```

Also remove their declarations from `MainScene.h`.

- [ ] **Step 6: Clear UI**

In `MainScene::clearScene()`, remove direct font closing and add:

```cpp
ui.clear();
```

- [ ] **Step 7: Build**

Run:

```powershell
cmake --build build
```

Expected: main scene compiles and no remaining `font` references exist in `MainScene`.

## Task 4: Enhance Result Screen and Add History Toggle

**Files:**
- Modify: `src/ResultScene.h`
- Modify: `src/ResultScene.cpp`

- [ ] **Step 1: Update `ResultScene.h`**

Add:

```cpp
#include <vector>
#include "GameUI.h"
```

Add private members:

```cpp
GameUI ui;
bool showHistory=false;
std::vector<HistoryDisplayRecord> historyRecords;
```

Remove `TTF_Font* font=nullptr;` after replacing text rendering.

Declare:

```cpp
void loadHistoryRecords();
```

Remove `renderText(...)` and `renderTextCentered(...)` declarations once they are no longer used.

- [ ] **Step 2: Initialize result UI**

In `ResultScene::initialScene()`, replace the font load with:

```cpp
ui.initialize(game.getRenderer(),"assets/font/VonwaonBitmap-16px.ttf",24);
```

After `saveResultHistory();`, call:

```cpp
loadHistoryRecords();
```

- [ ] **Step 3: Add H toggle**

In `ResultScene::handleEvent()`, add:

```cpp
case SDL_SCANCODE_H:
    showHistory=!showHistory;
    break;
```

Keep existing `R` and `ESC`.

- [ ] **Step 4: Render result or history**

Replace `ResultScene::renderScene()` body after background drawing with:

```cpp
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
```

- [ ] **Step 5: Parse recent history**

Implement `ResultScene::loadHistoryRecords()`:

```cpp
void ResultScene::loadHistoryRecords()
{
    historyRecords.clear();
    std::ifstream file("result_history.txt");
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
        HistoryDisplayRecord record;
        std::getline(stream,record.no,'|');
        std::getline(stream,timeText,'|');
        std::getline(stream,record.score,'|');
        std::getline(stream,record.wave,'|');
        std::getline(stream,record.kills,'|');
        std::getline(stream,record.difficulty,'|');
        std::string rankScoreText;
        std::getline(stream,rankScoreText,'|');
        std::getline(stream,record.rank,'|');
        historyRecords.push_back(record);
    }

    constexpr size_t maxRows=10;
    if(historyRecords.size()>maxRows)
    {
        historyRecords.erase(historyRecords.begin(),
                             historyRecords.end()-static_cast<long>(maxRows));
    }
}
```

- [ ] **Step 6: Clear UI**

In `ResultScene::clearScene()`, replace direct font closing with:

```cpp
ui.clear();
```

- [ ] **Step 7: Remove duplicate rank helpers if desired**

Keep `ResultScene::getRankText()` and `getRankColor()` if they are still used for saving and best-record logic. Do not force-delete them unless all references are cleanly replaced.

- [ ] **Step 8: Build**

Run:

```powershell
cmake --build build
```

Expected: result scene compiles and can toggle display state with `H`.

## Task 5: End-to-End Verification

**Files:**
- No source changes unless defects are found.

- [ ] **Step 1: Build the project**

Run:

```powershell
cmake --build build
```

Expected: successful build.

- [ ] **Step 2: Launch the executable**

Run the generated executable from the project root, for example:

```powershell
.\SpaceFighter-Windows.exe
```

Expected: the game starts at the unified main menu.

- [ ] **Step 3: Verify main menu**

Expected:

- Up/down moves between `Start Game` and `Difficulty`.
- Left/right changes difficulty while `Difficulty` is selected.
- Enter on `Start Game` starts gameplay.
- Space quick-starts gameplay.

- [ ] **Step 4: Verify pause menu**

Expected:

- `P` pauses gameplay.
- Up/down changes the highlighted pause option.
- Enter on `Continue` resumes gameplay.
- Enter on `Restart` restarts gameplay.
- Enter on `Main Menu` returns to main menu.
- Shortcut keys still work: `P`, `R`, `ESC`.

- [ ] **Step 5: Verify HUD**

Expected:

- HP bar and numeric HP render correctly.
- Score, difficulty, weapon level, wave, wave prompts, pickup notice, and game-over prompt still render.

- [ ] **Step 6: Verify result screen**

Expected:

- Result screen shows score, wave, kills, difficulty, rank, best score, best wave, and best rank.
- `H` toggles to history.
- `H` toggles back to result summary.
- `R` restarts.
- `ESC` returns to main menu.

- [ ] **Step 7: Verify history fallback**

Temporarily rename `result_history.txt` outside the game process, then reach the result screen.

Expected:

- History view shows a fallback message.
- Game does not crash.

Restore `result_history.txt` after the check.
