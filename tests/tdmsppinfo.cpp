#include <iostream>
#include <vector>

#include <tdms.hpp>
#include <log.hpp>

#include "optionparser.h"

// Define options

enum optionIndex{
  UNKNOWN, HELP, PROPERTIES, DEBUG
};

const option::Descriptor usage[] = {
  {UNKNOWN, 0, "", "", option::Arg::None, "USAGE: tdmsppinfo [options] filename ...\n\n"
    "Options:" },
  {HELP, 0, "h", "help", option::Arg::None, "  --help, \tPrint usage and exit." },
  {PROPERTIES, 0, "p", "properties", option::Arg::None, "  --properties, \tPrint channel properties." },
  {DEBUG, 0, "d", "debug", option::Arg::None, "  --debug, \tPrint debugging information to stderr." },
  {0, 0, 0, 0, 0, 0 }
};

class l : public TDMS::listener{
public:

  virtual void data( const std::string& channelname, const unsigned char* datablock, TDMS::data_type_t datatype, size_t num_vals ) override {
    std::cout << "reading " << num_vals << " for channel: " << channelname << std::endl;

    std::vector<double> vals;
    vals.reserve( num_vals );

    memcpy(&vals[0], datablock, datatype.length * num_vals );
    std::cout << "\t";
    for ( size_t i = 0; i < num_vals; i++ ) {
      std::cout << vals[i] << " ";
    }
    std::cout << std::endl;
  }
};

int main( int argc, char** argv ) {
  // Parse options
  argc -= ( argc > 0 );
  argv += ( argc > 0 ); // Skip the program name if present
  option::Stats stats( usage, argc, argv );
  option::Option options[stats.options_max], buffer[stats.buffer_max];
  option::Parser parse( usage, argc, argv, options, buffer );

  if ( parse.error( ) ) {
    std::cerr << "parse.error() != 0" << std::endl;
    return 1;
  }
  if ( options[HELP] || argc == 0 || options[UNKNOWN] ) {
    option::printUsage( std::cout, usage );
    return 0;
  }
  if ( options[DEBUG] ) {
    TDMS::log::debug.debug_mode = true;
  }

  std::vector<std::string> _filenames;
  for ( size_t i = 0; i < parse.nonOptionsCount( ); ++i ) {
    _filenames.push_back( parse.nonOption( i ) );
  }
  for ( std::string filename : _filenames ) {
    if ( _filenames.size( ) > 1 )
      std::cout << filename << ":" << std::endl;
    TDMS::file f( filename );
    std::cout << f.segments( ) << " segments parsed" << std::endl;

    for ( TDMS::object* o : f ) {
      std::cout << o->get_path( ) << std::endl;
      if ( options[PROPERTIES] ) {
        for ( auto p : o->get_properties( ) ) {
          void * val = p.second->value;
          TDMS::data_type_t valtype = p.second->data_type;

          if ( valtype.name == "tdsTypeString" ) {
            std::string a = *( ( std::string* ) val );
            std::cout << "  " << p.first << ": " << a << std::endl;
          }
          else if ( valtype.name == "tdsTypeDoubleFloat" ) {
            double dbl = *( (double*) val );
            std::cout << "  " << p.first << ": " << dbl << std::endl;
          }
          else if ( valtype.name == "tdsTypeTimeStamp" ) {
            time_t timer = *( (time_t*) val );
            tm * pt = gmtime( &timer );

            char buffer[80];
            sprintf( buffer, "%d.%02d.%d %02d:%02d:%02d,%f",
                pt->tm_mday, pt->tm_mon + 1, 1900 + pt->tm_year, pt->tm_hour, pt->tm_min, pt->tm_sec, 0.0 );
            std::cout << "  " << p.first << ": " << buffer << std::endl;
          }
          else {
            std::cout << "  " << p.first << ": " << valtype.name << std::endl;
          }
        }
      }

      std::cout << o->number_values( ) << " values present ("
          << o->bytes( ) << ")"
          << std::endl;
    }

    l listener;
    for ( size_t i = 0; i < 20; i++ ) {
      std::cout << "loading segment " << i << std::endl;
      f.loadSegment( i, &listener );
    }
  }
}
