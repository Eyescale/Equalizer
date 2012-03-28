
# Creates a FROM->TO symlink during installation
function(INSTALL_SYMLINK FROM TO COMPONENT)
  if (MSVC)
    install(CODE
      "get_filename_component(FROM_ABSOLUTE ${CMAKE_INSTALL_PREFIX}/${FROM}
         ABSOLUTE)
       file(TO_NATIVE_PATH \${FROM_ABSOLUTE} FROM_ABSOLUTE)
       file(TO_NATIVE_PATH \${CMAKE_INSTALL_PREFIX}/${TO} TO)
       message(\"mklink /j \${TO} \${FROM_ABSOLUTE}\")
       execute_process(COMMAND mklink /j \${TO} \${FROM_ABSOLUTE})"
      COMPONENT "${COMPONENT}")
  else()
    install(CODE
      "execute_process(COMMAND rm -f ${CMAKE_INSTALL_PREFIX}/${TO})
       get_filename_component(FROM_ABSOLUTE ${CMAKE_INSTALL_PREFIX}/${FROM}
         ABSOLUTE)
       execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink  
         \${FROM_ABSOLUTE} ${CMAKE_INSTALL_PREFIX}/${TO})"
      COMPONENT "${COMPONENT}")
  endif()
endfunction()
