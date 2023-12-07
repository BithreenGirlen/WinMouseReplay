# WinMouseReplay
Simple mouse-left-click record/replay using Win32API.

 This is useful when
- Just left click input is enough, no need to record/replay other mouse or keyboard inputs.
- You want to know the position to be clicked in replaying, though in recording you do not want to click.

## Feature

- Easy modification
  - The record file is simple text file, so you can easily modify it by manual typing.
- Loose restriction
  - You can do other tasks during replaying if the interval is not too short. 

## How to use

### Record

1. Click "Record" button on the window.
2. Press "insert" key leaving the mouse cursor on the position where to be clicked in replaying.
3. Press "delete" key when you want to stop recording.
4. Click "Save" button on the window, and then type file name.

The record file will be written like this: _1.txt_
<pre>
X:900, Y:398, D:2630;
X:1065, Y:674, D:1270;
X:1110, Y:666, D:5980;
X:931, Y:611, D:29703;
X:1071, Y:695, D:10;
X:1094, Y:609, D:624;
X:939, Y:568, D:424;
</pre>

`X` and `Y` are coordinates, and `D` is the time in milliseconds passed from the previous input.

The record feature can be used just to know the position of the mouse, for example, to modify the previously created record, or to know the area you want to avoid clicking.

### Replay

1. Click "Replay" button on the window.
2. Select the record file to replay.
3. Press "shift" + "delete" when you stop replaying.

## Remarks
Stopping replaying uses hot-key registration. If the registraion would conflict with other applications, a message box would appear.  
"Clear" button clears the reocrd in memory and messages on the window.
