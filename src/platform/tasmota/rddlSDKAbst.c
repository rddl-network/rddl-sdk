#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

 

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

bool sdk_readSeed = false;

static char curlCmd[256];
static char curlOutput[1024];

char responseArr[4096];

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


int ResponseAppendAbst(const char* msg) 
{
  return ResponseAppendAbstTasmota(msg);
}


int ResponseJsonEnd(void)
{
  return ResponseAppendAbst(PSTR("}}"));
}


char* getGPSstring(){
  return getGPSstringTasmota();
}


bool getAccountInfo( uint64_t* account_id, uint64_t* sequence )
{
  bool ret = getAccountInfoTasmota(sdk_address, account_id, sequence);
  if( !ret ){
    sprintf(responseArr, "Account parsing issue\n");
    ResponseAppendAbst(responseArr);
  }

  return ret;
}


int broadcast_transaction( char* tx_payload ){
  char http_answr[512];
  int status = broadcastTransactionTasmota(tx_payload, http_answr);

  sprintf(responseArr, PSTR(",\"%s\":\"%u\"\n"), "respose code", status);
  ResponseAppendAbst(responseArr);
  sprintf(responseArr, PSTR(",\"%s\":\"%s\"\n"), "respose string", http_answr);
  ResponseAppendAbst(responseArr);

  return status;
}


char* getSetting(uint32_t index){
  return tasmotaGetSetting(index);
}


bool setSetting(uint32_t index, const char* replacementText){
  return tasmotaSetSetting( index, replacementText);
}
