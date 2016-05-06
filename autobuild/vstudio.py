import os
import shutil
import sys
import subprocess

#
# The default location for a Unity install
# Override by changing 

def checkPlatform():
	if not sys.platform == "win32":
		raise Exception("Visual Studio scripts are only available on windows!")
	return


def getMSBuildPath(in_path):
	ret = in_path
	if (in_path is None):
		# assume 2013 for now
		ret = "C:\\Program Files (x86)\\MSBuild\\12.0\\bin\\MSBuild.exe"
			
	return ret


#
# Method runs msbuild for the given project and targets
#
# @param in_projectPath Path to the project
# @param in_targets Semi-colon delimited list of targets to build
# @param in_configuration The build configuration to use
# @param in_switches Optional list of switches to send to msbuild
# @param in_msbuildPath Optional path to msbuild, will use default otherwise
#
def buildProject(in_projectPath, in_targets, in_configuration, in_switches=None, in_msbuildPath=None):
	checkPlatform()	

	msbuildPath = getMSBuildPath(in_msbuildPath)

	print()
	print("Building Visual Studio Project")
	print("  Project path: " + in_projectPath)
	print("  Targets: " + in_targets)
	print("  MSBuild path: " + msbuildPath)

	cmd = []
	cmd.append(msbuildPath)

	if not in_switches is None:
		cmd.append(in_switches)

	cmd.append("/p:Configuration=" + in_configuration)
	if not in_targets is None:
		cmd.append("/t:" + in_targets)

	cmd.append(in_projectPath)

	print(cmd)

	subprocess.check_call(cmd)
	return

