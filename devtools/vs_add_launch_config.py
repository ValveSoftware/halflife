import os
import sys

def usage():
    print("\nUsage: vs_add_launch_config.py <visual_studio_project (e.g. 'cl_dll/client.vcxproj')> <executable (e.g. 'hl')> optional(debug|release)\n")
    sys.exit(1)

# Check if the right number of arguments are provided
if len(sys.argv) < 3:
    usage()

contents = '''<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="Current" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='%BUILD_CONFIG%|Win32'">
    <LocalDebuggerCommand>$(ProjectDir)..\..\..\game\%EXECUTABLE%.exe</LocalDebuggerCommand>
    <LocalDebuggerCommandArguments>+developer 2 -dev</LocalDebuggerCommandArguments>
    <LocalDebuggerWorkingDirectory>$(ProjectDir)..\..\..\game</LocalDebuggerWorkingDirectory>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
</Project>'''

exe = sys.argv[2]

config = "debugoptimized"
if len(sys.argv) > 3:
    config_arg = sys.argv[3]
    
    if config_arg != "debug":
        config = config_arg
        
if config != "debugoptimized" and config != "release":
    print(f"\nInvalid build config specified: {config}")
    usage()

contents = contents.replace('%BUILD_CONFIG%', config)
contents = contents.replace('%EXECUTABLE%', exe)

proj = sys.argv[1]

# project comes from command-line argument
vs_project_path = (f"build-{config}-sln/{proj}")
target_filename = vs_project_path + ".user"

# Write text to the file
with open(target_filename, 'w') as file:
    file.write(contents)