#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

 
#ifdef TASMOTA
#include "base64_plntmnt.h"
#include "ed25519.h"
#else
#include "base64.h"
#include "ed25519-donna/ed25519.h"
#endif


#include "rddl.h"
#include "rddl_cid.h"
#include "hmac.h"
#include "sha3.h"
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
#include "rddlSDKSettings.h"
#include "configFile.h"

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

static char curlCmd[256];
static char curlOutput[1024];

char responseArr[4096];


/* MAKE IT GENERIC */
/* Bir urlye http get yapip attested olup olmadigina bakiyor */
bool hasMachineBeenAttested() {
  const char *search_string = "name";
  bool status = false;

  // Construct the cURL command
  snprintf(curlCmd, sizeof(curlCmd),
            "curl -X GET \"https://testnet-api.rddl.io/planetmint/machine/get_machine_by_public_key/%s\" -H \"accept: application/json\"",
            sdk_ext_pub_key_planetmint);

  printf("\n%s\n", curlCmd);
  FILE* pipe = popen(curlCmd, "r");

  if (!pipe) {
      perror("popen");
      return false;
  }

  while (fgets(curlOutput, sizeof(curlOutput), pipe) != NULL) {
      printf("%s\n", curlOutput);

      if (strstr(curlOutput, search_string)) {
          printf("SUCCESS\n");
          status = true;
          break;
      }
  }

  pclose(pipe);
  return status;
}


bool rddl_writefile( const char* filename, uint8_t* content, size_t length) {
  int fd;
  ssize_t result = -1;
  char fileExactName[1024] = "./";

  strcat(fileExactName, filename);

  /* If file exist, should I return with O_EXCL, or should I truncate it with O_TRUNC and write new seed? */
  if ((fd = open(fileExactName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1){
    sprintf( responseArr, "ERROR rddl_writefile! Couldnt open file %s", fileExactName);
    printMsg(responseArr);
    close(fd);
    return result;
  }
    
  
  if ((result = write(fd, content, length)) == -1){
    sprintf( responseArr, "ERROR rddl_writefile! Couldnt write file %s", fileExactName);
    printMsg(responseArr);
    close(fd);
    return result;
  }
    

  close(fd);

  return result;
}


/* MAKE IT GENERIC */
/* bir file dan length kadar oku content e yaz */
int readfile( const char* filename, uint8_t* content, size_t length){
  int fd;
  ssize_t result = -1;
  char fileExactName[1024] = "./";

  strcat(fileExactName, filename);

  if ((fd = open(fileExactName, O_RDONLY)) == -1){
    sprintf( responseArr, "ERROR readfile! Couldnt open file %s", fileExactName);
    printMsg(responseArr);
    perror("Error");
    close(fd);
    return result;
  }
  
  if ((result = read(fd, content, length)) == -1){
    sprintf( responseArr, "ERROR readfile! Couldnt read file %s", fileExactName);
    printMsg(responseArr);
    close(fd);
    return result;
  }

  close(fd);

  return result;
}


int printMsg(const char* msg){
  printf("%s\n", msg);
  return 0;
}


int ResponseAppendAbst(const char* msg)   
{
  printf("%s\n", msg);
  return 0;
}


/* MAKE IT GENERIC */
/* Cozemedim */
int ResponseJsonEnd(void)
{
  return ResponseAppendAbst(PSTR("}}"));
}


/* MAKE IT GENERIC */
/* Cozemedim */
char* getGPSstring(){
  return NULL;
}


bool getAccountInfo( uint64_t* account_id, uint64_t* sequence )
{
  // Construct the cURL command
  snprintf(curlCmd, sizeof(curlCmd),
            "curl -X GET \"https://testnet-api.rddl.io/cosmos/auth/v1beta1/account_info/%s\" -H \"accept: application/json\"",
            sdk_address);

  FILE* pipe = popen(curlCmd, "r");

  if (!pipe) {
      perror("popen");
      return false;
  }

  while (fgets(curlOutput, sizeof(curlOutput), pipe) != NULL) {
      printf("%s\n", curlOutput);
  }

  pclose(pipe);

  int _account_id = 0;
  int _sequence = 0;

  bool ret = get_account_info( curlOutput ,&_account_id, &_sequence );
  if( ret )
  {
    *account_id = (uint64_t) _account_id;
    *sequence = (uint64_t) _sequence;
  }
  else{
    sprintf( responseArr, "Account parsing issue\n");
    printMsg(responseArr);
  }

  return ret;
}


/* MAKE IT GENERIC */
/* Cozemedim */
int broadcast_transaction( char* tx_payload ){
  const char* curlCommand = "curl -X POST";
  char url[4096];
  snprintf(url, sizeof(url), "%s/cosmos/tx/v1beta1/txs", getSetting( SDK_SET_PLANETMINT_API));
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


char* getSetting(uint32_t index){
  switch(index) {
SDK_SET_NOTARIZTATION_PERIODICITY:
    if( strlen( sdk_periodicity) == 0 ){
      if( !readfile(SETTINGS_PERIODICITY_FILE, ((uint8_t*)sdk_periodicity, 20) ) )
        strcpy(sdk_periodicity, DEFAULT_PERIODICITY_TEXT);
    }
    return sdk_periodicity;
SDK_SET_PLANETMINT_API:
    if( strlen( sdk_planetmintapi) == 0 ){
      if( !readfile(SETTINGS_API_FILE, ((uint8_t*)sdk_planetmintapi, 100) ) )
        strcpy(sdk_planetmintapi, DEFAULT_API_TEXT);
    }
    return sdk_planetmintapi;
SDK_SET_PLANETMINT_CHAINID:
    if( strlen( sdk_chainid) == 0 ){
      if( !readfile(SETTINGS_CHAINID_FILE, ((uint8_t*)sdk_chainid, 30) ) )
        strcpy(sdk_chainid, DEFAULT_API_TEXT);
    }
    return sdk_chainid;
SDK_SET_PLANETMINT_DENOM:
    if( strlen( sdk_denom) == 0 )
      if( !readfile(SETTINGS_DENOM_FILE, ((uint8_t*)sdk_chainid, 20) ) )
        strcpy(sdk_denom, DEFAULT_DENOM_TEXT);
    return sdk_denom;
default:
    return NULL;
  }
}

bool setSetting(uint32_t index, const char* replacementText){
  bool retValue = false;
  switch(index) {
SDK_SET_NOTARIZTATION_PERIODICITY:
    retValue = rddl_writefile( SETTINGS_PERIODICITY_FILE, (uint8_t*)replacementText, strlen(replacementText));
    memset(sdk_periodicity,0,20);
    break;  
SDK_SET_PLANETMINT_API:   
    retValue = rddl_writefile( SETTINGS_API_FILE, (uint8_t*)replacementText, strlen(replacementText));
    memset(sdk_planetmintapi,0,100);
    break;  
SDK_SET_PLANETMINT_CHAINID:
    retValue = rddl_writefile( SETTINGS_CHAINID_FILE, (uint8_t*)replacementText, strlen(replacementText));
    memset(sdk_chainid,0,30);
    break;  
SDK_SET_PLANETMINT_DENOM:
    retValue = rddl_writefile( SETTINGS_DENOM_FILE, (uint8_t*)replacementText, strlen(replacementText));
    memset(sdk_denom,0,20);
    break;  
default:
    retValue =  false;
  }
  return retValue;
}
