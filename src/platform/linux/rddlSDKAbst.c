#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
 
#include "planetmint.h"
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

uint8_t g_priv_key_planetmint[32+1] = {0};
uint8_t g_priv_key_liquid[32+1] = {0};
uint8_t g_pub_key_planetmint[33+1] = {0};
uint8_t g_pub_key_liquid[33+1] = {0};
uint8_t g_machineid_public_key[33+1]={0}; 


char g_address[64] = {0};
char g_ext_pub_key_planetmint[EXT_PUB_KEY_SIZE+1] = {0};
char g_ext_pub_key_liquid[EXT_PUB_KEY_SIZE+1] = {0};
char g_machineid_public_key_hex[33*2+1] = {0};

char g_planetmintapi[100] = {0};
char g_accountid[20] = {0};
char g_chainid[30] = {0};
char g_denom[20] = {0};

bool g_readSeed = false;

static char curlCmd[256];
static char curlOutput[1024];


const char* getPlanetmintAPI() {
    // Implement your logic to get the API URL here
    return "https://testnet-api.rddl.io";
}


/* MAKE IT GENERIC */
/* Bir urlye http get yapip attested olup olmadigina bakiyor */
bool hasMachineBeenAttested() {
  const char *search_string = "name";
  bool status = false;

  // Construct the cURL command
  snprintf(curlCmd, sizeof(curlCmd),
            "curl -X GET \"https://testnet-api.rddl.io/planetmint/machine/get_machine_by_public_key/%s\" -H \"accept: application/json\"",
            g_ext_pub_key_planetmint);

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


/* MAKE IT GENERIC */
/* bir file a content i length kadar yaz */
bool rddl_writefile( const char* filename, uint8_t* content, size_t length) {
  int fd;
  ssize_t result = -1;
  char fileExactName[1024] = "./";

  strcat(fileExactName, filename);

  if ((fd = open(fileExactName, O_RDWR | O_CREAT | O_EXCL)) == -1){
    printMsg("ERROR rddl_writefile! Couldnt open file %s", fileExactName);
    close(fd);
    return result;
  }
    
  
  if ((result = write(fd, content, length)) == -1){
    printMsg("ERROR rddl_writefile! Couldnt write file %s", fileExactName);
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
    printMsg("ERROR readfile! Couldnt open file %s", fileExactName);
    close(fd);
    return result;
  }
  
  if ((result = read(fd, content, length)) == -1){
    printMsg("ERROR readfile! Couldnt read file %s", fileExactName);
    close(fd);
    return result;
  }

  close(fd);

  return result;
}


/* MAKE IT GENERIC */
/* Cozemedim */
int ResponseAppend_P(const char* format, ...)  // Content send snprintf_P char data
{
  return 0;
}


/* MAKE IT GENERIC */
/* Cozemedim */
int ResponseJsonEnd(void)
{
  return ResponseAppend_P(PSTR("}}"));
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
            g_address);

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
  else
    ResponseAppend_P("Account parsing issue\n");

  return ret;
}



