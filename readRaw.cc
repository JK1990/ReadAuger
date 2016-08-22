#include "readRaw.h"
#include "intChecker.h"

#include <string.h>

#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

#include <TFile.h>

readRaw::readRaw(int nFiles, char **fileNames){
    input=new AugerIoSd(nFiles,fileNames);
};

readRaw::~readRaw(){
    delete trace00;
    delete trace01;
    delete trace10;
    delete trace11;
    delete trace20;
    delete trace21;
};

void readRaw::addStationID(int id){
    stationIDs.push_back(id);
};

void readRaw::delStationID(int id){
    for(vector<int>::iterator it=stationIDs.begin();it!=stationIDs.end();++it){
        if(*it==id){
            stationIDs.erase(it);
            break;
        }
    }
};

int readRaw::optionChecker(char* output){
    if(strcmp(output,"-FADC")==0){
        FADC=true;
        return 1;
    }
    if(strcmp(output,"-HISTO")==0){
        HISTO=true;
        return 1;
    }
    if(strcmp(output,"-TIME")==0){
        TIME=true;
        return 1;
    }
    if(strstr(output,"-CH=")==output){
        char* channel=output+4;
        if(isInt(channel)){
            channels.push_back(stoi(channel));
            return 1;
        }
        return 0;
    }
    for(auto &allOut : allOutputs){
        if(strcmp(output,allOut.c_str())==0)return 1;
    }
    for(auto &allOutFile : allOutputFileTypes){
        if(strstr(output,allOutFile.c_str())==output){
            const int len=strlen(output);
            for(int i=0;i<len;i++){
                if(output[i]=='='){
                    if(i<len-1){
                        if(strcmp(allOutFile.c_str(),"-ASCIIOUT")==0){
                            asciiFileName=output+i+1;
                            return 2;
                        }
                        else if(strcmp(allOutFile.c_str(),"-ROOTOUT")==0){
                            rootFileName=output+i+1;
                            return 2;
                        }
                        return 2;
                    }
                }
            }
        }
    }
    return 0;
}

int readRaw::addOption(vector<char*> output){
    for(auto &out : output){
        if(optionChecker(out)==1){
            outputs.push_back(string(out));
            continue;
        }
        if(optionChecker(out)==2){
            continue;
        }
        cout << "Input error: Invalid option '" << out << "' specified" << endl;
        return 1;
    }
    return 0;
};

void readRaw::asciiWriterInit(ofstream& file,string type){
    file << "ID";

    if(TIME){
        file << "\tUTCTime";
    }
    if(type=="FADC"){
        file << "\tGain\tChannel\tFADCTrace\n";
    }
    if(type=="HISTO"){
        file << "\tChannel\tCalibrationHistogram\n";
    }
};

void readRaw::asciiWriterFADC(ofstream& file,TErEvent event,int n){
    vector<int> gains = {IoSdEvent::eHigh,IoSdEvent::eLow};

    for(auto &gain : gains){
        for(auto &channel : channels){
            file << event.fSelectedStations[n]->Id;
            if(TIME){
                file << "\t" << boost::lexical_cast<string>(event.UTCTime);
            }
            file << "\t" << gain << "\t" << channel;
            short* trace = event.fSelectedStations[n]->Fadc->Trace[channel][gain];
            for(int m=0;m<768;m++){
                file << "\t" << trace[m];
            }
            file << "\n";
        }
    }
};

void readRaw::asciiWriterHISTO(ofstream& file,TErEvent event,int n){
    for(auto &channel : channels){
        file << event.fSelectedStations[n]->Id;
        if(TIME){
            file << "\t" << boost::lexical_cast<string>(event.UTCTime);
        }
        file << "\t" << channel;
        unsigned short* trace = event.fSelectedStations[n]->Histo->Charge[channel];
        for(int m=0;m<600;m++){
            file << "\t" << trace[m];
        }
        file << "\n";
    }
}

