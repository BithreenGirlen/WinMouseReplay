# WinMouseReplay
Mouse left-click and drag replay using Win32API

## Demonstration

https://github.com/BithreenGirlen/WinMouseReplay/assets/152838289/f0694323-055c-43a6-b8a8-3e35c5082fe6

## Button functions

![000](https://github.com/BithreenGirlen/WinMouseReplay/assets/152838289/4844bbfe-a876-4783-b576-c8184f199bdf)

| Button  | Action  |
| --- | --- |
| Record | Starts recording. |
| Save | Opens file-save-dialogue to write records. |
| Replay | Opens dialogue to select a record file to start replaying.  |
| Clear | Clears the records in memory and messages on the window.|

## Key functions

| Input  | Action  |
| --- | --- |
| Insert | Records mouse position and the time passed since the last record. |
| Delete | Ends recording. |
| Shift + Delete | Ends replaying. |

To receive `Shift` + `Delete` input with the window unfocussed, the application registers hot-key. If this registration conficts with other applications, a message box appears when launching.

![002](https://github.com/BithreenGirlen/WinMouseReplay/assets/152838289/5a2ca20d-2ff4-49c9-bd49-1e1a0860d54d)

## Record file format

The record file is composed of mouse position(`X` and `Y`), elapsed time(`D`), and input type(`E`).
<pre>
X:1185, Y:557, D:756, E:0;
X:960, Y:546, D:943, E:0;
X:994, Y:565, D:100, E:1;
X:998, Y:656, D:100, E:1;
X:998, Y:656, D:0, E:0;
X:949, Y:656, D:100, E:0;
X:983, Y:614, D:100, E:0;
X:942, Y:562, D:879, E:0;
</pre>

| Input type  | Meaning  |
| --- | --- |
| 0 | Left-click. |
| 1 | Left being pressed. |

Here `D:0` means setting minimum interval timer.  
So the following records mean dragging start at the position of (994, 565), then release at (998, 656).
<pre>
X:994, Y:565, D:100, E:1;
X:998, Y:656, D:100, E:1;
X:998, Y:656, D:0, E:0;
</pre>

Unfortunately, the application always creates record file with input type 0. So dragging can be achived only by manual modification of the record file.
