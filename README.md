# Super Hydric Guy
This is my own platformer game that i coded in C, i made all the graphics, sounds and music myself (that's why the music sounds funny). i used GIMP, Piskel, Audacity, and LMMS for the media and i used the Raylib library for the game.

Here is the details about the files:
  umm, well actually this isnt the first version of the game. i actually started this project on 17/08/2026. i dont have any copies of the older versions, even though VS Code's timeline saved me when   i made errors, it doesnt keep that old files. but whatever, i will call this version 1. and here it is, the whole project folder dumped.

As you can see, the game is Mario-like.
I made an editor that edits level.c, and the game itself which loads level.c, you will understand how the editor works once you open it.

I also just dropped a lot of binaries which i will clean up later and put them to Releases.

The build commands are inside build.sh, make sure to install the Raylib library if you are going to compile it yourself.
`super-hydric-guy` is the game itself built for Linux x86 (i built it on Linux Mint GCC).
Same for `editor`.

## Game controls:
- A/D: walk
- Hold Left Shift while walking: run (with acceleration and friction slide)
- SPACE: jump, hit blocks containing the Jump Boost powerup
- S while in air: ground pound, can be cancelled with W while ground pounding (hit blocks containing the Jump Boost powerup)
- Right Click while in air: twirl
- SPACE while in air (with Jump Boost): double jump once (resets when grounded)

## CHANGELOG

### v1.1:
- **FINALLY FIXED AI COLLISION ETC.**
- added a Mini HG powerup that can be used by changing the powerup through the code (will make a powerup later)
- changed the powerup system so SUPER is unreachable without code edit, and getting hurt turns you to SMALL/NORMAL which has an incomplete sprite
- cleaned up the code a bit
- added some macros for convenience
- some other minor changes

### NEXT: AI editor
- i will make a separate app that will allow you to create and edit custom AIs and assign them to any sprite,
- i will also add tabs to switch between the tileset and sprite placement menu inside `editor`
- this will take long
