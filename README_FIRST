/*
 *  Argila Engine 2.0
 *  Based on the Shadows of Isildur RPI Engine
 *
 *  Getting Started Walkthrough
 */

FOREWORD:

First things first, if you haven't already, you should read the DIKU_LICENSE and SOI_LICENSE files in this directory. This engine is released under both of these licenses, and adherence to them is required. If you do not agree with any of these license provisions, save yourself the trouble and look elsewhere for a codebase; if you breach any of these terms at any time while operating a MUD derived from this code, you will be potentially opening yourself up to a lawsuit for copyright infringement. In other words, please respect the licenses and the code's authors!


GENERAL NOTES:

This codebase is based on the December, 2004 version of the player port server, of Shadows Of Isildur. It contains all of the functionality, fixes, dervived form the orginal SOI code, as well as many more fixes, upgrades and enhancemnts by the Argila staff. It is moderately stable, only a few small memory leaks remaining, however, it has never been used in a full production setting. It includes all of the files needed to run Lua scripting as well. Instructions for installing Lua are below.

This codebase IS NOT supported by any Shadows of Isildur personnel, or Argila Project, however, you may find discussion and support from the forums at Shadows of Isildur. (http://www.middle-earth.us/forums/viewforum.php?f=31). Additional discussion may be found at http://argila.rpimud.net/forums/index.php


SYSTEM REQUIREMENTS:

In order to run the Argila Engine, you'll need the following:

- A computer running Linux, UNIX, Free BSD or OS X. The engine has been
  run successfully on Fedora Core Linux and OS X; it should work on others. 
- At least 200 megs of hard drive space to start. The more, the better!
- At least 48 megabytes of RAM, for small playerbases.
- A mysql database server installation of version 3.23 or higher.
- Access to -three- -separate- mysql databases on that installation.
- GCC, to compile and run the server's C-based sourcecode.


THE FIRST STEP:

If you're running a non-standard operating system, e.g. Free BSD, OS X, or some
other UNIX variant, you'll need to edit the makefile and uncomment the appropriate flag lines up at the top where it directs you to. 

INSTALLATION:

In the src/lua directory, choose a platform: aix ansi bsd generic linux macosx mingw posix solaris and type "make <platform>" This will install Lua in the src/lua directory. This includes all of the files needed to run the scripts, as well as a virtual Lua application that allows you to program in Lua alone. You may never need to bother with these files, but they are available if you do wish to work with a stand alone Lua interpeter.

Then go up to your src/ directory and type "make" to compile the code.

TO COMPLETE THE INSTALLATION:

Please see the README_SECOND file in the sql/ subdirectory.


