# UI Flow System Design

## Goal

Make SpaceFighter feel more like a complete game project by improving the menu flow, pause flow, HUD presentation, and result/history presentation without adding a settings screen or expanding the project into a large UI framework.

## Confirmed Scope

This design includes:

- A single lightweight `GameUI` module.
- A unified main menu using selectable menu items.
- A selectable pause menu with shortcut keys preserved.
- A thinner `MainScene` HUD rendering path.
- A cleaner result screen drawn through the shared UI module.
- A history view inside `ResultScene`, toggled by key.

This design excludes:

- Settings screen.
- Independent `HistoryScene`.
- README synchronization.
- Mouse UI.
- General-purpose widget tree or layout engine.

## Existing Project Context

The project already has a scene-based architecture:

- `Game` owns the SDL lifecycle, main loop, and scene switching.
- `Scene` defines the lifecycle interface.
- `MenuScene` already provides difficulty selection and start input.
- `MainScene` already provides gameplay, pause state, HUD drawing, scoring, wave display, and game-over flow.
- `ResultScene` already shows result data, records best score information, and writes `result_history.txt`.

The new design should reuse this structure. Scene classes keep control over state and navigation. The new UI layer only draws and helps with small input selection state.

## UI Module

Add:

- `src/GameUI.h`
- `src/GameUI.cpp`

`GameUI` is a lightweight helper owned by scenes that need UI rendering. It should manage its own font pointer and expose drawing helpers. It should not own game state.

Responsibilities:

- Load and release a TTF font.
- Render left-aligned text.
- Render centered text.
- Measure text where needed for centering and right alignment.
- Draw translucent panels.
- Draw simple separators and borders.
- Draw a health bar.
- Draw a highlighted menu list.
- Draw result summary rows.
- Draw recent history rows parsed by `ResultScene`.

Non-responsibilities:

- Scene switching.
- Gameplay state mutation.
- Result file reading or writing.
- SDL event polling.
- Complex layout rules.
- Mouse hit testing.

This keeps the UI layer complete enough for this game without creating unnecessary header files or a full GUI framework.

## Main Menu

`MenuScene` should use `GameUI` for title text, hint text, and menu items.

Menu items:

- `Start Game`
- `Difficulty: Easy / Normal / Hard`

`Quit` is not part of this pass because `Game` does not currently expose a clean public quit method, and adding one would broaden the scope beyond the confirmed UI work.

Input:

- Up/down changes the selected menu item.
- Left/right changes difficulty when the difficulty item is selected.
- Enter starts the game when `Start Game` is selected.
- Space can continue to act as quick start.

Difficulty behavior remains the same:

- Easy sets `gDifficultyOffset` to `-0.30f`.
- Normal sets it to `0.0f`.
- Hard sets it to `0.40f`.

## Pause Menu

`MainScene` keeps `isPaused` and the actual pause behavior. The UI layer only renders the pause menu and highlights the selected item.

Pause menu labels:

- `Continue`
- `Restart`
- `Main Menu`

`Continue` is the displayed replacement for the English word `Resume`; it means leaving pause mode and continuing the current game.

Input:

- `P` toggles pause and continue.
- Up/down changes the selected pause menu item while paused.
- Enter confirms the selected item.
- `R` restarts while paused.
- `ESC` returns to the main menu.

Confirm behavior:

- `Continue` sets `isPaused` to false.
- `Restart` requests a new `MainScene`.
- `Main Menu` requests a new `MenuScene`.

When the player is dead, the existing game-over behavior stays separate from the pause menu.

## Gameplay HUD

Move the drawing details from `MainScene::renderGameInfo()` into `GameUI` helper methods. `MainScene` still owns the data and decides when to show each element.

HUD content:

- HP bar and numeric HP.
- Score.
- Dynamic difficulty percentage.
- Weapon level.
- Current wave.
- Wave start text.
- Wave complete text.
- Power-up pickup notice.
- Game-over text.

`MainScene` should pass primitive display values to `GameUI`, rather than allowing `GameUI` to inspect gameplay subsystems directly. This keeps gameplay logic in `MainScene`.

## Result Screen

`ResultScene` default view remains the current result summary. It should stay focused on the result values players care about most.

Existing result content remains:

- Score.
- Wave.
- Kills.
- Difficulty.
- Rank.
- Best score.
- Best wave.
- Best rank.
- New record prompt.

Controls:

- `H` toggles between result summary and history view.
- `R` restarts.
- `ESC` returns to main menu.

## History View

History stays inside `ResultScene`. No new `HistoryScene` is added.

`ResultScene` should parse `result_history.txt` and keep a small list of recent records for display. The parser should support the current pipe-separated table format produced by `saveResultHistory()`.

Display:

- Show recent records, newest last or newest first consistently.
- Limit the list to around 8 to 10 rows so it fits on the laptop window.
- Include compact columns: number, score, wave, kills, difficulty, rank.
- Show a short fallback message if no history file exists or no records can be parsed.

`GameUI` only draws the parsed rows. `ResultScene` owns file parsing and view toggling.

## CMake

`CMakeLists.txt` must include `src/GameUI.cpp` in the executable target. No other build-system changes are intended.

## Risks

- `GameUI` can become a dumping ground if unrelated responsibilities are added. Keep it focused on drawing and simple selection helpers.
- The current source comments appear to have encoding issues. The implementation should avoid broad rewrites and keep edits near the required code.
- Main menu input changes must preserve the existing difficulty offset behavior.
- History parsing must be compatible with the existing `result_history.txt` format.
- Adding a new `.cpp` file requires a CMake update.

## Verification

Manual verification should include:

- Build succeeds with the new `GameUI.cpp` included.
- Main menu can select start and difficulty using keyboard.
- Starting a game preserves Easy, Normal, and Hard difficulty behavior.
- `P` pauses and continues gameplay.
- Pause menu supports up/down selection and Enter confirmation.
- Pause shortcuts still work: `P`, `R`, and `ESC`.
- HUD still shows HP, score, difficulty, weapon level, wave, pickup notice, and game-over text.
- Result screen shows score, wave, kills, difficulty, rank, and best record.
- Result screen shows score, wave, kills, difficulty, rank, and best record.
- `H` toggles result/history view.
- Missing or empty `result_history.txt` does not crash the result screen.
