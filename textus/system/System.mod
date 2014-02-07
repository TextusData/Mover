cc_sources = [ SysInfo.cc, Environment.cc ],
target=system.a,
deps=[ textus/system/$^O/Arch, textus/base/Base ],
