#pragma once

#include <iostream>

#include "exports.h"

namespace TDMS {

  class nullbuff : public std::streambuf {
  public:

    int overflow( int c ) {
      return c;
    }
  };

  class nullstream : public std::ostream {
  public:

    nullstream( ) : std::ostream( &buffer ) { }
  private:
    nullbuff buffer;
  };

  class log {
  public:
    static TDMS_EXPORT void setdebug( bool debug );
    static TDMS_EXPORT std::ostream& debug( );
  private:
    static bool quiet;
    static nullstream silencer;
  };
}
