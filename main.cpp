#include "SPSevt2root.h"
#include <TROOT.h>
#include <TApplication.h>
#include <string>
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
  TApplication app("app", &argc, argv);//if someone wants root graphics
  evt2root converter;
  cout<<"---------------SPS evt2root---------------"<<endl;
  converter.run();
}
