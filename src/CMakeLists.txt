add_library(tdmspp-osem SHARED log.cpp tdms_file.cpp tdms_segment.cpp)

install (TARGETS tdmspp-osem DESTINATION lib)
install (FILES log.hpp tdms.hpp data_extraction.hpp DESTINATION include)