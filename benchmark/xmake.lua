
target("rc-benchmark")
   set_kind("binary")
   set_targetdir("../bin")
   add_includedirs(".")
   add_deps("rc")
   add_files("**.cxx")
