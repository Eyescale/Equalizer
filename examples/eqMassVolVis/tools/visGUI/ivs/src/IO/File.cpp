/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#include "IO/File.h"

#include <iostream>

#include "System/Logger.h"

namespace ivs
{
namespace io
{

File::File(const std::string& _filename, Mode _mode):
  filename_(_filename),
  mode_(_mode)
{}

File::~File()
{}

bool File::exists()
{
  file_.exceptions(std::ios::goodbit);
  file_.open(filename_.c_str(), std::ios::in);

  if (file_.is_open())
  {
    file_.close();
    return true;
  }
  file_.close();
  return false;
}

void File::open()
{
  try
  {
    file_.exceptions(std::ios::failbit | std::ios::badbit);

    if ((mode_ & READ) == READ)
      file_.open(filename_.c_str(), std::ios::binary | std::ios::in);
    else if ((mode_ & WRITE) == WRITE)
      file_.open(filename_.c_str(), std::ios::binary | std::ios::out);
  }
  catch (std::ios_base::failure &_f)
  {
    throw GET_ERROR_MESSAGE(_f.what());
  }
}

void File::close()
{
  if (file_.is_open())
    file_.close();
}

}
}
