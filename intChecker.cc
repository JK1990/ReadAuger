#include "intChecker.h"

#include <string.h>
#include <iostream>

bool isInt(char* input){
    const int len=strlen(input);

    for(int i=0;i<len;i++){
        if(!isdigit(input[i]))return false;
    }
    return true;
}
