#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h> 
#include <errno.h>
#include <fcntl.h>

#include "rddl.h"
#include "rddl_cid.h"
#include "bip32.h"
#include "curves.h"
#include "secp256k1.h"

#include "rddl_types.h"
#include "planetmintgo.h"
#include "planetmintgo/machine/machine.pb-c.h"
#include "cosmos/tx/v1beta1/tx.pb-c.h"
#include "planetmintgo/machine/tx.pb-c.h"
#include "planetmintgo/asset/tx.pb-c.h"
#include "google/protobuf/any.pb-c.h"

#include "rddlSDKAbst.h"
#include "rddlSDKUtils.h"
#include "rddlSDKAPI.h"


const char* sdkGetRDDLAddress()          { return (const char*) sdk_address; }
const char* sdkGetExtPubKeyLiquid()      { return (const char*)sdk_ext_pub_key_liquid; }
const char* sdkGetExtPubKeyPlanetmint()  { return (const char*)sdk_ext_pub_key_planetmint; }
const uint8_t* sdkGetPrivKeyLiquid()     { return (const uint8_t*)sdk_priv_key_liquid; }
const uint8_t* sdkGetPrivKeyPlanetmint() { return (const uint8_t*)sdk_priv_key_planetmint; }
const char* sdkGetMachinePublicKey()     { return (const char*) sdk_machineid_public_key_hex; }
bool  sdkGetPlntmntKeys(){ return getPlntmntKeys(); }

int sdkReadFile( const char* filename, uint8_t* content, size_t length){
  return readfile(filename, content, length);
}

bool sdkGetAccountInfo( uint64_t* account_id, uint64_t* sequence ){
  return getAccountInfo( account_id, sequence );
}

char* sdkGetSetting( uint32_t index ){
  return getSetting(index);
}
bool sdkSetSetting( uint32_t index, const char* replacementText ){
  return setSetting(index, replacementText);
}

char* sdkSetSeed(char* pMnemonic, size_t len){

  char* mnemonic = NULL;
  
  if(len)
    mnemonic = (char*)setSeed( pMnemonic, len );
  else{
    mnemonic = (char*)getMnemonic();
    mnemonic = (char*)setSeed( mnemonic, strlen(mnemonic) );
  }  

  storeSeed();
  return mnemonic;
}


void sdkStoreSeed(char* new_seed){
  
  if(new_seed != NULL)
    memcpy(secret_seed, new_seed, SEED_SIZE);
  
  storeSeed();
}


uint8_t* sdkReadSeed(char* seed_arr, int* seed_size){
  uint8_t* temp_seed = readSeed();

  memcpy(seed_arr, secret_seed, SEED_SIZE);
  *seed_size = SEED_SIZE;

  return temp_seed;
}


void runRDDLSDKMachineAttestation(const char* machineCategory, const char* manufacturer, const char* cid ){
  Google__Protobuf__Any anyMsg = GOOGLE__PROTOBUF__ANY__INIT;
  clearStack();
  if( !getPlntmntKeys() )
    return;

  sprintf(responseArr, "Register: Machine\n");
  printMsg(responseArr);
  int status = registerMachine(&anyMsg, machineCategory, manufacturer, cid );
  if ( status >= 0 ){
    status = sendMessages( &anyMsg );
  }
}


void runRDDLSDKNotarizationWorkflow(const char* data_str, size_t data_length){
  Google__Protobuf__Any anyMsg = GOOGLE__PROTOBUF__ANY__INIT;
  clearStack();
  if( !getPlntmntKeys() )
    return;

  if( !hasMachineBeenAttested() )
    return;

  size_t data_size = data_length;
  uint8_t* local_data = getStack( data_size+2 );

  memcpy( local_data, data_str, data_size);
  char signature[128+1] = {0};
  signRDDLNetworkMessageContent((const char*)local_data, data_size, signature);

  //compute CID
  char* cid_str = create_cid_v1_from_string( (const char*) local_data );

  // store cid
  rddl_writefile( cid_str, (uint8_t*)local_data, data_size );

  // register CID
  //registerCID( cid_str );

  sprintf(responseArr, "Notarize: CID Asset\n");
  printMsg(responseArr);

  generateAnyCIDAttestMsg(&anyMsg, cid_str, sdk_priv_key_planetmint, sdk_pub_key_planetmint, sdk_address, sdk_ext_pub_key_planetmint );
  sendMessages( &anyMsg );

#ifdef LINUX_MACHINE
  free(cid_str);
#endif

  ResponseJsonEnd();
}

