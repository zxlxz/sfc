add_rules("mode.debug", "mode.release")

add_languages("cxx2a")

add_cxflags("-march=native -mtune=native -ffast-math")
add_cxflags("-Wall")

if is_os("linux", "macos") then
    add_ldflags("-pthread")
end

set_targetdir("lib")
set_objectdir(".xmake/")
set_dependir(".xmake/")

add_includedirs("src")

target("rc")
    set_kind("shared")
    set_pcxxheader("src/rc.inl")
    add_files("src/rc/**.cc")
    del_files("src/rc/sys/**.cc")
    del_files("src/rc/cuda/**.cc")
    add_files("src/rc/sys/unix/**.cc")

add_subdirs("test")
add_subdirs("benchmark")
