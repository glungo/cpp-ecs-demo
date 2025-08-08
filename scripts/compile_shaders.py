# Copyright (C) 2025 by Sascha Willems - www.saschawillems.de
# This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

import argparse
import fileinput
import os
import subprocess
import sys
# Removed _rename import as we don't need it for our shader compilation

parser = argparse.ArgumentParser(description='Compile all slang shaders')
parser.add_argument('--slangc', type=str, help='path to slangc executable')
parser.add_argument('--sample', type=str, help='can be used to compile shaders for a single sample only')
args = parser.parse_args()

def findCompiler():
    def isExe(path):
        return os.path.isfile(path) and os.access(path, os.X_OK)

    # First try to use slangc if specified via argument
    if args.slangc != None and isExe(args.slangc):
        return args.slangc

    # Try to find slangc in Vulkan SDK
    if "VULKAN_SDK" in os.environ:
        vulkan_sdk_path = os.environ["VULKAN_SDK"]
        slangc_path = os.path.join(vulkan_sdk_path, "Bin", "slangc.exe")
        if isExe(slangc_path):
            return slangc_path

    # Try to find slangc in PATH
    exe_name = "slangc"
    if os.name == "nt":
        exe_name += ".exe"
    
    for exe_dir in os.environ["PATH"].split(os.pathsep):
        full_path = os.path.join(exe_dir, exe_name)
        if isExe(full_path):
            return full_path

    sys.exit("Could not find slangc executable on PATH, and was not specified with --slangc")

def getShaderStages(filename):
    stages = []
    with open(filename) as f:
        filecontent = f.read()
        if '[shader("vertex")]' in filecontent:
            stages.append("vertex")
        if '[shader("fragment")]' in filecontent:
            stages.append("fragment")
        if '[shader("raygeneration")]' in filecontent:
            stages.append("raygeneration")
        if '[shader("miss")]' in filecontent:
            stages.append("miss")
        if '[shader("closesthit")]' in filecontent:
            stages.append("closesthit")
        if '[shader("callable")]' in filecontent:
            stages.append("callable")
        if '[shader("intersection")]' in filecontent:
            stages.append("intersection")
        if '[shader("anyhit")]' in filecontent:
            stages.append("anyhit")
        if '[shader("compute")]' in filecontent:
            stages.append("compute")
        if '[shader("amplification")]' in filecontent:
            stages.append("amplification")
        if '[shader("mesh")]' in filecontent:
            stages.append("mesh")
        if '[shader("geometry")]' in filecontent:
            stages.append("geometry")
        if '[shader("hull")]' in filecontent:
            stages.append("hull")            
        if '[shader("domain")]' in filecontent:
            stages.append("domain")            
        f.close()
    return stages

compiler_path = findCompiler()

print("Found slangc compiler at %s" % compiler_path)

compile_single_sample = ""
if args.sample != None:
    compile_single_sample = args.sample
    if (not os.path.isdir(compile_single_sample)):
        print("ERROR: No directory found with name %s" % compile_single_sample)
        exit(-1)

# Get path to graphics/shaders relative to the script
dir_path = os.path.dirname(os.path.realpath(__file__))
dir_path = os.path.normpath(os.path.join(dir_path, '../engine/graphics/shaders'))
dir_path = dir_path.replace('\\', '/')
print("Looking for shaders in: %s" % dir_path)

for root, dirs, files in os.walk(dir_path):
    folder_name = os.path.basename(root)
    if (compile_single_sample != "" and folder_name != compile_single_sample):
        continue
    for file in files:
        if file.endswith(".slang"):
            input_file = os.path.join(root, file)
            # Slang can store multiple shader stages in a single file, we need to split into separate SPIR-V files for the sample framework
            stages = getShaderStages(input_file)
            print("Compiling %s" % input_file)
            output_base_file_name = input_file
            for stage in stages:
                entry_point = stage + "Main"
                output_ext = ""
                if stage == "vertex":
                    output_ext = ".vert"
                elif stage == "fragment":
                    output_ext = ".frag"
                elif stage == "raygeneration":
                    output_ext = ".rgen"
                elif stage == "miss":
                    output_ext = ".rmiss"
                elif stage == "closesthit":
                    output_ext = ".rchit"
                elif stage == "callable":
                    output_ext = ".rcall"
                elif stage == "intersection":
                    output_ext = ".rint"
                elif stage == "anyhit":
                    output_ext = ".rahit"
                elif stage == "compute":
                    output_ext = ".comp"
                elif stage == "mesh":
                    output_ext = ".mesh"
                elif stage == "amplification":
                    output_ext = ".task"
                elif stage == "geometry":
                    output_ext = ".geom"
                elif stage == "hull":
                    output_ext = ".tesc"
                elif stage == "domain":
                    output_ext = ".tese"
                output_file = output_base_file_name + output_ext + ".spv"
                output_file = output_file.replace(".slang", "")
                print(output_file)
                # Use slangc with appropriate flags for Slang files
                cmd = '"%s" "%s" -profile spirv_1_4 -matrix-layout-column-major -target spirv -o "%s" -entry %s -stage %s -warnings-disable 39001' % (compiler_path, input_file, output_file, entry_point, stage)
                
                print("Running: %s" % cmd)
                res = subprocess.call(cmd, shell=True)
                if res != 0:
                    print("Error %s", res)
                    sys.exit(res)
    # Removed checkRenameFiles call as it's not needed for our shader compilation