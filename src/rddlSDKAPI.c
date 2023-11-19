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


void sdkReadSeed(char* seed_arr, int* seed_size){
  readSeed();

  memcpy(seed_arr, secret_seed, SEED_SIZE);
  *seed_size = SEED_SIZE;
}


void runRDDLSDKNotarizationWorkflow(const char* data_str, size_t data_length){
  Google__Protobuf__Any anyMsg = GOOGLE__PROTOBUF__ANY__INIT;
  clearStack();
  getPlntmntKeys();  
  int status = 0;

  if( hasMachineBeenAttested() )
  {
    size_t data_size = data_length;

    /* Globalda tanimlanan bir arrayin, kullanilmayan ilk adresini donduruyor */
    uint8_t* local_data = getStack( data_size+2 );

    memcpy( local_data, data_str, data_size);
    char signature[128+1] = {0};
    signRDDLNetworkMessageContent((const char*)local_data, data_size, signature);  

    // compute CID
    char* cid_str = create_cid_v1_from_string( (const char*) local_data );

    // store cid
    rddl_writefile( cid_str, (uint8_t*)local_data, data_size );

    // register CID
    // registerCID( cid_str );
  
    sprintf(responseArr, "Notarize: CID Asset %s\n", cid_str);
    ResponseAppendAbst(responseArr);

    generateAnyCIDAttestMsg(&anyMsg, cid_str, sdk_priv_key_planetmint, sdk_pub_key_planetmint, sdk_address, sdk_ext_pub_key_planetmint );
  
#ifdef LINUX_MACHINE
    free(cid_str);
#endif
  
  }
  else{
    sprintf(responseArr, "Register: Machine\n");
    ResponseAppendAbst(responseArr);
    status = registerMachine(&anyMsg);
  }
  if (status >= 0) {
    sprintf(responseArr, "TX processing:\n");
    ResponseAppendAbst(responseArr);
    char* tx_payload = create_transaction(&anyMsg, "2");

    if(!tx_payload)
      return;
    sprintf(responseArr, "TX broadcast:\n");
    ResponseAppendAbst(responseArr);
    broadcast_transaction( tx_payload );
  }
  ResponseJsonEnd();
}
