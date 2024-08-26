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
#include <stdarg.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
 
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
#include "keys.h"
#include "planetmintgo.h"
#include "planetmintgo/machine/machine.pb-c.h"
#include "cosmos/tx/v1beta1/tx.pb-c.h"
#include "planetmintgo/machine/tx.pb-c.h"
#include "planetmintgo/asset/tx.pb-c.h"
#include "google/protobuf/any.pb-c.h"

#include "rddlSDKAbst.h"
#include "rddlSDKSettings.h"
#include "rddlSDKUtils.h"
#include "configFile.h"

char sdk_periodicity[20] = {0};
char sdk_planetmintapi[100] = {0};

char sdk_chainid[30] = {0};
char sdk_denom[20] = {0};

bool sdk_readSeed = false;

static char curlCmd[512];
static char curlOutput[2048];

char responseArr[4096];
uint32_t num_of_cid_files = 0;

char challengedCID[ 64 ] = {0};
PoPInfo popParticipation;
void resetPopInfo(){  memset( &popParticipation, 0, sizeof(PoPInfo)); }

/* MAKE IT GENERIC */
/* Bir urlye http get yapip attested olup olmadigina bakiyor */
bool hasMachineBeenAttested() {
  const char *search_string = "name";
  bool status = false;

  // Construct the cURL command
  snprintf(curlCmd, sizeof(curlCmd),
            "curl -X GET \"%s/planetmint/machine/get_machine_by_public_key/%s\" -H \"accept: application/json\"",
            DEFAULT_API_TEXT, getExtPubKeyPlanetmint());

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


void AddLogLineAbst(const char* msg, ...)   
{
  va_list args;
  va_start(args, msg);
  vAddLogLineAbst(msg, args);
  va_end(args);
}

void vAddLogLineAbst( const char* msg, va_list args ){
  vprintf(msg, args);
}



/* MAKE IT GENERIC */
/* Cozemedim */
char* getGPSstring(){
  return NULL;
}

uint8_t* abstGetStack( size_t size ){
  return getStack( size );
}


void abstClearStack() {
  clearStack();
}


bool getAccountInfo( uint64_t* account_id, uint64_t* sequence )
{
  // Construct the cURL command
  snprintf(curlCmd, sizeof(curlCmd),
            "curl -X GET \"%s/cosmos/auth/v1beta1/account_info/%s\" -H \"accept: application/json\"",
            DEFAULT_API_TEXT, getRDDLAddress());

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





bool getPoPInfo( const char* blockHeight){

  // Construct the cURL command
  snprintf(curlCmd, sizeof(curlCmd),
            "curl -X GET \"%s/planetmint/planetmint-go/dao/get_challenge/%s\" -H \"accept: application/json\"",
            DEFAULT_API_TEXT, blockHeight);

  FILE* pipe = popen(curlCmd, "r");

  if (!pipe) {
      perror("popen");
      return false;
  }
  while (fgets(curlOutput, sizeof(curlOutput), pipe) != NULL) {
      printf("%s\n", curlOutput);
  }

  pclose(pipe);

  return getPoPInfoFromJSON( curlOutput );
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
case SDK_SET_NOTARIZTATION_PERIODICITY:
    if( strlen( sdk_periodicity) == 0 ){
      if( !readfile(SETTINGS_PERIODICITY_FILE, (uint8_t*)sdk_periodicity, 20) )
        strcpy(sdk_periodicity, DEFAULT_PERIODICITY_TEXT);
    }
    return sdk_periodicity;
case SDK_SET_PLANETMINT_API:
    if( strlen( sdk_planetmintapi) == 0 ){
      if( !readfile(SETTINGS_API_FILE, (uint8_t*)sdk_planetmintapi, 100) )
        strcpy(sdk_planetmintapi, DEFAULT_API_TEXT);
    }
    return sdk_planetmintapi;
case SDK_SET_PLANETMINT_CHAINID:
    if( strlen( sdk_chainid) == 0 ){
      if( !readfile(SETTINGS_CHAINID_FILE, (uint8_t*)sdk_chainid, 30) )
        strcpy(sdk_chainid, DEFAULT_API_TEXT);
    }
    return sdk_chainid;
case SDK_SET_PLANETMINT_DENOM:
    if( strlen( sdk_denom) == 0 )
      if( !readfile(SETTINGS_DENOM_FILE, (uint8_t*)sdk_chainid, 20) )
        strcpy(sdk_denom, DEFAULT_DENOM_TEXT);
    return sdk_denom;
default:
    return NULL;
  }
}

bool setSetting(uint32_t index, const char* replacementText){
  bool retValue = false;
  switch(index) {
case SDK_SET_NOTARIZTATION_PERIODICITY:
    retValue = rddl_writefile( SETTINGS_PERIODICITY_FILE, (uint8_t*)replacementText, strlen(replacementText));
    memset(sdk_periodicity,0,20);
    break;  
case SDK_SET_PLANETMINT_API:   
    retValue = rddl_writefile( SETTINGS_API_FILE, (uint8_t*)replacementText, strlen(replacementText));
    memset(sdk_planetmintapi,0,100);
    break;  
case SDK_SET_PLANETMINT_CHAINID:
    retValue = rddl_writefile( SETTINGS_CHAINID_FILE, (uint8_t*)replacementText, strlen(replacementText));
    memset(sdk_chainid,0,30);
    break;  
case SDK_SET_PLANETMINT_DENOM:
    retValue = rddl_writefile( SETTINGS_DENOM_FILE, (uint8_t*)replacementText, strlen(replacementText));
    memset(sdk_denom,0,20);
    break;  
default:
    retValue =  false;
  }
  return retValue;
}


char* getCIDsLinux( const char* address, int cidsToBeQueried ){
  // Construct the cURL command
  char uri[200] = {0};
  const char* cmd = "planetmint/asset/get_cids_by_address";
  snprintf( uri, sizeof(uri), "%s/%s/%s/%i", DEFAULT_API_TEXT, cmd, address, cidsToBeQueried);
  snprintf(curlCmd, sizeof(curlCmd),
            "curl -X GET \"%s\" -H \"accept: application/json\"", uri);

  FILE* pipe = popen(curlCmd, "r");

  if (!pipe) {
      perror("popen");
      return false;
  }

  while (fgets(curlOutput, sizeof(curlOutput), pipe) != NULL) {
      printf("%s\n", curlOutput);
  }

  pclose(pipe);
  return curlOutput;
}

char* getCIDtoBeChallenged(int cidsToBeQueried){
  char* jsonObject = getCIDsLinux( (const char*)popParticipation.challengee, cidsToBeQueried );
  memset( challengedCID, 0, 64);
  if (GetRandomElementFromCIDJSONList(jsonObject, challengedCID, 64) < 0)
    return NULL;
  return challengedCID;
}

int abstGetNumOfCIDFiles(const char* path){
  DIR *dir;
  struct dirent *entry;
  int count = 0;

  if ((dir = opendir(path)) != NULL) {
      while ((entry = readdir(dir)) != NULL) {
          // Check if the file name length is longer than 20 characters
          if (strlen(entry->d_name) > 20) {
              count++;
          }
      }
      closedir(dir);
  } else {
      perror("Unable to open directory");
      return -1;
  }

  return count;
}


int abstDeleteOldestCIDFile(const char* path){
  DIR *dir;
  struct dirent *entry;
  struct stat fileStat;
  time_t oldestTime = time(NULL);
  char oldestFileName[256];
  int found = 0;

  if(num_of_cid_files < MAX_CID_FILE_SIZE)
    return -1;

  if ((dir = opendir(path)) != NULL) {
    while ((entry = readdir(dir)) != NULL) {
        char filePath[512];
        sprintf(filePath, "%s/%s", path, entry->d_name);

        if (stat(filePath, &fileStat) == 0) {
            // Check if the file name length is longer than 20 characters
            if (strlen(entry->d_name) > 20) {
                if (difftime(fileStat.st_mtime, oldestTime) < 0) {
                    oldestTime = fileStat.st_mtime;
                    strcpy(oldestFileName, entry->d_name);
                    found = 1;
                }
            }
        }
    }
    closedir(dir);

    if (found) {
        char filePathToDelete[512];
        sprintf(filePathToDelete, "%s/%s", path, oldestFileName);
        if (remove(filePathToDelete) != 0) {
            perror("Unable to delete file");
            return -1;
        } else {
            return 0;
        }
    } else {
        return -1;
    }
  } else {
    perror("Unable to open directory");
    return -1;
  }
}

void SubscribeAbst( const char * ){
  return;
}

void UnsubscribeAbst( const char * ){
  return;
}

int createAccountCall( const char* baseURI, const char* account_address,
  const char* machineID, const char* signature, char*) {
  
  const char* curlCommand = "curl -X POST";
  char url[4096];
  snprintf(url, sizeof(url), "%s/create-account", baseURI);
  const char* headers = "-H \"accept: application/json\" -H \"Content-Type: application/json\"";
  char payload [4000] = {0};
  char* formatString = "{ \"machine-id\": \"%s\", \"plmnt-address\": \"%s\", \"signature\": \"%s\" }";
  sprintf( payload, formatString, machineID, account_address, signature);


  char curlCmd[8192];
  snprintf(curlCmd, sizeof(curlCmd), "%s \"%s\" %s -d '%s'", curlCommand, url, headers, payload);
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
