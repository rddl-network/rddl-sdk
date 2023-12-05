#include <cstdint>
#include <cstddef>
#include "planetmintgo.h"
#include "tasmotaUtils.h"
#include "HttpClientLight.h"


extern bool TfsSaveFile(const char *fname, const uint8_t *buf, uint32_t len);
extern bool TfsLoadFile(const char *fname, uint8_t *buf, uint32_t len);
extern int  ResponseAppend_P(const char* format, ...);


static const char* getPlanetmintAPI() {
    // Implement your logic to get the API URL here
    return "https://testnet-api.rddl.io";
}


bool hasMachineBeenAttestedTasmota(const char* g_ext_pub_key_planetmint) {
  HTTPClientLight http;

  String uri = "/planetmint/machine/get_machine_by_public_key/";
  uri = getPlanetmintAPI() + uri;
  uri = uri + g_ext_pub_key_planetmint;
  http.begin(uri);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.GET();

  return (httpResponseCode == 200);
}


bool rddlWritefileTasmota( const char* filename, uint8_t* content, size_t length) {
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
    //AddLog(LOG_LEVEL_DEBUG, PSTR("Failed to write file content"));
    return false;
  } 
  return true;
#else
  return false;
#endif  // USE_UFILESYS
}


int readfileTasmota( const char* filename, uint8_t* content, size_t length){
  char* filename_local = (char*) getStack( strlen( filename ) +2 );
  filename_local[0]= '/';
  int limit = 35;
  int offset = 0;
  if( strlen(filename) > limit)
    offset = strlen(filename) -limit;
  strcpy( &filename_local[1], filename+ offset);

  return TfsLoadFile(filename_local, (uint8_t*)content, length);
}


char* getGPSstringTasmota(){
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


int broadcastTransactionTasmota( char* tx_payload, char *http_answ){
  HTTPClientLight http;
  String uri = "/cosmos/tx/v1beta1/txs";
  uri = getPlanetmintAPI() + uri;
  http.begin(uri);
  http.addHeader("accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  
  int ret = 0;
  ret = http.POST( (uint8_t*)tx_payload, strlen(tx_payload) );
  strcpy(http_answ, http.getString().c_str());
  return ret;
}


bool getAccountInfoTasmota( const char* account_address, uint64_t* account_id, uint64_t* sequence )
{
  // get account info from planetmint-go
  HTTPClientLight http;
  String uri = "/cosmos/auth/v1beta1/account_info/";

  uri = getPlanetmintAPI() + uri;
  uri = uri + account_address;
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

  return ret;
}


int ResponseAppendAbstTasmota(const char* msg){
  return ResponseAppend_P(msg);
}


int tasmotaSerialPrint(const char* msg){
  return Serial.println(msg);
}