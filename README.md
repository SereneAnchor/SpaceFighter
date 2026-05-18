<p align="center">
  <img src="https://img.shields.io/badge/language-C%2B%2B17-%23f34b7d?style=flat-square" alt="C++17">
  <img src="https://img.shields.io/badge/framework-SDL2-%2343a047?style=flat-square" alt="SDL2">
  <img src="https://img.shields.io/badge/platform-Windows-%230078d7?style=flat-square" alt="Windows">
  <img src="https://img.shields.io/badge/build-CMake-%23e05d44?style=flat-square" alt="CMake">
  <img src="https://img.shields.io/badge/fps-60-%23ffa000?style=flat-square" alt="60 FPS">
</p>

# SpaceFighter

Language: [中文](#中文) | [English](#english)

---

## 中文

SpaceFighter 是一个使用 **C++17** 和 **SDL2** 开发的 2D 太空射击游戏。项目包含场景系统、敌人 FSM AI、波次系统、粒子系统、道具系统、动态难度、主菜单、暂停菜单、结算页和历史记录展示。

### 截图

> Coming soon

### 功能特性

#### 核心架构

- **场景管理**：通过 `Scene` 抽象基类统一管理 `initialScene`、`updateScene`、`renderScene`、`handleEvent` 和 `clearScene` 生命周期。
- **Game 单例**：集中管理 SDL 初始化、窗口、渲染器、主循环、帧率限制和场景切换。
- **事件总线**：碰撞检测只发布事件，特效、音效、计分等响应逻辑由事件分发统一处理。
- **对象池**：复用子弹、敌人、粒子和特效对象，减少频繁堆分配。
- **轻量 UI 层**：`GameUI` 统一绘制主菜单、暂停菜单、HUD、结算页和历史记录页。

#### 游戏系统

- **敌人系统**：支持不同敌人类型和不同生命、速度、分数配置。
- **FSM AI**：敌人可在 `PATROL`、`CHASE`、`ATTACK`、`FLEE` 状态间切换。
- **波次系统**：敌人按波次生成，波次之间有短暂休整和提示。
- **动态难度**：根据分数、命中表现和近期受伤情况动态调整难度。
- **道具系统**：敌人死亡后可掉落回血、护盾、射速增益等道具。
- **武器成长**：击杀敌人后提升武器等级，增强玩家火力。

#### 界面与反馈

- **主菜单**：支持开始游戏和 Easy / Normal / Hard 难度选择。
- **暂停菜单**：游戏中可暂停，并选择继续、重开或返回主菜单。
- **HUD**：显示生命值、分数、动态难度、武器等级、波次和道具拾取提示。
- **结算页**：显示本局分数、波次、击杀数、难度、评级和历史最佳记录。
- **历史记录页**：结算页按 `H` 可查看最近成绩记录。
- **视觉反馈**：包含受击闪烁、护盾光效、引擎火焰、爆炸和命中特效。
- **音频反馈**：使用 `SDL_mixer` 播放背景音乐、射击、爆炸、命中和拾取音效。

### 操作说明

#### 主菜单

| 按键 | 作用 |
|------|------|
| `↑` / `↓` | 选择菜单项 |
| `←` / `→` | 选中 Difficulty 时切换难度 |
| `Enter` | 选中 Start Game 时开始游戏 |
| `Space` | 快速开始游戏 |

#### 游戏中

| 按键 | 作用 |
|------|------|
| `W` `A` `S` `D` | 移动玩家飞船 |
| `J` | 射击 |
| `P` | 暂停游戏 |
| `ESC` | 返回主菜单 |

#### 暂停菜单

| 按键 | 作用 |
|------|------|
| `↑` / `↓` | 选择 Continue / Restart / Main Menu |
| `Enter` | 确认当前选项 |
| `P` | 继续游戏 |
| `R` | 重新开始 |
| `ESC` | 返回主菜单 |

#### 结算页

| 按键 | 作用 |
|------|------|
| `H` | 切换结果页 / 历史记录页 |
| `R` | 重新开始 |
| `ESC` | 返回主菜单 |

### 技术栈

| 组件 | 技术 |
|------|------|
| 语言 | C++17 |
| 窗口与输入 | SDL2 |
| 图片加载 | SDL2_image |
| 音频 | SDL2_mixer |
| 字体渲染 | SDL2_ttf |
| 构建系统 | CMake 3.10+ |

### 项目结构

```text
SpaceFighter/
├── assets/
│   ├── effect/              # 爆炸等序列帧特效
│   ├── font/                # 字体资源
│   ├── image/               # 飞船、敌人、子弹、背景、道具等图片
│   ├── music/               # 背景音乐
│   └── sound/               # 音效
├── src/
│   ├── main.cpp             # 程序入口
│   ├── Game.h / Game.cpp    # 游戏主控制器
│   ├── Scene.h / Scene.cpp  # 场景基类
│   ├── MenuScene.*          # 主菜单
│   ├── MainScene.*          # 游戏主场景
│   ├── ResultScene.*        # 结算与历史记录
│   ├── GameUI.*             # 轻量 UI 绘制模块
│   ├── Object.h             # 游戏对象和对象池
│   ├── EnemySystem.*        # 敌人系统
│   ├── ParticleSystem.*     # 粒子系统
│   ├── PowerUpSystem.*      # 道具系统
│   ├── WaveSystem.*         # 波次系统
│   ├── EventBus.*           # 事件总线
│   └── Config.h             # 可调参数
├── CMakeLists.txt
├── ResultHistory.txt        # 本地历史记录
└── README.md
```

### 构建与运行

#### 依赖

- 支持 C++17 的编译器，例如 MSVC 2019+、GCC 8+、Clang 7+
- SDL2、SDL2_image、SDL2_mixer、SDL2_ttf

#### 编译

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

生成的可执行文件会输出到项目根目录，例如：

```text
SpaceFighter-Windows.exe
```

#### 运行

直接启动生成的可执行文件即可。

### 场景流程

```text
MenuScene
  ├─ Start Game -> MainScene
  └─ Difficulty -> Easy / Normal / Hard

MainScene
  ├─ P -> Pause Menu
  ├─ ESC -> MenuScene
  └─ Player Dead -> ResultScene

Pause Menu
  ├─ Continue -> MainScene
  ├─ Restart -> MainScene
  └─ Main Menu -> MenuScene

ResultScene
  ├─ H -> History View / Result View
  ├─ R -> MainScene
  └─ ESC -> MenuScene
```

### Roadmap

- 更多武器类型，例如散射、追踪导弹。
- Boss 战或波次节点事件。
- 设置页，例如音量、键位提示或键位配置。
- 更完整的排行榜展示。
- 跨平台构建支持。

---

## English

SpaceFighter is a 2D space shooter built with **C++17** and **SDL2**. It includes scene management, FSM-based enemy AI, waves, particles, power-ups, dynamic difficulty, a main menu, pause menu, result screen, and in-game history display.

### Screenshots

> Coming soon

### Features

#### Core Architecture

- **Scene Management**: An abstract `Scene` base class defines `initialScene`, `updateScene`, `renderScene`, `handleEvent`, and `clearScene`.
- **Game Singleton**: Centralized SDL lifecycle, window, renderer, main loop, frame pacing, and scene switching.
- **Event Bus**: Collision code publishes events, while effects, audio, scoring, and other reactions are handled through event dispatch.
- **Object Pool**: Bullets, enemies, particles, and effects are recycled to reduce repeated heap allocations.
- **Lightweight UI Layer**: `GameUI` renders the main menu, pause menu, HUD, result screen, and history view.

#### Gameplay Systems

- **Enemy System**: Multiple enemy types with different health, speed, and score values.
- **FSM AI**: Enemies switch between `PATROL`, `CHASE`, `ATTACK`, and `FLEE`.
- **Wave System**: Enemies spawn in waves with rest periods and wave prompts.
- **Dynamic Difficulty**: Difficulty changes based on score, player accuracy behavior, and recent damage.
- **Power-Up System**: Enemies may drop healing, shield, and fire-rate power-ups.
- **Weapon Growth**: Player weapon level increases through kills.

#### UI, Visuals, And Audio

- **Main Menu**: Start game and choose Easy / Normal / Hard difficulty.
- **Pause Menu**: Continue, restart, or return to the main menu.
- **HUD**: Shows HP, score, dynamic difficulty, weapon level, wave, and pickup notice.
- **Result Screen**: Shows final score, wave, kills, difficulty, rank, and best records.
- **History View**: Press `H` on the result screen to view recent run history.
- **Visual Feedback**: Damage flash, shield glow, engine flame, explosions, and hit effects.
- **Audio Feedback**: Background music and sound effects via `SDL_mixer`.

### Controls

#### Main Menu

| Key | Action |
|-----|--------|
| `↑` / `↓` | Select menu item |
| `←` / `→` | Change difficulty when Difficulty is selected |
| `Enter` | Start game when Start Game is selected |
| `Space` | Quick start |

#### Gameplay

| Key | Action |
|-----|--------|
| `W` `A` `S` `D` | Move player ship |
| `J` | Shoot |
| `P` | Pause |
| `ESC` | Return to main menu |

#### Pause Menu

| Key | Action |
|-----|--------|
| `↑` / `↓` | Select Continue / Restart / Main Menu |
| `Enter` | Confirm selected item |
| `P` | Continue |
| `R` | Restart |
| `ESC` | Main menu |

#### Result Screen

| Key | Action |
|-----|--------|
| `H` | Toggle result view / history view |
| `R` | Restart |
| `ESC` | Main menu |

### Tech Stack

| Component | Technology |
|-----------|------------|
| Language | C++17 |
| Windowing & Input | SDL2 |
| Image Loading | SDL2_image |
| Audio | SDL2_mixer |
| Font Rendering | SDL2_ttf |
| Build System | CMake 3.10+ |

### Project Structure

```text
SpaceFighter/
├── assets/
│   ├── effect/              # Sprite-sheet effects
│   ├── font/                # Font assets
│   ├── image/               # Player, enemies, bullets, background, power-ups
│   ├── music/               # Background music
│   └── sound/               # Sound effects
├── src/
│   ├── main.cpp             # Entry point
│   ├── Game.h / Game.cpp    # Game controller
│   ├── Scene.h / Scene.cpp  # Scene base class
│   ├── MenuScene.*          # Main menu
│   ├── MainScene.*          # Gameplay scene
│   ├── ResultScene.*        # Result and history view
│   ├── GameUI.*             # Lightweight UI rendering module
│   ├── Object.h             # Game objects and object pool
│   ├── EnemySystem.*        # Enemy system
│   ├── ParticleSystem.*     # Particle system
│   ├── PowerUpSystem.*      # Power-up system
│   ├── WaveSystem.*         # Wave system
│   ├── EventBus.*           # Event bus
│   └── Config.h             # Tunable constants
├── CMakeLists.txt
├── ResultHistory.txt        # Local result history
└── README.md
```

### Build And Run

#### Prerequisites

- A C++17 compiler, such as MSVC 2019+, GCC 8+, or Clang 7+
- SDL2, SDL2_image, SDL2_mixer, and SDL2_ttf

#### Compile

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

The executable is generated in the project root, for example:

```text
SpaceFighter-Windows.exe
```

#### Run

Launch the generated executable directly.

### Scene Flow

```text
MenuScene
  ├─ Start Game -> MainScene
  └─ Difficulty -> Easy / Normal / Hard

MainScene
  ├─ P -> Pause Menu
  ├─ ESC -> MenuScene
  └─ Player Dead -> ResultScene

Pause Menu
  ├─ Continue -> MainScene
  ├─ Restart -> MainScene
  └─ Main Menu -> MenuScene

ResultScene
  ├─ H -> History View / Result View
  ├─ R -> MainScene
  └─ ESC -> MenuScene
```

### Roadmap

- More weapon types, such as spread shots and homing missiles.
- Boss encounters or wave milestone events.
- Settings screen for volume, key hints, or key binding.
- More complete leaderboard presentation.
- Cross-platform build support.
