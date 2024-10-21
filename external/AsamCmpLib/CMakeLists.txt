list(APPEND CMAKE_MESSAGE_CONTEXT AsamCmpLib)

set(ASAM_CMP_LIB_BUILD_EXAMPLE OFF)
set(ASAM_CMP_LIB_ENABLE_TESTS OFF)

FetchContent_Declare(
    AsamCmpLib
    GIT_REPOSITORY git@github.com:openDAQ/ASAM-CMP-Library.git
    GIT_TAG        v1.0.0-rc1
)
FetchContent_MakeAvailable(AsamCmpLib)
