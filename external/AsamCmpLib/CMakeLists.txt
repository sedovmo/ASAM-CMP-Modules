list(APPEND CMAKE_MESSAGE_CONTEXT AsamCmpLib)

set(ASAM_CMP_LIB_BUILD_EXAMPLE OFF)
set(ASAM_CMP_LIB_ENABLE_TESTS OFF)

FetchContent_Declare(
    AsamCmpLib
    GIT_REPOSITORY git@github.com:openDAQ/ASAM-CMP-Library.git
    GIT_TAG        fb166a6be0d0db478ed911fba1320f974d50cd3c
)
FetchContent_MakeAvailable(AsamCmpLib)