void readRaw::rootWriterInit(TTree* tree){
    if(FADC){
        if(find(channels.begin(),channels.end(),0)!=channels.end()){
            tree->Branch("FADCTraceCh0Gain0","TH1S",&trace00);
            tree->Branch("FADCTraceCh0Gain1","TH1S",&trace01);
        }
        if(find(channels.begin(),channels.end(),1)!=channels.end()){
            tree->Branch("FADCTraceCh1Gain0","TH1S",&trace10);
            tree->Branch("FADCTraceCh1Gain1","TH1S",&trace11);
        }
        if(find(channels.begin(),channels.end(),2)!=channels.end()){
            tree->Branch("FADCTraceCh2Gain0","TH1S",&trace20);
            tree->Branch("FADCTraceCh2Gain1","TH1S",&trace21);
        }
    }
    if(HISTO){
        if(find(channels.begin(),channels.end(),0)!=channels.end())tree->Branch("CalibrationHistoCh0","TH1F",&hist0);
        if(find(channels.begin(),channels.end(),1)!=channels.end())tree->Branch("CalibrationHistoCh1","TH1F",&hist1);
        if(find(channels.begin(),channels.end(),2)!=channels.end())tree->Branch("CalibrationHistoCh2","TH1F",&hist2);
    }
}

void readRaw::rootWriter(TErEvent ev,TCalibStation station,TTree* tree,int n){
    tree->Branch("ID",&(station.Id),"ID/i");

    if(TIME){
        tree->Branch("Time",&(ev.UTCTime),"ID/i");
    }
    if(HISTO){
        if(find(channels.begin(),channels.end(),0)!=channels.end())hist0=(TH1F*)station.HCharge(0);
        if(find(channels.begin(),channels.end(),1)!=channels.end())hist1=(TH1F*)station.HCharge(1);
        if(find(channels.begin(),channels.end(),2)!=channels.end())hist2=(TH1F*)station.HCharge(2);
    }
    if(FADC){
        short* trace;
        if(find(channels.begin(),channels.end(),0)!=channels.end()){
            trace=station.Fadc->Trace[0][0];
            for(int i=0;i<NFADCSAMPLES;i++){
                trace00->SetBinContent(i+1,trace[i]);
            }
            trace=station.Fadc->Trace[0][1];
            for(int i=0;i<NFADCSAMPLES;i++){
                trace01->SetBinContent(i+1,trace[i]);
            }
        }
        if(find(channels.begin(),channels.end(),1)!=channels.end()){
            trace=station.Fadc->Trace[1][0];
            for(int i=0;i<NFADCSAMPLES;i++){
                trace10->SetBinContent(i+1,trace[i]);
            }
            trace=station.Fadc->Trace[1][1];
            for(int i=0;i<NFADCSAMPLES;i++){
                trace11->SetBinContent(i+1,trace[i]);
            }
        }
        if(find(channels.begin(),channels.end(),2)!=channels.end()){
            trace=station.Fadc->Trace[2][0];
            for(int i=0;i<NFADCSAMPLES;i++){
                trace20->SetBinContent(i+1,trace[i]);
            }
            trace=station.Fadc->Trace[2][1];
            for(int i=0;i<NFADCSAMPLES;i++){
                trace21->SetBinContent(i+1,trace[i]);
            }
        }
    }

    tree->Fill();
}

