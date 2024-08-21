Half Life 1 SDK LICENSE
======================

Half Life 1 SDK Copyright © Valve Corp.  

THIS DOCUMENT DESCRIBES A CONTRACT BETWEEN YOU AND VALVE CORPORATION (“Valve”).  PLEASE READ IT BEFORE DOWNLOADING OR USING THE HALF LIFE 1 SDK (“SDK”). BY DOWNLOADING AND/OR USING THE SOURCE ENGINE SDK YOU ACCEPT THIS LICENSE. IF YOU DO NOT AGREE TO THE TERMS OF THIS LICENSE PLEASE DON’T DOWNLOAD OR USE THE SDK.

You may, free of charge, download and use the SDK to develop a modified Valve game running on the Half-Life engine.  You may distribute your modified Valve game in source and object code form, but only for free. Terms of use for Valve games are found in the Steam Subscriber Agreement located here: http://store.steampowered.com/subscriber_agreement/ 

You may copy, modify, and distribute the SDK and any modifications you make to the SDK in source and object code form, but only for free.  Any distribution of this SDK must include this license.txt and third_party_licenses.txt.  
 
Any distribution of the SDK or a substantial portion of the SDK must include the above copyright notice and the following: 

DISCLAIMER OF WARRANTIES.  THE SOURCE SDK AND ANY OTHER MATERIAL DOWNLOADED BY LICENSEE IS PROVIDED “AS IS”.  VALVE AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES WITH RESPECT TO THE SDK, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, TITLE AND FITNESS FOR A PARTICULAR PURPOSE.  

LIMITATION OF LIABILITY.  IN NO EVENT SHALL VALVE OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THE ENGINE AND/OR THE SDK, EVEN IF VALVE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
 
 
If you would like to use the SDK for a commercial purpose, please contact Valve at sourceengine@valvesoftware.com.


Half-Life 1
======================

This is the README for the Half-Life 1 engine and its associated games.

Please use this repository to report bugs and feature requests for Half-Life 1 related products.

Reporting Issues
----------------

If you encounter an issue while using Half-Life 1 games, first search the [issue list](https://github.com/ValveSoftware/halflife/issues) to see if it has already been reported. Include closed issues in your search.

If it has not been reported, create a new issue with at least the following information:

- a short, descriptive title;
- a detailed description of the issue, including any output from the command line;
- steps for reproducing the issue;
- your system information.\*; and
- the `version` output from the in-game console.

Please place logs either in a code block (press `M` in your browser for a GFM cheat sheet) or a [gist](https://gist.github.com).

\* The preferred and easiest way to get this information is from Steam's Hardware Information viewer from the menu (`Help -> System Information`). Once your information appears: right-click within the dialog, choose `Select All`, right-click again, and then choose `Copy`. Paste this information into your report, preferably in a code block.

Conduct
-------


There are basic rules of conduct that should be followed at all times by everyone participating in the discussions.  While this is generally a relaxed environment, please remember the following:

- Do not insult, harass, or demean anyone.
- Do not intentionally multi-post an issue.
- Do not use ALL CAPS when creating an issue report.
- Do not repeatedly update an open issue remarking that the issue persists.

Remember: Just because the issue you reported was reported here does not mean that it is an issue with Half-Life.  As well, should your issue not be resolved immediately, it does not mean that a resolution is not being researched or tested.  Patience is always appreciated.


Building the SDK code
-------

Visual Studio 2019 is required to build mod DLLs on Windows. In the Visual Studio installer, install "Desktop development with C++" under "Workloads" and "C++ MFC for latest v142 build tools (x86 & x64)" under "Individual components". Python must also be installed to run scripts used as part of the build process (Python 3.9 has been tested).

To generate a Visual Studio solution with projects for each mod, run `create_vs_projects.bat` from a command line. This will create a solution file called `goldsrc.sln` in a folder called `build-debugoptimized-sln`. You can add `release` to the command line to generate a solution that builds the Release configuration instead, e.g. `create_vs_projects.bat release`. 

create_vs_projects uses [Meson](https://mesonbuild.com/) to generate the projects and solution. Each mod DLL must have a corresponding `meson.build` file that specifies the included files and dependencies.

By default, as a post-build step, built mod DLLs are copied to a folder called `game` one level up from the root SDK folder. This behavior can be customized by modifying `vs_add_build_steps.py` under `devtools`. 

Tools have not yet been updated for Meson / VS2019, but can be built using the VS2010 projects in the projects\vs2010 folder. See the `readme.txt` file there.

Linux binaries are built using Makefiles found in the `linux` folder, and are expected to be built / run in the Steam Runtime "scout" environment. You will need to set the environment variables VALVE_NO_AUTO_P4=1 and USE_STEAM_RUNTIME=1 while building.
