#include "Outputs.h"
#include "EEStore.h"
#include "../CommInterface/CommManager.h"
#include "../DCC/DCC.h"

void Output::activate(int s){
    data.oStatus=(s>0);                                               // if s>0, set status to active, else inactive
    digitalWrite(data.pin,data.oStatus ^ bitRead(data.iFlag,0));      // set state of output pin to HIGH or LOW depending on whether bit zero of iFlag is set to 0 (ACTIVE=HIGH) or 1 (ACTIVE=LOW)
    if(num>0)
        EEPROM.put(num,data.oStatus);
    CommManager::printf("<Y %d %d>", data.id, data.oStatus);
}

Output* Output::get(int n){
    Output *tt;
    for(tt=firstOutput;tt!=NULL && tt->data.id!=n;tt=tt->nextOutput);
    return(tt);
}

void Output::remove(int n){
    Output *tt,*pp;
    tt=firstOutput;
    pp=tt;

    for( ; tt!=NULL && tt->data.id!=n; pp=tt,tt=tt->nextOutput);

    if(tt==NULL){
        CommManager::printf("<X>");
        return;
    }

    if(tt==firstOutput)
        firstOutput=tt->nextOutput;
    else
        pp->nextOutput=tt->nextOutput;

    free(tt);

    CommManager::printf("<O>");
}

void Output::show(int n){
    Output *tt;

    if(firstOutput==NULL){
        CommManager::printf("<X>");
        return;
    }

    for(tt=firstOutput;tt!=NULL;tt=tt->nextOutput){
        if(n==1) {
        CommManager::printf("<Y %d %d %d %d>", tt->data.id, tt->data.pin, tt->data.iFlag, tt->data.oStatus);
        } else {
        CommManager::printf("<Y %d %d>", tt->data.id, tt->data.oStatus);
        }
    }
}

void Output::load(){
    struct OutputData data;
    Output *tt;

    for(int i=0;i<EEStore::eeStore->data.nOutputs;i++){
        EEPROM.get(EEStore::pointer(),data);
        tt=create(data.id,data.pin,data.iFlag);
        tt->data.oStatus=bitRead(tt->data.iFlag,1)?bitRead(tt->data.iFlag,2):data.oStatus;      // restore status to EEPROM value is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
        digitalWrite(tt->data.pin,tt->data.oStatus ^ bitRead(tt->data.iFlag,0));
        pinMode(tt->data.pin,OUTPUT);
        tt->num=EEStore::pointer();
        EEStore::advance(sizeof(tt->data));
    }
}

void Output::store(){
    Output *tt;

    tt=firstOutput;
    EEStore::eeStore->data.nOutputs=0;

    while(tt!=NULL){
        tt->num=EEStore::pointer();
        EEPROM.put(EEStore::pointer(),tt->data);
        EEStore::advance(sizeof(tt->data));
        tt=tt->nextOutput;
        EEStore::eeStore->data.nOutputs++;
    }

}

Output *Output::create(int id, int pin, int iFlag, int v){
    Output *tt;

    if(firstOutput==NULL){
        firstOutput=(Output *)calloc(1,sizeof(Output));
        tt=firstOutput;
    } else if((tt=get(id))==NULL){
        tt=firstOutput;
        while(tt->nextOutput!=NULL)
        tt=tt->nextOutput;
        tt->nextOutput=(Output *)calloc(1,sizeof(Output));
        tt=tt->nextOutput;
    }

    if(tt==NULL){       // problem allocating memory
        if(v==1)
        CommManager::printf("<X>");
        return(tt);
    }

    tt->data.id=id;
    tt->data.pin=pin;
    tt->data.iFlag=iFlag;
    tt->data.oStatus=0;

    if(v==1){
        tt->data.oStatus=bitRead(tt->data.iFlag,1)?bitRead(tt->data.iFlag,2):0;      // sets status to 0 (INACTIVE) is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
        digitalWrite(tt->data.pin,tt->data.oStatus ^ bitRead(tt->data.iFlag,0));
        pinMode(tt->data.pin,OUTPUT);
        CommManager::printf("<O>");
    }

    return(tt);

}

Output *Output::firstOutput=NULL;
