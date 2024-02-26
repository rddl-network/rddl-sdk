#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "rddl.h"
#include "rddlSDKAbst.h"
#include "rddlSDKSettings.h"
#include "rddlSDKUtils.h"
#include "configFile.h"
#include "arduinoUtils.h"
#include "planetmintgo.h"

uint8_t sdk_priv_key_planetmint[32+1] = {0};
uint8_t sdk_priv_key_liquid[32+1] = {0};
uint8_t sdk_pub_key_planetmint[33+1] = {0};
uint8_t sdk_pub_key_liquid[33+1] = {0};
uint8_t sdk_machineid_public_key[33+1]={0}; 

char sdk_address[64] = {0};
char sdk_ext_pub_key_planetmint[EXT_PUB_KEY_SIZE+1] = {0};
char sdk_ext_pub_key_liquid[EXT_PUB_KEY_SIZE+1] = {0};
char sdk_machineid_public_key_hex[33*2+1] = {0};

char sdk_periodicity[20] = {0};
char sdk_planetmintapi[100] = {0};

char sdk_chainid[30] = {0};
char sdk_denom[20] = {0};

bool sdk_readSeed = false;

char responseArr[4096];
uint32_t num_of_cid_files = 0;

char challengedCID[ 64 ] = {0};

void resetPopInfo(){  memset( &popParticipation, 0, sizeof(PoPInfo)); }


bool hasMachineBeenAttested() {
  return hasMachineBeenAttestedArduino((const char*)sdk_ext_pub_key_planetmint, getSetting( SDK_SET_PLANETMINT_API) );
}


bool rddl_writefile( const char* filename, uint8_t* content, size_t length) {
  bool status = rddlWritefileArduino(filename, content, length);

  return status;
}


int readfile( const char* filename, uint8_t* content, size_t length){
  bool status = readfileArduino(filename, content, length);

  return (status) ? length : 0;
}


int printMsg(const char* msg){
  return arduinoSerialPrint(msg);
}


void AddLogLineAbst(const char* msg, ...)   
{

}

void vAddLogLineAbst( const char* msg, va_list args ){

}

char* getGPSstring(){
  return NULL;
}

uint8_t* abstGetStack( size_t size ){
  return getStack( size );
}


void abstClearStack() {
  clearStack();
}


bool getAccountInfo( uint64_t* account_id, uint64_t* sequence )
{
  bool ret = getAccountInfoArduino(sdk_address, account_id, sequence, getSetting(SDK_SET_PLANETMINT_API));
  if( !ret ){
    sprintf(responseArr, "Account parsing issue\n");
    AddLogLineAbst(responseArr);
  }

  return ret;
}


bool getPoPInfo( const char* blockHeight){
  return getPoPInfoArduino(blockHeight, getSetting(SDK_SET_PLANETMINT_API));
}

int broadcast_transaction( char* tx_payload ){
  char http_answr[512];
  int status = broadcastTransactionArduino(tx_payload, http_answr, getSetting(SDK_SET_PLANETMINT_API));

  sprintf(responseArr, PSTR(",\"%s\":\"%u\"\n"), "respose code", status);
  AddLogLineAbst(responseArr);
  sprintf(responseArr, PSTR(",\"%s\":\"%s\"\n"), "respose string", http_answr);
  AddLogLineAbst(responseArr);

  return status;
}


char* getSetting(uint32_t index){
  switch(index) {
  case SDK_SET_NOTARIZTATION_PERIODICITY:
      if( strlen( sdk_periodicity) == 0 ){
        if( !readfile(SETTINGS_PERIODICITY_FILE, (uint8_t*)sdk_periodicity, 20) )
          strcpy(sdk_periodicity, DEFAULT_PERIODICITY_TEXT);
      }
      return sdk_periodicity;
  case SDK_SET_PLANETMINT_API:
      if( strlen( sdk_planetmintapi) == 0 ){
        if( !readfile(SETTINGS_API_FILE, (uint8_t*)sdk_planetmintapi, 100) )
          strcpy(sdk_planetmintapi, DEFAULT_API_TEXT);
      }
      return sdk_planetmintapi;
  case SDK_SET_PLANETMINT_CHAINID:
      if( strlen( sdk_chainid) == 0 ){
        if( !readfile(SETTINGS_CHAINID_FILE, (uint8_t*)sdk_chainid, 30) )
          strcpy(sdk_chainid, DEFAULT_CHAINID_TEXT);
      }
      return sdk_chainid;
  case SDK_SET_PLANETMINT_DENOM:
      if( strlen( sdk_denom) == 0 )
        if( !readfile(SETTINGS_DENOM_FILE, (uint8_t*)sdk_denom, 20) )
          strcpy(sdk_denom, DEFAULT_DENOM_TEXT);
      return sdk_denom;
  default:
      return NULL;
  }
}

bool setSetting(uint32_t index, const char* replacementText){
  bool retValue = false;
  switch(index) {
  case SDK_SET_NOTARIZTATION_PERIODICITY:
      retValue = rddl_writefile( SETTINGS_PERIODICITY_FILE, (uint8_t*)replacementText, strlen(replacementText));
      memset(sdk_periodicity,0,20);
      break;  
  case SDK_SET_PLANETMINT_API:   
      retValue = rddl_writefile( SETTINGS_API_FILE, (uint8_t*)replacementText, strlen(replacementText));
      memset(sdk_planetmintapi,0,100);
      break;  
  case SDK_SET_PLANETMINT_CHAINID:
      retValue = rddl_writefile( SETTINGS_CHAINID_FILE, (uint8_t*)replacementText, strlen(replacementText));
      memset(sdk_chainid,0,30);
      break;  
  case SDK_SET_PLANETMINT_DENOM:
      retValue = rddl_writefile( SETTINGS_DENOM_FILE, (uint8_t*)replacementText, strlen(replacementText));
      memset(sdk_denom,0,20);
      break;  
  default:
      retValue =  false;
  }
  return retValue;
}


char* getCIDsLinux( const char* address ){
  return NULL;
}

char* getCIDtoBeChallenged(){
  clearStack();
  char* jsonObject = getCIDsArduino( (const char*)popParticipation.challengee, getSetting(SDK_SET_PLANETMINT_API) );
  if (GetRandomElementFromCIDJSONList(jsonObject, challengedCID, 64) < 0)
    return NULL;
  return challengedCID;
}

int abstGetNumOfCIDFiles(const char* path){
  int count = 0;

  return count;
}


int abstDeleteOldestCIDFile(const char* path){
  int count = 0;

  return count;
}

void SubscribeAbst( const char *topic ){
  return;
}

void UnsubscribeAbst( const char *topic ){
  return;
}
