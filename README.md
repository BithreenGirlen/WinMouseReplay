# WinMouseReplay
Simple left mouse button replay using Win32API.

 This is useful when
- Just left-click is enough, no need to record/replay other mouse or keyboard inputs.
- You want to know the position to be clicked in replaying, but avoid actual clicking in recording.

## Demonstration

https://github.com/BithreenGirlen/WinMouseReplay/assets/152838289/f0694323-055c-43a6-b8a8-3e35c5082fe6

## How to use

![000](https://github.com/BithreenGirlen/WinMouseReplay/assets/152838289/4844bbfe-a876-4783-b576-c8184f199bdf)

### Record

1. `Record` button to start recording.
2. `Insert` key to record the position to be clicked; `R-Shift` + `Insert` key to record the position to start/end dragging.
3. `Delete` key to stop recording.
4. `Save` button to write records into a file through file-select-dialogue.

### Replay

1. `Replay` button to select a record file to replay through file-select-dialogue.
2. `Shift` + `Delete` key to stop replaying.

To receive `Shift` + `Delete` input with the window unfocussed, the application registers hot-key. If this registration conficts with other applications, a message box appears.


## Record file format

The record file is composed of mouse position(`X` and `Y`), elapsed time(`D`) since the previous record, and input type(`E`).
<pre>
X:1185, Y:557, D:756, E:0;
X:960, Y:546, D:943, E:0;
X:994, Y:565, D:100, E:1;
X:998, Y:656, D:100, E:2;
X:949, Y:656, D:100, E:0;
X:983, Y:614, D:100, E:0;
X:942, Y:562, D:879, E:0;
</pre>

| E value | Meaning  |
| --- | --- |
| 0 | Left click. |
| 1 | Left drag start. |
| 2 | Left drag end. |
