list(APPEND CMAKE_MESSAGE_CONTEXT AsamCmpLib)

set(ASAM_CMP_BUILD_EXAMPLE OFF)

FetchContent_Declare(
    AsamCmpLib
    GIT_REPOSITORY git@github.com:openDAQ/ASAM-CMP-Library.git
    GIT_TAG        df3948f78e1eaef6e7044393706a3a7785b5ec35
)
FetchContent_MakeAvailable(AsamCmpLib)
