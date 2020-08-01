#include "log.hpp"

namespace TDMS{

  bool log::quiet = true;
  nullstream log::silencer;

  void log::setdebug( bool dbg ) {
    quiet = !dbg;
  }

  std::ostream& log::debug( ) {
    return ( quiet
        ? silencer
        : std::cout );
  }

}
