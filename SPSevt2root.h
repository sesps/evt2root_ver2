/*SPSevt2root.h
 *Takes .evt files from nscldaq 11 and converts them into .root files.
 *This version is kept up to date for the Super-Enge Splitpole at FSU, and was built
 *using a framework devised by Nabin, ddc, KTM et. al. in Dec 2015
 *Also uses information provided by NSCL at:
 *http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.0/index.html
 *
 *Gordon M. Feb. 2019
 *
 *Updated to properly address ringbuffers, cut down on dynamic memory allocation,
 *and remove dependance on stack ordering
 *Gordon M. April 2019
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
    void unpack(uint16_t* eventPointer, uint32_t ringSize);
    void Reset();
    void Rebin(vector<Int_t> &module);
    Float_t nanos_per_chan = 0.0625;//ps->ns conv. for mtdc
    string fileName;
    TFile *rootFile;
    TTree *DataTree;
    TRandom3 *rand;

    //ROOT branch parameters
    vector<Int_t> adc1, adc2, adc3, tdc1, mtdc1;
    Int_t anode1, anode2, scint1, scint2, cathode;    
    Float_t fp_plane1_tdiff, fp_plane2_tdiff, fp_plane1_tsum, fp_plane2_tsum,
            fp_plane1_tave, fp_plane2_tave, plastic_sum, anode1_time, anode2_time,
             plastic_time;

    //geoaddresses
    int adc1_geo, adc2_geo, adc3_geo, tdc1_geo, mtdc1_id;
    vector<int> adc_geos;

    //module unpackers
    ADCUnpacker adc_unpacker;
    mTDCUnpacker mtdc_unpacker;
};

#endif
