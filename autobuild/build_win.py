import sys
import os
import shutil
import zipfile
import glob
import subprocess
import argparse
import re

import vstudio
import util


def cleanArtifacts(in_artifactsPath):
	print()
	print("Cleaning artifacts")
	if os.path.exists(in_artifactsPath):
		shutil.rmtree(in_artifactsPath)
	os.mkdir(in_artifactsPath)
	return

def createVersionNumber():
    output = os.getenv("P4_CHANGELIST", "dev")
    return output

def stampVersion(in_version):
	cppfile = "../src/BrainCloudClient.cpp"; 
	with open("BrainCloudClient.tmp", "wt") as fout:
		with open(cppfile, "rt") as fin:
			for line in fin:
				line = re.sub(r"s_brainCloudClientVersion = .*", "s_brainCloudClientVersion = \"" + in_version + "\";", line);
				fout.write(line)

	# this will fail if file is read-only
	shutil.move("BrainCloudClient.tmp", cppfile)
	return

def stampReadme(in_platform, in_version):
	readmefile = "docs/README.TXT"; 
	with open("readme.tmp", "wt") as fout:
		with open(readmefile, "rt") as fin:
			for line in fin:
				line = re.sub(r"Platform\:.*", "Platform: " + in_platform, line);
				line = re.sub(r"Version\:.*", "Version: " + in_version, line);
				fout.write(line)

	# this will fail if file is read-only
	shutil.move("readme.tmp", readmefile)
	return


def buildWinDesktop(artifacts, version, rebuild):

	# msbuild vars
	projectPath = os.path.abspath("..")
	projectFile = projectPath + os.sep + "solutions" + os.sep + "windowsDesktop_vc120" + os.sep + "brainCloud.sln"

	if rebuild:
		targets = "Rebuild"
	else:
		targets = "Build"

	print()
	print("Building windows api project")
	print()
	sys.stdout.flush()
	switches = []
	switches.append("/p:Platform=Win32")

	# build release version of lib
	config = "Release"
	vstudio.buildProject(projectFile, targets, config, in_switches=switches)

	# build debug version of lib
	config = "Debug"
	vstudio.buildProject(projectFile, targets, config, in_switches=switches)

	switches = []
	switches.append("/p:Platform=x64")

	# build release version of lib
	config = "Release"
	vstudio.buildProject(projectFile, targets, config, in_switches=switches)

	# build debug version of lib
	config = "Debug"
	vstudio.buildProject(projectFile, targets, config, in_switches=switches)
	
	print()
	print("Zipping library")

	rootPath = os.path.abspath("..")
	binPath = projectPath + os.sep + "brainCloud" + os.sep + "Output"

	# zip up build directly from source files	
	with zipfile.ZipFile(artifacts + os.sep + "brainCloudClient_WindowsDesktop_" + version + ".zip", "w", compression=zipfile.ZIP_DEFLATED) as myzip:
		
		for fname in glob.iglob(binPath + os.sep + "Win32" + os.sep + "Release" + os.sep + "*.*"):
			myzip.write(fname, "lib" + os.sep + "release" + os.sep + os.path.basename(fname))

		for fname in glob.iglob(binPath + os.sep + "Win32" + os.sep + "Debug" + os.sep + "*.*"):
			myzip.write(fname, "lib" + os.sep + "debug" + os.sep + os.path.basename(fname))

		for fname in glob.iglob(binPath + os.sep + "x64" + os.sep + "Release" + os.sep + "*.*"):
			myzip.write(fname, "lib" + os.sep + "release" + os.sep + os.path.basename(fname))

		for fname in glob.iglob(binPath + os.sep + "x64" + os.sep + "Debug" + os.sep + "*.*"):
			myzip.write(fname, "lib" + os.sep + "debug" + os.sep + os.path.basename(fname))
		
		util.zipdir(rootPath + os.sep + "include" + os.sep, myzip, "include")
		util.zipdir(rootPath + os.sep + "lib" + os.sep + "jsoncpp-1.0.0", myzip, "thirdparty" + os.sep + "jsoncpp-1.0.0")
		util.zipdir(rootPath + os.sep + "lib" + os.sep + "win32" + os.sep + "cpprestsdk-static", myzip, "thirdparty" + os.sep + "casablanca")
		myzip.write("docs/README.TXT")
	return


