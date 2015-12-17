# Maya-Soft-Body-Simulation
This project is for the course TNCG13 - "SFX - Tricks of the Trade", at Linköping University. The project is a soft body simulation plugin for Maya, and a rendering using imaged-based lighting.

# Folders
* Fina Bilder: This folder contains some images
* ImageBasedLighting: This folder contains the image used for the image based lighting
* SoftBodySimulationPlugin: This folder contains the plug-in and its source files.


# Maya Soft Body Deformer Plugin
A Maya plugin that generates a soft body simulation deformer node.

Based on the following tutorials:
* http://www.chadvernon.com/blog/resources/maya-api-programming/your-first-plug-in/
* http://www.chadvernon.com/blog/maya/compiling-maya-plug-ins-with-cmake/
* http://www.ngreen.org/2014/08/implementing-a-simple-maya-python-deformer/

# Build and run
The cmakelists are written so that it should be possible to run this project on Linux, Windows and OSX. (However, it has only been tested on OSX.) The default Maya version is set to 2016, and can be changed in FindMaya.cmake

OSX:

  Build:
    * Use CMAKE and store the binaries in a folder called build. "mkdir build"

  Compile:
    * in "/build" folder: "make"

 The plugin is created as "sampleplugin.bundle"
 In Maya, load the plugin using the plug-in manager.
 Run the plugin using any of the MEL scene scripts in the /scripts folder, using the script editor.
