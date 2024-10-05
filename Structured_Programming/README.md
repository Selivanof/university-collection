**Note:** This project **does NOT** work correctly on the latest versions of Windows 10/11 due to major changes in the CMD/Terminal. Items are probably **NOT** going to be desplayed correctly unless run with an older version of the Windows CMD.

<div align="center">
<h3 align="center">Structured Programming</h3>

  <a href=""> ![Static Badge](https://img.shields.io/badge/Language-C-6866fb)</a>
  <a href=""> ![Static Badge](https://img.shields.io/badge/Semester-1-green)</a>
</div>


|   Course Information     |                           |
|--------------------------|---------------------------|
| Semester                 | 1                         |
| Attendance Year          | 2020-2021                 |
| Project Type             | Optional                  |
| Team Project             | No                        |
| Languages Used           | C                         |
| Software Used            | Visual Studio Code        |

## Description

This is a local multiplayer game for 1-4 players for the purposes of the lesson "Structured Programming".
It uses the windows console to output the game to the player.
Players can choose between 3 different ships (each with a unique advantage) and 3 difficulty levels (that affect the enviroment).
Winds can influence the ship's movement, making it harder to reach the goal.
**Note that the code comments are in Greek.**

## Notes
The project received a +1.5 bonus for being among the top 3 submissions of 2020-2021 (excact position unknown).

## Specifications for the game (as given by the lesson's instructor)

### Game type
Round based game with one or more players; in each round the players define their move; the moves are executed for all players at the same time.
### Playground
A large lake with a large island; it has wide and narrow passages between the shores.
### Game
The direction and the intensity of the wind changes on each round. Variable and strong wind means increased difficulty.
Each player determines the direction and range of his/her movement.
The program calculates with specific rules and with a some randomness (which can be changed depending on the desired difficulty) the cruising and the next point that each boat reaches.
Optionally, each player's boats can collide with each other.
### Winner
Whoever reaches the finish line first, completing a full cruising around the island.