# Weighted Enemy Spawn Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the current enemy-2 probability rule with a reusable weighted-random enemy type selection system.

**Architecture:** Add a small header-only weighted-random helper and an `EnemySpawnType` enum. `WaveSystem` chooses a spawn type from wave-based weights, while `EnemySystem` remains responsible for creating the selected enemy.

**Tech Stack:** C++17, SDL2, existing `WaveSystem` and `EnemySystem`.

---

## File Structure

- Create: `src/WeightedRandom.h`
  - Header-only weighted choice helper.
- Modify: `src/EnemySystem.h`
  - Add `EnemySpawnType` enum and `generateByType(...)`.
- Modify: `src/EnemySystem.cpp`
  - Implement type-based enemy generation.
- Modify: `src/WaveSystem.h`
  - Replace enemy-2 chance helper with spawn type selection.
- Modify: `src/WaveSystem.cpp`
  - Use weighted random selection per wave.
- Optionally modify: `README.md`
  - Add a short note about weighted enemy spawning after implementation is verified.

## Task 1: Add Weighted Random Helper

**Files:**
- Create: `src/WeightedRandom.h`

- [ ] **Step 1: Create `WeightedRandom.h`**

```cpp
#ifndef WEIGHTEDRANDOM
#define WEIGHTEDRANDOM

#include <random>
#include <vector>

template<typename T>
struct WeightedChoice
{
    T value;
    float weight=0.0f;
};

template<typename T>
T chooseWeighted(const std::vector<WeightedChoice<T>>& choices,
                 std::mt19937& gen,
                 T fallback)
{
    float totalWeight=0.0f;
    for(const auto& choice:choices)
    {
        if(choice.weight>0.0f)
        {
            totalWeight+=choice.weight;
        }
    }

    if(totalWeight<=0.0f)
    {
        return fallback;
    }

    std::uniform_real_distribution<float> distribution(0.0f,totalWeight);
    float roll=distribution(gen);
    float cursor=0.0f;

    for(const auto& choice:choices)
    {
        if(choice.weight<=0.0f)
        {
            continue;
        }
        cursor+=choice.weight;
        if(roll<=cursor)
        {
            return choice.value;
        }
    }

    return fallback;
}

#endif
```

- [ ] **Step 2: Build check**

Run:

```powershell
cmake --build build
```

Expected: build succeeds, because the new header is not used yet.

## Task 2: Add Enemy Spawn Type API

**Files:**
- Modify: `src/EnemySystem.h`
- Modify: `src/EnemySystem.cpp`

- [ ] **Step 1: Add enum and method declaration**

In `src/EnemySystem.h`, before `class EnemySystem`, add:

```cpp
enum class EnemySpawnType
{
    Light,
    Heavy,
    Elite
};
```

Add public method:

```cpp
void generateByType(EnemySpawnType type,float difficulty,std::mt19937& gen,
                    std::uniform_real_distribution<float>& dis,int windowW,
                    bool forceSpawn=false);
```

- [ ] **Step 2: Inspect current `generate(...)` implementation**

Open `src/EnemySystem.cpp` and locate `EnemySystem::generate(...)`. Identify the common code that:

- Acquires an enemy from the pool.
- Copies the selected template.
- Applies difficulty scaling.
- Sets spawn position and movement data.
- Pushes the enemy into `enemyList`.

- [ ] **Step 3: Implement `generateByType(...)`**

Implement `generateByType(...)` by reusing the same spawn setup as `generate(...)`, but choose the template from `type`:

```cpp
const Enemy* source=nullptr;
if(type==EnemySpawnType::Heavy||type==EnemySpawnType::Elite)
{
    source=&insect1Template;
}
else
{
    source=&insect2Template;
}
```

`Elite` intentionally falls back to the heavy template for now.

- [ ] **Step 4: Keep old `generate(...)` compatible**

Update existing `generate(...)` so it chooses:

```cpp
EnemySpawnType type=EnemySpawnType::Light;
if(dis(gen)<insect1Chance)
{
    type=EnemySpawnType::Heavy;
}
generateByType(type,difficulty,gen,dis,windowW,forceSpawn);
```

`forceSpawn` must keep its existing meaning: it allows spawning even when the random spawn gate would otherwise skip. It must not force the selected type to `Heavy`.

- [ ] **Step 5: Build check**

Run:

```powershell
cmake --build build
```

Expected: build succeeds and existing spawn behavior remains available.

## Task 3: Use Weighted Selection In WaveSystem

**Files:**
- Modify: `src/WaveSystem.h`
- Modify: `src/WaveSystem.cpp`

- [ ] **Step 1: Include new APIs**

In `src/WaveSystem.cpp`, include:

```cpp
#include "WeightedRandom.h"
```

`EnemySystem.h` already exposes `EnemySpawnType`.

- [ ] **Step 2: Replace helper declaration**

In `WaveSystem.h`, replace:

```cpp
float getEnemy2SpawnChance() const;
```

with:

```cpp
EnemySpawnType chooseEnemySpawnType(std::mt19937& gen) const;
```

Forward declaration is not enough for return-by-value enum unless the enum is declared before use. Include `EnemySystem.h` in `WaveSystem.h`, or move `EnemySpawnType` into a small shared header. Prefer including `EnemySystem.h` to avoid another file.

- [ ] **Step 3: Replace spawn call**

In `WaveSystem::update(...)`, replace:

```cpp
float enemy2Chance=getEnemy2SpawnChance();
enemySystem.generate(difficulty,gen,dis,windowW,enemy2Chance,true);
```

with:

```cpp
auto spawnType=chooseEnemySpawnType(gen);
enemySystem.generateByType(spawnType,difficulty,gen,dis,windowW,true);
```

- [ ] **Step 4: Implement weighted wave table**

Replace `getEnemy2SpawnChance()` with:

```cpp
EnemySpawnType WaveSystem::chooseEnemySpawnType(std::mt19937& gen) const
{
    float lightWeight=100.0f;
    float heavyWeight=0.0f;

    if(currentWave>=9)
    {
        lightWeight=50.0f;
        heavyWeight=50.0f;
    }
    else if(currentWave>=7)
    {
        lightWeight=60.0f;
        heavyWeight=40.0f;
    }
    else if(currentWave>=5)
    {
        lightWeight=70.0f;
        heavyWeight=30.0f;
    }
    else if(currentWave>=3)
    {
        lightWeight=85.0f;
        heavyWeight=15.0f;
    }

    std::vector<WeightedChoice<EnemySpawnType>> choices={
        {EnemySpawnType::Light,lightWeight},
        {EnemySpawnType::Heavy,heavyWeight},
        {EnemySpawnType::Elite,0.0f}
    };
    return chooseWeighted(choices,gen,EnemySpawnType::Light);
}
```

- [ ] **Step 5: Build check**

Run:

```powershell
cmake --build build
```

Expected: build succeeds.

## Task 4: Manual Verification

**Files:**
- No source changes unless defects are found.

- [ ] **Step 1: Run the game**

Run:

```powershell
.\SpaceFighter-Windows.exe
```

- [ ] **Step 2: Verify early waves**

Expected:

- Waves 1-2 spawn only light enemies.
- Gameplay starts normally.

- [ ] **Step 3: Verify later waves**

Expected:

- Heavy enemies begin appearing from wave 3 onward.
- Later waves show heavy enemies more often.
- No missing texture or unsupported enemy appears.

- [ ] **Step 4: Verify existing behavior**

Expected:

- Enemy speed and fire rate still scale with difficulty.
- Scoring works.
- Result screen still appears after death.
