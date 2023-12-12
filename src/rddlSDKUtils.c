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
#ifdef LINUX_MACHINE
  #include "base64.h"
#else
  #include "base64_plntmnt.h"
#endif
#include "curves.h"  
#include "secp256k1.h"

#include "rddl_types.h"
#include "planetmintgo.h"
#include "planetmintgo/machine/machine.pb-c.h"
#include "cosmos/tx/v1beta1/tx.pb-c.h"
#include "planetmintgo/machine/tx.pb-c.h"
#include "planetmintgo/asset/tx.pb-c.h"
#include "google/protobuf/any.pb-c.h"

#include "configFile.h"
#include "rddlSDKAbst.h"
#include "rddlSDKUtils.h"
#include "rddlSDKSettings.h"


char* create_transaction( void* anyMsg, char* tokenAmount )
{
  uint64_t account_id = 0;
  uint64_t sequence = 0;
  bool gotAccountID = getAccountInfo( &account_id, &sequence );
  if( !gotAccountID )
  {
    return NULL;
  }

  Cosmos__Base__V1beta1__Coin coin = COSMOS__BASE__V1BETA1__COIN__INIT;
  coin.denom = getSetting( SDK_SET_PLANETMINT_DENOM ); //getDenom();
  coin.amount = tokenAmount;
  
  uint8_t* txbytes = NULL;
  size_t tx_size = 0;
  char* chain_id = getSetting( SDK_SET_PLANETMINT_CHAINID ); //getChainID();
  int ret = prepareTx( (Google__Protobuf__Any*)anyMsg, &coin, sdk_priv_key_planetmint, sdk_pub_key_planetmint, sequence, chain_id, account_id, &txbytes, &tx_size);
  if( ret < 0 )
    return NULL;

  size_t allocation_size = ceil( ((tx_size+3-1)/3)*4)+2 + 150;
  char* payload = (char*) getStack( allocation_size );
  if( payload )
  {
    memset( payload, 0, allocation_size );
    strcpy(payload, "{ \"tx_bytes\": \"" );
    bintob64( payload+ strlen( payload ), txbytes, tx_size);
    strcpy( payload+ strlen( payload ), "\", \"mode\": \"BROADCAST_MODE_SYNC\" }");
  }
  else{
    sprintf(responseArr, "not engouth memory:\n");
    ResponseAppendAbst(responseArr);
  }

  return payload;
}


void storeSeed()
{
  rddl_writefile( (const char*)"seed", secret_seed, SEED_SIZE );
  sdk_readSeed = false;
}


uint8_t* readSeed()
{
  if( sdk_readSeed )
    return secret_seed;

  int readbytes = readfile( "seed", secret_seed, SEED_SIZE);
  /* For TEST PURPOSE */
  // memcpy(secret_seed, "12345d8b5ee5bcefd523ee4d4340a8956affbef5bb1978eb1e3f640318f87f4b6438733879360f460932fa68bfc06ae8aaf837b5d45891d114a58d8ce19a17d7", SEED_SIZE);
  // int readbytes = SEED_SIZE;

  sprintf(responseArr, "{ \"%s\":\"%d\" }", "READ SIZE", readbytes);
  ResponseAppendAbst(responseArr);

  if( readbytes != SEED_SIZE )
    return NULL;

  sdk_readSeed = true;
  return secret_seed;
}


const uint8_t *fromhex(const char *str) {
  static uint8_t buf[FROMHEX_MAXLEN];
  size_t len = strlen(str) / 2;
  if (len > FROMHEX_MAXLEN) len = FROMHEX_MAXLEN;
  for (size_t i = 0; i < len; i++) {
    uint8_t c = 0;
    if (str[i * 2] >= '0' && str[i * 2] <= '9') c += (str[i * 2] - '0') << 4;
    if ((str[i * 2] & ~0x20) >= 'A' && (str[i * 2] & ~0x20) <= 'F')
      c += (10 + (str[i * 2] & ~0x20) - 'A') << 4;
    if (str[i * 2 + 1] >= '0' && str[i * 2 + 1] <= '9')
      c += (str[i * 2 + 1] - '0');
    if ((str[i * 2 + 1] & ~0x20) >= 'A' && (str[i * 2 + 1] & ~0x20) <= 'F')
      c += (10 + (str[i * 2 + 1] & ~0x20) - 'A');
    buf[i] = c;
  }
  return buf;
}


