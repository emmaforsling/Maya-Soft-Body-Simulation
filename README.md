# Maya-Soft-Body-Simulation
This project is for the course TNCG13 - "SFX - Tricks of the Trade", at Linköping University. The project is a soft body simulation plugin for Maya, and a rendering using imaged-based lighting.

# Folders
* Fina Bilder: This folder contains some images
* ImageBasedLighting: Contains the image used for the image based lighting
* SoftBodySimulationPlugin: This folder contains the plug-in and its source files.


# Maya Soft Body Deformer Plugin
A Maya plugin that generates a soft body simulation deformer node.

Based on the following tutorials:
* http://www.chadvernon.com/blog/resources/maya-api-programming/your-first-plug-in/
* http://www.chadvernon.com/blog/maya/compiling-maya-plug-ins-with-cmake/
* http://www.ngreen.org/2014/08/implementing-a-simple-maya-python-deformer/


OSX:

  Build:
    * "mkdir build"
    * "cd build"
    * "cmake -G "Unix Makefiles" -DMAYA_VERSION=2016 ../" (or whatever maya version)

  Compile:
    * in "/build" folder: "make" OR "sudo cmake --build . --config Release --target install"

  The plugin will be created as "sampleplugin.bundle"
  In Maya, load the plugin using the plug-in manager.
  Run the plugin using any of the MEL scene scripts in the /scripts folder, using the script editor.
