#include <Arduino.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <WiFi.h>
#include <vector>
#include <utility>
#include "planetmintgo.h"
#include "rddlSDKUtils.h"

extern "C" {
  #include "arduinoUtils.h"
}

using header_vector = std::vector<std::pair<String, String>>;

extern size_t portentaReadFile(const char * path, uint8_t* content, uint32_t len);
extern bool portentaWriteFile(const char * path, uint8_t * message, size_t messageSize);
extern void portentaInitFS();
extern bool portentaCheckFS();

std::pair<int, String> sendHttpsGetRequest(const char* urlPath, const header_vector& headers);
std::pair<int, String> sendHttpsPostRequest(const char* urlPath, const header_vector& headers, const uint8_t* tx_payload, size_t tx_size);

int arduinoSerialPrint(const char* msg){
  return Serial.print(msg);
}


bool hasMachineBeenAttestedArduino(const char* g_ext_pub_key_planetmint, const char* api_url) {
  String uri{api_url};
  uri += "/planetmint/machine/get_machine_by_public_key/";
  uri += g_ext_pub_key_planetmint;

  header_vector headers;
  headers.push_back(std::make_pair(String{"Content-Type"}, String{"application/json"}));

  auto response = sendHttpsGetRequest(uri.c_str(), headers);

  return response.first == 200;
}


int broadcastTransactionArduino( char* tx_payload, char *http_answ, const char* api_url){
  String uri{api_url};
  uri += "/cosmos/tx/v1beta1/txs";

  header_vector headers;
  headers.push_back(std::make_pair(String{"accept"},       String{"application/json"}));
  headers.push_back(std::make_pair(String{"Content-Type"}, String{"application/json"}));

  auto response = sendHttpsPostRequest(uri.c_str(), headers, (const uint8_t*)tx_payload, strlen(tx_payload));
  
  strcpy(http_answ, response.second.c_str());
  return response.first;
}


bool getAccountInfoArduino( const char* account_address, uint64_t* account_id, uint64_t* sequence, const char* api_url )
{
  String uri{api_url};
  uri += "/cosmos/auth/v1beta1/account_info/";
  uri += account_address;

  header_vector headers;
  headers.push_back(std::make_pair(String("Content-Type"), String("application/json")));

  auto response = sendHttpsGetRequest(uri.c_str(), headers);
  int _account_id = 0;
  int _sequence = 0;

  bool ret = get_account_info( response.second.c_str() ,&_account_id, &_sequence );
  if( ret )
  {
    *account_id = (uint64_t) _account_id;
    *sequence = (uint64_t) _sequence;
  }

  return ret;
}


bool getPoPInfoArduino( const char* blockHeight, const char* api_url){
  String uri{api_url};
  uri += "/planetmint/planetmint-go/dao/get_challenge/";
  uri += blockHeight;

  header_vector headers;
  headers.push_back(std::make_pair(String{"Content-Type"}, String{"application/json"}));

  auto response = sendHttpsGetRequest(uri.c_str(), headers);
  return getPoPInfoFromJSON( response.second.c_str() );
}


char* getCIDsArduino( const char* address,  const char* api_url){
  String uri{api_url};
  uri += "planetmint/asset/get_cids_by_address";
  uri += address;
  uri += "2000";

  header_vector headers;
  headers.push_back(std::make_pair(String{"Content-Type"}, String{"application/json"}));

  auto response = sendHttpsGetRequest(uri.c_str(), headers);

  if(response.first != 200)
    return NULL;

  uint8_t* buffer = getStack( response.second.length());
  memcpy(buffer, response.second.begin() , response.second.length());

  return (char*)buffer;
}


bool rddlWritefileArduino( const char* filename, uint8_t* content, size_t length) {
  if(!portentaCheckFS())
    portentaInitFS();

  return portentaWriteFile(filename, content, length);
}


