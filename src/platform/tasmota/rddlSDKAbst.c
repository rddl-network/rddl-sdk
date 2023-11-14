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
  HTTPClientLight http;

  String uri = "/planetmint/machine/get_machine_by_public_key/";
  uri = getPlanetmintAPI() + uri;
  uri = uri + g_ext_pub_key_planetmint;
  http.begin(uri);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.GET();

  return (httpResponseCode == 200);
}


bool rddl_writefile( const char* filename, uint8_t* content, size_t length) {
  char* filename_local = (char*) getStack( strlen( filename ) +3 );
  memset( filename_local, 0, strlen( filename ) +3 );
  filename_local[0]= '/';
  int limit = 35;
  int offset = 0;
  if( strlen(filename) > limit)
    offset = strlen(filename) -limit;
  strcpy( &filename_local[1], filename+ offset);

#ifdef USE_UFILESYS
  if (TfsSaveFile(filename_local, (const uint8_t*)content, length)) {
    AddLog(LOG_LEVEL_DEBUG, PSTR("Failed to write file content"));
    return false;
  } 
  return true;
#else
  return false;
#endif  // USE_UFILESYS
}


/* MAKE IT GENERIC */
/* bir file dan length kadar oku content e yaz */
int readfile( const char* filename, uint8_t* content, size_t length){
  char* filename_local = (char*) getStack( strlen( filename ) +2 );
  filename_local[0]= '/';
  int limit = 35;
  int offset = 0;
  if( strlen(filename) > limit)
    offset = strlen(filename) -limit;
  strcpy( &filename_local[1], filename+ offset);

  return TfsLoadFile(filename_local, (uint8_t*)content, length);
}


int ResponseAppend_P(const char* format, ...)  // Content send snprintf_P char data
{
  // This uses char strings. Be aware of sending %% if % is needed
#ifdef MQTT_DATA_STRING
  va_list arg;
  va_start(arg, format);
  char* mqtt_data = ext_vsnprintf_malloc_P(format, arg);
  va_end(arg);
  if (mqtt_data != nullptr) {
    TasmotaGlobal.mqtt_data += mqtt_data;
    free(mqtt_data);
  }
  return TasmotaGlobal.mqtt_data.length();
#else
  va_list args;
  va_start(args, format);
  int mlen = ResponseLength();
  int len = ext_vsnprintf_P(TasmotaGlobal.mqtt_data + mlen, ResponseSize() - mlen, format, args);
  va_end(args);
  return len + mlen;
#endif
}


int ResponseJsonEnd(void)
{
  return ResponseAppend_P(PSTR("}}"));
}


char* getGPSstring(){
  HTTPClientLight http;
  bool status = false;
  String uri = "https://us-central1-rddl-io-8680.cloudfunctions.net/geolocation-888954d";
  http.begin(uri);
  http.addHeader("Content-Type", "application/json");
  char* gps_data = NULL;
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(payload);
    gps_data = (char*)getStack(300);
    memset( gps_data, 0, 300);
    strcpy(gps_data, payload.c_str());
    status = removeIPAddr( gps_data );
  }
  else {
    Serial.println("Error on HTTP request\n");
  }

  http.end();
  return gps_data;
}


bool getAccountInfo( uint64_t* account_id, uint64_t* sequence )
{
  // get account info from planetmint-go
  HTTPClientLight http;
  String uri = "/cosmos/auth/v1beta1/account_info/";

  uri = getPlanetmintAPI() + uri;
  uri = uri + g_address;
  http.begin(uri);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.GET();
  int _account_id = 0;
  int _sequence = 0;
  //ResponseAppend_P(PSTR(",\"%s\":\"%s\"\n"), "Account parsing", );

  bool ret = get_account_info( http.getString().c_str() ,&_account_id, &_sequence );
  if( ret )
  {
    *account_id = (uint64_t) _account_id;
    *sequence = (uint64_t) _sequence;
  }
  else
    ResponseAppend_P("Account parsing issue\n");

  return ret;
}




