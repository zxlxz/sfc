-- the debug mode
if is_mode("debug") then

    -- enable the debug symbols
    set_symbols("debug")

    -- disable optimization
    set_optimize("none")
end

-- the release mode
if is_mode("release") then

    -- set the symbols visibility: hidden
    set_symbols("hidden")

    -- enable fastest optimization
    set_optimize("fastest")

    -- strip all symbols
    set_strip("all")
end

-- c++: language
set_languages("cxx1z")
add_cxxflags("-fms-extensions", "-Wall", "-Wextra", "-Wno-unknown-pragmas", "-include config.h")
add_ldflags("Ur")

-- c++: dir
add_includedirs(".")
set_objectdir("$(tmpdir)/ustd")

-- add target
target("ustd.sys")
    set_kind("static")
    add_files("sys/posix/*.c")
    set_targetdir("publish/lib")

-- add target
target("ustd")
    set_kind("static")
    add_files("ustd/**.cc")
    add_links("ustd.sys")
    set_targetdir("publish/lib")

-- add target
target("ustd.test")
    set_kind("binary")
    add_files("tools/test/**.cc")
    add_deps("ustd")
    add_links("pthread")
    set_targetdir("publish/bin")