Update Mar 2012
Released under MIT license

Inspired by the Infinity MMO http://www.infinity-universe.com and http://www.gamedev.net/blog/73-journal-of-ysaneya/
At the time of writing this game is still yet to be released.

This uses a 'spherized cube' where the twelve faces are subdivided dynamically in realtime, then stitched together.
The original concept was by 'HexiDave' with code demonstrated only on a single face of a cube http://www.ogre3d.org/phpBB2/viewtopic.php?t=30819
Permission was sought from HexiDave to publish this derived work back in 2008.
Others have since taken the core concept & run with it http://www.ogre3d.org/forums/viewtopic.php?t=49849

This project has been abandoned due to problems with texture seams.  
Texture media needs to be cube maps - arguably the best way to do this is with procedurally generated textures (eg. Perlin noise).
Popping is also a big issue - this can be limited by generating two levels of detail & alpha blending between them.


LICENSE:
OgrePlanet dynamic level of detail for planetary rendering
Copyright (C) 2008 Beau Hardy 
http://www.gamepsychogony.co.nz
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.



-------------------------------------------------------------------------------
Beau Hardy June 2008
This work is based on the concepts described by 'HexiDave' 'Project OgrePlanet - Slowly coming back to life' http://www.ogre3d.org/phpBB2/viewtopic.php?t=30819

OVERVIEW
Revisiting the QuadOgre project with a redesign from the ground up.
Instead of brute force precalculation of all vertex positions and patch types a more elegant approach is used as described by another individual at http://www.ogre3d.org/phpBB2/viewtopic.php?t=30819
A quad tree is built for each face of a cube during runtime and then 'spherized' (http://mathproofs.blogspot.com/2005/07/mapping-cube-to-sphere.html).
While the startup processing time is significantly reduced, this application is still CPU bound on my CPU - quad tree restructuring and frustum culling is performed on the CPU in real time.
Frustum and occulsion culling have been implemented to reduce the polygon counts to managable levels.

There is still one issue with texturing - all textures must be 'cubic'. Standard 'tileable' textures do not tile to the face of a cube.
Given a single tileable 2D texture http://en.wikibooks.org/wiki/Blender_3D:_Noob_to_Pro/Build_a_skybox has a method which can produce a texture for each cube face.
Basically you texture the inside of a sphere and environment map this on to the faces of a cube. 

Procedural generation of the Planet from random vectors is still quite slow.  More traditional height map solutions will provide quicker startup performance.


EXECUTING
In the highly unlikely event that executing this program should in any way damage your computing systems or data, I take no responsibility to the full extent of applicable regional and internation laws.
You will require a modern CPU and graphics card.  Development and testing was performed on an Intel 925D (3Ghz Dual core) processor with an ATI radeon graphics card 9550/X1050 128Mb dedicated RAM.
Prebuilt executables for release and debug versions can be found in the bin/Release and bin/Debug directories respectively.

USAGE
The planet is rotated with the mouse and/or 'up', 'down', 'left', 'right', 'pgup' and 'pgdown'.
The camera is moved with 'W', 'S', 'A' and 'D'.
The 'R' key toggles the render mode (solid, wireframe, point).
The 'printscreen' key can be used to take screenshots.
Camera details can be displayed with the 'P' key.
The 'numpad0' key toggles a freeze on the level of detail changes (shows what is going on for debugging).
'ESC' or 'Q' quit the program (this will only work after the planet has been built).



BUILDING
Requirements:
Visual C++ 2008         (Developed under free "Express Edition 9.0.21022.8 RTM")
Ogre 1.4.7 Source       (http://www.ogre3d.org/)
Ogre 1.4.7 Dependencies (http://www.ogre3d.org/)
boost 1.35.0            (http://www.boost.org/)

Extract Ogre to a known directory eg. c:/ogre-w32-v1-4-7
There should be a single directory named ogrenew/ under this directory containing the Ogre source

Extract the Ogre dependencies to a directory named Dependencies under ogrenew directory in the Ogre directory eg c:/ogre-win32-v1-4-7/ogrenew/Dependencies
There should be two directories include/ and lib/ directly underneath this directory

Extract boost to a known directory eg. c:/boost_1_35_0
Build all boost libraries (release/debug, single/multithreaded) to stage/lib using bjam (refer to boost documentation for details)


Set up enviroment variables (Right click my computer, properties, Advanced, Environment variables)
BOOST_HOME place where you extracted boost eg. c:/boost_1_35_0
OGRE_HOME place where you extracted Ogre eg. c:/ogre-win32-v1-4-7
OGRE_SRC place where you extracted Ogre eg. c:/ogre-win32-v1-4-7
Restart the Visual C++ IDE if it is open so that environment variables take effect.


Build Ogre Debug and Release targets (refer to Ogre documentation)
You need to change the data type used by Ogre from 32 bit float to 64 bit double before building.
This enables you to use planets with a larger radius without encountering rounding issues, however there is a performance trade off.
$(OGRE_SRC)/ogrenew/OgreMain/include/OgreConfig.h '#define OGRE_DOUBLE_PRECISION 0' -> '#define OGRE_DOUBLE_PRECISION 1'

Finally copy the generated Ogre .dll from $(OGRE_SRC)/ogrenew/lib to the place where you extracted this file under bin/
namexxx_d.dll are the debug versions of Ogre libraries and namexxx.dll are the release versions
eg. If you extracted to c:/OgreSolution
copy $(OGRE_SRC)/ogrenew/lib/*_d.dll to c:/OgreSolution/bin/Debug
copy $(OGRE_SRC)/ogrenew/lib/*(no _d).dll  to c:/OgreSolution/bin/Release


CODE NOTES
Search for 'XXX' and/or 'TODO' to highlight issues
The OgrePlanet project has a post build command to package .obj files to a .lib for OgrePlanetTest
The OgrePlanetTest project automatically executes via a post build command, set OgrePlanetTest as the 'startup project' (right click project in IDE) and press play to debug failed tests.


KNOWN ISSUES
Serialisation has not been implemented (different planet every time).
There are some texture seams due to the texture media being used.

