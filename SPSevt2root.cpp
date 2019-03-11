/*SPSevt2root.cpp
 *Takes .evt files from nscldaq 11 and converts them into .root files.
 *This version is kept up to date for the Super-Enge Splitpole at FSU, and was built
 *using a framework devised by Nabin, ddc, KTM et. al. in Dec 2015
 *
 *Gordon M. Feb. 2019
 */


#include "SPSevt2root.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace std;

//constructor
evt2root::evt2root() {

  cout << "Enter evt list  file: ";
  cin>>fileName;
  adc1_geo = 3;//Set geo addresses here
  adc2_geo = 4;
  adc3_geo = 5;
  tdc1_geo = 8;
  mtdc1_id = 9;
  rand = new TRandom3();

}

/* Reset()
 * Each event needs to be processed separately; so clean the variables
 */
void evt2root::Reset() {
 
  for (int i = 0; i<32; i++) { 
    adc1[i] = 0;
    adc2[i] = 0;
    adc3[i] = 0;
    tdc1[i] = 0;
    mtdc1[i] = 0;
  }
  anode1 = 0;
  anode2 = 0;
  scint1 = 0;
  scint2 = 0;
  cathode = 0;    
  fp_plane1_tdiff = 0.0;
  fp_plane2_tdiff = 0.0;
  fp_plane1_tsum = 0.0;
  fp_plane2_tsum = 0.0;
  fp_plane1_tave = 0.0;
  fp_plane2_tave = 0.0;
  plastic_sum = 0.0;
  anode1_time = 0.0;
  anode2_time = 0.0;
  plastic_time = 0.0;

}


/*setParameters()
 *Does the heavy lifting of setting all non-raw channel paramters.
 */
void evt2root::setParameters() {
  Float_t r[4];
  for (int i=0; i<4; i++) {
    r[i] = rand->Rndm();//converting int to float; add uncert
  }
  Float_t mtdc102 = ((Float_t)mtdc1[2]+r[0])*nanos_per_chan; 
  Float_t mtdc101 = ((Float_t)mtdc1[1]+r[1])*nanos_per_chan;
  Float_t mtdc103 = ((Float_t)mtdc1[3]+r[2])*nanos_per_chan;
  Float_t mtdc104 = ((Float_t)mtdc1[4]+r[3])*nanos_per_chan;
 
  fp_plane1_tdiff = (mtdc102-mtdc101)/2.0;
  fp_plane1_tave = (mtdc102+mtdc101)/2.0;
  fp_plane1_tsum = (mtdc102+mtdc101);
  
  fp_plane2_tdiff = (mtdc104-mtdc103)/2.0;
  fp_plane2_tave = (mtdc104+mtdc103)/2.0;
  fp_plane2_tsum = (mtdc104+mtdc103);
   
  anode1 = adc3[4];
  anode2 = adc3[5];
  scint1 = adc3[6];
  scint2 = adc3[9];
  cathode = adc3[8];

  plastic_sum = ((Float_t)scint1+rand->Rndm())+((Float_t)scint2+rand->Rndm());
  anode1_time = (Float_t)mtdc1[5]+rand->Rndm();
  anode2_time = (Float_t)mtdc1[6]+rand->Rndm();
  plastic_time = (Float_t)mtdc1[7]+rand->Rndm();

}

/*unpack()
 *This is where the file is actually parsed. Takes a short pointer and traverses the .evt file, 
 *calling each of the necessary modules as they are listed in the stack order. Current verison
 *requires knowledge of the stack order; possible area of improvement.
 */
void evt2root::unpack(uint16_t* eventPointer) {

  uint16_t* iterPointer = eventPointer;
  uint32_t numWords = *iterPointer++;
  uint16_t* end =  eventPointer + numWords+1;
  vector<ParsedmTDCEvent> mtdcData;
  vector<ParsedADCEvent> adcData;

  Reset();//wipe variables

  while (iterPointer<end && *iterPointer == 0xffff){
    iterPointer++;
  }
  if (iterPointer<end) {
    auto adc =  adc_unpacker.parse(iterPointer, end, adc1_geo);
    adcData.push_back(adc.second);
    iterPointer = adc.first;
  }

  while (iterPointer<end && *iterPointer == 0xffff){
    iterPointer++;
  }
  if (iterPointer<end) {
    auto adc =  adc_unpacker.parse(iterPointer, end, adc2_geo);
    adcData.push_back(adc.second);
    iterPointer = adc.first;
  }

  while (iterPointer<end && *iterPointer == 0xffff){
    iterPointer++;
  }
  if (iterPointer<end) {
    auto adc =  adc_unpacker.parse(iterPointer, end, adc3_geo);
    adcData.push_back(adc.second);
    iterPointer = adc.first;
  }
  
  while (iterPointer<end && *iterPointer == 0xffff){
    iterPointer++;
  }
  if (iterPointer<end) {
    auto adc =  adc_unpacker.parse(iterPointer, end, tdc1_geo);
    adcData.push_back(adc.second);
    iterPointer = adc.first;
  }
  
  while (iterPointer<end && *iterPointer == 0xffff){
    iterPointer++;
  }
  if (iterPointer<end) {
    auto mtdc = mtdc_unpacker.parse(iterPointer, end, mtdc1_id);
    mtdcData.push_back(mtdc.second);
    iterPointer = mtdc.first;
  }

  for (auto& event : adcData) {
    for (auto& chanData : event.s_data) {
      if (event.s_geo == adc1_geo) {
         adc1[chanData.first] = chanData.second;
      }
      if (event.s_geo == adc2_geo) {
        adc2[chanData.first] = chanData.second;
      }
      if (event.s_geo == adc3_geo) {
        adc3[chanData.first] = chanData.second;
      }
      if (event.s_geo == tdc1_geo) {
        tdc1[chanData.first] = chanData.second;
      }
    }
  }

  for(auto& event : mtdcData) {
    for(auto& chanData : event.s_data) {
      mtdc1[chanData.first] = chanData.second;
    }
  }

  setParameters();
  DataTree->Fill();
}