def buildWinStore(artifacts, version, rebuild):

	print()
	print("Building windows store project")
	print()
	sys.stdout.flush()


	projectPath = os.path.abspath(".." + os.sep + "GameClientLib_winstore")
	projectFile = projectPath + os.sep + "GameClientLib_winstore.sln"

	if rebuild:
		targets = "Rebuild;Publish"
	else:
		targets = "Publish"

	configs = []
	configs.append("Debug")
	configs.append("Release")

	platforms = []
	platforms.append("Win32")
	platforms.append("ARM")
	platforms.append("x64")


	for platform in platforms:
		for config in configs:
			print()
			print("BUILDING FOR PLATFORM: " + platform + " CONFIG: " + config)
			switches = []
			switches.append("/p:Platform=" + platform)
			vstudio.buildProject(projectFile, targets, config, in_switches=switches)

	print()
	print("Zipping library files")

	rootPath = os.path.abspath("..")
	binPath = projectPath + os.sep + "bin" 

	# zip up build directly from source files	
	with zipfile.ZipFile(artifacts + os.sep + "brainCloudClient_WindowsStore_" + version + ".zip", "w", compression=zipfile.ZIP_DEFLATED) as myzip:
		util.zipdir(rootPath + os.sep + "GameClientLib" + os.sep + "src" + os.sep + "ApiHeaders" + os.sep, myzip, "include")
		util.zipdir(rootPath + os.sep + "BrainCloudWrapper", myzip, "src")

		util.zipdir(binPath, myzip, "lib")
	#	util.zipdir(rootPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "casablanca" + os.sep + "Binaries", myzip, "lib-casablanca")

#		for fname in glob.iglob(binPath + os.sep + "Release" + os.sep + "*.lib"):
#			myzip.write(fname, "lib" + os.sep + "release" + os.sep + os.path.basename(fname))

#		for fname in glob.iglob(binPath + os.sep + "Debug" + os.sep + "*.lib"):
#			myzip.write(fname, "lib" + os.sep + "debug" + os.sep + os.path.basename(fname))

#		for fname in glob.iglob(binPath + os.sep + "Release" + os.sep + "*.pdb"):
#			myzip.write(fname, "lib" + os.sep + "release" + os.sep + os.path.basename(fname))

#		for fname in glob.iglob(binPath + os.sep + "Debug" + os.sep + "*.pdb"):
#			myzip.write(fname, "lib" + os.sep + "debug" + os.sep + os.path.basename(fname))

#		for fname in glob.iglob(rootPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "pthread-w32-2-8-0" + os.sep + "Pre-built.2" + os.sep + "lib" + os.sep + "*.dll"):
#			myzip.write(fname, "bin" + os.sep + os.path.basename(fname))

#		myzip.write(projectPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "curl" + os.sep + "curl-7.21.6" +os.sep + "lib" + os.sep + "LIB-Release" + os.sep + "libcurl.lib", "lib" + os.sep + "release" + os.sep + "libcurl.lib")	

#		myzip.write(projectPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "curl" + os.sep + "curl-7.21.6" +os.sep + "lib" + os.sep + "LIB-Debug" + os.sep + "libcurld.lib", "lib" + os.sep + "debug" + os.sep + "libcurld.lib")	

		util.zipdir(rootPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "jsoncpp-1.0.0", myzip, "thirdparty" + os.sep + "jsoncpp-1.0.0")

		casaPath = rootPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "casablanca" 
		util.zipdir(casaPath, myzip, "thirdparty" + os.sep + "casablanca", [casaPath + os.sep + "Intermediate"])

		myzip.write("../../Common/docs/README.TXT", "README.TXT")

	return


