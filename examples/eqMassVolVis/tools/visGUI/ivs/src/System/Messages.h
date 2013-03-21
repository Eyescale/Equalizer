/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_SYSTEM_MESSAGES_H
#define IVS_SYSTEM_MESSAGES_H

namespace ivs
{
namespace sys
{
  // error messages
  extern const char ERR_MEMORY[];
  extern const char ERR_VALUE[];
  extern const char ERR_INDEX[];
  extern const char ERR_SIZE[];
  extern const char ERR_FILENAME[];
  extern const char ERR_FILE_OPEN_READ[];
  extern const char ERR_FILE_OPEN_WRITE[];
  extern const char ERR_FILE_READ[];
  extern const char ERR_FILE_WRITE[];
  extern const char ERR_DIR_CREATE[];
  extern const char ERR_DIR_CHANGE[];
  extern const char ERR_READ_HEADER[];
  extern const char ERR_SETJMP[];
  extern const char ERR_UNKNOWN[];
}

namespace png
{
  // png error messages
  extern const char PNG_ERR_SIG[];
  extern const char PNG_ERR_IO[];
  extern const char PNG_ERR_HEADER[];
  extern const char PNG_ERR_READ_DATA[];
  extern const char PNG_ERR_WRITE_DATA[];
  extern const char PNG_ERR_END[];
}

namespace ogl
{
  // entities
  extern const char DEPTH_RB[];

  extern const char EXT_INV_FORMAT[];
  extern const char EXT_PRESENT[];
  extern const char EXT_NOT_PRESENT[];
  extern const char EXT_MISSING[];

  // error messages
  extern const char ERR_LOAD_TEXTURE[];
  extern const char ERR_APPLYING_TEXTURE[];
  extern const char ERR_TEXTURE_FORMAT[];
  extern const char ERR_FBO[];
  extern const char ERR_FBO_INC_ATTACHMENT_EXT[];
  extern const char ERR_FBO_INC_MISSING_ATTACHMENT_EXT[];
  extern const char ERR_FBO_INC_DIMENSIONS_EXT[];
  extern const char ERR_FBO_INC_FORMATS_EXT[];
  extern const char ERR_FBO_INC_DRAW_BUFFER_EXT[];
  extern const char ERR_FBO_INC_READ_BUFFER_EXT[];
  extern const char ERR_FBO_UNSUPPORTED_EXT[];
  extern const char ERR_VBO_DATA[];
  extern const char ERR_VBO_MAP[];
  extern const char ERR_VBO_UNMAP[];
  extern const char ERR_PBO_DATA[];
  extern const char ERR_PBO_MAP[];
  extern const char ERR_PBO_UNMAP[];
  extern const char ERR_SHD_PROG[];
  extern const char ERR_SHD_CONTEXT[];
  extern const char ERR_SHD_DEF[];
  extern const char ERR_SHD_TYPE[];
  extern const char ERR_SHD_PROF[];
  extern const char ERR_SHD_COMPILE[];
  extern const char ERR_SHD_LNK[];
  extern const char ERR_GLEW_INIT[];
  extern const char WAR_SHD_FALLBACK_VERTEX[];
  extern const char WAR_SHD_FALLBACK_FRAGMENT[];
}

  // entities
  extern const char FOOTPRINT_TEXTURE[];
  extern const char SHEET_FBO[];
  extern const char SHEET_BUF_TEXTURE[];
  extern const char COPY_FBO[];
  extern const char COPY_BUF_TEXTURE[];
  extern const char IMAGE_FBO[];
  extern const char IMAGE_BUF_TEXTURE[];
  extern const char TRANSFER_RGBA_TEXTURE[];
  extern const char TRANSFER_SDA_TEXTURE[];

  // warnings
  extern const char WAR_FILESIZE_MISMATCH[];

  // error messages
  extern const char ERR_READ_FOOTPRINTS[];
  extern const char ERR_BUILD_PERSPECTIVE_INDEX[];
  extern const char ERR_READ_TRANSFER_FUNCTION[];
  extern const char ERR_WRITE_TRANSFER_FUNCTION[];     
  extern const char ERR_LOAD_TRANSFER_FUNCTION[];
  extern const char ERR_SPLAT_DIAMETER[];
  extern const char ERR_DIMENSIONS[];
  extern const char ERR_BUCKET_SIZE[];
  extern const char ERR_LOAD_MODEL[];
  extern const char ERR_CHANGE_MODEL[];
  extern const char ERR_UNSUPPORTED_GRAPH[];
  extern const char ERR_INVALID_FILE_FORMAT[];
}

#endif
