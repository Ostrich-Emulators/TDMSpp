/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tdms_exceptions.h
 * Author: ryan
 *
 * Created on August 1, 2020, 1:11 PM
 */

#ifndef TDMS_EXCEPTIONS_H
#define TDMS_EXCEPTIONS_H

#include "tdms_exports.h"

namespace TDMS {

  class no_segment_error : public std::runtime_error {
  public:

      TDMS_EXPORT no_segment_error( ) : std::runtime_error( "Not a segment" ) { }
  };

  class read_error : public std::runtime_error {
  public:

      TDMS_EXPORT read_error( ) : std::runtime_error( "Segment read error" ) { }
  };
}
#endif /* TDMS_EXCEPTIONS_H */

