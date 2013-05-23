/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#include "System/Messages.h"

namespace ivs
{
namespace sys
{
  // error messages
  const char ERR_MEMORY[]                             = "Error while allocating memory: ";
  const char ERR_VALUE[]                              = "Invalid value: ";
  const char ERR_INDEX[]                              = "Index out of range: ";
  const char ERR_SIZE[]                               = "Unequal sizes: ";
  const char ERR_FILENAME[]                           = "Empty or invalid file name!";
  const char ERR_FILE_OPEN_READ[]                     = "File could not be opened for reading: ";
  const char ERR_FILE_OPEN_WRITE[]                    = "File could not be opened for writing: ";
  const char ERR_FILE_READ[]                          = "Error while reading file: ";
  const char ERR_FILE_WRITE[]                         = "Error while writing file: ";
  const char ERR_DIR_CREATE[]                         = "Error while creating directory: ";
  const char ERR_DIR_CHANGE[]                         = "Error while changing directory: ";
  const char ERR_READ_HEADER[]                        = "Error while reading the header!";
  const char ERR_SETJMP[]                             = "Error calling setjmp!";
  const char ERR_UNKNOWN[]                            = "Unknown.";
}

namespace png
{
  // png error messages
  const char PNG_ERR_SIG[]                            = "File has no valid png signature: "; 
  const char PNG_ERR_IO[]                             = "Error while initialzing io!"; 
  const char PNG_ERR_HEADER[]                         = "Error while writing the header!"; 
  const char PNG_ERR_READ_DATA[]                      = "Error while read the data!"; 
  const char PNG_ERR_WRITE_DATA[]                     = "Error while writing the data!"; 
  const char PNG_ERR_END[]                            = "Error while end write!";
}

namespace ogl
{
  // entities
  const char DEPTH_RB[]                               = " depth render buffer";

  const char EXT_INV_FORMAT[]                         = " is not a valid extension format.";
  const char EXT_PRESENT[]                            = " extension is supported.";
  const char EXT_NOT_PRESENT[]                        = " extension is not supported.";
  const char EXT_MISSING[]                            = "Mandatory extension is missing!";

  // error messages
  const char ERR_LOAD_TEXTURE[]                       = "OpenGL: Error while loading the texture: ";
  const char ERR_APPLYING_TEXTURE[]                   = "OpenGL: Error while applying the texture: ";
  const char ERR_TEXTURE_FORMAT[]                     = "Invalid texture format!";
  const char ERR_FBO[]                                = "OpenGL: Error while setting up the framebuffer object: ";
  const char ERR_FBO_INC_ATTACHMENT_EXT[]             = "Incomplete attachment.";
  const char ERR_FBO_INC_MISSING_ATTACHMENT_EXT[]     = "Missing attachment.";
  const char ERR_FBO_INC_DIMENSIONS_EXT[]             = "Invalid dimensions.";
  const char ERR_FBO_INC_FORMATS_EXT[]                = "Invalid format.";
  const char ERR_FBO_INC_DRAW_BUFFER_EXT[]            = "Invalid draw buffer.";
  const char ERR_FBO_INC_READ_BUFFER_EXT[]            = "Invalid read buffer.";
  const char ERR_FBO_UNSUPPORTED_EXT[]                = "Unsupported.";
  const char ERR_VBO_DATA[]                           = "OpenGL: Error while creating a vertex buffer object data store: ";
  const char ERR_VBO_MAP[]                            = "OpenGL: Error while mapping vertex buffer object.";
  const char ERR_VBO_UNMAP[]                          = "OpenGL: Error while unmapping vertex buffer object.";
  const char ERR_PBO_DATA[]                           = "OpenGL: Error while creating a pixel buffer object data store: ";
  const char ERR_PBO_MAP[]                            = "OpenGL: Error while mapping vertex pixel object.";
  const char ERR_PBO_UNMAP[]                          = "OpenGL: Error while unmapping vertex pixel object.";
  const char ERR_SHD_PROG[]                           = "OpenGL: Error obtaining a program object: ";
  const char ERR_SHD_CONTEXT[]                        = "OpenGL: Error getting a context!";
  const char ERR_SHD_DEF[]                            = "OpenGL: Shader already defined!";
  const char ERR_SHD_TYPE[]                           = "OpenGL: Invalid shader type!";
  const char ERR_SHD_PROF[]                           = "OpenGL: Invalid shader profile!";
  const char ERR_SHD_COMPILE[]                        = "OpenGL: Error while compiling the shader: ";
  const char ERR_SHD_LNK[]                            = "OpenGL: Error while linking the shader: ";
  const char ERR_GLEW_INIT[]                          = "OpenGL: Error initializing GLEW: ";
  const char WAR_SHD_FALLBACK_VERTEX[]                = "OpenGL: Warning: Software fallback for vertex shader: ";
  const char WAR_SHD_FALLBACK_FRAGMENT[]              = "OpenGL: Warning: Software fallback for fragment shader: ";
}

  // entities
  const char FOOTPRINT_TEXTURE[]                      = " footprint texture";
  const char SHEET_FBO[]                              = " sheet framebuffer object";
  const char SHEET_BUF_TEXTURE[]                      = " sheet buffer texture";
  const char COPY_FBO[]                               = " copy framebuffer object";
  const char COPY_BUF_TEXTURE[]                       = " copy buffer texture";
  const char IMAGE_FBO[]                              = " image framebuffer object";
  const char IMAGE_BUF_TEXTURE[]                      = " image buffer texture";
  const char TRANSFER_RGBA_TEXTURE[]                  = " transfer function rgba texture";
  const char TRANSFER_SDA_TEXTURE[]                   = " transfer functio sda texture";
  
  // warnings
  const char WAR_FILESIZE_MISMATCH[]                  = "File size does not match with the given parameters!";

  // error messages
  const char ERR_READ_FOOTPRINTS[]                    = "Error while reading the footprints: ";
  const char ERR_BUILD_PERSPECTIVE_INDEX[]            = "Error while building the perspective index!";
  const char ERR_READ_TRANSFER_FUNCTION[]             = "Error while reading the transfer function: ";
  const char ERR_WRITE_TRANSFER_FUNCTION[]            = "Error while writing the transfer function: ";
  const char ERR_LOAD_TRANSFER_FUNCTION[]             = "Error: Transfer function could not be loaded : ";
  const char ERR_SPLAT_DIAMETER[]                     = "Error: The splat diameter must be > 0.0";
  const char ERR_DIMENSIONS[]                         = "Error: The volume dimensions must be > 0";
  const char ERR_BUCKET_SIZE[]                        = "Error: Bucket size too small: ";
  const char ERR_LOAD_MODEL[]                         = "Error: Model could not be loaded!";
  const char ERR_CHANGE_MODEL[]                       = "Error: Model could not be changed: ";
  const char ERR_UNSUPPORTED_GRAPH[]                  = "Error: Graph type is not supported: ";
  const char ERR_INVALID_FILE_FORMAT[]                = "Error: Invalid file format: ";
}
