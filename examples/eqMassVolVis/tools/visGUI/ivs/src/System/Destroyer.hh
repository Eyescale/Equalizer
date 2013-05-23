/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

namespace ivs
{
namespace sys
{

template<typename Singleton>
Destroyer<Singleton>::Destroyer(Singleton *_singleton):
  singleton_(_singleton)
{}

template<typename Singleton>
Destroyer<Singleton>::~Destroyer()
{
  delete singleton_;
}

}
}
