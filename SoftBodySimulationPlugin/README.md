# Maya_Soft_Body_Deformer_Plugin
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