int readRaw::Run(){
    if(outputs.size()==0){
        cout << "Output error: no output variables specified!" << endl;
        return 1;
    }
    if(channels.size()==0)channels={0,1,2};

    ofstream asciiFileFADC;
    ofstream asciiFileHISTO;
    TFile* rootFile=nullptr;
    TTree* outTree=nullptr;

    if(asciiFileName!=nullptr){
        string strAsciiFileName(asciiFileName);

        if(FADC){
            string strAsciiFileNameFADC=remakeFileName(strAsciiFileName,"_FADC");
            if(!boost::filesystem::exists(strAsciiFileNameFADC))asciiFileFADC.open(strAsciiFileNameFADC.c_str());
            else{
                string in;
                cout << "Output file '" << strAsciiFileNameFADC << "' already exists! Overwrite? [y,N]";
                cin >> in;
                if(in=="y" || in=="Y"){
                    asciiFileFADC.open(strAsciiFileNameFADC.c_str());
                }
                else return 1;
            }
            if(!asciiFileFADC.is_open()){
                cout << "I/O error: Could not open file '" << strAsciiFileNameFADC << "'" << endl;
                return 1;
            }
            asciiWriterInit(asciiFileFADC,"FADC");
        }

        if(HISTO){
            string strAsciiFileNameHISTO=remakeFileName(strAsciiFileName,"_HISTO");
            if(!boost::filesystem::exists(strAsciiFileNameHISTO))asciiFileHISTO.open(strAsciiFileNameHISTO.c_str());
            else{
                string in;
                cout << "Output file '" << strAsciiFileNameHISTO << "' already exists! Overwrite? [y,N]";
                cin >> in;
                if(in=="y" || in=="Y"){
                    asciiFileHISTO.open(strAsciiFileNameHISTO.c_str());
                }
                else return 1;
            }
            if(!asciiFileHISTO.is_open()){
                cout << "I/O error: Could not open file '" << strAsciiFileNameHISTO << "'" << endl;
                return 1;
            }
            asciiWriterInit(asciiFileHISTO,"HISTO");
        }
    }

    if(rootFileName!=nullptr){
        if(!boost::filesystem::exists(rootFileName)) rootFile=new TFile(rootFileName,"RECREATE");
        else{
            string in;
            cout << "Output file '" << rootFileName << "' already exists! Overwrite? [y,N]";
            cin >> in;
            if(in=="y" || in=="Y"){
                rootFile=new TFile(rootFileName,"RECREATE","RawDataFile",9);
            }
            else return 1;
        }
        if(rootFile->IsZombie()){
            cout << "I/O error: Could not open file '" << rootFileName << "'" << endl;
            return 1;
        }
        outTree=new TTree("RawData","Raw Data");
        rootWriterInit(outTree);
    }

    EventPos min=input->FirstEvent();
    EventPos max=input->LastEvent();
    EventPos pos=min;

    while(pos<max){
        IoSdEvent event(pos);
        TErEvent ev(event);

        for(unsigned int i=0;i<ev.fSelectedStations.size();i++){
            if(find(stationIDs.begin(),stationIDs.end(),ev.fSelectedStations[i]->Id)==stationIDs.end())continue;
            TCalibStation station=*(ev.fSelectedStations[i]);

            if(FADC && asciiFileName!=nullptr){
                asciiWriterFADC(asciiFileFADC,ev,i);
            }
            if(HISTO && asciiFileName!=nullptr){
                asciiWriterHISTO(asciiFileHISTO,ev,i);
            }
            if(rootFileName!=nullptr){
                rootWriter(ev,station,outTree,i);
            }
        }
        pos++;
    }
    if(asciiFileFADC.is_open()){
        asciiFileFADC.close();
    }
    if(asciiFileHISTO.is_open()){
        asciiFileHISTO.close();
    }
    if(rootFile!=nullptr){
        rootFile->cd();
        outTree->Write();
        rootFile->Close();
        delete rootFile;
    }

    return 0;
};

string remakeFileName(string fileName,string add){
    size_t pos = fileName.find_last_of(".");

    string reString;

    if(pos!=string::npos){
        reString.append(fileName.substr(0,pos));
        reString.append(add);
        reString.append(fileName.substr(pos));
    }
    else{
        reString.append(fileName);
        reString.append(add);
    }
    return reString;
};

void rawHelp(){
    cout << "RAWDATA Options:\n"
        << "  -TIME\t\tOutput the UTC time\n"
        << "  -FADC\t\tOutput the FADC traces of each event\n"
        << "  -HISTO\tOutput the calibration histograms of each channel\n"
        << "  -CH=[channel]\tSpecifies the output channel. May be used multiple times\n"
        << "               \tIf no channel is specified, all channels will be in the output\n\n" 
        << "  -ROOTOUT=[file]\tOutput will be written to the ROOT file [file]\n"
        << "  -ASCIIOUT=[file]\tOutput will be written to the ASCII file [file]\n"
        << "                  \tThe ASCII-filename will be modified with\n"
        << "                  \t*_FADC for the FADC traces\n"
        << "                  \t*_HISTO for the calibration histograms\n\n";
}