bool getPlntmntKeys(){
  
  if( readSeed() == NULL )
    return false;

  /* Seedden Priv key ve diger seyleri elde et */
  HDNode node_planetmint;
  hdnode_from_seed( secret_seed, SEED_SIZE, SECP256K1_NAME, &node_planetmint);
  hdnode_private_ckd_prime(&node_planetmint, 44);
  hdnode_private_ckd_prime(&node_planetmint, 8680);
  hdnode_private_ckd_prime(&node_planetmint, 0);
  hdnode_private_ckd(&node_planetmint, 0);
  hdnode_private_ckd(&node_planetmint, 0);
  hdnode_fill_public_key(&node_planetmint);
  /* Global e kopyaliyor pub ve priv keyi */
  memcpy(sdk_priv_key_planetmint, node_planetmint.private_key, 32);
  memcpy(sdk_pub_key_planetmint, node_planetmint.public_key, PUB_KEY_SIZE);


  /* Seedden Priv key ve diger seyleri elde et */
  HDNode node_rddl;
  hdnode_from_seed( secret_seed, SEED_SIZE, SECP256K1_NAME, &node_rddl);
  hdnode_private_ckd_prime(&node_rddl, 44);
  hdnode_private_ckd_prime(&node_rddl, 1776);
  hdnode_private_ckd_prime(&node_rddl, 0);
  hdnode_private_ckd(&node_rddl, 0);
  hdnode_private_ckd(&node_rddl, 0);
  hdnode_fill_public_key(&node_rddl);
  /* Global e kopyaliyor pub ve priv keyi */
  memcpy(sdk_priv_key_liquid, node_rddl.private_key, 32);
  memcpy(sdk_pub_key_liquid, node_rddl.public_key, PUB_KEY_SIZE);

  uint8_t address_bytes[ADDRESS_TAIL] = {0};
  pubkey2address( sdk_pub_key_planetmint, PUB_KEY_SIZE, address_bytes );
  getAddressString( address_bytes, sdk_address);
  uint32_t fingerprint = hdnode_fingerprint(&node_planetmint);
  hdnode_serialize_public( &node_planetmint, fingerprint, PLANETMINT_PMPB, sdk_ext_pub_key_planetmint, EXT_PUB_KEY_SIZE);
  hdnode_serialize_public( &node_rddl, fingerprint, VERSION_PUBLIC, sdk_ext_pub_key_liquid, EXT_PUB_KEY_SIZE);

  ecdsa_get_public_key33(&secp256k1, private_key_machine_id, sdk_machineid_public_key);
  toHexString( sdk_machineid_public_key_hex, sdk_machineid_public_key, 33*2);
  return true;
}


void signRDDLNetworkMessageContent( const char* data_str, size_t data_length, char* sig_out){
  char pubkey_out[66+1] = {0};
  char hash_out[64+1] = {0};
  if( readSeed() != NULL )
  {
    SignDataHash( data_str, data_length,  pubkey_out, sig_out, hash_out);
  }

  /* Bunlar PlatformIO Specific */
  sprintf(responseArr, PSTR(",\"%s\":\"%s\"\n"), "Hash", hash_out);
  ResponseAppendAbst(responseArr);
  sprintf(responseArr, PSTR(",\"%s\":\"%s\"\n"), "Signature", sig_out);
  ResponseAppendAbst(responseArr);
  sprintf(responseArr, PSTR(",\"%s\":\"%s\"\n"), "PublicKey", pubkey_out);
  ResponseAppendAbst(responseArr);
}

 
// int registerMachine(void* anyMsg){
  
//   char machinecid_buffer[58+1] = {0};

//   uint8_t signature[64]={0};
//   char signature_hex[64*2+1]={0};
//   uint8_t hash[32];

//   bool ret_bool = getMachineIDSignature(  private_key_machine_id,  sdk_machineid_public_key, signature, hash);
//   if( ! ret_bool )
//   {
//     sprintf(responseArr, "No machine signature\n");
//     ResponseAppendAbst(responseArr);
//     return -1;
//   }
  
//   toHexString( signature_hex, signature, 64*2);

//   char* gps_str = getGPSstring();
//   if (!gps_str )
//     gps_str = "";

//   int readbytes = readfile( "machinecid", (uint8_t*)machinecid_buffer, 58+1);
//   if( readbytes < 0 )
//     memset((void*)machinecid_buffer,0, 58+1);

//   Planetmintgo__Machine__Metadata metadata = PLANETMINTGO__MACHINE__METADATA__INIT;
//   metadata.additionaldatacid = machinecid_buffer;
//   metadata.gps = gps_str;
//   metadata.assetdefinition = "{\"Version\": \"0.1\"}";
//   metadata.device = "{\"Manufacturer\": \"RDDL\",\"Serial\":\"otherserial\"}";

