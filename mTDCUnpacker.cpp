/*
mTDCUnpacker.cpp

Class to parse through data coming from mtdc 
Most of the program's structure borrowed from http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/x5013.html
Ken H. & Gordon M.
Nov 2018

This particular version is designed to work with SPSevt2root and uses 16-bit integer pointers to 
traverse, instead of the spectcl 32-bit translator pointers. Possible area for improvement?
Gordon M. Feb 2019

NOTE: This version is specifically designed to unpack data that come in reverse!!
*/
#include "mTDCUnpacker.h"
#include <string>
#include <stdexcept>
#include <iostream>

using namespace std;

//This is the main place where changes need to be made from one module to another; all else mostly name changes
//useful masks and shifts for mTDC:
//in spectcl is just 0xc000, here use f to remove readout errors
static const uint16_t TYPE_MASK (0xf000); 
static const uint16_t TYPE_HDR (0x4000);
static const uint16_t TYPE_DATA (0x0000);
static const uint16_t TYPE_TRAIL (0xc000);


//header-specific:
static const unsigned HDR_ID_SHIFT (0);
static const uint16_t HDR_ID_MASK (0x00ff);
static const unsigned HDR_RES_SHIFT (12);
static const uint16_t HDR_RES_MASK (0xf000);
static const unsigned HDR_COUNT_SHIFT (0);
static const uint16_t HDR_COUNT_MASK (0x03ff);

//data-specific:
static const unsigned DATA_CHANSHIFT (0);
static const uint16_t DATA_CHANMASK (0x001f);
static const uint16_t DATA_CONVMASK (0xffff);


pair< uint16_t*, ParsedmTDCEvent> mTDCUnpacker::parse( uint16_t* begin,  uint16_t* end, int id) {

  ParsedmTDCEvent event;

  auto iter = begin;
  int bad_flag;
  unpackHeader(iter, event);
  if (iter > end){
    bad_flag =1;
  }
  iter += 2;

  int nWords = (event.s_count-1)*2;//count includes the eob 
  auto dataEnd = iter + nWords;

  if (event.s_id != id) {//If unexpected id, skip data; either bad event or bad stack
    bad_flag = 1;
    iter+=nWords;
    //Error testing
    //cout<<"Bad mTDC id: "<<event.s_id<<" Expected: "<<id<<endl;
  } else {
    iter = unpackData(iter, dataEnd, event);
  }
  if (iter>end || bad_flag || !isEOE(*(iter+1))){
    //Error testing
    //cout<<"mTDCUnpacker::parse() ";
    //cout<<"Unable to unpack event"<<endl;
  }
  iter +=2;

  return make_pair(iter, event);

}

bool mTDCUnpacker::isHeader(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_HDR);
}

void mTDCUnpacker::unpackHeader(uint16_t* word, ParsedmTDCEvent& event) {
  //Error handling: if not valid header, throw to 0 at chan 0 and invalid id
  try{ 
    if (!isHeader(*(word+1))) {
      string errmsg("mTDCUnpacker::parseHeader() ");
      errmsg += "Found non-header word when expecting header. ";
      errmsg +="Word = ";
      unsigned int w = *(word+1);
      errmsg += to_string(w);
      throw errmsg;
    }
    event.s_res = (*word&HDR_RES_MASK) >> HDR_RES_SHIFT;
    event.s_count = (*word&HDR_COUNT_MASK) >> HDR_COUNT_SHIFT;
    word++;
    event.s_id = (*word&HDR_ID_MASK)>>HDR_ID_SHIFT;
  } catch(string errmsg) {
    event.s_res = 0;
    event.s_count = 1;
    event.s_id = 99; //should NEVER match valid id
    uint16_t data = 0;
    int channel = 0;
    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
    //cout<<errmsg<<endl;//only use if testing
  }

}

bool mTDCUnpacker::isData(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_DATA );
}

void mTDCUnpacker::unpackDatum(uint16_t* word, ParsedmTDCEvent& event) {
  //Error handling: if not valid data, throw again
  try {
    if (!isData(*(word+1))) {
      string errmsg("mTDCUnpacker::unpackDatum() ");
      errmsg += "Found non-data word when expecting data.";
      throw errmsg;
    }
    uint16_t data = *word&DATA_CONVMASK;
    ++word;
    int channel = (*word&DATA_CHANMASK) >> DATA_CHANSHIFT;
    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
  } catch(string errmsg) {
    event.s_res = 0;
    event.s_count = 1;
    event.s_id = 99; //should NEVER match valid id
    uint16_t data = 0;
    int channel = 0;
    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
    //cout<<errmsg<<endl;//only use if testing
  }
  
}

 uint16_t* mTDCUnpacker::unpackData( uint16_t* begin, uint16_t* end, ParsedmTDCEvent& event) {

  event.s_data.reserve(event.s_count+1); //memory allocation
  auto iter = begin;
  while (iter<end) {
    unpackDatum(iter, event);
    iter = iter+2;
  }

  return iter;

}

bool mTDCUnpacker::isEOE(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_TRAIL);
}


