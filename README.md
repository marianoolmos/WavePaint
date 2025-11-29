<p align="left">
  <img src="/img/WavePaint.png" width="200" alt="logo">
</p>
Visual editor for digital waveforms in C++/Qt, intended as a kind of "WaveDrom by hand": you don't have to write code, you just draw with the mouse.

## Main features
- Cross-platform: Linux and Windows (Qt5/Qt6 + Qt Widgets).
- Discrete timeline (samples / `time steps`), configurable from the toolbar.

- Add signals via context menu (right-click on the waveform area):
  - **Bit** signal: typical digital waveform (high/low).
  ![Paint Bit Signal](/img/paintbit.gif)
  - **Vector** signal (bit vector): a bar with numeric value and optional label per segment.
  ![Paint Vector Signal](/img/paintvec.gif)
  - **Clock** signal: automatic clock generator (pulses, high time, low time).
- Add Arrows 
   ![Arrows](/img/arrow.gif)
- Add Markers
   ![Markers](/img/markers.gif) 
- Deleting:
  - Right-click on a cell of any signal ⇒ clears the value at that timestep (becomes undefined).
- Color per signal:
  - Double left-click on the **signal name** ⇒ dialog to choose the color for that signal.
  - Right-click on the name ⇒ menu with:
    - **Rename signal...**
    - **Change color...**
     ![color](/img/color.gif)
- Cut tool (**scissors**):
    ![Cut Tool](/img/cut.gif)
  - In the toolbar (below the menu) there's a button with the emoticon **✂**.
  - Clicking it enters **cut mode**:
    - Left-click on a time point ⇒ marks the start.
    - Left-click on another point ⇒ the document is cropped and **only the interval** between those two points remains (everything to the left and right is removed).
  - Useful, for example, after importing a long VCD to keep only the time window of interest.
- Import:
  - `File → Open...` can open:
    - Native `.wp` / `.json` format (JSON).
    - TODO : Standard **VCD** files (signals are parsed and the waveform is constructed).
- Persistence:
  - `File → Open...` / `Save As...` load and save in JSON (`.wp` / `.json`).
  - The following are saved:
    - Number of samples,
    - Signal type, width, color,
    - Values per sample,
    - Labels per sample (vectors).
- Export PNG:
  ![Export as PNG](/img/png.gif)
  - `File → Export PNG...`
  - Asks for a **white** or **black** background.
  - TODO : ADD Themes : ex . A calssic theme in bw.s
  - The image is fitted **exactly** to the waveform's width and height (including names, time axis, etc.).

## Requirements

- CMake >= 3.16
- Qt6 or Qt5 (Widgets module)
- C++17 compiler (gcc, clang, MSVC)

## Build

### Linux

```bash
mkdir build
cd build
cmake ..
cmake --build .
./WavePaint
```

### Windows

```bat
mkdir build
cd build
cmake -G "Ninja" ..
cmake --build .
WavePaint.exe
```

Or open `CMakeLists.txt` with **Qt Creator**, select a kit with Qt5/Qt6 and build from the IDE.

## Quick start

1. Run `WavePaint`.
2. Adjust the number of samples (waveform size) from the top bar.
3. Right-click on the drawing area to add:
   - bit / vector / clock.
4. Editing:
   - Bit:
     - Drag in the top half of the row ⇒ paint `1` (with gradient shading under the high level).
     - Drag in the bottom half ⇒ paint `0`.
   - Vector:
     - Drag to select a range ⇒ enter a value and optionally a label.
5. Right-click on a signal name to rename it or change its color.
6. Use the **✂** button to cut a time interval (choose two points).
7. `File → Save As...` to save as `.wp` / `.json`, `File → Open...` to restore the project open the `.wp` / `.json`.
8. `File → Export PNG...` to export the view fitted to the waveform size, with white or black background.


