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
  #include "libs/base64_planetmint/src/base64_plntmnt.h"
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
  char* payload = (char*) abstGetStack( allocation_size );
  if( payload )
  {
    memset( payload, 0, allocation_size );
    strcpy(payload, "{ \"tx_bytes\": \"" );
    bintob64( payload+ strlen( payload ), txbytes, tx_size);
    strcpy( payload+ strlen( payload ), "\", \"mode\": \"BROADCAST_MODE_SYNC\" }");
  }
  else{
    sprintf(responseArr, "not engouth memory:\n");
    AddLogLineAbst(responseArr);
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
  AddLogLineAbst(responseArr);

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
  AddLogLineAbst(responseArr);
  sprintf(responseArr, PSTR(",\"%s\":\"%s\"\n"), "Signature", sig_out);
  AddLogLineAbst(responseArr);
  sprintf(responseArr, PSTR(",\"%s\":\"%s\"\n"), "PublicKey", pubkey_out);
  AddLogLineAbst(responseArr);
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
//     AddLogLineAbst(responseArr);
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
//     AddLogLineAbst(responseArr);
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
    AddLogLineAbst(responseArr);
    return -1;
  }
  
  toHexString( signature_hex, signature, 64*2);

  char* gps_str = getGPSstring();
  if (!gps_str )
    gps_str = "";
  
  size_t desLength = strlen( manufacturer) + strlen( machineCategory )+ 36;
  char* deviceDescription = (char*)abstGetStack( desLength );
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

#ifdef LINUX_MACHINE
  free(deviceDescription);
#endif

  if( ret<0 )
  {
    sprintf(responseArr, "No Attestation message\n");
    AddLogLineAbst(responseArr);
    return -1;
  }

  return 0;
}

int sendMessages( void* pAnyMsg) {
  sprintf(responseArr, "TX processing:\n");
  AddLogLineAbst(responseArr);
  char* tx_payload = create_transaction(pAnyMsg, "1");

  if(!tx_payload)
    return -1;
  sprintf(responseArr, "TX broadcast:\n");
  AddLogLineAbst(responseArr);
  int broadcast_return = broadcast_transaction( tx_payload );

#ifdef LINUX_MACHINE
  free(tx_payload);
#endif

  return broadcast_return;
}


int copyJsonValueString(char *buffer, size_t buffer_len, const char *json, const char *key) {
    char key_pattern[100];  // Size depends on expected length of keys
    sprintf(key_pattern, "\"%s\": \"", key); // Constructs the key pattern

    char *start = strstr(json, key_pattern); // Finds the key in JSON
    if (start == NULL) {
      sprintf(key_pattern, "\"%s\":\"", key);
      start = strstr(json, key_pattern);
      if (start == NULL) {
        printf("Key not found.\n");
        return -1;
      }
    }

    start += strlen(key_pattern); // Moves to the value part
    char *end = strchr(start, '\"'); // Finds the end of the value

    if (end == NULL) {
        printf("Invalid JSON format.\n");
        return -2;
    }

    size_t value_len = end - start;
    if (value_len >= buffer_len) {
        printf("Buffer too small.\n");
        return -3;
    }

    strncpy(buffer, start, value_len);
    buffer[value_len] = '\0'; // Null-terminate the string
    return 0;
}



int parseJsonBoolean(const char *json, const char *key, bool *result) {
    char key_pattern[100]; // Adjust size as needed
    sprintf(key_pattern, "\"%s\": ", key);

    char *key_ptr = strstr(json, key_pattern);
    if (key_ptr == NULL) {
      sprintf(key_pattern, "\"%s\":", key);
      key_ptr = strstr(json, key_pattern);
      if (key_ptr == NULL) {
        return -1; // Key not found
      }
    }

    key_ptr += strlen(key_pattern); // Move to value position

    if (strncmp(key_ptr, "true", 4) == 0) {
        *result = true;
        return 0;
    } else if (strncmp(key_ptr, "false", 5) == 0) {
        *result = false;
        return 0;
    }

    return -2; // Invalid boolean value
}

bool convertStringToInt64( const char* valueString, int64_t* targetValue ){

  char* endptr;
  errno = 0;
  *targetValue = strtoll(valueString, &endptr, 10); // Convert string to int64_t

  // Check for various possible errors
  if ((errno == ERANGE && (*targetValue == LLONG_MAX || *targetValue == LLONG_MIN)) || (errno != 0 && *targetValue == 0)) {
      perror("Conversion error");
      return false;
  }
  if (endptr == valueString) {
      fprintf(stderr, "No digits were found\n");
      return false;
  }
  return true;
}

bool getPoPInfoFromJSON( const char* json){
  AddLogLineAbst( "PoPInfo: %s", json );
  resetPopInfo();
  int result = copyJsonValueString( popParticipation.challengee, sizeof(popParticipation.challengee), json, "challengee");
  if( result ){
    return false;
  }
  result = copyJsonValueString( popParticipation.challenger, sizeof(popParticipation.challenger), json, "challenger");
  if( result ){
    resetPopInfo();
    return false;
  }

  char blockHeight[30] = {0};
  result = copyJsonValueString( blockHeight, sizeof(blockHeight), json, "height");
  if( result ){
    resetPopInfo();
    return false;
  }
  result = convertStringToInt64( blockHeight, &popParticipation.blockHeight);
  if( !result ){
    resetPopInfo();
    return false;
  }
  
  result = parseJsonBoolean(json, "finished", &popParticipation.finished);
  if( result ){
    resetPopInfo();
    return false;
  }
  
  return true;
}
