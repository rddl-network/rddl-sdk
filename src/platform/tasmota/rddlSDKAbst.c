#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>

 

#include "rddl.h"
#include "rddl_cid.h"
#include "bip39.h"
#include "bip32.h"
#include "curves.h"
#include "ed25519.h"
#include "base58.h"
#include "math.h"
#include "secp256k1.h"

#include "rddl_types.h"
#include "planetmintgo.h"
#include "planetmintgo/machine/machine.pb-c.h"
#include "cosmos/tx/v1beta1/tx.pb-c.h"
#include "planetmintgo/machine/tx.pb-c.h"
#include "planetmintgo/asset/tx.pb-c.h"
#include "google/protobuf/any.pb-c.h"

#include "rddlSDKAbst.h"
#include "tasmotaUtils.h"


uint8_t sdk_priv_key_planetmint[32+1] = {0};
uint8_t sdk_priv_key_liquid[32+1] = {0};
uint8_t sdk_pub_key_planetmint[33+1] = {0};
uint8_t sdk_pub_key_liquid[33+1] = {0};
uint8_t sdk_machineid_public_key[33+1]={0}; 


char sdk_address[64] = {0};
char sdk_ext_pub_key_planetmint[EXT_PUB_KEY_SIZE+1] = {0};
char sdk_ext_pub_key_liquid[EXT_PUB_KEY_SIZE+1] = {0};
char sdk_machineid_public_key_hex[33*2+1] = {0};

char sdk_planetmintapi[100] = {0};
char sdk_chainid[30] = {0};
char sdk_denom[20] = {0};

char challengedCID[ 64 ] = {0};

bool sdk_readSeed = false;

static char curlCmd[256];
static char curlOutput[1024];

char responseArr[4096];


PoPInfo popParticipation;
void resetPopInfo(){  memset( &popParticipation, 0, sizeof(PoPInfo)); }

bool hasMachineBeenAttested() {
  bool status = hasMachineBeenAttestedTasmota((const char*)sdk_ext_pub_key_planetmint);

  return status;
}


bool rddl_writefile( const char* filename, uint8_t* content, size_t length) {
  bool status = rddlWritefileTasmota(filename, content, length);

  return status;
}


int readfile( const char* filename, uint8_t* content, size_t length){
  bool status = readfileTasmota(filename, content, length);

  return (status) ? length : 0;
}


int printMsg(const char* msg) 
{
  return tasmotaSerialPrint(msg);
}


char* getGPSstring(){
  return getGPSstringTasmota();
}

void AddLogLineAbst(const char* msg, ...)   
{
  va_list args;
  va_start(args, msg);
  vAddLogLineTasmota(msg, args); // Using vprintf as an example of working with va_list
  va_end(args);
}

void vAddLogLineAbst(const char* msg, va_list args)   
{
  vAddLogLineTasmota(msg, args); // Using vprintf as an example of working with va_list
}

bool getAccountInfo( uint64_t* account_id, uint64_t* sequence )
{
  bool ret = getAccountInfoTasmota(sdk_address, account_id, sequence);
  if( !ret ){
    sprintf(responseArr, "Account parsing issue\n");
    AddLogLineAbst(responseArr);
  }

  return ret;
}

bool getPoPInfo( const char* blockHeight){
  return getPoPInfoTasmota( blockHeight );
}


int broadcast_transaction( char* tx_payload ){
  char http_answr[512];
  int status = broadcastTransactionTasmota(tx_payload, http_answr);

  sprintf(responseArr, PSTR(",\"%s\":\"%u\"\n"), "respose code", status);
  AddLogLineAbst(responseArr);
  sprintf(responseArr, PSTR(",\"%s\":\"%s\"\n"), "respose string", http_answr);
  AddLogLineAbst(responseArr);

  return status;
}


char* getSetting(uint32_t index){
  return tasmotaGetSetting(index);
}


bool setSetting(uint32_t index, const char* replacementText){
  return tasmotaSetSetting( index, replacementText);
}

uint8_t* abstGetStack( size_t size ){
  return getStack( size );
}

void abstClearStack() {
  clearStack();
}

void SubscribeAbst( const char *topic ){
  SubscribeTasmota( topic );
}

void PublishPayloadAbst(const char* topic, const char* payload){
  PublishPayloadTasmota( topic, payload );
}
