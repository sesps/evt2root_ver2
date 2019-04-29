/*ADCUnpacker.h
 *Class to parse through data coming from ADC 
 *Most of the program's structure borrowed from 
 *http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/x5013.html
 *Ken H. & Gordon M.
 *Oct 2018
 *
 *This particular version is designed to work with SPSevt2root and uses 16-bit integer pointers 
 *to traverse, instead of the spectcl 32-bit translator pointers. Possible area for improvement?
 *Gordon M. Feb 2019
 *
 *Updated to better handle errors and work with a more general version of evt2root
 *Gordon M. April 2019
 *
 *NOTE: This version is specifically designed to unpack data that come in reverse!!
 */


#ifndef adcunpacker_h
#define adcunpacker_h

#include <vector>
#include <utility>
#include <cstdint>
#include <stdexcept>

struct ParsedADCEvent {
  int s_geo;
  int s_crate;
  int s_count;
  int s_eventNumber;   
  std::vector<std::pair<int, std::uint16_t>> s_data;
};

class ADCUnpacker {
  public:
    std::pair< uint16_t*, ParsedADCEvent> parse( uint16_t* begin,  uint16_t* end,
                                                 std::vector<int> geos);
    bool isHeader(std::uint16_t word);

  private:
    bool isData(std::uint16_t word);
    bool isEOE(std::uint16_t word); 
   
    void unpackHeader(std::uint16_t* word, ParsedADCEvent& event);
    void unpackDatum(std::uint16_t* word, ParsedADCEvent& event); 
    uint16_t* unpackData( uint16_t* begin,  uint16_t* end, ParsedADCEvent& event); 
};

#endif
        
