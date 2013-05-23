/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_SYSTEM_DESTROYER_H
#define IVS_SYSTEM_DESTROYER_H

namespace ivs
{
namespace sys
{

/**
 * Destroyer for singletons. The destroyer destroys singletons when the
 * application is terminated. This enables the destructor of the singleton to
 * clean up before the application is left.
 * @tparam Singleton: type of the singleton
 **/
template<typename Singleton>
class Destroyer
{
public:
  Destroyer(Singleton *_singleton = 0);
  ~Destroyer();

  void setSingleton(Singleton *_singleton);
private:
  Singleton *singleton_;
};

template<typename Singleton>
inline void Destroyer<Singleton>::setSingleton(Singleton *_singleton)
{
  singleton_ = _singleton;
}

}
}

#include "System/Destroyer.hh"

#endif
