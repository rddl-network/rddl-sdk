#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h> 
#include <errno.h>
#include <fcntl.h>
#include <time.h>

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


uint8_t* sdkGetStack( size_t size ){
  return abstGetStack(size);
}

void sdkClearStack(){
  abstClearStack();
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
  abstClearStack();
  if( !getPlntmntKeys() )
    return;

  if( !hasMachineBeenAttested() )
    return;

  size_t data_size = data_length;
  uint8_t* local_data = abstGetStack( data_size+2 );
  memcpy( local_data, data_str, data_size);

  //compute CID
  char* cid_str = create_cid_v1_from_string( (const char*) local_data );

  // Add timestamp to the end of cid
  char cid_name[100] = {0};
  long curr_time;
  time(&curr_time);
  sprintf(cid_name, "%s%ld",cid_str, curr_time);

  // store cid
  rddl_writefile( cid_name, (uint8_t*)local_data, data_size );
  checkNumOfCIDFiles(CID_FILE_DIRECTORY);

  // register CID
  //registerCID( cid_str );

  sprintf(responseArr, "Notarize: CID Asset %s - file %s\n", cid_str, cid_name);
  printMsg(responseArr);
  AddLogLineAbst(responseArr);

  generateAnyCIDAttestMsg(&anyMsg, cid_str, sdk_priv_key_planetmint, sdk_pub_key_planetmint, sdk_address, sdk_ext_pub_key_planetmint );
  sendMessages( &anyMsg );
}

bool amIChallenger(){
  if( getPlntmntKeys() )
    return (strcmp( (const char*)sdk_address, (const char*)popParticipation.challenger ) == 0);
  return false;
}

bool getPoPFromChain(const char* blockHeight ){
  return getPoPInfo( blockHeight );
}
bool verifyCIDIntegrity( const char* cid, const char* content )
{
  bool valid = false;
  if( !content )
    return valid;

  char* cid_str = create_cid_v1_from_string( content );
  if( cid_str && cid && strcasecmp( cid, cid_str) == 0)
    valid = true;

  return valid;
}

bool processPoPChallengeResponse( const char* jsonResponse, size_t length){
    Google__Protobuf__Any anyMsg = GOOGLE__PROTOBUF__ANY__INIT;


    char subscriptionTopic[200] = {0};
    sprintf( subscriptionTopic, "stat/%s/POPCHALLENGERESULT", popParticipation.challengee);
    UnsubscribeAbst(subscriptionTopic);

    sdkClearStack();
    uint8_t* dataBuffer = sdkGetStack( length );
    uint8_t* cidBuffer = sdkGetStack( 64 );
    uint8_t encoding[10] = {0};
    int failed = copyJsonValueString( dataBuffer, length, jsonResponse, "data");
    if( failed ) {
      AddLogLineAbst( "RET: could not extract data");
      return false;
    }
    failed = copyJsonValueString( encoding, 10, jsonResponse, "encoding");
    if( failed ) {
      AddLogLineAbst( "RET: could not extract encoding");
      return false;
    }
    failed = copyJsonValueString( cidBuffer, 64, jsonResponse, "cid");
    if( failed ) {
      AddLogLineAbst( "RET: could not extract CID");
      return false;
    }
    if (strcmp( "hex", encoding) != 0 ){
      AddLogLineAbst( "RET: unsupported encoding %s", encoding);
      return false;
    }
    if( strcmp( challengedCID, cidBuffer ) != 0 ){
      AddLogLineAbst( "RET: wrong CID - exp: %s deliviered: %s ", challengedCID, cidBuffer);
      return false;
    }
    const uint8_t* contentBytes = fromHexString(dataBuffer);
    AddLogLineAbst( "RET: %s", jsonResponse);
    bool PoPSuccess = verifyCIDIntegrity( cidBuffer, contentBytes);

    //send out pop result to network
    int resultPoPResult = CreatePoPResult( &anyMsg, PoPSuccess );
    if( resultPoPResult >= 0 )
      sendMessages(&anyMsg);
    else
      AddLogLineAbst( "RET: PoP unable to create PoPResult: %i", resultPoPResult);

    if( !PoPSuccess ){
      AddLogLineAbst( "RET: CID verification failed: %s - %s ", cidBuffer, contentBytes);
      return false;
    }
    AddLogLineAbst( "RET: PoP successfully passed");
    return true;
}

#ifndef LINUX_MACHINE
extern void MqttSubscribe(const char *topic);
extern void MqttPublishPayload(const char* topic, const char* payload);


bool ChallengeChallengee( const char* cid, const char* address ){
  
  if( !address )
    address = (const char*) popParticipation.challengee;

  char subscriptionTopic[200] = {0};
  char publishingTopic[200] = {0};
  sprintf( subscriptionTopic, "stat/%s/POPCHALLENGERESULT", address);
  SubscribeAbst(subscriptionTopic);

  
  //get cid for challengee address
  strcpy(challengedCID, cid);

  //const char* cid = ;
  sprintf( publishingTopic, "cmnd/%s/PoPChallenge", address);
  PublishPayloadAbst(publishingTopic, challengedCID);
  return true;

}
#endif

char* getCIDofChallengee(){
  return getCIDtoBeChallenged();
}
