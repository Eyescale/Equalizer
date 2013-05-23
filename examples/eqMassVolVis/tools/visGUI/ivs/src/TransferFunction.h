/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_TRANSFER_FUNCTION_H
#define IVS_TRANSFER_FUNCTION_H

#include <boost/static_assert.hpp>

#include "System/OpenGL.h"
#include "System/Messages.h"
#include "System/Logger.h"
#include "System/Types.h"

namespace ivs
{
namespace tf
{
enum Format
{
  RGBA8,
  FLOAT16,
  FLOAT32
};
}

/**
 * Transfer function using OpenGL 1D texture.
 **/
template<int Components>
class GlTransferFunction
{
public:
  typedef tf::Format Format;

  //TODO: exchange hard coded component count everywhere
  enum
  {
    COMPONENTS = Components
  } ComponentsE;

  GlTransferFunction();
  GlTransferFunction(const GlTransferFunction &_transferFunction);
  GlTransferFunction(unsigned int _size, Format _format, const float *_data = 0);

  void init( unsigned int _size, Format _format, const float *_data = 0 );

  virtual ~GlTransferFunction();

        float *getData()         { return data_;   }
  const float *getData()   const { return data_;   }
  unsigned int    size()   const { return size_;   }
        Format getFormat() const { return format_; }

  GLuint getTexture() const;

  /** calculate the range in which the component is > 0 **/
  Vec2ui calculateRange(int _component) const;

/*  virtual void load();
  virtual void reload();
  virtual void unload();

  void use(int _unit) const;
*/
  // determine if the transfer function is ready for use
  bool isLoaded() const;

  GlTransferFunction &operator=(const GlTransferFunction &_transferFunction);

protected:
  void initializeData(const float *_data);

  static GLenum getPixelDataFormat();

  // the data stored as interleaved float array
  float *data_;

  unsigned int size_;
  Format format_;

  GLuint texture_;

  bool loaded_;

  static const GLenum formats_[3];
};

typedef GlTransferFunction<4>  RgbaTransferFunction;
typedef GlTransferFunction<3>  SdaTransferFunction;

struct TransferFunctionPair
{
    TransferFunctionPair(){}

    TransferFunctionPair( const RgbaTransferFunction first_,
                          const SdaTransferFunction  second_ )
        : first( first_ )
        , second( second_ )
    {
    }

    RgbaTransferFunction first;
    SdaTransferFunction  second;
};
///typedef std::pair<RgbaTransferFunction, SdaTransferFunction> TransferFunctionPair;
/*
template<int Components>
inline float *GlTransferFunction<Components>::getData()
{
  return data_;
}

template<int Components>
inline const float *GlTransferFunction<Components>::getData() const
{
  return data_;
}

template<int Components>
inline unsigned int GlTransferFunction<Components>::size() const
{
  return size_;
}*/
/*
template<int Components>
inline GLuint GlTransferFunction<Components>::getTexture() const
{
  return texture_;
}


template<int Components>
inline void GlTransferFunction<Components>::use(int _unit) const
{
  if (!data_ && loaded_)
  {
    glActiveTexture(GL_TEXTURE0 + _unit);
    glBindTexture(GL_TEXTURE_1D, texture_);

#ifdef DEBUG
    int error;
    if ((error = glGetError()) != GL_NO_ERROR)
      throw GET_ERROR_MESSAGE(ogl::ERR_APPLYING_TEXTURE) + TRANSFER_RGBA_TEXTURE + reinterpret_cast<const char *>(gluErrorString(error));
#endif
  }
}


template<int Components>
inline bool GlTransferFunction<Components>::isLoaded() const
{
  return loaded_;
}

template<int Components>
inline GLenum GlTransferFunction<Components>::getPixelDataFormat()
{
  BOOST_STATIC_ASSERT(true);
  return 0;
};

template<>
inline GLenum GlTransferFunction<3>::getPixelDataFormat()
{
  return GL_RGB;
};

template<>
inline GLenum GlTransferFunction<4>::getPixelDataFormat()
{
  return GL_RGBA;
};
*/
}

#include "TransferFunction.hh"

#endif
