# RP2040Eye
Halloween Project to use two WavreShare RP2040 1.28 Displays to produce animating eyes for a skull or monster. Share on my YouTube channel [DrJoneA](https://youtube.com/@drjonea)

## Hardware
Each eye is a [Waveshare RP2040-LCD-1.28](https://www.waveshare.com/wiki/RP2040-LCD-1.28). 

The dominant eye broadcasts move commands using UART2 to the submissive eye.  To do this GPIO4 from the dominant module should be commented to GPIO5 on the sub module. Ground will need to be shared between the two modules as well.

## Build
I have used submodules so please clone with the git clone --recurse-submodules option.

Normal build process for the dominant eye:
```
mkdir build
cd build
cmake ..
make
```
Flash the module using bootsel

For the Submissive eye you will need to uncomment a line in the "src/CMakeLists.txt file"
```
    #SUB_EYE=1
```
This will tell the build it is a submissive eye.
     
