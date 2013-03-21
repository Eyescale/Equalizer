/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_TRANSFER_FUNCTION_FILE_H
#define IVS_TRANSFER_FUNCTION_FILE_H

#include <string>
#include <vector>

#include "IO/File.h"
#include "TransferFunctionFactory.h"

namespace ivs
{
namespace io
{

/**
 * Transfer function file.
 **/
class TransferFunctionFile: public File
{
public:
  typedef TransferFunctionFactory::Item       Item;
  typedef TransferFunctionFactory::ItemVector ItemVector;

  TransferFunctionFile(const std::string& _filename, Mode _mode = READ);

  ItemVector readTransferFunction();
  void writeTransferFunction(const ItemVector &_data);
};

}
}

#endif
