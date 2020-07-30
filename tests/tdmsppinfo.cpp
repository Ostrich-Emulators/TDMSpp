#include <iostream>
#include <vector>

#include <tdms.hpp>
#include <log.hpp>
#include <iomanip>

#include "optionparser.h"

// Define options

enum optionIndex{
  UNKNOWN, HELP, PROPERTIES, DEBUG, DATA, SIGNAL
};

const option::Descriptor usage[] = {
  {UNKNOWN, 0, "", "", option::Arg::None, "USAGE: tdmsppinfo [options] filename ...\n\nOptions:" },
  {HELP, 0, "h", "help", option::Arg::None, "  --help, \tPrint usage and exit." },
  {PROPERTIES, 0, "p", "properties", option::Arg::None, "  --properties, \tPrint channel properties." },
  {DEBUG, 0, "d", "debug", option::Arg::None, "  --debug, \tPrint debugging information to stderr." },
  {DATA, 0, "D", "data", option::Arg::None, "  --data, \tPrint data (BIG!)." },
  {SIGNAL, 0, "s", "signal", option::Arg::Optional, "  --signal, \tOnly look at this signal." },
  {0, 0, 0, 0, 0, 0 }
};

class listener : public TDMS::listener{
public:
  bool printdata = false;
  std::string signal;

  virtual void data( const std::string& channelname, const unsigned char* datablock, TDMS::data_type_t datatype, size_t num_vals ) override {
    if ( signal.empty( ) || !( signal.empty( ) || std::string::npos == channelname.find( signal ) ) ) {
      std::cout << "reading " << num_vals << " for channel: " << channelname << std::endl;

      if ( printdata ) {
        std::vector<double> vals;
        vals.reserve( num_vals );

        //    std::cout << "\t";
        //    for( size_t i=0; i< num_vals; i++ ){
        //      double out;
        //      memcpy(&out, datablock+(i*datatype.length), datatype.length);
        //      //vals.push_back(out);
        //
        //      if( i< 50 ){
        //        std::cout << out << " ";
        //      }
        //
        //    }

        memcpy( &vals[0], datablock, datatype.length * num_vals );

        std::cout << channelname << std::endl;
        for ( size_t i = 0; i < num_vals; i++ ) {
          std::cout << "  " << std::setprecision( 4 ) << std::fixed << vals[i] << std::endl;
        }
      }
    }
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
  for ( int i = 0; i < parse.nonOptionsCount( ); ++i ) {
    _filenames.push_back( parse.nonOption( i ) );
  }
  for ( std::string filename : _filenames ) {
    if ( _filenames.size( ) > 1 )
      std::cout << filename << ":" << std::endl;
    TDMS::tdmsfile f( filename );
    std::cout << f.segments( ) << " segments parsed" << std::endl;

    for ( const auto& o : f ) {
      std::cout << o->get_path( ) << std::endl;
      if ( options[PROPERTIES] ) {
        for ( auto p : o->get_properties( ) ) {
          TDMS::data_type_t valtype = p.second->data_type;

          if ( valtype.name == "tdsTypeString" ) {
            std::cout << "  " << p.first << " (string): " << p.second->asString( ) << std::endl;
          }
          else if ( valtype.name == "tdsTypeDoubleFloat" ) {
            std::cout << "  " << p.first << " (double): " << p.second->asDouble( ) << std::endl;
          }
          else if ( valtype.name == "tdsTypeTimeStamp" ) {
            time_t timer = p.second->asUTCTimestamp( );
            tm * pt = gmtime( &timer );

            char buffer[80];
            sprintf( buffer, "%d.%02d.%d %02d:%02d:%02d,%f",
                pt->tm_mday, pt->tm_mon + 1, 1900 + pt->tm_year, pt->tm_hour, pt->tm_min, pt->tm_sec, 0.0 );
            std::cout << "  " << p.first << " (timestamp): " << buffer << std::endl;
          }
          else {
            std::cout << "  " << p.first << "(" << valtype.name << "): <unhandled>" << std::endl;
          }
        }
      }

      std::cout << o->number_values( ) << " values present ("
          << o->bytes( ) << ")"
          << std::endl;
    }

    listener listener;
    if ( options[DATA] ) {
      listener.printdata = true;
    }
    if ( options[SIGNAL] ) {
      listener.signal = options[SIGNAL].arg;
    }

    for ( size_t i = 0; i < f.segments( ); i++ ) {
      //std::cout << "loading segment " << i << std::endl;
      f.loadSegment( i, &listener );
    }
  }
}
