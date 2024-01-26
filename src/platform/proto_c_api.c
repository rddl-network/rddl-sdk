#include <stdbool.h>
#include <unistd.h>
#include <stdint.h> 
#include <stdlib.h>
#include <stdarg.h>
#include "configFile.h"
#include "rddlSDKAbst.h"
#include "rddl_types.h"
#include "planetmintgo.h"
#include "planetmintgo/machine/machine.pb-c.h"
#include "planetmintgo/machine/tx.pb-c.h"
#include "google/protobuf/any.pb-c.h"

void* fullfill_planetmintgo_machine_metadata(char* cid, char* gps, char* assetdef, char* device){
    Planetmintgo__Machine__Metadata* metadata = (Planetmintgo__Machine__Metadata*)abstGetStack( sizeof(Planetmintgo__Machine__Metadata) );
    planetmintgo__machine__metadata__init(metadata);

    metadata->additionaldatacid = cid;
    metadata->gps               = gps;
    metadata->assetdefinition   = assetdef;
    metadata->device            = device;

    return (void*)metadata;
}


void* fullfill_planetmintgo_machine_machine(void* metadata_ptr, char* signature_hex){
    Planetmintgo__Machine__Machine* machine = (Planetmintgo__Machine__Machine*)abstGetStack( sizeof(Planetmintgo__Machine__Machine) );;
    planetmintgo__machine__machine__init(machine);

    machine->name               = (char*)sdk_address;
    machine->issuerplanetmint   = sdk_ext_pub_key_planetmint;
    machine->issuerliquid       = sdk_ext_pub_key_liquid;
    machine->machineid          = sdk_machineid_public_key_hex;
    machine->metadata           = (Planetmintgo__Machine__Metadata*)metadata_ptr;
    machine->type               = RDDL_MACHINE_POWER_SWITCH;
    machine->machineidsignature = signature_hex;
    machine->address            = (char*)sdk_address;

    return (void*)machine;
}

void* fullfill_planetmintgo_machine_msgAttestMachine(void* machine_ptr){
    Planetmintgo__Machine__MsgAttestMachine* machineMsg = (Planetmintgo__Machine__MsgAttestMachine*)abstGetStack( sizeof(Planetmintgo__Machine__MsgAttestMachine) );;
    planetmintgo__machine__msg_attest_machine__init(machineMsg);

    machineMsg->creator = (char*)sdk_address;
    machineMsg->machine = (Planetmintgo__Machine__Machine*)machine_ptr;

    return (void*)machineMsg;
}


int bind_msgAttestMachine_to_anyMsg(void* anyMsg, void* machineMsg){
    return generateAnyAttestMachineMsg((Google__Protobuf__Any*)anyMsg, (Planetmintgo__Machine__MsgAttestMachine*)machineMsg);
}