int readfileArduino( const char* filename, uint8_t* content, size_t length){
  if(!portentaCheckFS())
    portentaInitFS();

  return portentaReadFile(filename, (uint8_t*)content, length);
}


std::pair<int, String> sendHttpsGetRequest(const char* urlPath, const header_vector& headers)
{
  WiFiClient Client;
  BearSSLClient sslClient(Client);

  Serial.println("Sending HTTPS GET request to...");
  Serial.println(urlPath);

  if (!sslClient.connect("testnet-api.rddl.io", 443)) {
      Serial.println("Connection failed!");
      return {0, ""}; // Return an empty response with status code 0
  }

  // Construct the request with the provided URL path and headers
  sslClient.print("GET ");
  sslClient.print(urlPath);
  sslClient.print(" HTTP/1.1\r\n");
  sslClient.print("Host: testnet-api.rddl.io\r\n");

  // Include additional headers
  for (const auto& header : headers) {
      sslClient.print(header.first);
      sslClient.print(": ");
      sslClient.print(header.second);
      sslClient.print("\r\n");
  }
  
  sslClient.print("Connection: close\r\n\r\n");

  Serial.println("Waiting for response...");

  String responseBody = "";
  int responseCode = 0;

  // Wait for the response headers
  while (sslClient.connected()) {
      String line = sslClient.readStringUntil('\n');
      if (line.startsWith("HTTP/1.")) {
          responseCode = line.substring(9, line.indexOf(' ', 9)).toInt();
      } else if (line == "\r") {
          // Headers received, skip remaining part
          break;
      }
  }

  Serial.println("Response received:");

  // Now read the remainder of the response, which is the JSON data
  while (sslClient.available()) {
      String line = sslClient.readStringUntil('\n');
      responseBody += line + "\n";
  }

  Serial.println("HTTPS GET request complete.");
  Serial.println(responseBody.c_str());

  sslClient.stop();

  return {responseCode, responseBody};
}


std::pair<int, String> sendHttpsPostRequest(const char* urlPath, const header_vector& headers, const uint8_t* tx_payload, size_t tx_size)
{
  WiFiClient Client;
  BearSSLClient sslClient(Client);

    Serial.println("Sending HTTPS POST request...");
    Serial.println(urlPath);

    if (!sslClient.connect("testnet-api.rddl.io", 443)) {
        Serial.println("Connection failed!");
        return {0, ""}; // Return an empty response with status code 0
    }

    // Construct the request with the provided URL path and headers
    sslClient.print("POST ");
    sslClient.print(urlPath);
    sslClient.print(" HTTP/1.1\r\n");
    sslClient.print("Host: testnet-api.rddl.io\r\n");

    // Include additional headers
    for (const auto& header : headers) {
        sslClient.print(header.first);
        sslClient.print(": ");
        sslClient.print(header.second);
        sslClient.print("\r\n");
    }

    // Add Content-Length header
    sslClient.print("Content-Length: ");
    sslClient.print(tx_size);
    sslClient.print("\r\n");

    sslClient.print("Connection: close\r\n\r\n");

    // Send payload
    sslClient.write(tx_payload, tx_size);

    Serial.println("Waiting for response...");

    String responseBody = "";
    int responseCode = 0;

    // Wait for the response headers
    while (sslClient.connected()) {
        String line = sslClient.readStringUntil('\n');
        if (line.startsWith("HTTP/1.")) {
            responseCode = line.substring(9, line.indexOf(' ', 9)).toInt();
        } else if (line == "\r") {
            // Headers received, skip remaining part
            break;
        }
    }

    Serial.println("Response received:");

    // Now read the remainder of the response, which is the JSON data
    while (sslClient.available()) {
        String line = sslClient.readStringUntil('\n');
        responseBody += line + "\n";
    }

    Serial.println("HTTPS POST request complete.");
    Serial.println(responseBody.c_str());

    sslClient.stop();

    return {responseCode, responseBody};
}