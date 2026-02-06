set_project("OffhandMod")
set_version("1.0.0")

set_languages("c99", "cxx23")

add_rules("mode.release")

add_cxflags(
    "-O2",
    "-fvisibility=hidden",
    "-ffunction-sections",
    "-fdata-sections",
    "-w"
)

add_cflags(
    "-O2",
    "-fvisibility=hidden",
    "-ffunction-sections",
    "-fdata-sections",
    "-w"
)

add_ldflags(
    "-Wl,--gc-sections",
    "-Wl,--strip-all",
    "-s",
    {force = true}
)

add_repositories("xmake-repo https://github.com/xmake-io/xmake-repo.git")

add_requires(
    "fmt 10.2.1",
    "glm 0.9.9+8"
)

target("preloader")
    set_kind("static")
    add_files("preloader-android/src/pl/**.cpp")
    add_headerfiles("preloader-android/src/pl/**.h")
    add_packages("fmt", "glm")
    add_includedirs("preloader-android/src", {public = true})

target("dobby")
    set_kind("static")
    add_linkdirs("Dobby/$(arch)")
    add_links("dobby")
    set_default(false)

target("OffhandMod")
    set_kind("shared")
    add_files(
        "src/**.cpp",
        "src/**.c",
        "include/**.cpp"
    )
    add_headerfiles(
        "src/**.hpp",
        "include/**.hpp",
        "include/**.h",
        "Dobby/**.hpp",
        "Dobby/**.h"
    )
    add_includedirs(
        "include",
        "src",
        "Dobby",
        {public = true}
    )
    add_deps("preloader")
    add_packages("fmt", "glm")
    add_links(
        "log",
        "dobby"
    )