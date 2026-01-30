# Heatseeker Goal Blocker

A BakkesMod plugin that allows you to block goals in Heatseeker mode, perfect for practicing redirects and maintaining rallies. Basically Heatseeker freeplay.

## Known Issues & Plas

- The main menu button when enabled overlays on top of the garage menu etc - will see if I can fix this in the future.. If it bothers you just turn off the main menu button in the plugin settings.
- I will style the main menu button eventually so it looks nicer :)

## Features

- Block either goal independently
- Enable/disable scoring on unblocked goals
- One-click launch for Heatseeker practice in settings menu and on main menu (LAN match, unlimited time/score (100 goal max as RL doesn't like me trying to do unlimited))
- Only Works in LAN matches or multiplayer using Rocket Plugin
- Heatseeker mode only
- Toggle goals on/off during gameplay
- Safe to use - won't affect online matches

## Installation

1) Install from Bakkesmod plugins site (recommended) - https://bakkesplugins.com/plugins/view/494

or

2) Download manually from Github releases and place in your plugins folder

## Usage

1. Click "Start Heatseeker Freeplay" on the main menu (top left, above Play)
   - OR Open BakkesMod (F2) -> Plugins -> Heatseeker Goal Blocker -> Launch Heatseeker Practice
2. Toggle which goal(s) you want to block
3. Optionally disable "Enable Scoring" to  prevent goals if a goal is unblocked for whatever reason

## Multiplayer Usage

For LAN matches with multiple players:
- Only the host needs to have rocket plugin installed
- All players in the match will see the goal blocking effect
- The host can toggle goals on/off during gameplay
- Other players don't need to install anything

## Source Code

Available on [GitHub](https://github.com/ncjsvr/HeatseekerGoalBlocker)

## Credits

Thanks to SoulDaMeep for help with the bCanBeScored memory flag.
Thanks to Acc70 for help with optimizations and bug fixes.

## Support

If you encounter any issues or have suggestions, please open an issue on GitHub or join Heatseeker Heaven Discord Server & ping ncj.

## Version History

### 1.9 Release
- Added "Start Heatseeker Freeplay" button to main menu and settings menu for instant heatseeker freeplay 
- Added "Enable Scoring" toggle using bCanBeScored memory flag (thanks to SoulDaMeep for the help!)
- Added one-click "Launch Heatseeker Practice" button (LAN match with no bots, unlimited time, 100 goal max)
- Plugin now LAN/Rocket Plugin MP only (removed exhibition mode support for stability and bug fixes - doesn't really make any difference)
- Fixed crash when toggling goal blocks during a match
- Goal blocker settings now persist between sessions

### 1.8 Release
Now works in multiplayer LAN using Rocket Plugin :)

### 1.7 Release
60fps bug now fixed :)

### 1.6 Release
Ability to now block both goals independently.
Includes bug fixes and optimizations from Acc70 (Thanks!).