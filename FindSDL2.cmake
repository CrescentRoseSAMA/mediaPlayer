set(SDL2ROOT /home/ruby/sdl2_loc)

set(SDL2_INCLUDE_DIRS ${SDL2ROOT}/include)

set(SDL2_LIBRARY_DIRS ${SDL2ROOT}/lib)

find_library(SDL2_LIBS SDL2 ${SDL2_LIBRARY_DIRS})