def buildWinApi(artifacts, version, rebuild):

	# msbuild vars
	projectPath = os.path.abspath("..")
	projectFile = projectPath + os.sep + "GameClient_vs2013desktop_cpprest.sln"

	if rebuild:
		targets = "Rebuild"
	else:
		targets = "Build"

	print()
	print("Building windows api project")
	print()
	sys.stdout.flush()
	switches = []
	switches.append("/p:Platform=Win32")

	# build release version of lib
	config = "LIB Release"
	vstudio.buildProject(projectFile, targets, config, in_switches=switches)

	# build debug version of lib
	config = "LIB Debug"
	vstudio.buildProject(projectFile, targets, config, in_switches=switches)

	print()
	print("Zipping library")

	rootPath = os.path.abspath("..")
	binPath = projectPath + os.sep + "GameClientLib" + os.sep + "bin" 

	# zip up build directly from source files	
	with zipfile.ZipFile(artifacts + os.sep + "brainCloudClient_WindowsDesktop_" + version + ".zip", "w", compression=zipfile.ZIP_DEFLATED) as myzip:
		util.zipdir(rootPath + os.sep + "GameClientLib" + os.sep + "src" + os.sep + "ApiHeaders" + os.sep, myzip, "include")
		util.zipdir(rootPath + os.sep + "BrainCloudWrapper", myzip, "src")

		for fname in glob.iglob(binPath + os.sep + "Release" + os.sep + "*.lib"):
			myzip.write(fname, "lib" + os.sep + "release" + os.sep + os.path.basename(fname))

		for fname in glob.iglob(binPath + os.sep + "Debug" + os.sep + "*.lib"):
			myzip.write(fname, "lib" + os.sep + "debug" + os.sep + os.path.basename(fname))

		for fname in glob.iglob(binPath + os.sep + "Release" + os.sep + "*.pdb"):
			myzip.write(fname, "lib" + os.sep + "release" + os.sep + os.path.basename(fname))

		for fname in glob.iglob(binPath + os.sep + "Debug" + os.sep + "*.pdb"):
			myzip.write(fname, "lib" + os.sep + "debug" + os.sep + os.path.basename(fname))

	#	for fname in glob.iglob(rootPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "pthread-w32-2-8-0" + os.sep + "Pre-built.2" + os.sep + "lib" + os.sep + "*.dll"):
	#		myzip.write(fname, "bin" + os.sep + os.path.basename(fname))

	#	myzip.write(projectPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "curl" + os.sep + "curl-7.21.6" +os.sep + "lib" + os.sep + "LIB-Release" + os.sep + "libcurl.lib", "lib" + os.sep + "release" + os.sep + "libcurl.lib")	

	#	myzip.write(projectPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "curl" + os.sep + "curl-7.21.6" +os.sep + "lib" + os.sep + "LIB-Debug" + os.sep + "libcurld.lib", "lib" + os.sep + "debug" + os.sep + "libcurld.lib")	

		util.zipdir(projectPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "jsoncpp-1.0.0", myzip, "thirdparty" + os.sep + "jsoncpp-src-0.6.0-rc2")

		util.zipdir(projectPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "casablanca", myzip, "thirdparty" + os.sep + "casablanca")

		myzip.write("../../Common/docs/README.TXT", "README.TXT")
	return


def buildWinAll(artifacts, version, rebuild):

	print()
	print("Building windows all project")
	print()
	sys.stdout.flush()


	projectPath = os.path.abspath("..")
	projectFile = projectPath + os.sep + "GameClient_vs2013windows.sln"

# the stupid way to do multiple targets...

# WinStore
	if rebuild:
		targets = "GameClientLib_winstore:Rebuild;GameClientLib_winstore:Publish"
	else:
		targets = "GameClientLib_winstore:Publish"

	configs = []
	configs.append("Debug")
	configs.append("Release")

	platforms = []
	platforms.append("Win32")
	platforms.append("ARM")
	platforms.append("x64")

	for platform in platforms:
		for config in configs:
			print()
			print("BUILDING FOR PLATFORM: " + platform + " CONFIG: " + config)
			switches = []
			switches.append("/p:Platform=" + platform)
			vstudio.buildProject(projectFile, targets, config, in_switches=switches)

# winphone 8.1
	if rebuild:
		targets = "GameClientLib_winstore_WindowsPhone:Rebuild;GameClientLib_winstore_WindowsPhone:Publish"
	else:
		targets = "GameClientLib_winstore_WindowsPhone:Build"

	configs = []
	configs.append("Debug")
	configs.append("Release")

	platforms = []
	platforms.append("Win32")
	platforms.append("ARM")

	for platform in platforms:
		for config in configs:
			print()
			print("BUILDING FOR PLATFORM: " + platform + " CONFIG: " + config)
			switches = []
			switches.append("/p:Platform=" + platform)
			vstudio.buildProject(projectFile, targets, config, in_switches=switches)

