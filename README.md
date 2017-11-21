# Water Simulation

Project for FIT BUT 2017

#---------------------------------------
Build
#---------------------------------------

1. Download SDL2 (https://www.libsdl.org/download-2.0.php) and and glm (https://glm.g-truc.net/0.9.8/index.html)
2. Build and install it (Debug and Release).
3. Download GPUEngine: You can try official github repo: https://github.com/Rendering-FIT/GPUEngine
   If this produces error with "SLD::SLD2mainConfig.cmake" while building the final app, use this alternative repo: https://github.com/Rendering-FIT/GPUEngine/archive/dormonCmakeHell.zip
4. Build and install it (Debug and Release): Don't forget to select geGL, geAd and geUtils options in CMake!
5. Build app, if necessary, point CMake to install folders with appropriate Config.cmake files.