//   Planetmintgo__Machine__Machine machine = PLANETMINTGO__MACHINE__MACHINE__INIT;
//   machine.name = (char*)sdk_address;
//   machine.ticker = NULL;
//   machine.domain = DEFAULT_DOMAIN_TEXT;
//   machine.reissue = false;
//   machine.amount = 1;
//   machine.precision = 8;
//   machine.issuerplanetmint = sdk_ext_pub_key_planetmint;
//   machine.issuerliquid = sdk_ext_pub_key_liquid;
//   machine.machineid = sdk_machineid_public_key_hex;
//   machine.metadata = &metadata;
//   machine.type = RDDL_MACHINE_POWER_SWITCH;
//   machine.machineidsignature = signature_hex;
//   machine.address = (char*)sdk_address;
 
//   Planetmintgo__Machine__MsgAttestMachine machineMsg = PLANETMINTGO__MACHINE__MSG_ATTEST_MACHINE__INIT;
//   machineMsg.creator = (char*)sdk_address;
//   machineMsg.machine = &machine;
//   int ret = generateAnyAttestMachineMsg((Google__Protobuf__Any*)anyMsg, &machineMsg);
//   if( ret<0 )
//   {
//     sprintf(responseArr, "No Attestation message\n");
//     ResponseAppendAbst(responseArr);
//     return -1;
//   }

//   return 0;
// }


int registerMachine(void* anyMsg, const char* machineCategory, const char* manufacturer, const char* cid){
  
  uint8_t signature[64]={0};
  char signature_hex[64*2+1]={0};
  uint8_t hash[32];

  bool ret_bool = getMachineIDSignature(  private_key_machine_id,  sdk_machineid_public_key, signature, hash);
  if( ! ret_bool )
  {
    sprintf(responseArr, "No machine signature\n");
    ResponseAppendAbst(responseArr);
    return -1;
  }
  
  toHexString( signature_hex, signature, 64*2);

  char* gps_str = getGPSstring();
  if (!gps_str )
    gps_str = "";
  
  size_t desLength = strlen( manufacturer) + strlen( machineCategory )+ 36;
  char* deviceDescription = (char*)getStack( desLength );
  sprintf( deviceDescription, "{\"Category\":\"%s\", \"Manufacturer\":\"%s\"}", machineCategory, manufacturer);


  Planetmintgo__Machine__Metadata metadata = PLANETMINTGO__MACHINE__METADATA__INIT;
  metadata.additionaldatacid = (char*)cid;
  metadata.gps = gps_str;
  metadata.assetdefinition = "{\"Version\":\"0.2\"}";
  metadata.device = deviceDescription;

  Planetmintgo__Machine__Machine machine = PLANETMINTGO__MACHINE__MACHINE__INIT;
  machine.name = (char*)sdk_address;
  
  machine.ticker = NULL;                 //obsolete
  machine.domain = "";                   //obsolete
  machine.reissue = false;               //obsolete
  machine.amount = 1;                    //obsolete
  machine.precision = 8;                 //obsolete
  
  machine.issuerplanetmint = sdk_ext_pub_key_planetmint;
  machine.issuerliquid = sdk_ext_pub_key_liquid;
  machine.machineid = sdk_machineid_public_key_hex;
  machine.metadata = &metadata;
  machine.type = RDDL_MACHINE_POWER_SWITCH;
  machine.machineidsignature = signature_hex;
  machine.address = (char*)sdk_address;

 
  Planetmintgo__Machine__MsgAttestMachine machineMsg = PLANETMINTGO__MACHINE__MSG_ATTEST_MACHINE__INIT;
  machineMsg.creator = (char*)sdk_address;
  machineMsg.machine = &machine;
  int ret = generateAnyAttestMachineMsg((Google__Protobuf__Any*)anyMsg, &machineMsg);
  if( ret<0 )
  {
    sprintf(responseArr, "No Attestation message\n");
    ResponseAppendAbst(responseArr);
    return -1;
  }

  return 0;
}

int sendMessages( void* pAnyMsg) {
  sprintf(responseArr, "TX processing:\n");
  ResponseAppendAbst(responseArr);
  char* tx_payload = create_transaction(pAnyMsg, "1");

  if(!tx_payload)
    return -1;
  sprintf(responseArr, "TX broadcast:\n");
  ResponseAppendAbst(responseArr);
  int broadcast_return = broadcast_transaction( tx_payload );
  return broadcast_return;
}