# winphone 8.0
	if rebuild:
		targets = "GameClientLib_winstore_WP8:Rebuild;GameClientLib_winstore_WP8:Publish"
	else:
		targets = "GameClientLib_winstore_WP8:Publish"

	configs = []
	configs.append("Debug")
	configs.append("Release")

	platforms = []
	platforms.append("Win32")
	platforms.append("ARM")

	for platform in platforms:
		for config in configs:
			print()
			print("BUILDING FOR PLATFORM: " + platform + " CONFIG: " + config)
			switches = []
			switches.append("/p:Platform=" + platform)
			vstudio.buildProject(projectFile, targets, config, in_switches=switches)

			
	print()
	print("Zipping library files")

	rootPath = os.path.abspath("..")
	binPath = projectPath + os.sep + "bin" 

	# zip up build directly from source files	
	with zipfile.ZipFile(artifacts + os.sep + "brainCloudClient_WinStore_" + version + ".zip", "w", compression=zipfile.ZIP_DEFLATED) as myzip:
		util.zipdir(rootPath + os.sep + "GameClientLib" + os.sep + "src" + os.sep + "ApiHeaders" + os.sep, myzip, "include")
		util.zipdir(rootPath + os.sep + "BrainCloudWrapper", myzip, "src")

		util.zipdir(binPath, myzip, "lib")
		util.zipdir(rootPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "jsoncpp-1.0.0", myzip, "thirdparty" + os.sep + "jsoncpp-1.0.0")

		casaPath = rootPath + os.sep + "GameClientLib" + os.sep + "lib" + os.sep + "casablanca" 
		util.zipdir(casaPath, myzip, "thirdparty" + os.sep + "casablanca", [casaPath + os.sep + "Intermediate"])

		myzip.write("../../Common/docs/README.TXT", "README.TXT")

	return

def main():

	parser = argparse.ArgumentParser(description="Run the build")
	parser.add_argument("--winDesktop", dest="buildWinDesktop", action="store_true", help="Build for win7 + win8 + win10 desktop")
	parser.add_argument("--winStore", dest="buildWinStore", action="store_true", help="Build for windows 8/pro")
	parser.add_argument("--winAll", dest="buildWinAll", action="store_true", help="Build for windows 8/pro")
	parser.add_argument("--baseVersion", dest="baseVersion", action="store", required=True, help="Set the library version ie 1.5.0")
	parser.add_argument("--rebuild", dest="rebuild", action="store_true", help="Rebuild solution instead of just build")
	args = parser.parse_args()

	# general vars
	scmRev = createVersionNumber()
	version = args.baseVersion + "." + scmRev

	# general vars
	artifacts = os.path.abspath("artifacts")

	# clean up old builds
	cleanArtifacts(artifacts)

	# stamp version
	stampVersion(args.baseVersion)

	# and our final library output folder
#	binPath = os.path.abspath(".." + os.pathsep + "bin")
#	if os.path.exists(binPath):
#		shutil.rmtree(binPath)

#	with open(artifacts + os.sep + "build.properties", "w") as f:
#		f.write("build.version=" + version)

#	with open(os.path.abspath(".." + os.sep + "Resources" + os.sep + "version.txt"), "w") as f:
#		f.write(version)

	if args.buildWinDesktop:
		stampReadme("Windows Desktop", version)
		buildWinApi(artifacts, version, args.rebuild)

	if args.buildWinStore:
		stampReadme("Windows Store", version)
		buildWinStore(artifacts, version, args.rebuild)

	if args.buildWinAll:
		stampReadme("Windows Bundled", version)
		buildWinAll(artifacts, version, args.rebuild)
		
	return

def test():
	with zipfile.ZipFile("x.zip", "w", compression=zipfile.ZIP_DEFLATED) as myzip:
		util.zipdir("tmp", myzip, "thirdparty" + os.sep + "casablanca", ["tmp/ignore"], ["*.meta"])
	return

#test()
main()

