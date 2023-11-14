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
#include "base64.h"
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


void runRDDLNotarizationWorkflow(const char* data_str, size_t data_length){
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
  
    printMsg("Notarize: CID Asset\n");
    // ResponseAppend_P("Notarize: CID Asset %s\n", cid_str);

    generateAnyCIDAttestMsg(&anyMsg, cid_str, g_priv_key_planetmint, g_pub_key_planetmint, g_address, g_ext_pub_key_planetmint );
    free(cid_str);
  }
  else{
    printMsg("Register: Machine\n");
    ResponseAppend_P("Register: Machine\n");
    status = registerMachine(&anyMsg);
  }
  if (status >= 0) {
    ResponseAppend_P("TX processing:\n");
    char* tx_payload = create_transaction(&anyMsg, "2");

    if(!tx_payload)
      return;
    ResponseAppend_P("TX broadcast:\n");
    broadcast_transaction( tx_payload );
  }
  ResponseJsonEnd();
}
