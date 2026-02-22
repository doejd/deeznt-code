#!/usr/bin/env python
import os
import sys
from SCons.Script import *

# Import godot-cpp build environment
env = SConscript("godot-cpp/SConstruct")

env["strip"] = False
env.Append(CPPPATH=["src"])

# ---------------------------------------
# Debug flags (optional)
# ---------------------------------------
env.Append(CCFLAGS=["-g", "-O0"])
env.Append(CXXFLAGS=["-g", "-O0"])

# ---------------------------------------
# Platform specific sources
# ---------------------------------------
platform = env["platform"]

common_sources = Glob("src/Common/*.cpp")

if platform == "linux":
    platform_sources = Glob("src/Linux/*.cpp")
    target_name = "linuxhost"
elif platform == "windows":
    platform_sources = Glob("src/Windows/*.cpp")
    target_name = "windowshost"
elif platform == "macos":
    platform_sources = Glob("src/Mac/*.cpp")
    target_name = "machost"
else:
    print("Unsupported platform:", platform)
    Exit(1)

sources = common_sources + platform_sources

# ---------------------------------------
# Windows specific fixes
# ---------------------------------------
if platform == "windows":
    env.Append(CPPDEFINES=["WIN32", "_WINDOWS", "UNICODE"])
    env.Append(LIBS=["user32", "kernel32"])
    env.Append(LINKFLAGS=["/SUBSYSTEM:WINDOWS"])

# ---------------------------------------
# Build library
# ---------------------------------------
library = env.SharedLibrary(
    "bin/{}{}{}".format(target_name, env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)