/*SPSevt2root.h
 *Takes .evt files from nscldaq 11 and converts them into .root files.
 *This version is kept up to date for the Super-Enge Splitpole at FSU, and was built
 *using a framework devised by Nabin, ddc, KTM et. al. in Dec 2015
 *
 *Gordon M. Feb. 2019
 */


#ifndef SPSEVT2ROOT_H
#define SPSEVT2ROOT_H

#include <string>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include <vector>
#include <cstdint>
#include "ADCUnpacker.h"
#include "mTDCUnpacker.h"
#include "TRandom3.h"

using namespace std;

class evt2root {

  public:
    evt2root();
    ~evt2root();
    int run();
 
  private:
    void setParameters();
    void unpack(uint16_t* eventPointer);
    void Reset();
    const int BufferWords = 13328; //I have no idea where this comes from; left over from past ver
    const int BufferBytes = BufferWords*2;
    static const int BufferLength = 26656;//the same value as buffer bytes?
    char buffer[BufferLength];
    Float_t nanos_per_chan = 0.0625;//ps->ns conv. for mtdc
    string fileName;
    TFile *rootFile;
    TTree *DataTree;
    TRandom3 *rand;

    //ROOT branch parameters
    Int_t adc1[32];
    Int_t adc2[32];
    Int_t adc3[32];
    Int_t tdc1[32];
    Int_t mtdc1[32];
    Int_t anode1;
    Int_t anode2;
    Int_t scint1;
    Int_t scint2;
    Int_t cathode;    
    Float_t fp_plane1_tdiff;
    Float_t fp_plane2_tdiff;
    Float_t fp_plane1_tsum;
    Float_t fp_plane2_tsum;
    Float_t fp_plane1_tave;
    Float_t fp_plane2_tave;
    Float_t plastic_sum;
    Float_t anode1_time;
    Float_t anode2_time;
    Float_t plastic_time;

    //geoaddresses
    int adc1_geo;
    int adc2_geo;
    int adc3_geo;
    int tdc1_geo;
    int mtdc1_id;

    //module unpackers
    ADCUnpacker adc_unpacker;
    mTDCUnpacker mtdc_unpacker;
};

#endif
