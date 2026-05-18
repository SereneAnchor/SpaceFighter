# Weighted Enemy Spawn Design

## Goal

Replace the current two-enemy probability rule with a small weighted-random spawn system that is easier to extend. The game still only spawns the two existing enemy types, but the design reserves a third enemy type for future content.

## Scope

This design includes:

- A reusable weighted-random helper.
- A spawn type enum with `Light`, `Heavy`, and reserved `Elite`.
- Wave-based enemy type weights.
- Integration with the existing `WaveSystem` and `EnemySystem`.

This design excludes:

- New enemy art.
- New enemy AI.
- Boss enemies.
- A full spawn director.
- Player-state-based weights.
- Dynamic difficulty changing enemy type weights.

## Existing Context

`WaveSystem` currently calls:

```cpp
enemySystem.generate(difficulty,gen,dis,windowW,enemy2Chance,true);
```

`enemy2Chance` is calculated by `WaveSystem::getEnemy2SpawnChance()` and increases by wave. This works for two enemy types, but it is a single probability value rather than a reusable weighted choice.

`EnemySystem` already owns the two enemy templates:

- `insect2Template`: current light enemy.
- `insect1Template`: current heavy enemy.

The new design should preserve existing behavior as much as possible while making the enemy type selection more explicit and extensible.

## Spawn Types

Add an enemy spawn type enum:

```cpp
enum class EnemySpawnType
{
    Light,
    Heavy,
    Elite
};
```

Current mapping:

- `Light` uses the current `insect2Template`.
- `Heavy` uses the current `insect1Template`.
- `Elite` is reserved and should not be selected yet.

`Elite` exists so future work can add a third enemy without redesigning the spawn algorithm.

## Weighted Random Helper

Add a small header-only helper:

- `src/WeightedRandom.h`

Responsibilities:

- Store values with positive or zero weights.
- Sum all positive weights.
- Return a selected value based on a random roll.
- Fall back to a caller-provided default when all weights are zero.

The helper should be generic enough for future reuse by power-ups or other systems, but it should stay small and dependency-light.

## Wave-Based Weights

Enemy type weights are based only on wave number. Dynamic difficulty does not affect enemy type weights.

Weights:

| Wave | Light | Heavy | Elite |
|------|-------|-------|-------|
| 1-2  | 100   | 0     | 0     |
| 3-4  | 85    | 15    | 0     |
| 5-6  | 70    | 30    | 0     |
| 7-8  | 60    | 40    | 0     |
| 9+   | 50    | 50    | 0     |

This keeps early waves approachable and gradually increases the chance of heavy enemies.

## Integration

`WaveSystem` should:

- Replace `getEnemy2SpawnChance()` with a weighted spawn type selection helper.
- Build a small list of weighted choices for the current wave.
- Select an `EnemySpawnType`.
- Ask `EnemySystem` to spawn that type.

`EnemySystem` should:

- Add a public `generateByType(...)` method.
- Keep the existing `generate(...)` method if useful for compatibility.
- Use `Light` and `Heavy` to choose the correct template.
- Treat `Elite` as `Heavy` or ignore it safely only if called accidentally. The preferred behavior is to fall back to `Heavy` because `Elite` is not active yet.

## Risks

- If weights are tuned too aggressively, later waves can feel too dense with heavy enemies.
- Adding `Elite` as a reserved type must not cause missing-texture behavior.
- The weighted helper should not introduce undefined behavior when all weights are zero.
- Existing enemy speed, cooldown, scoring, and AI behavior must remain unchanged.

## Verification

Manual verification:

- Build succeeds.
- Early waves mostly spawn light enemies.
- Heavy enemies begin appearing from wave 3 onward.
- The game does not attempt to spawn an unsupported third enemy.
- Existing difficulty effects on enemy movement and fire rate still work.

