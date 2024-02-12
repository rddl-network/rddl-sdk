#include <Arduino.h>
#include <HttpClient.h>

extern "C" {
  #include "arduinoUtils.h"
}



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