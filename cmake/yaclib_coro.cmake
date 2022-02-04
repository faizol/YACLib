# TODO(mkornaukhov03) need a more general approach 
if (NOT YACLIB_CXX_STANDARD STREQUAL 20)
  message("Error: coroutines are supported only with C++20!")
  set (CORO OFF) 
elseif (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  list(APPEND YACLIB_COMPILE_OPTIONS -fcoroutines)
  list(APPEND YACLIB_LINK_OPTIONS -fcoroutines)
endif()    
