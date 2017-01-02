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
    if(find(outputs.begin(),outputs.end(),"-PMTV")!=outputs.end()){
        file << "\tUPMTV1\tUPMTV2\tUPMTV3";
    }
    if(find(outputs.begin(),outputs.end(),"-PMTI")!=outputs.end()){
        file << "\tUPMTI1\tUPMTI2\tUPMTI3";
    }
    if(find(outputs.begin(),outputs.end(),"-PMTT")!=outputs.end()){
        file << "\tUPMTT1\tUPMTT2\tUPMTT3";
    }
    if(find(outputs.begin(),outputs.end(),"-ENV")!=outputs.end()){
        file << "\tExtT\tAirT\tAirP";
    }
    if(find(outputs.begin(),outputs.end(),"-ADC")!=outputs.end()){
        file << "\tADC0\tADC1\tADC2\tADC3\tADC4\tADC5\tADC6\tADC7\tADC8\tADC9";
    }
    if(find(outputs.begin(),outputs.end(),"-VOLTAGES")!=outputs.end()){
        file << "\t12V_WT\t12VPMT0\t12VPMT1\t12VPMT2\t12VPM\t24VExt0\t24VExt1\t3V3\t5VGPS\t3V3Analog\t-3V3Analog\t5VUSB\t1V0\t1V2\t1V8";
    }
    if(find(outputs.begin(),outputs.end(),"-CURRENTS")!=outputs.end()){
        file << "\t12VRadioI\t12VPMI\t3V3AnalogI\t_3V3AnalogI\t5VGPSI\t1V0I\t1V2I\t1V8I\t3V3I\tVInI\t3V3SCI";
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

/* Initializes the writing of the root file
 * The Branches for the different monitoring and calibration
 * quantities are initialized
 */
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
    if(find(outputs.begin(),outputs.end(),"-PMTV")!=outputs.end()){
        outTree->Branch("UPMTV1",&(outVals[n]),"UPMTV1/F");
        outTree->Branch("UPMTV2",&(outVals[n+1]),"UPMTV2/F");
        outTree->Branch("UPMTV3",&(outVals[n+2]),"UPMTV3/F");
        n+=3;
    }
    if(find(outputs.begin(),outputs.end(),"-PMTI")!=outputs.end()){
        outTree->Branch("UPMTI1",&(outVals[n]),"UPMTI1/F");
        outTree->Branch("UPMTI2",&(outVals[n+1]),"UPMTI2/F");
        outTree->Branch("UPMTI3",&(outVals[n+2]),"UPMTI3/F");
        n+=3;
    }
    if(find(outputs.begin(),outputs.end(),"-PMTT")!=outputs.end()){
        outTree->Branch("UPMTT1",&(outVals[n]),"UPMTT1/F");
        outTree->Branch("UPMTT2",&(outVals[n+1]),"UPMTT2/F");
        outTree->Branch("UPMTT3",&(outVals[n+2]),"UPMTT3/F");
        n+=3;
    }
    if(find(outputs.begin(),outputs.end(),"-ENV")!=outputs.end()){
        outTree->Branch("ExtT",&(outVals[n]),"ExtT/F");
        outTree->Branch("AirT",&(outVals[n+1]),"AirT/F");
        outTree->Branch("AirP",&(outVals[n+2]),"AirP/F");
        n+=3;
    }
    if(find(outputs.begin(),outputs.end(),"-ADC")!=outputs.end()){
        outTree->Branch("ADC0",&(outVals[n]),"ADC0/F");
        outTree->Branch("ADC1",&(outVals[n+1]),"ADC1/F");
        outTree->Branch("ADC2",&(outVals[n+2]),"ADC2/F");
        outTree->Branch("ADC3",&(outVals[n+3]),"ADC3/F");
        outTree->Branch("ADC4",&(outVals[n+4]),"ADC4/F");
        outTree->Branch("ADC5",&(outVals[n+5]),"ADC5/F");
        outTree->Branch("ADC6",&(outVals[n+6]),"ADC6/F");
        outTree->Branch("ADC7",&(outVals[n+7]),"ADC7/F");
        outTree->Branch("ADC8",&(outVals[n+8]),"ADC8/F");
        outTree->Branch("ADC9",&(outVals[n+9]),"ADC9/F");
        n+=10;
    }
    if(find(outputs.begin(),outputs.end(),"-VOLTAGES")!=outputs.end()){
        outTree->Branch("12V_WT",&(outVals[n]),"12V_WT/F");
        outTree->Branch("12VPMT0",&(outVals[n+1]),"12VPMT0/F");
        outTree->Branch("12VPMT1",&(outVals[n+2]),"12VPMT1/F");
        outTree->Branch("12VPMT2",&(outVals[n+3]),"12VPMT2/F");
        outTree->Branch("12VPM",&(outVals[n+4]),"12VPM/F");
        outTree->Branch("24VExt0",&(outVals[n+5]),"24VExt0/F");
        outTree->Branch("24VExt1",&(outVals[n+6]),"24VExt1/F");
        outTree->Branch("3V3",&(outVals[n+7]),"3V3/F");
        outTree->Branch("5VGPS",&(outVals[n+8]),"5VGPS/F");
        outTree->Branch("3V3Analog",&(outVals[n+9]),"3V3Analog/F");
        outTree->Branch("-3V3Analog",&(outVals[n+10]),"-3V3Analog/F");
        outTree->Branch("5VUSB",&(outVals[n+11]),"5VUSB/F");
        outTree->Branch("1V0",&(outVals[n+12]),"1V0/F");
        outTree->Branch("1V2",&(outVals[n+13]),"1V2/F");
        outTree->Branch("1V8",&(outVals[n+14]),"1V8/F");
        n+=15;
    }
    if(find(outputs.begin(),outputs.end(),"-CURRENTS")!=outputs.end()){
        outTree->Branch("12VRadioI",&(outVals[n]),"12VRadioI/F");
        outTree->Branch("12VPMI",&(outVals[n+1]),"12VPMI/F");
        outTree->Branch("3V3AnalogI",&(outVals[n+2]),"3V3AnalogI/F");
        outTree->Branch("-3V3AnalogI",&(outVals[n+3]),"-3V3AnalogI/F");
        outTree->Branch("5VGPSI",&(outVals[n+4]),"5VGPSI/F");
        outTree->Branch("1V0I",&(outVals[n+5]),"1V0I/F");
        outTree->Branch("1V2I",&(outVals[n+6]),"1V2I/F");
        outTree->Branch("1V8I",&(outVals[n+7]),"1V8I/F");
        outTree->Branch("3V3I",&(outVals[n+8]),"3V3I/F");
        outTree->Branch("VInI",&(outVals[n+9]),"VInI/F");
        outTree->Branch("3V3SCI",&(outVals[n+10]),"3V3SCI/F");
        n+=11;
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
            //if(find(stationIDs.begin(),stationIDs.end(),fSDMonCal->fLsId)!=stationIDs.end()){cout << "Found station " << fSDMonCal->fMonitoring.fIsUUB << endl;}
            //if((fSDMonCal->fMonitoring.fIsMonitoring!=1 || fSDMonCal->fCalibration.fIsCalibration!=1)  && !fSDMonCal->fMonitoring.fIsUUB)continue;
            //if(find(stationIDs.begin(),stationIDs.end(),fSDMonCal->fLsId)!=stationIDs.end()){cout << "Found station UUB " << fSDMonCal->fLsId << endl;}
            //if(fSDMonCal->fMonitoring.fIsMonitoring!=1 && fSDMonCal->fMonitoring.fIsUUB)continue;

            //Check for the correct station IDs
            int id = fSDMonCal->fLsId;
            if(find(stationIDs.begin(),stationIDs.end(),id)==stationIDs.end())continue;

            cout << "Found station " << id << endl;

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
            //Additional output for UUB data
            if(find(outputs.begin(),outputs.end(),"-PMTV")!=outputs.end() && !fSDMonCal->fMonitoring.fIsUUB){
                outVals.push_back(m->fUPMV[0]);
                outVals.push_back(m->fUPMV[1]);
                outVals.push_back(m->fUPMV[2]);
            }
            if(find(outputs.begin(),outputs.end(),"-PMTI")!=outputs.end() && !fSDMonCal->fMonitoring.fIsUUB){
                outVals.push_back(m->fUPMI[0]);
                outVals.push_back(m->fUPMI[1]);
                outVals.push_back(m->fUPMI[2]);
            }
            if(find(outputs.begin(),outputs.end(),"-PMTT")!=outputs.end() && !fSDMonCal->fMonitoring.fIsUUB){
                outVals.push_back(m->fUPMT[0]);
                outVals.push_back(m->fUPMT[1]);
                outVals.push_back(m->fUPMT[2]);
            }
            if(find(outputs.begin(),outputs.end(),"-ENV")!=outputs.end() && !fSDMonCal->fMonitoring.fIsUUB){
                outVals.push_back(m->fExtT);
                outVals.push_back(m->fAirT);
                outVals.push_back(m->fAirP);
            }
            if(find(outputs.begin(),outputs.end(),"-ADC")!=outputs.end() && !fSDMonCal->fMonitoring.fIsUUB){
                for(int k=0;k<10;k++){
                    outVals.push_back(m->fADC[k]);
                }
            }
            if(find(outputs.begin(),outputs.end(),"-VOLTAGES")!=outputs.end() && !fSDMonCal->fMonitoring.fIsUUB){
                outVals.push_back(m->f12V_WT);
                outVals.push_back(m->f12VPMT[0]);
                outVals.push_back(m->f12VPMT[1]);
                outVals.push_back(m->f12VPMT[2]);
                outVals.push_back(m->f12VPM);
                outVals.push_back(m->f24VExt[0]);
                outVals.push_back(m->f24VExt[1]);
                outVals.push_back(m->f3V3);
                outVals.push_back(m->f5VGPS);
                outVals.push_back(m->f3V3Analog);
                outVals.push_back(m->f_3V3Analog);
                outVals.push_back(m->f5VUSB);
                outVals.push_back(m->f1V0);
                outVals.push_back(m->f1V2);
                outVals.push_back(m->f1V8);
            }
            if(find(outputs.begin(),outputs.end(),"-CURRENTS")!=outputs.end() && !fSDMonCal->fMonitoring.fIsUUB){
                outVals.push_back(m->f12VRadioI);
                outVals.push_back(m->f12VPMI);
                outVals.push_back(m->f3V3AnalogI);
                outVals.push_back(m->f_3V3AnalogI);
                outVals.push_back(m->f5VGPSI);
                outVals.push_back(m->f1V0I);
                outVals.push_back(m->f1V2I);
                outVals.push_back(m->f1V8I);
                outVals.push_back(m->f3V3I);
                outVals.push_back(m->fVInI);
                outVals.push_back(m->f3V3SCI);
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
        << "        \tf12VRadio: 12V going to radio (V)\n"
        << "  -ENV\t\tOutput the environment data (only for UUB)\n"
        << "        \tContaints external temperature, air temperature, air pressure\n"
        << "  -ADC\t\tOutput the 10 ADC values (only for UUB)\n"
        << "  -VOLTAGES\tOutput the monitored voltages (12V,+-3.3V,...) (only for UUB)\n"
        << "  -CURRENTS\tOutput the monitored currents (12V current, 24V current,...) (only for UUB)\n\n"
        << "  -ROOTOUT=[file]\tOutput will be written to the ROOT file [file]\n"
        << "  -ASCIIOUT=[file]\tOutput will be written to the ASCII file [file]\n\n";
}
