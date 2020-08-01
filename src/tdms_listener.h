/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tdmslistener.h
 * Author: ryan
 *
 * Created on August 1, 2020, 12:54 PM
 */

#ifndef TDMSLISTENER_H
#define TDMSLISTENER_H

#include "data_type.h"
#include <string>

namespace TDMS {

  class listener {
  public:
    virtual void data( const std::string& channelname, const unsigned char* rawdata,
        data_type_t, size_t num_vals ) = 0;
  };

}
#endif /* TDMSLISTENER_H */

