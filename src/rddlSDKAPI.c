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
#include "rddlSDKAPI.h"


/* MAKE IT GENERIC */
/* Cozemedim */
int broadcast_transaction( char* tx_payload ){
  const char* curlCommand = "curl -X POST";
  char url[4096];
  snprintf(url, sizeof(url), "%s/cosmos/tx/v1beta1/txs", getPlanetmintAPI());
  const char* headers = "-H \"accept: application/json\" -H \"Content-Type: application/json\"";
  
  char curlCmd[8192];
  snprintf(curlCmd, sizeof(curlCmd), "%s \"%s\" %s -d '%s'", curlCommand, url, headers, tx_payload);
  printf("\n%s\n", curlCmd);

  static char curlOutput[1024];
  FILE* pipe = popen(curlCmd, "r");

  if (!pipe) {
      perror("popen");
      return false;
  }

  while (fgets(curlOutput, sizeof(curlOutput), pipe) != NULL) {
      printf("CURL RESPONSE:\n%s\n", curlOutput);
  }

  pclose(pipe);

  return 0;
}


/* MAKE IT GENERIC */
/* Cozemedim */
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
  int ret = prepareTx( (Google__Protobuf__Any*)anyMsg, &coin, g_priv_key_planetmint, g_pub_key_planetmint, sequence, chain_id, account_id, &txbytes, &tx_size);
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


uint8_t* readSeed()
{
  if( g_readSeed )
    return secret_seed;

  // int readbytes = readfile( "seed", secret_seed, SEED_SIZE);
  int readbytes = SEED_SIZE;
  memcpy(secret_seed, "46642d8b5ee5bcefd523ee4d4340a8956affbef5bb1978eb1e3f640318f87f4b6438733879360f460932fa68bfc06ae8aaf837b5d45891d114a58d8ce19a17d7", SEED_SIZE);
  
  if( readbytes != SEED_SIZE )
    return NULL;

  g_readSeed = true;
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
  memcpy(g_priv_key_planetmint, node_planetmint.private_key, 32);
  memcpy(g_pub_key_planetmint, node_planetmint.public_key, PUB_KEY_SIZE);

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
  memcpy(g_priv_key_liquid, node_rddl.private_key, 32);
  memcpy(g_pub_key_liquid, node_rddl.public_key, PUB_KEY_SIZE);

  uint8_t address_bytes[ADDRESS_TAIL] = {0};
  pubkey2address( g_pub_key_planetmint, PUB_KEY_SIZE, address_bytes );
  getAddressString( address_bytes, g_address);
  uint32_t fingerprint = hdnode_fingerprint(&node_planetmint);
  hdnode_serialize_public( &node_planetmint, fingerprint, PLANETMINT_PMPB, g_ext_pub_key_planetmint, EXT_PUB_KEY_SIZE);
  hdnode_serialize_public( &node_rddl, fingerprint, VERSION_PUBLIC, g_ext_pub_key_liquid, EXT_PUB_KEY_SIZE);

  // printf("%s\n",g_ext_pub_key_planetmint);

  ecdsa_get_public_key33(&secp256k1, private_key_machine_id, g_machineid_public_key);
  toHexString( g_machineid_public_key_hex, g_machineid_public_key, 33*2);
  // printf("new: %s\n", g_machineid_public_key_hex);
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

  bool ret_bool = getMachineIDSignature(  private_key_machine_id,  g_machineid_public_key, signature, hash);
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
  machine.name = (char*)g_address;
  machine.ticker = NULL;
  machine.domain = "lab.r3c.network";
  machine.reissue = false;
  machine.amount = 1;
  machine.precision = 8;
  machine.issuerplanetmint = g_ext_pub_key_planetmint;
  machine.issuerliquid = g_ext_pub_key_liquid;
  machine.machineid = g_machineid_public_key_hex;
  machine.metadata = &metadata;
  machine.type = RDDL_MACHINE_POWER_SWITCH;
  machine.machineidsignature = signature_hex;
  machine.address = (char*)g_address;
 
  Planetmintgo__Machine__MsgAttestMachine machineMsg = PLANETMINTGO__MACHINE__MSG_ATTEST_MACHINE__INIT;
  machineMsg.creator = (char*)g_address;
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
  memset((void*)g_denom,0, sizeof(g_denom));
}


char* getDenom()
{
  if( strlen( g_denom) == 0 ){
      int readbytes = readfile( "planetmintdenom", (uint8_t*)g_denom, 20);
    if( readbytes < 0 )
      memset((void*)g_denom,0, sizeof(g_denom));
  }
  if( strlen( g_denom) == 0 )
    strcpy(g_denom, "plmnt");
  return g_denom;
}


char* getChainID()
{
  if( strlen( g_chainid) == 0 ){
      int readbytes = readfile( "planetmintchainid", (uint8_t*)g_chainid, 30);
    if( readbytes < 0 )
      memset((void*)g_chainid,0, sizeof(g_chainid));
  }
  if( strlen( g_chainid) == 0 )
    strcpy(g_chainid, "planetmint-testnet-1");

  return g_chainid;
}


/* MAKE IT GENERIC */
/* Cozemedim */
char* getAccountID()
{
  if( strlen( g_accountid) == 0 ){
      int readbytes = readfile( "accountid", (uint8_t*)g_accountid, 20);
    if( readbytes < 0 )
      memset((void*)g_planetmintapi,0, sizeof(g_planetmintapi));
  }
  return g_accountid;
}


/* MAKE IT GENERIC */
/* Cozemedim */
void setAccountID( const char* account, size_t len)
{
  rddl_writefile( "accountid", (uint8_t*)account, len);
  memset((void*)g_accountid,0, sizeof(g_accountid));
}


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
