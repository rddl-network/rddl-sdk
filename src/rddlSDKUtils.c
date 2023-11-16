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
#include "base64.h"
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


char* create_transaction( void* anyMsg, char* tokenAmount )
{
  uint64_t account_id = 0;
  uint64_t sequence = 0;
  bool gotAccountID = getAccountInfo( &account_id, &sequence );
  if( !gotAccountID )
  {
    account_id = (uint64_t) atoi( (const char*) getAccountID());
  }

  Cosmos__Base__V1beta1__Coin coin = COSMOS__BASE__V1BETA1__COIN__INIT;
  coin.denom = getDenom();
  coin.amount = tokenAmount;
  
  uint8_t* txbytes = NULL;
  size_t tx_size = 0;
  char* chain_id = getChainID();
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
  else
    ResponseAppend_P("not engouth memory:\n");

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


void getPlntmntKeys(){
  
  /* Seed i oku*/
  readSeed();

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

  // char hexBuf[256];
  // toHexString(hexBuf, node_planetmint.public_key, PUB_KEY_SIZE*2);
  // printf("%s\n", hexBuf);

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

  // printf("%s\n",sdk_ext_pub_key_planetmint);

  ecdsa_get_public_key33(&secp256k1, private_key_machine_id, sdk_machineid_public_key);
  toHexString( sdk_machineid_public_key_hex, sdk_machineid_public_key, 33*2);
  printf("Machine Public Key: %s\n", sdk_machineid_public_key_hex);
}


void signRDDLNetworkMessageContent( const char* data_str, size_t data_length, char* sig_out){
  char pubkey_out[66+1] = {0};
  char hash_out[64+1] = {0};
  if( readSeed() != NULL )
  {
    SignDataHash( data_str, data_length,  pubkey_out, sig_out, hash_out);
  }

  /* Bunlar PlatformIO Specific */
  ResponseAppend_P(PSTR(",\"%s\":\"%s\"\n"), "Hash", hash_out);
  ResponseAppend_P(PSTR(",\"%s\":\"%s\"\n"), "Signature", sig_out);
  ResponseAppend_P(PSTR(",\"%s\":\"%s\"\n"), "PublicKey", pubkey_out);
}


int registerMachine(void* anyMsg){
  
  char machinecid_buffer[58+1] = {0};

  uint8_t signature[64]={0};
  char signature_hex[64*2+1]={0};
  uint8_t hash[32];

  bool ret_bool = getMachineIDSignature(  private_key_machine_id,  sdk_machineid_public_key, signature, hash);
  if( ! ret_bool )
  {
    ResponseAppend_P("No machine signature\n");
    return -1;
  }
  
  toHexString( signature_hex, signature, 64*2);

  char* gps_str = getGPSstring();
  if (!gps_str )
    gps_str = "";

  int readbytes = readfile( "machinecid", (uint8_t*)machinecid_buffer, 58+1);
  if( readbytes < 0 )
    memset((void*)machinecid_buffer,0, 58+1);

  Planetmintgo__Machine__Metadata metadata = PLANETMINTGO__MACHINE__METADATA__INIT;
  metadata.additionaldatacid = machinecid_buffer;
  metadata.gps = gps_str;
  metadata.assetdefinition = "{\"Version\": \"0.1\"}";
  metadata.device = "{\"Manufacturer\": \"RDDL\",\"Serial\":\"otherserial\"}";

  Planetmintgo__Machine__Machine machine = PLANETMINTGO__MACHINE__MACHINE__INIT;
  machine.name = (char*)sdk_address;
  machine.ticker = NULL;
  machine.domain = DEFAULT_DOMAIN_TEXT;
  machine.reissue = false;
  machine.amount = 1;
  machine.precision = 8;
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
    ResponseAppend_P("No Attestation message\n");
    return -1;
  }

  return 0;
}


void setDenom( const char* denom, size_t len)
{
  rddl_writefile( "planetmintdenom", (uint8_t*)denom, len);
  memset((void*)sdk_denom,0, sizeof(sdk_denom));
}


char* getDenom()
{
  if( strlen( sdk_denom) == 0 )
    strcpy(sdk_denom, DEFAULT_DENOM_TEXT);
  return sdk_denom;
}


char* getChainID()
{
  if( strlen( sdk_chainid) == 0 )
    strcpy(sdk_chainid, DEFAULT_CHAINID_TEXT);

  return sdk_chainid;
}


char* getAccountID()
{
  if( strlen( sdk_accountid) == 0 ){
      int readbytes = readfile( "accountid", (uint8_t*)sdk_accountid, 20);
    if( readbytes < 0 )
      memset((void*)sdk_planetmintapi,0, sizeof(sdk_planetmintapi));
  }
  return sdk_accountid;
}


void setAccountID( const char* account, size_t len)
{
  rddl_writefile( "accountid", (uint8_t*)account, len);
  memset((void*)sdk_accountid,0, sizeof(sdk_accountid));
}
