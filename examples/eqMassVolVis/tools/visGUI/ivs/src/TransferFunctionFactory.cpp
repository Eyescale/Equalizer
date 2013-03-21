/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#include "TransferFunctionFactory.h"

#include "System/Messages.h"
#include "System/Logger.h"

namespace ivs
{

TransferFunctionFactory::TransferFunctionFactory()
{}

TransferFunctionFactory::~TransferFunctionFactory()
{}

void TransferFunctionFactory::createRgbaSdaData(const ItemVector &_data, float *_rgba, float *_sda)
{
  for (unsigned int i = 0; i < _data.size(); ++i)
  {
    _rgba[4 * i] = _data[i].r;
    _rgba[4 * i + 1] = _data[i].g;
    _rgba[4 * i + 2] = _data[i].b;
    _rgba[4 * i + 3] = _data[i].a;
    _sda[3 * i] = _data[i].s;
    _sda[3 * i + 1] = _data[i].d;
    _sda[3 * i + 2] = _data[i].am;
  }
}

TransferFunctionFactory::ItemVector TransferFunctionFactory::createItemVector(unsigned int _size, const float *_rgba, float *_sda)
{
  ItemVector data;
  for (unsigned int i = 0; i < _size; ++i)
  {
    Item item;
    item.r  = _rgba[4 * i];
    item.g  = _rgba[4 * i + 1];
    item.b  = _rgba[4 * i + 2];
    item.a  = _rgba[4 * i + 3];
    item.s  = _sda[3 * i];
    item.d  = _sda[3 * i + 1];
    item.am = _sda[3 * i + 2];
    data.push_back(item);
  }
  return data;
}

TransferFunctionPair * TransferFunctionFactory::createTransferFunction(tf::Format _format)
{
  Item item;
  item.r = 1.0f;
  item.g = 1.0f;
  item.b = 1.0f;
  item.s = 0.3f;
  item.d = 0.5f;
  item.am = 0.8f;
  ItemVector data;
  for (int i = 0; i < 256; ++i)
  {
    item.a = static_cast<float>(i) / 256.0f;
    data.push_back(item);
  }

  return createTransferFunction(_format, data);
}

TransferFunctionPair * TransferFunctionFactory::createTransferFunction(tf::Format _format,
    const ItemVector &_data)
{
  unsigned int levels = _data.size();

  TransferFunctionPair *transferFunction =
    new TransferFunctionPair(RgbaTransferFunction(levels, _format),
                             SdaTransferFunction(levels, _format));

  if (levels > 0)
  {
    float *rgba = transferFunction->first.getData();
    float *sda  = transferFunction->second.getData();
    createRgbaSdaData(_data, rgba, sda);
  }

  return transferFunction;
}

}
