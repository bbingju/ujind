set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER arm-buildroot-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-buildroot-linux-gnueabihf-g++)

# # where is the target environment located
# set(CMAKE_FIND_ROOT_PATH  /usr/i586-mingw32msvc
#       /home/alex/mingw-install)

# # adjust the default behavior of the FIND_XXX() commands:
# # search programs in the host environment
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# # search headers and libraries in the target environment
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
