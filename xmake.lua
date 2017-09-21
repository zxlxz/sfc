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

-- add target
target("nms")

    -- set kind
    set_kind("shared")

    -- set flags
    add_cxxflags("-std=c++1z", "-frtti", "-Wall", "-Wextra", "-Wconversion", "-Wno-unknown-pragmas")
    add_defines("NMS_BUILD")
    -- add files
    add_files("nms/**.cc")
    add_includedirs(".")
    set_pcxxheader("nms/config.h")

    -- test: blas?
    -- add_defines("NMS_OPENBLAS")
    -- add_links("openblas")

    -- build dir
    set_targetdir("publish/bin")
    set_objectdir("/tmp/nmscc")

-- add target
target("nms.test")

    -- set kind
    set_kind("binary")

    -- set flags
    add_cxxflags("-std=c++1z", "-frtti", "-I .")
    add_includedirs(".")
    set_targetdir("publish/bin")
    set_objectdir("/tmp/nmscc")

    -- add files
    add_deps("nms")
    add_files("nms.test/**.cc")
