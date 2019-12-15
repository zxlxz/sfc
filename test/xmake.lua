
target("rc-test")
   set_kind("binary")
   set_targetdir("../bin")
   add_includedirs(".")
   add_deps("rc")
   add_files("**.cxx")
   del_files("rc/cuda/**.cxx")
