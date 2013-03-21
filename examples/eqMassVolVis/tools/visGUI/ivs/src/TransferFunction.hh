/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#include <cstring>
#include <string>

namespace ivs
{
template<int Components>
const GLenum GlTransferFunction<Components>::formats_[3] = {GL_RGBA8,
    GL_RGBA16F_ARB,
    GL_RGBA32F_ARB
                                                           };

template<int Components>
GlTransferFunction<Components>::GlTransferFunction():
  data_(0),
  size_(0),
  format_(tf::RGBA8),
  texture_(static_cast<GLuint>(0)),
  loaded_(false)
{}

template<int Components>
GlTransferFunction<Components>::GlTransferFunction(const GlTransferFunction &_transferFunction):
  size_(_transferFunction.size_),
  format_(_transferFunction.format_),
  texture_(static_cast<GLuint>(0)),
  loaded_(false)
{
  initializeData(_transferFunction.data_);
}

template<int Components>
GlTransferFunction<Components>::GlTransferFunction(unsigned int _levels, Format _format, const float *_data):
  size_(_levels),
  format_(_format),
  texture_(static_cast<GLuint>(0)),
  loaded_(false)
{
  initializeData(_data);
}

template<int Components>
void GlTransferFunction<Components>::init( unsigned int _size, Format _format, const float *_data )
{
  delete [] data_; data_ = 0;
  size_    = _size;
  format_  = _format;
  texture_ = static_cast<GLuint>(0);
  initializeData( _data );
}


template<int Components>
GlTransferFunction<Components>::~GlTransferFunction()
{
///  unload();
  delete [] data_;
}

template<int Components>
Vec2ui GlTransferFunction<Components>::calculateRange(int _component) const
{
  Vec2ui range(0u);
  if (data_)
  {
    unsigned int i =  0;
    while (i < size() && data_[COMPONENTS * i + _component] < 1e-5)
      ++i;
    range.x() = i;
    i = size();
    while (i > 0 && data_[COMPONENTS * (i - 1) + _component] < 1e-5)
      --i;
    range.y() = i;
  }
  return range;
}
/*
template<int Components>
void GlTransferFunction<Components>::load()
{
  if (data_ && !loaded_)
  {
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_1D, texture_);

    glTexImage1D(GL_TEXTURE_1D, 0, formats_[format_], size_, 0, getPixelDataFormat(), GL_FLOAT, data_);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // NEVER change to GL_LINEAR otherwise strange effects popup
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int error;
    if ((error = glGetError()) != GL_NO_ERROR)
      throw GET_ERROR_MESSAGE(ogl::ERR_LOAD_TEXTURE) + "transfer function: " + reinterpret_cast<const char *>(gluErrorString(error));
    loaded_ = true;
  }
}

template<int Components>
void GlTransferFunction<Components>::reload()
{
  if (data_ && loaded_)
  {
    glBindTexture(GL_TEXTURE_1D, texture_);
    glTexImage1D(GL_TEXTURE_1D, 0, formats_[format_], size_, 0, getPixelDataFormat(), GL_FLOAT, data_);

    int error;
    if ((error = glGetError()) != GL_NO_ERROR)
      throw GET_ERROR_MESSAGE(ogl::ERR_LOAD_TEXTURE) + "transfer function: " + reinterpret_cast<const char *>(gluErrorString(error));
  }
}

template<int Components>
void GlTransferFunction<Components>::unload()
{
  if (loaded_)
  {
    glDeleteTextures(1, &texture_);
    loaded_ = false;
  }
}
*/

template<int Components>
GlTransferFunction<Components> &GlTransferFunction<Components>::operator=(const GlTransferFunction &_transferFunction)
{
///  unload();
  init( _transferFunction.size_, _transferFunction.format_, _transferFunction.data_ );

  return *this;
}

template<int Components>
void GlTransferFunction<Components>::initializeData(const float *_data)
{
  if (size_ != 0)
  {
    data_ = new float[Components * size_];
    if (_data != 0)
      memcpy(data_, _data, Components * size_ * sizeof(float));
    else
      memset(data_, 0, Components * size_ * sizeof(float));
  }
  else
    data_ = 0;
}

}
