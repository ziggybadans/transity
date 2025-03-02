# Tests configuration
include(CTest)

# Find Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Add unit tests
add_subdirectory(unit)

# Add integration tests
add_subdirectory(integration) 