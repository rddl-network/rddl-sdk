#include <Arduino.h>

extern "C" {
  #include "arduinoUtils.h"
}

extern size_t portentaReadFile(const char * path, uint8_t* content, uint32_t len);
extern bool portentaWriteFile(const char * path, const char * message, size_t messageSize);
extern void portentaInitFS();
extern bool portentaCheckFS();


int arduinoSerialPrint(const char* msg){
  return Serial.print(msg);
}


bool getAccountInfoArduino( const char* account_address, uint64_t* account_id, uint64_t* sequence )
{
  // // get account info from planetmint-go
  // HttpClient http(c);
  // String uri = "/cosmos/auth/v1beta1/account_info/";

  // uri = tasmotaGetSetting( SDK_SET_PLANETMINT_API) + uri;
  // uri = uri + account_address;
  // http.begin(uri);
  // http.addHeader("Content-Type", "application/json");

  // int httpResponseCode = http.GET();
  // int _account_id = 0;
  // int _sequence = 0;
  // //ResponseAppend_P(PSTR(",\"%s\":\"%s\"\n"), "Account parsing", );

  // bool ret = get_account_info( http.getString().c_str() ,&_account_id, &_sequence );
  // if( ret )
  // {
  //   *account_id = (uint64_t) _account_id;
  //   *sequence = (uint64_t) _sequence;
  // }

  // return ret;
}


bool rddlWritefileTasmota( const char* filename, uint8_t* content, size_t length) {
  if(!portentaCheckFs())
    portentaInitFS();

  return portentaWriteFile(filename, content, length);
}


int readfileTasmota( const char* filename, uint8_t* content, size_t length){
  if(!portentaCheckFs())
    portentaInitFS();

  return portentaReadFile(filename, (uint8_t*)content, length);
}