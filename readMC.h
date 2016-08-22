#ifndef READMC_H
#define READMC_H

#include <vector>
#include <memory>

#include <TTree.h>
#include <TFile.h>

#include "TChain.h"
#include "TSDMonCal.h"

using namespace std;

typedef shared_ptr<TChain> ptrTChain;

class readMC{
    public:
        readMC(int nFiles,char **fileNames);

        ~readMC();
        
        void addStationID(int id);
        void delStationID(int id);

        int addOption(vector<char*> output);

        int Run();

    private:
        vector<int> stationIDs; // Will hold the specified stations IDs for the output

        //Needed to read the Auger file format
        vector<ptrTChain> fChains;
        TSDMonCal SDMonCal;
        TSDMonCal *fSDMonCal;

        vector<string> outputs; // Will hold the specified outputs

        vector<string> allOutputs={"-PMTV","-PMTI","-PMTT","-TIME","-ADCBASELINE","-PSU","-ELECBOX","-BATTERY","-SOLAR","-WATER","-VEM","-OTHER"}; //To add new outputs see note at bottom of this file (just to check the given options)
        vector<string> allOutputFileTypes={"-ASCIIOUT","-ROOTOUT"}; //The possible output formats (just to check the given options)

        // The specified filenames
        char* asciiFileName=nullptr;
        char* rootFileName=nullptr;

        // Checks if the given options are correct
        int optionChecker(char* output);

        // Functions to write the ASCII files
        void asciiWriterInit(ofstream& file);
        void asciiWriter(ofstream& file);

        // Functions to write the ROOT files
        void rootWriterInit();
        void rootWriter();
        TTree* outTree=new TTree("MCData","MC data");

        // Will hold the values to be written to the output files
        vector<float> outVals;
};

void mcHelp();

#endif //READMC_H

/* How to add a new output:
 * 1. Add the output flag to "allOutputs" in this file
 * 2. Add the needed header strings to "readMC::asciiWriterInit" in readMC.cc
 * 3. Add the needed vlues to "readMC::rootWriter" in readMC.cc
 * 3. Add the needed values in "readMC::Run" in readMC.cc
 *    The values have to be added to vector "outVals"
 * Important note:
 *    The order of calling the flags has to be the same in "readMC::asciiWriterInit",
 *    "readMC::rootWriter" and "readMC::Run"
 */
