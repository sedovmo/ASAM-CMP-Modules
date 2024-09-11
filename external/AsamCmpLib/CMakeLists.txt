list(APPEND CMAKE_MESSAGE_CONTEXT AsamCmpLib)

FetchContent_Declare(
    AsamCmpLib
    GIT_REPOSITORY git@github.com:openDAQ/ASAM-CMP-Library.git
    GIT_TAG        ee1fe90ab43bddc4b371ad28a200fe0cf25170a8
)
FetchContent_MakeAvailable(AsamCmpLib)
