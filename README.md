# ChronoDS

Website link for progress reports: https://strayacode.github.io/

## Installation:
### Linux:
Dependencies:
cmake 
SDL2 (if building for the SDL2 frontend)
Qt (if building for the Qt frontend)
1. `git clone https://github.com/strayacode/ChronoDS.git`
2. `cd ChronoDS`
3. make a build directory where the binary will be built and a roms directory to place your roms dumped from your DS
4. create a bios directory and firmware directory as you will need to provide a copy of the ARM9 BIOS, ARM7 BIOS and firmware. the files should be named bios9.bin, bios7.bin and firmware.bin
5. navigate into the build directory
6. run `cmake ..` and `cmake --build .` to build the binary
7. If using the SDL2 frontend run `./ChronoDS <path-to-rom>` otherwise if for the Qt Frontend you can run normally and select a rom to run using `File -> Load ROM...`

A star is always appreciated as it will further motivate me to push development further!