/*run()
 *function to be called at exectuion. Takes the list of evt files and opens them one at a time,
 *calls unpack() to unpack them, and then either completes or moves on to the next evt file.
 *If a condition is not met, returns 0.
 */
int evt2root::run() {

  DataTree = new TTree("DataTree", "DataTree");
  ifstream evtListFile;
  evtListFile.open(fileName.c_str());
  if (evtListFile.is_open()) {
   cout << "Successfully opened: "<< fileName << endl;
  } else { 
   cout<<"Unable to open evt list input file"<<endl;
   return 0;
  }

  string temp;
  evtListFile>>temp;
  char rootName[temp.size()+1];
  strcpy(rootName, temp.c_str());
  
  rootFile = new TFile(rootName, "RECREATE");
  cout<<"ROOT File: "<<temp<<endl;
  
  //Add branches here
  DataTree->Branch("adc1", &adc1, "adc1[32]/I");
  DataTree->Branch("adc2", &adc2, "adc2[32]/I");
  DataTree->Branch("adc3", &adc3, "adc3[32]/I");
  DataTree->Branch("tdc1", &tdc1, "tdc1[32]/I");
  DataTree->Branch("mtdc1", &mtdc1, "mtdc1[32]/I");
  DataTree->Branch("fp_plane1_tdiff", &fp_plane1_tdiff, "fp_plane1_tdiff/F");
  DataTree->Branch("fp_plane1_tsum", &fp_plane1_tsum, "fp_plane1_tsum/F");
  DataTree->Branch("fp_plane1_tave", &fp_plane1_tdiff, "fp_plane1_tave/F");
  DataTree->Branch("fp_plane2_tdiff", &fp_plane2_tdiff, "fp_plane2_tdiff/F");
  DataTree->Branch("fp_plane2_tsum", &fp_plane2_tsum, "fp_plane2_tsum/F");
  DataTree->Branch("fp_plane2_tave", &fp_plane2_tdiff, "fp_plane2_tave/F");
  DataTree->Branch("anode1", &anode1, "anode1/I");
  DataTree->Branch("anode2", &anode2, "anode2/I");
  DataTree->Branch("scint1", &scint1, "scint1/I");
  DataTree->Branch("scint2", &scint2, "scint2/I");
  DataTree->Branch("cathode", &cathode, "cathode/I");
  DataTree->Branch("plastic_sum", &plastic_sum, "plastic_sum/F");
  DataTree->Branch("anode1_time", &anode1_time, "anode1_time/F");
  DataTree->Branch("anode2_time", &anode2_time, "anode2_time/F");
  DataTree->Branch("plastic_time", &plastic_time, "plastic_time/F");
  
  string evtName; 
  evtListFile >> evtName;

  int physBuffers = 0; //can report number of event buffers; consistency check with spectcl
        
  while (!evtListFile.eof()) {
    ifstream evtFile;
    evtFile.clear(); //make sure that evtFile is always empty before trying a new one
    evtFile.open(evtName.c_str(), ios::binary);
    if (evtFile.is_open()) {
      cout<<"evt file: "<<evtName<<endl;
      while (!evtFile.eof()) {
        evtFile.read(buffer, 8);//take first 8 characters
        evtFile.read(buffer+8, *(uint32_t*)buffer-8);//read the remainder
        uint32_t subheader = *(uint32_t*)(buffer+8); //pull the subheader

        if (subheader>0) {
          cout <<"Unexpected subheader: " << subheader << endl; //relic from old version
        }

        auto eventPointer = ((uint16_t*)buffer)+6;//where we try to start a phys event
        auto bufferType = *(unsigned int*)(buffer+4);//determine what part of the file we're at
        int runNum;

        switch (bufferType) {
          case 30: //Physics event buffer
             unpack(eventPointer);
             physBuffers++;
             break;
          case 1: //start of run buffer
            runNum = *(eventPointer+8);
            cout <<"Run number = "<<runNum<<endl;
            cout <<"Should match with file name: " <<evtName<<endl;
            break;
        }
      }
    } else{
      cout<<"Unable to open evt file: "<<evtName<<endl;
      rootFile->Close();
      return 0;
    }
    cout<<"Number of physics buffers: "<<physBuffers<<endl;
    evtFile.close();
    evtFile.clear();
    evtListFile >> evtName;
  }

  DataTree->Write();
  rootFile->Close();
  return 1;
}
