
# Configures rules for publishing open source packages.

include(UploadPPA)
include(MacPorts)
if(UPLOADPPA_FOUND)
  upload_ppas()
endif()
