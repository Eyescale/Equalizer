/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_FILE_H
#define IVS_FILE_H

#include <string>
#include <fstream>

namespace ivs
{
namespace io
{

/**
 * Abstract class for basic file operations.
 **/
class File
{
public:

  enum Mode
  {
    READ  = 1,
    WRITE = 2
  };

  File(const std::string& _filename, Mode _mode = READ);
  virtual ~File();

  // returns the filename
  virtual const std::string &getFilename() const;

  // True if the file exists, false else.
  virtual bool exists();

  // Open the file. Throw an exception in case of failure.
  virtual void open();

  // Close the file. No status is returned.
  virtual void close();

protected:
  // filename
  std::string filename_;

  // mode
  Mode mode_;

  // file
  std::fstream file_;
};

inline const std::string &File::getFilename() const
{
  return filename_;
}

}
}

#endif
