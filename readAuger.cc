#include <string.h>
#include <iostream>

#include "readMC.h"
#include "readRaw.h"
#include "intChecker.h"

using namespace std;

int printHelp() {
    cout << "Command:\n"
        << "readAuger --help\n\n"
        << "Usage:\n"
        << "readAuger [Data type] [ID(s)] [Options] filename(s)\n\n";

    cout << "Data types:\n"
        << "  MCDATA\tThe files include monitoring and calibration data\n"
        << "  RAWDATA\tThe files include raw data (calibration histograms, FADC traces,...)\n\n";

    cout << "IDs:\n"
        << "  -ID [ID]\tSpecifies the station ID for the output\n"
        << "   \t\t(May be used multiple times)\n";

    mcHelp();   
    rawHelp();

    cout << "Important notes:\n"
        << "Only one data type may be used\n";

    return 0;
}

/*bool isInt(char* input){
    const int len=strlen(input);

    for(int i=0;i<len;i++){
        if(!isdigit(input[i]))return false;
    }
    return true;
}*/

int invalidInput(){
    cout << "Commandline syntax error (use --help for help)" << endl;
    return 127;
}

int main(int argc, char** argv) {
    bool mcData=false;
    bool rawData=false;

    int nArgs=1;
    int minArgs=0; // First option which is not data type or ID

    vector<int> ids;
    vector<char*> options;
    
    // Check the data type and options
    for(int i=0;i<argc;i++){
        if(strcmp(argv[i],"--help")==0) return printHelp();
        if(strcmp(argv[i],"MCDATA")==0){
            mcData=true;
            nArgs++;
        }
        if(strcmp(argv[i],"RAWDATA")==0){
            rawData=true;
            nArgs++;
        }
        if(strcmp(argv[i],"-ID")==0){
            if(i==argc-1){ // "ID" must be followed by the ID so it cannot be the last arg
                cout << "Commandline syntax error (Specify an ID)" << endl;
                return 127;
            }
            if(!isInt(argv[i+1])){ // Check if the given ID is an int
               cout << "Commandline syntax error (ID must be an integer)" << endl;
               return 127;
            }
            ids.push_back(stoi(argv[i+1]));
            nArgs+=2;
            i++;
        }
    }

    if(ids.size()==0){
        cout << "Commandline syntax error (At least one ID must be specified)" << endl;
        return 127;
    }

    if(argc-nArgs==1){
        cout << "Commandline syntax error (No input file name has been specified)" << endl;
        return 127;
    }

    minArgs=nArgs;

    // Check if exactly one datatype was specified
    if((mcData && rawData) || (!mcData && !rawData)){
        return invalidInput();
    }

    // Check the options
    for(int i=minArgs;i<argc;i++){
        if(!(argv[i][0]=='-')){
            break;
        }
        nArgs++;
        options.push_back(argv[i]);
    }

    if(mcData){
        readMC MCData(argc-nArgs,argv+nArgs);
        for(auto &id : ids)MCData.addStationID(id); // Add the specified IDs to the mc data reader
        if(MCData.addOption(options))return 1;
        if(MCData.Run())return 1;
    }

    if(rawData){
        readRaw RawData(argc-nArgs,argv+nArgs);
        for(auto &id : ids)RawData.addStationID(id); // Add the specified IDs to the raw data reader
        if(RawData.addOption(options))return 1;
        if(RawData.Run())return 1;
    }

    return 0;
}
