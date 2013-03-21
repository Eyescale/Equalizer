/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_TRANSFER_FUNCTION_FACTORY_H
#define IVS_TRANSFER_FUNCTION_FACTORY_H

#include <vector>

#include "TransferFunction.h"

namespace ivs
{

/**
 * A factory for transfer functions.
 * Pattern: Factory
 **/
class TransferFunctionFactory
{
public:
  struct Item
  {
    float r, g, b, a, s, d, am;
  };

  typedef std::vector<Item> ItemVector;

  TransferFunctionFactory();
  ~TransferFunctionFactory();

  /**
   * Creates a rgba and a sda array from an item vector. These arrays are used
   * to construct the transfer function.
   **/
  static void createRgbaSdaData(const ItemVector &_data, float *_rgba, float *_sda);

  /** Creates an item vector from a rgba and a sda array **/
  static ItemVector createItemVector(unsigned int _size, const float *_rgba, float *_sda);

  /**
   * Create a pair of transfer functions. The first transfer function contains
   * the mapping from the voxel values to the respective color. The second
   * transfer function contains the mapping from the voxel value to the
   * respective material property.
   **/
  static TransferFunctionPair *createTransferFunction(tf::Format _format);
  static TransferFunctionPair *createTransferFunction(tf::Format _format, const ItemVector &_data);
};

}

#endif
