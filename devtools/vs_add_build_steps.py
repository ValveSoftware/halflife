import os
import sys

skip_these_projects = ["REGEN", "RUN_INSTALL", "RUN_TESTS"]
valid_configs = ["all", "debugoptimized", "release"]

def add_post_build_to_all(directory_path):
    print(f"Adding post build for all vcxproj in {directory_path}")
    
    # Recursively find all .vcxproj files under the specified directory
    for foldername, subfolders, filenames in os.walk(directory_path):
        for filename in filenames:
            if filename.endswith('.vcxproj'):
                process = True
            
                for skip in skip_these_projects:
                    if skip in filename:               
                        process = False
                        break

                if process:
                    file_path = os.path.join(foldername, filename)
                    add_post_build_to_vcxproj(file_path)


def add_post_build_to_vcxproj(file_path):

    # this script assumes the meson build process output a file named OUTDIR which has the info we need to generate the
    #  post_build_command below.
    file_dir = os.path.dirname(file_path)
    outdir_file_path = os.path.join(file_dir, "OUTDIR")
    vcxproj = os.path.basename(file_path)
    
    if not os.path.exists(file_dir) or not os.path.exists(outdir_file_path):
        print(f"Couldn't find OUTDIR file next to {file_path}")
        return

    projectoutput = ""
    dll_name = ""
    
    with open(outdir_file_path, 'r') as file:
        line = file.readline().strip()
        parts = line.split('=')
        dll_name = parts[0]
        projectoutput = parts[1]
        
    projectoutput = projectoutput.strip('/').strip('\\').replace('/', '\\')    

    print(f"Adding post build for '{file_path}', dll_name='{dll_name}', projectoutput='{projectoutput}'")

    # Read the file contents
    with open(file_path, 'r', encoding='utf-8') as f:
        contents = f.read()

    # Check if the post-build step already exists to avoid adding it multiple times
    if 'filecopy.bat' in contents:
        print(f"Post build step already exists in {file_path}. Skipping.")
        return

    relative_dir = "..\\..\\..\\"
    
    #the expected root dir is 3 back from where the vcxproj exists, but some projects are further nested.
    # THIS IS NOT BULLET PROOF and if you're getting vstudio build errors during the filecopy.bat step it's probably here
    #  that caused the bug.
    expected_root_dir = os.path.join(file_dir, "../../../game/")
    if not os.path.exists(expected_root_dir):
        relative_dir = "..\\..\\..\\..\\"

    command = '''call $(SolutionDir)..\\filecopy.bat $(TargetDir)$(TargetName).pdb $(ProjectDir)%RELATIVE_DIR%\\game\\%PROJECT_OUTPUT%\\$(TargetName).pdb
    call $(SolutionDir)..\\filecopy.bat $(TargetPath) $(ProjectDir)%RELATIVE_DIR%\\game\\%PROJECT_OUTPUT%\\$(TargetFileName)'''

    #################################################################################
    # DUMB EXCEPTIONS !!!!!!!!!!!!!!!!!
    #
    #  Some of the DLLs get copied to other mods or utilities after they are built.
    #

    if( vcxproj == 'sw.vcxproj' ):
        command += '''
    call $(SolutionDir)..\\filecopy.bat $(TargetPath) $(ProjectDir)%RELATIVE_DIR%\\game\\%PROJECT_OUTPUT%\\swds.dll'''
        print("The software engine gets special handling.")

    if( 'cstrike' in file_dir ):
        if( vcxproj == 'client.vcxproj' ):
            command += '''
    call $(SolutionDir)..\\filecopy.bat $(TargetDir)$(TargetName).pdb $(ProjectDir)%RELATIVE_DIR%\\game\\czero\\cl_dlls\$(TargetName).pdb
    call $(SolutionDir)..\\filecopy.bat $(TargetPath) $(ProjectDir)%RELATIVE_DIR%\\game\\czero\\cl_dlls\$(TargetFileName)'''
            print("cstrike client DLL also going to czero/client.dll")
        elif( vcxproj == 'mp.vcxproj' ):
            command += '''
    call $(SolutionDir)..\\filecopy.bat $(TargetDir)$(TargetName).pdb $(ProjectDir)%RELATIVE_DIR%\\game\\czero\\dlls\\$(TargetName).pdb
    call $(SolutionDir)..\\filecopy.bat $(TargetPath) $(ProjectDir)%RELATIVE_DIR%\\game\\czero\\dlls\\$(TargetFileName)'''
            print("cstrike mp.dll also going to czero/mp.dll")

    # Define the post build command as text
    post_build_command = f'''  <PostBuildEvent>
    <Command>{command}</Command>
        </PostBuildEvent>
    '''
    
    post_build_command = post_build_command.replace("%PROJECT_OUTPUT%", projectoutput)
    post_build_command = post_build_command.replace("%RELATIVE_DIR%", relative_dir)

    # Locate the position to insert the post build command after the <Link> node within <ItemDefinitionGroup>
    insert_after = '</Link>'
    insert_position = contents.rfind(insert_after)
    if insert_position == -1:
        print(f"Couldn't find the {insert_after} tag in {file_path}!")
        return

    # Insert the post build command into the contents
    contents = contents[:insert_position + len(insert_after)] + post_build_command + contents[insert_position + len(insert_after):]

    # Write the updated contents back to the file
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(contents)




def usage():
    print("")
    print(" Usage: TODO")

if __name__ == "__main__":

    print_usage = False
    
    config = "all"
    projectpath = ""
    
    if len(sys.argv) > 1:
        config = sys.argv[1]
        
    if len(sys.argv) > 1 and not config in valid_configs:
        print_usage = True

    if print_usage:
        usage()
        sys.exit(1)

    if config == "all":
        add_post_build_to_all("build-debugoptimized-sln")
        add_post_build_to_all("build-release-sln")
    else:
        add_post_build_to_all(f"build-{config}-sln")    
        
    print("------------------------------------------------------------------")
    print("done updating vcxprojs.")