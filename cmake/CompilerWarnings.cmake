function(setup_compiler_warnings target)
  if (MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive- /EHsc)
  else ()
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic
      -Wconversion -Wsign-conversion
      -Wshadow -Wnon-virtual-dtor
      -Wold-style-cast -Woverloaded-virtual
    )
  endif ()
endfunction()

