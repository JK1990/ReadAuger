#include "readMC.h"

#include <memory>
#include <fstream>

#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

readMC::readMC(int nFiles,char **fileNames){
    for(int n=0;n<nFiles;n++){
        TChain* fChain = new TChain("SDMonCal");
        fChain->SetBranchStatus("fRawMonitoring.fListOfMembers",0);
        fChain->SetBranchStatus("fCalibration.fListOfMembers",0);
        fChain->Add(fileNames[n]);

        ptrTChain ptrfChain (fChain);

        fChains.push_back(ptrfChain);
    }
    fSDMonCal=&SDMonCal;
};

readMC::~readMC(){
    delete outTree;
}

void readMC::addStationID(int id){
    stationIDs.push_back(id);
}

void readMC::delStationID(int id){
    for(vector<int>::iterator it=stationIDs.begin();it!=stationIDs.end();++it){
        if(*it==id){
            stationIDs.erase(it);
            break;
        }
    }
}

/* Checks if the option format for the output is correct
 * Returns 1 if the option is only the output type
 * Returns 2 if the option determines the output file type
 */
int readMC::optionChecker(char* output){
    for(auto &allOut : allOutputs){
        if(strcmp(output,allOut.c_str())==0)return 1;
    }

    for(auto &allOutFile : allOutputFileTypes){
        if(strstr(output,allOutFile.c_str())==output){
            const int len=strlen(output);
            for(int i=0;i<len;i++){
                if(output[i]=='='){
                    if(i<len-1){
                        if(strcmp(allOutFile.c_str(),"-ASCIIOUT")==0) {
                            asciiFileName=output+i+1;
                            return 2;
                        }
                        else if(strcmp(allOutFile.c_str(),"-ROOTOUT")==0) {
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

int readMC::addOption(vector<char*> output){
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
}

/* Initiliazes the writing of the ASCII file.
 * So far it only writes the header containing
 * information about the data in each column.
 */
void readMC::asciiWriterInit(ofstream& file){
    file << "ID";

    if(find(outputs.begin(),outputs.end(),"-TIME")!=outputs.end()){
        file << "\tTime\tCDASTime";
    }
    if(find(outputs.begin(),outputs.end(),"-PMTV")!=outputs.end()){
        file << "\tPMTV1\tPMTV2\tPMTV3";
    }
    if(find(outputs.begin(),outputs.end(),"-PMTI")!=outputs.end()){
        file << "\tPMTI1\tPMTI2\tPMTI3";
    }
    if(find(outputs.begin(),outputs.end(),"-PMTT")!=outputs.end()){
        file << "\tPMTT1\tPMTT2\tPMTT3";
    }
    if(find(outputs.begin(),outputs.end(),"-ADCBASELINE")!=outputs.end()){
        file << "\tADCBaseline";
    }
    if(find(outputs.begin(),outputs.end(),"-PSU")!=outputs.end()){
        file << "\t3.3V\t-3.3V\t5V\t12V\t24V";
    }
    if(find(outputs.begin(),outputs.end(),"-ELECBOX")!=outputs.end()){
        file << "\tElectT";
    }
    if(find(outputs.begin(),outputs.end(),"-BATTERY")!=outputs.end()){
        file << "\tBatteryT0\tBatteryT1\tBatteryV0\tBatteryV1";
    }
    if(find(outputs.begin(),outputs.end(),"-SOLAR")!=outputs.end()){
        file << "\tSolarPanelV\tSolarPanelI";
    }
    if(find(outputs.begin(),outputs.end(),"-WATER")!=outputs.end()){
        file << "\tWaterLevel\tWaterT";
    }
    if(find(outputs.begin(),outputs.end(),"-VEM")!=outputs.end()){
        file << "\tVEMCharge0\tVEMCharge1\tVEMCharge2\tVEMPeak0\tVEMPeak1\tVEMPeak2";
    }
    if(find(outputs.begin(),outputs.end(),"-OTHER")!=outputs.end()){
        file << "\tCurrentLoad\tDAC4Voltage\t3.3VAnalogPower\t12VMultiplexer\t12VRadio";
    }
    file << "\n";
}

/* Writes the ASCII data to the file */
void readMC::asciiWriter(ofstream& file){
    cout.precision(9);
    for(auto &val : outVals){
        file << boost::lexical_cast<string>(val) << "\t";
    }
    file << "\n";
}

/* Does nothing but maybe useful later */
void readMC::rootWriterInit(){
    int n=0;

    outTree->Branch("ID",&(outVals[n]),"ID/F");
    n++;

    if(find(outputs.begin(),outputs.end(),"-TIME")!=outputs.end()){
        outTree->Branch("Time",&(outVals[n]),"Time/F");
        outTree->Branch("CDASTime",&(outVals[n+1]),"CDASTime/F");
        n+=2;
    }
    if(find(outputs.begin(),outputs.end(),"-PMTV")!=outputs.end()){
        outTree->Branch("PMTV1",&(outVals[n]),"PMTV1/F");
        outTree->Branch("PMTV2",&(outVals[n+1]),"PMTV2/F");
        outTree->Branch("PMTV3",&(outVals[n+2]),"PMTV3/F");
        n+=3;
    }
    if(find(outputs.begin(),outputs.end(),"-PMTI")!=outputs.end()){
        outTree->Branch("PMTI1",&(outVals[n]),"PMTI1/F");
        outTree->Branch("PMTI2",&(outVals[n+1]),"PMTI2/F");
        outTree->Branch("PMTI3",&(outVals[n+2]),"PMTI3/F");
        n+=3;
    }
    if(find(outputs.begin(),outputs.end(),"-PMTT")!=outputs.end()){
        outTree->Branch("PMTT1",&(outVals[n]),"PMTT1/F");
        outTree->Branch("PMTT2",&(outVals[n+1]),"PMTT2/F");
        outTree->Branch("PMTT3",&(outVals[n+2]),"PMTT3/F");
        n+=3;
    }
    if(find(outputs.begin(),outputs.end(),"-ADCBASELINE")!=outputs.end()){
        outTree->Branch("ADCBaseline",&(outVals[n]),"ADCBaseline/F");
        n++;
    }
    if(find(outputs.begin(),outputs.end(),"-PSU")!=outputs.end()){
        outTree->Branch("3.3V",&(outVals[n]),"3.3V/F");
        outTree->Branch("-3.3V",&(outVals[n+1]),"-3.3V/F");
        outTree->Branch("5V",&(outVals[n+2]),"5V/F");
        outTree->Branch("12V",&(outVals[n+3]),"12V/F");
        outTree->Branch("24V",&(outVals[n+4]),"24V/F");
        n+=5;
    }
    if(find(outputs.begin(),outputs.end(),"-ELECBOX")!=outputs.end()){
        outTree->Branch("ElectBoxT",&(outVals[n]),"ElecBoxT/F");
        n++;
    }
    if(find(outputs.begin(),outputs.end(),"-BATTERY")!=outputs.end()){
        outTree->Branch("BatteryT0",&(outVals[n]),"BatteryT0/F");
        outTree->Branch("BatteryT1",&(outVals[n+1]),"BatteryT1/F");
        outTree->Branch("BatteryV0",&(outVals[n+2]),"BatteryV0/F");
        outTree->Branch("BatteryV1",&(outVals[n+3]),"BatteryV1/F");
        n+=4;
    }
    if(find(outputs.begin(),outputs.end(),"-SOLAR")!=outputs.end()){
        outTree->Branch("SolarPanelV",&(outVals[n]),"SolarPanelV/F");
        outTree->Branch("SolarPanelI",&(outVals[n+1]),"SolarPanelI/F");
        n+=2;
    }
    if(find(outputs.begin(),outputs.end(),"-WATER")!=outputs.end()){
        outTree->Branch("WaterLevel",&(outVals[n]),"WaterLevel/F");
        outTree->Branch("WaterT",&(outVals[n+1]),"WaterT/F");
        n+=2;
    }
    if(find(outputs.begin(),outputs.end(),"-VEM")!=outputs.end()){
        outTree->Branch("VEMCharge0",&(outVals[n]),"VEMCharge0/F");
        outTree->Branch("VEMCharge1",&(outVals[n+1]),"VEMCharge1/F");
        outTree->Branch("VEMCharge2",&(outVals[n+2]),"VEMCharge2/F");
        outTree->Branch("VEMPeak0",&(outVals[n+3]),"VEMPeak0/F");
        outTree->Branch("VEMPeak1",&(outVals[n+4]),"VEMPeak1/F");
        outTree->Branch("VEMPeak2",&(outVals[n+5]),"VEMPeak2/F");
        n+=6;
    }
    if(find(outputs.begin(),outputs.end(),"-OTHER")!=outputs.end()){
        outTree->Branch("CurrentLoad",&(outVals[n]),"CurrentLoad/F");
        outTree->Branch("DAC4Voltage",&(outVals[n+1]),"DAC4Voltage/F");
        outTree->Branch("3.3VAnalogPower",&(outVals[n+2]),"3.3VAnalogPower/F");
        outTree->Branch("12VMultiplexer",&(outVals[n+3]),"12VMultiplexer/F");
        outTree->Branch("12VRadio",&(outVals[n+4]),"12VRadio/F");
        n+=5;
    }
}

/* Writes the rootData to the TTree */
void readMC::rootWriter(){
    outTree->Fill();
}

int readMC::Run(){
    if(outputs.size()==0){
        cout << "Output error: no output variables specified!" << endl;
        return 1;
    }
    ofstream asciiFile;
    TFile* rootFile=nullptr;

    if(asciiFileName!=nullptr){
        if(!boost::filesystem::exists(asciiFileName))asciiFile.open(asciiFileName);
        else{
            string in;
            cout << "Output file '" << asciiFileName << "' already exists! Overwrite? [y,N]";
            cin >> in;
            if(in=="y" || in=="Y"){
                asciiFile.open(asciiFileName);
            }
            else return 1;
        }
        if(!asciiFile.is_open()){
            cout << "I/O error: Could not open file '" << asciiFileName << "'" << endl;
            return 1;
        }
        asciiWriterInit(asciiFile);
    }

    if(rootFileName!=nullptr){
        if(!boost::filesystem::exists(rootFileName)) rootFile=new TFile(rootFileName,"RECREATE");
        else{
            string in;
            cout << "Output file '" << rootFileName << "' already exists! Overwrite? [y,N]";
            cin >> in;
            if(in=="y" || in=="Y"){
                rootFile=new TFile(rootFileName,"RECREATE");
            }
            else return 1;
        }
        if(rootFile->IsZombie()){
            cout << "I/O error: Could not open file '" << rootFileName << "'" << endl;
            return 1;
        }
    }

    for(auto &fChain : fChains){
        int count=0;
        fChain->SetBranchAddress("SDMonCalBranch",&fSDMonCal);
        int fNumberOfEntries = (int)fChain->GetEntries();

        for(int i=0;i<fNumberOfEntries;i++){
            fChain->GetEvent(i);

            //Check if monitoring block is filled
            if(fSDMonCal->fMonitoring.fIsMonitoring!=1 || fSDMonCal->fCalibration.fIsCalibration!=1)continue;

            //Check for the correct station IDs
            int id = fSDMonCal->fLsId;
            if(find(stationIDs.begin(),stationIDs.end(),id)==stationIDs.end())continue;

            TSDMonitoring* m=&(fSDMonCal->fMonitoring);
            TSDCalibration* c=&(fSDMonCal->fCalibration);
            outVals.clear();
            outVals.push_back((float)id);
            if(find(outputs.begin(),outputs.end(),"-TIME")!=outputs.end()){
                outVals.push_back(fSDMonCal->fTime);
                outVals.push_back(fSDMonCal->fCDASTime);
            }
            if(find(outputs.begin(),outputs.end(),"-PMTV")!=outputs.end()){
                outVals.push_back(m->fPMV[0]);
                outVals.push_back(m->fPMV[1]);
                outVals.push_back(m->fPMV[2]);
            }
            if(find(outputs.begin(),outputs.end(),"-PMTI")!=outputs.end()){
                outVals.push_back(m->fPMI[0]);
                outVals.push_back(m->fPMI[1]);
                outVals.push_back(m->fPMI[2]);
            }
            if(find(outputs.begin(),outputs.end(),"-PMTT")!=outputs.end()){
                outVals.push_back(m->fPMT[0]);
                outVals.push_back(m->fPMT[1]);
                outVals.push_back(m->fPMT[2]);
            }
            if(find(outputs.begin(),outputs.end(),"-ADCBASELINE")!=outputs.end()){
                outVals.push_back(m->fADCBaseline);
            }
            if(find(outputs.begin(),outputs.end(),"-PSU")!=outputs.end()){
                outVals.push_back(m->f3V);
                outVals.push_back(m->f_3V);
                outVals.push_back(m->f5V);
                outVals.push_back(m->f12V);
                outVals.push_back(m->f24V);
            }
            if(find(outputs.begin(),outputs.end(),"-ELECBOX")!=outputs.end()){
                outVals.push_back(m->fElectT);
            }
            if(find(outputs.begin(),outputs.end(),"-BATTERY")!=outputs.end()){
                outVals.push_back(m->fBatteryT[0]);
                outVals.push_back(m->fBatteryT[1]);
                outVals.push_back(m->fBatteryV[0]);
                outVals.push_back(m->fBatteryV[1]);
            }
            if(find(outputs.begin(),outputs.end(),"-SOLAR")!=outputs.end()){
                outVals.push_back(m->fSolarPanelV);
                outVals.push_back(m->fSolarPanelI);
            }
            if(find(outputs.begin(),outputs.end(),"-WATER")!=outputs.end()){
                outVals.push_back(m->fWaterLevel);
                outVals.push_back(m->fWaterT);
            }
            if(find(outputs.begin(),outputs.end(),"-VEM")!=outputs.end()){
                outVals.push_back(c->fArea[0]);
                outVals.push_back(c->fArea[1]);
                outVals.push_back(c->fArea[2]);
                outVals.push_back(c->fPeak[0]);
                outVals.push_back(c->fPeak[1]);
                outVals.push_back(c->fPeak[2]);
            }
            if(find(outputs.begin(),outputs.end(),"-OTHER")!=outputs.end()){
                outVals.push_back(m->fCurrentLoad);
                outVals.push_back(m->fDAC4Voltage);
                outVals.push_back(m->f3VAnalogPower);
                outVals.push_back(m->f12VMultiplexer);
                outVals.push_back(m->f12VRadio);
            }
          
            if(asciiFile.is_open() && (asciiFileName!=nullptr)) asciiWriter(asciiFile);
            if(rootFile!=nullptr && (rootFileName!=nullptr)){ 
                if(count==0)rootWriterInit();
                rootWriter();
            }
            count++;
        }
    }

    if(asciiFile.is_open()) asciiFile.close();
    if(rootFile!=nullptr){
        outTree->Write();
        rootFile->Close();
        delete rootFile;
    }

    return 0;
}

void mcHelp(){
    cout << "MCDATA Options:\n"
        << "  -TIME\t\tOutput the time and CDAS time\n"
        << "  -PMTV\t\tOutput the PMT voltages (V)\n"
        << "  -PMTI\t\tOutput the PMT current (uA)\n"
        << "  -PMTT\t\tOutput the PMT temperature (C)\n"
        << "  -ADCBASELINE\tOutput baseline of all ADC measurements (ch)\n"
        << "  -PSU\t\tOutput the PSU voltages (+/- 3.3V,5V,12V,24V) (V)\n"
        << "  -ELECBOX\tOutput the electronic box temperature (C)\n"
        << "  -BATTERY\tOutput the battery temperature (C) and voltage (V)\n"
        << "          \tFirst value (0)  = battery center\n"
        << "          \tSecond value (1) = total\n"
        << "  -SOLAR\tOutput solar panel voltage (V) and current (A)\n"
        << "  -WATER\tOutput water level and temperature (C)\n"
        << "  -VEM\t\tOutput the VEM Charge (ADC) and VEM Peak (ADC)\n"
        << "  -OTHER\tOutput the\n"
        << "        \tCurrentLoad: current consumed by the LS (mA)\n"
        << "        \tDAC4Voltage: Voltage of the LED flasher (V)\n"
        << "        \t3VAnalogPower: 3.3V analog power (V)\n"
        << "        \t12VMultiplexer: 12V before the multiplexer (V)\n"
        << "        \tf12VRadio: 12V going to radio (V)\n\n"
        << "  -ROOTOUT=[file]\tOutput will be written to the ROOT file [file]\n"
        << "  -ASCIIOUT=[file]\tOutput will be written to the ASCII file [file]\n\n";
}
