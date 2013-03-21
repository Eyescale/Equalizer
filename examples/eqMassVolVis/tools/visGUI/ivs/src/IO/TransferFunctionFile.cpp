/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#include "IO/TransferFunctionFile.h"

#include <fstream>

#include <boost/foreach.hpp>

#include "System/Messages.h"
#include "System/Logger.h"

namespace ivs
{
namespace io
{

TransferFunctionFile::TransferFunctionFile(const std::string& _filename, Mode _mode):
  File(_filename, _mode)
{}

TransferFunctionFile::ItemVector TransferFunctionFile::readTransferFunction()
{
  ItemVector data;

  if (file_.is_open())
  {
    try
    {
      Item item;
      file_.seekg(0, std::ios_base::beg);
      file_ >> item.r >> item.g >> item.b >> item.a >> item.s >> item.d >> item.am;
      while (file_.good())
      {
        data.push_back(item);
        file_ >> item.r >> item.g >> item.b >> item.a >> item.s >> item.d >> item.am;
      }
    }
    catch (std::ios_base::failure &_f)
    {
      if (!file_.eof())
        throw GET_ERROR_MESSAGE(ERR_READ_TRANSFER_FUNCTION) + _f.what();
    }
    catch (std::string &_s)
    {
      throw GET_ERROR_MESSAGE(ERR_READ_TRANSFER_FUNCTION) + _s;
    }
  }

  return data;
}

void TransferFunctionFile::writeTransferFunction(const ItemVector &_data)
{
  if (file_.is_open())
  {
    try
    {
      file_.seekg(0, std::ios_base::beg);

      BOOST_FOREACH(const Item & item, _data)
      {
        file_ << item.r << " " << item.g << " " << item.b << " " << item.a << " " << item.s << " " << item.d << " " << item.am << std::endl;
      }
    }
    catch (std::ios_base::failure &_f)
    {
      throw GET_ERROR_MESSAGE(ERR_WRITE_TRANSFER_FUNCTION) + _f.what();
    }
    catch (std::string &_s)
    {
      throw GET_ERROR_MESSAGE(ERR_WRITE_TRANSFER_FUNCTION) + _s;
    }
  }
}

}
}
