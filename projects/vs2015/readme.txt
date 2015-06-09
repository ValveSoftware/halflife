Directory Contents
==================

projects.sln
  Solution file for Visual C++ 2010 Express, containing the main projects for
  compiling the valve, dmc and ricochet DLL files.
  
utils.sln
  Solution file containing projects for the utilities.
  Special user interaction is required in order to compile a share of the
  projects, see "Installing GLUT and GLAUX" bellow.
  Special user interaction is also required for the smdlexp project, see
  "smdlexp project" bellow.
  The serverctrl project is problematic, see "serverctrl project" bellow.
  Further project specific notes are also provided bellow.
  
[other required files]



Installing GLUT and GLAUX
=========================

Some projects in utils.sln use the GLUT (mdlviwer) and the GLAUX (qbsp2, qcsg)
libraries, which are not shipped with Visual C++ 2010 Express / Windows SDK
v7.0A.

Thus you need to install the GLUT and GLAUX libraries manually:


Installing GLUT library
-----------------------

Required by: mdlviewer

There are several ways to do this, an example can be found here:
http://stackoverflow.com/a/10467488

For alternate implementations check the web (i.e. freeglut).


Installing GLAUX library:
-------------------------

Required by: qbsp2, qcsg

Obtaining the library:
http://stackoverflow.com/a/6211119

Copy glaux.h into 'C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\GL\'
(You might need to create the GL directory.).
Copy glaux.lib into 'C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\lib\'.


TODO
----

In the future the projects using GLUT and GLAUX could be ported to use the SDL2
library shipped with the SDK, this should be fairly easy, however this might
be beyond the scope of the main repository.



Other
=====

Please note, that some of the utils have additional documentation (.doc / .txt)
in the appropriate sub-folders of the utils folder.



qrad project
============

Please not that qrad.exe requires additional files lights.rad and valve.rad
side-by-side. You can find those files in the Half-Life SDK in the Hammer
tools folder.



serverctrl project
==================

The serverctrl project is problematic for the following reasons:
- It doesn't seem to work anymore, I tried with a recent HLDS beta installation
  from steamcmd (make sure to set the hardcoded path in ServerCtrlDlg.cpp line
  477, i.e. to "." (which would mean current directory) or s.th. you like).
  While the hlds.exe seems to know about the -HFILE -HCHILD and -HPARENT
  command line arguments used by serverctrl, it doesn't seem to handle the
  events properly anymore.
- It requires MFC to be installed and needs to be configured in order to find
  the MFC installation (see "Installing MFC" bellow).
  
A further note:
The serverctrl.vcxproj differs from the original .dsp project a bit:
It is compiled against dynamic libraries (MFC and Windows Runtime), meaning
/MD or /MDd instead of /MT or /MTd now. The reason is that the MFC from the
Windows Driver Kit won't compile in static mode (missing resource files i.e.).
This means you'll have to redistribute the runtime and MFC DLLs along with
serverctrl in case you install it on another system.


Installing MFC
--------------

Required by: serverctrl

If you are not using an Express edition, you can most likely skip this step.

The MFC is also shipped as part of the Windows Driver Kit for Windows XP.

Download Windows Driver Kit Version 7.1.0:
http://www.microsoft.com/en-us/download/details.aspx?id=11800

Burn it to a CD and start KitSetup.exe.
It's enough to select "Build Environment" in the options you want to install.

Now we need to point Visual C++ 2010 Express to the folders for the MFC/ATL
includes and libraries:

To do this open utils.sln and right click the serverctrl project and select
Properties. Select Configuration: All Configurations, then select
Configuration -> VC++ Directories in the tree. Adjust the Include and
Library Directories settings to match your WDK installation (click on the lines
and then click on the drop-down selector that appears and select edit).



smdlexp project
===============

Please note that this project requires the 3D Studio Max 4.2 SDK.
You might need to adjust Include and Additional Library Directories according
to your intallation in C++ and Linker settings.

The MAX 4.2 SDK needs adjustment:
Comment out the following line in max.h
#include <ctl3d.h>
So that it reads
//#include <ctl3d.h
.

You also need the phyexp.h from Character Studio, which you should place in
c:\3dsmax42\cstudio\sdk or adjust the Include Directories accordingly.

Someone should port that project to a newer MAX SDK Version, but that is
really beyond the current scope.