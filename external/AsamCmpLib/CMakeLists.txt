list(APPEND CMAKE_MESSAGE_CONTEXT AsamCmpLib)

FetchContent_Declare(
    AsamCmpLib
    GIT_REPOSITORY git@github.com:openDAQ/ASAM-CMP-Library.git
    GIT_TAG        ea9c2fc277034a3466e9ea155779d29c59f2d247
)
FetchContent_MakeAvailable(AsamCmpLib)