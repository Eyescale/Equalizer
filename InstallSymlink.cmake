
# Creates a FROM->TO symlink during installation
function(INSTALL_SYMLINK FROM TO)
  if (MSVC)
    install(CODE
      "get_filename_component(FROM_ABSOLUTE ${CMAKE_INSTALL_PREFIX}/${FROM}
         ABSOLUTE)
       execute_process(COMMAND mlink \${FROM_ABSOLUTE} ${TO})")
  else()
    install(CODE
      "execute_process(COMMAND rm -f ${TO})
       get_filename_component(FROM_ABSOLUTE ${CMAKE_INSTALL_PREFIX}/${FROM}
         ABSOLUTE)
       execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink  
         \${FROM_ABSOLUTE} ${TO})")
  endif()
endfunction()
