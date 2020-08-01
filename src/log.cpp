#include "log.hpp"

namespace TDMS{

  bool log::quiet = true;
  std::stringstream log::silencer;

  void log::setdebug( bool dbg ) {
    quiet = !dbg;
  }

  std::ostream& log::debug( ) {
    return ( quiet
        ? ( std::ostream& ) silencer
        : std::cout );
  }

}
