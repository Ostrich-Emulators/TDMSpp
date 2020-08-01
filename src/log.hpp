#pragma once

#include <iostream>
#include <sstream>

#include "exports.h"

namespace TDMS {

  class log {
  public:
    static TDMS_EXPORT void setdebug( bool debug );
    static TDMS_EXPORT std::ostream& debug();
  private:
    static bool quiet;
    static std::stringstream silencer;
  };
}
