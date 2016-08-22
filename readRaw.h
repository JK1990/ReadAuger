#ifndef READRAW_H
#define READRAW_H

#include <vector>

#include <TTree.h>

#include "Er.h"
#include "Ec.h"
#include "IoAuger.h"

using namespace std;

class readRaw{
    public:
        readRaw(int nFiles,char **fileNames);

        ~readRaw();

        void addStationID(int id);
        void delStationID(int id);

        int addOption(vector<char*> output);

        int Run();
    private:
        vector<int> stationIDs;

        vector<string> outputs;

        vector<string> allOutputs={"-FADC"};
        vector<string> allOutputFileTypes={"-ROOTOUT","-ASCIIOUT"};

        vector<int> channels;

        int optionChecker(char* outpuit);

        char* asciiFileName=nullptr;
        char* rootFileName=nullptr;

        AugerIoSd* input=nullptr;

        void asciiWriterInit(ofstream& file,string type);
        void asciiWriterFADC(ofstream& file,TErEvent event,int n);
        void asciiWriterHISTO(ofstream& file,TErEvent event,int n);

        void rootWriterInit(TTree* tree);
        void rootWriter(TErEvent ev,TCalibStation station,TTree* tree,int n);

        bool FADC=false;
        bool HISTO=false;
        bool TIME=false;

        const int NCHANNELS=3;
        const int NGAIN=2;
        const int NFADCSAMPLES=768;

        TH1F* hist0=nullptr;
        TH1F* hist1=nullptr;
        TH1F* hist2=nullptr;

        TH1S* trace00=new TH1S("traceCh0Gain0","FADC Trace Ch0 LowGain",NFADCSAMPLES,0,NFADCSAMPLES-1);
        TH1S* trace01=new TH1S("traceCh0Gain1","FADC Trace Ch0 HighGain",NFADCSAMPLES,0,NFADCSAMPLES-1);
        TH1S* trace10=new TH1S("traceCh1Gain0","FADC Trace Ch1 LowGain",NFADCSAMPLES,0,NFADCSAMPLES-1);
        TH1S* trace11=new TH1S("traceCh1Gain1","FADC Trace Ch1 HighGain",NFADCSAMPLES,0,NFADCSAMPLES-1);
        TH1S* trace20=new TH1S("traceCh2Gain0","FADC Trace Ch2 LowGain",NFADCSAMPLES,0,NFADCSAMPLES-1);
        TH1S* trace21=new TH1S("traceCh2Gain1","FADC Trace Ch2 HighGain",NFADCSAMPLES,0,NFADCSAMPLES-1);
};

string remakeFileName(string fileName,string add);

void rawHelp();

#endif //READRAW_H
