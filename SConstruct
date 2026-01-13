#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")
env["strip"] = False
env.Append(CPPPATH=["src"])
sources = Glob("src/Linux-test-version/*.cpp")

env.Append(CCFLAGS=["-g"])
env.Append(CXXFLAGS=["-g"])
env.Append(CCFLAGS=["-O0"])
env.Append(CXXFLAGS=["-O0"])


library = env.SharedLibrary(
    "bin/linuxhost{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
