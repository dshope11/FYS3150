# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(boxed_particles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Options ---------------------------------------------------------------
option(WRAP_PARTICLES "Enable wrapping of particles around the map edges" OFF)

set(COST_FUNC "policy::baseline" CACHE STRING "What cost policy to use")
set_property(CACHE COST_FUNC PROPERTY STRINGS "policy::baseline;policy::simplified")

set(ILP "NONE" CACHE STRING "Use ILP policy NONE, BRAID2")
set_property(CACHE ILP PROPERTY STRINGS "NONE;BRAID2")
# ---- Dependencies ----------------------------------------------------------
find_package(Armadillo REQUIRED)
# SFML 3 with Graphics, System, Window
find_package(SFML 3 REQUIRED COMPONENTS Graphics System Window)

# ---- Helper to add one layout executable -----------------------------------
function(add_layout_exe target_name layout)
    add_executable(${target_name})

    # Common include path and PCH
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_precompile_headers(${target_name} PRIVATE include/precomp.h)

    # Shared compile-time toggle, per-target
    target_compile_definitions(${target_name} PRIVATE
            WRAP_PARTICLES=$<IF:$<BOOL:${WRAP_PARTICLES}>,1,0>
    )

    # Layout-specific sources and defines
    if(layout STREQUAL "AOS")
        target_compile_definitions(${target_name} PRIVATE USE_AOS)
        target_sources(${target_name} PRIVATE
                mwe.cpp
                src/aos/box.cpp
                src/aos/actor.cpp
        )
    elseif(layout STREQUAL "SOA")
        target_compile_definitions(${target_name} PRIVATE USE_SOA)
        target_sources(${target_name} PRIVATE
                mwe.cpp
                src/soa/box.cpp
                src/soa/particle.cpp
                src/soa/hole.cpp
        )
    elseif(layout STREQUAL "ECS")
        target_compile_definitions(${target_name} PRIVATE USE_ECS)
        target_sources(${target_name} PRIVATE
                mwe.cpp
                src/ecs/box.cpp
        )
    elseif(layout STREQUAL "SOAOPT")
        target_compile_definitions(${target_name} PRIVATE USE_SOAOPT COST_FUNC=${COST_FUNC} ILP=${ILP})
        target_sources(${target_name} PRIVATE
                mwe.cpp
                src/optimisation_soa/box.cpp
                src/optimisation_soa/particle.cpp
                src/optimisation_soa/hole.cpp
        )
    else()
        message(FATAL_ERROR "Unknown layout '${layout}'. Use AOS, SOA, ECS or SOAOPT.")
    endif()

    # Link libraries
    target_link_libraries(${target_name}
            PRIVATE
            SFML::Graphics
            SFML::System
            SFML::Window
            ${ARMADILLO_LIBRARIES}
    )
endfunction()

# ---- Build both executables -------------------------------------------------
add_layout_exe(boxed_particles_aos AOS)
add_layout_exe(boxed_particles_soa SOA)
add_layout_exe(boxed_particles_ecs ECS)
add_layout_exe(boxed_particles_soaOpt SOAOPT)
