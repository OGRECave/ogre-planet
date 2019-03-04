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


## LICENSE:
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

## OVERVIEW
Revisiting the QuadOgre project with a redesign from the ground up.
Instead of brute force precalculation of all vertex positions and patch types a more elegant approach is used as described by another individual at http://www.ogre3d.org/phpBB2/viewtopic.php?t=30819
A quad tree is built for each face of a cube during runtime and then 'spherized' (http://mathproofs.blogspot.com/2005/07/mapping-cube-to-sphere.html).
While the startup processing time is significantly reduced, this application is still CPU bound on my CPU - quad tree restructuring and frustum culling is performed on the CPU in real time.
Frustum and occulsion culling have been implemented to reduce the polygon counts to managable levels.

There is still one issue with texturing - all textures must be 'cubic'. Standard 'tileable' textures do not tile to the face of a cube.
Given a single tileable 2D texture http://en.wikibooks.org/wiki/Blender_3D:_Noob_to_Pro/Build_a_skybox has a method which can produce a texture for each cube face.
Basically you texture the inside of a sphere and environment map this on to the faces of a cube. 

Procedural generation of the Planet from random vectors is still quite slow.  More traditional height map solutions will provide quicker startup performance.


## EXECUTING
In the highly unlikely event that executing this program should in any way damage your computing systems or data, I take no responsibility to the full extent of applicable regional and internation laws.
You will require a modern CPU and graphics card.  Development and testing was performed on an Intel 925D (3Ghz Dual core) processor with an ATI radeon graphics card 9550/X1050 128Mb dedicated RAM.
Prebuilt executables for release and debug versions can be found in the bin/Release and bin/Debug directories respectively.

## USAGE
The planet is rotated with the mouse and/or 'up', 'down', 'left', 'right', 'pgup' and 'pgdown'.  
The camera is moved with 'W', 'S', 'A' and 'D'.
The 'R' key toggles the render mode (solid, wireframe, point).
The 'printscreen' key can be used to take screenshots.
Camera details can be displayed with the 'P' key.
The 'numpad0' key toggles a freeze on the level of detail changes (shows what is going on for debugging).
'ESC' or 'Q' quit the program (this will only work after the planet has been built).

## CODE NOTES
Search for 'XXX' and/or 'TODO' to highlight issues
The OgrePlanet project has a post build command to package .obj files to a .lib for OgrePlanetTest
The OgrePlanetTest project automatically executes via a post build command, set OgrePlanetTest as the 'startup project' (right click project in IDE) and press play to debug failed tests.


## KNOWN ISSUES
Serialisation has not been implemented (different planet every time).
There are some texture seams due to the texture media being used.

