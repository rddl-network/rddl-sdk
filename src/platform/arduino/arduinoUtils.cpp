#include <Arduino.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <Arduino_UnifiedStorage.h>
#include <WiFi.h>
#include <vector>
#include <utility>
#include "planetmintgo.h"
#include "rddlSDKUtils.h"

extern "C" {
  #include "arduinoUtils.h"
}

InternalStorage storage;

using header_vector = std::vector<std::pair<String, String>>;

std::pair<int, String> sendHttpsGetRequest (const char* domainUrl, const char* urlPath, const header_vector& headers);
std::pair<int, String> sendHttpsPostRequest(const char* domainUrl, const char* urlPath, const header_vector& headers, const uint8_t* tx_payload, size_t tx_size);
std::vector<std::pair<String, int>> cid_files;

int arduinoSerialPrint(const char* msg){
  return Serial.print(msg);
}


bool hasMachineBeenAttestedArduino(const char* g_ext_pub_key_planetmint, const char* api_url) {
  String uri{};
  uri += "/planetmint/machine/get_machine_by_public_key/";
  uri += g_ext_pub_key_planetmint;

  header_vector headers;
  headers.push_back(std::make_pair(String{"Content-Type"}, String{"application/json"}));

  auto response = sendHttpsGetRequest(api_url, uri.c_str(), headers);
  Serial.print("Response Code: ");
  Serial.println(response.first);
  return response.first == 200; 
}


int broadcastTransactionArduino( char* tx_payload, char *http_answ, const char* api_url){
  String uri{};
  uri += "/cosmos/tx/v1beta1/txs";

  header_vector headers;
  headers.push_back(std::make_pair(String{"accept"},       String{"application/json"}));
  headers.push_back(std::make_pair(String{"Content-Type"}, String{"application/json"}));

  auto response = sendHttpsPostRequest(api_url, uri.c_str(), headers, (const uint8_t*)tx_payload, strlen(tx_payload));
  
  strcpy(http_answ, response.second.c_str());
  return response.first;
}


bool getAccountInfoArduino( const char* account_address, uint64_t* account_id, uint64_t* sequence, const char* api_url )
{
  String uri{};
  uri += "/cosmos/auth/v1beta1/account_info/";
  uri += account_address;

  header_vector headers;
  headers.push_back(std::make_pair(String("Content-Type"), String("application/json")));

  auto response = sendHttpsGetRequest(api_url, uri.c_str(), headers);
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
  String uri{};
  uri += "/planetmint/planetmint-go/dao/get_challenge/";
  uri += blockHeight;

  header_vector headers;
  headers.push_back(std::make_pair(String{"Content-Type"}, String{"application/json"}));

  auto response = sendHttpsGetRequest(api_url, uri.c_str(), headers);
  return getPoPInfoFromJSON( response.second.c_str() );
}


char* getCIDsArduino( const char* address,  const char* api_url){
  String uri{};
  uri += "planetmint/asset/get_cids_by_address";
  uri += address;
  uri += "2000";

  header_vector headers;
  headers.push_back(std::make_pair(String{"Content-Type"}, String{"application/json"}));

  auto response = sendHttpsGetRequest(api_url, uri.c_str(), headers);

  if(response.first != 200)
    return NULL;

  uint8_t* buffer = getStack( response.second.length());
  memcpy(buffer, response.second.begin() , response.second.length());

  return (char*)buffer;
}


void initStorageArduino(){
  static int initCnt{0};

  if(!initCnt){
    storage = InternalStorage();
    if(!storage.begin()){
      Serial.println("Error mounting storage device.");
      return;
    }
    initCnt++;
  }
}

void printFolderContents(Folder dir, int indentation = 0) {
  std::vector<Folder> directories = dir.getFolders();
  std::vector<UFile> files = dir.getFiles();

  // Print directories
  for (Folder subdir : directories) {
    for (int i = 0; i < indentation; i++) {
      Serial.print("  ");
    }
    Serial.print("[D] ");
    Serial.println(subdir.getPath());
    printFolderContents(subdir, indentation + 1);
  }

  // Print files
  int cnt{1};
  for (UFile file : files) {
    for (int i = 0; i < indentation; i++) {
      Serial.print("  ");
    }
    Serial.print(cnt++);
    Serial.print(" [F] ");
    Serial.println(file.getPath());
  }
}


void printAllFSArduino(){
  initStorageArduino();
  
  printFolderContents(storage.getRootFolder());
}


bool writefileArduino( const char* filename, uint8_t* content, size_t length) {
  bool status{false}; 
  initStorageArduino();

  Folder root = storage.getRootFolder();
  UFile file = root.createFile(filename, FileMode::WRITE);
  if(!file.exists())
    return status;

  auto wrtLen = file.write(content, length);
  if(wrtLen == length)
    status = true;

  file.close();
  return status;
}


int readfileArduino( const char* filename, uint8_t* content, size_t length){
  initStorageArduino();

  Folder root = storage.getRootFolder();
  UFile file = root.createFile(filename, FileMode::READ);
  if(!file.exists())
    return 0;

  file.seek(0); // Move the file pointer to the beginning
  int bytesRead = file.read(content, length);

  file.close();
  return bytesRead;
}


int arduinoGetNumOfCIDFiles(const char *path){
  initStorageArduino();

  Folder root = storage.getRootFolder();
  std::vector<UFile> files = root.getFiles();
  return files.size();
}


String getFileNameFromPath(const String& path) {
    int found = path.lastIndexOf('/'); 
    if (found != -1) { 
        return path.substring(found + 1); 
    }

    // If "/" is not found, return the whole path
    return path; 
}


void arduinoGetCIDFiles(const char *path){
  initStorageArduino();

  Folder root = storage.getRootFolder();
  std::vector<UFile> files = root.getFiles();

  for(auto f : files){
    auto fileName = getFileNameFromPath(f.getPathAsString());
    if(fileName.length() > 20){
      String time_str = fileName.substring(fileName.length() - 10);
      cid_files.push_back(std::make_pair(fileName, std::atoi(time_str.c_str())));
    }
  }

  return;
}


void arduinoSortCIDFiles(){
  std::sort(cid_files.begin(), cid_files.end(), [](const std::pair<String, int> &a, const std::pair<String, int> &b){
    return a.second > b.second;
  });
}


void arduinoDeleteAllCID(){
  initStorageArduino();

  Folder root = storage.getRootFolder();
  std::vector<UFile> files = root.getFiles();

  for(auto f: files){
    auto fileName = getFileNameFromPath(f.getPathAsString());
    if(fileName.length() > 25){
      Serial.print("Deleting file: ");
      Serial.println(fileName);
      f.remove();
    }
  }
}


/* Delete last element on cid files vector, Return -1 if vector is empty */
int arduinoDeleteOldestCIDFiles(){
  initStorageArduino();

  if(cid_files.empty())
    return -1;

  Folder root = storage.getRootFolder();
  auto file = root.createFile(cid_files.back().first, FileMode::READ);
  if(!file.exists())
    return -1;

  file.remove();
  cid_files.pop_back();
  
  return 0;
}

 
std::pair<int, String> sendHttpsGetRequest(const char* domainUrl, const char* urlPath, const header_vector& headers)
{
  WiFiClient Client;
  BearSSLClient sslClient(Client);

  Serial.println("Sending HTTPS GET request to...");
  Serial.print(domainUrl);
  Serial.println(urlPath);

  if (!sslClient.connect(domainUrl, 443)) {
      Serial.println("Connection failed!");
      return {0, ""}; // Return an empty response with status code 0
  }

  // Construct the request with the provided URL path and headers
  sslClient.print("GET ");
  sslClient.print(urlPath);
  sslClient.print(" HTTP/1.1\r\n");
  sslClient.print("Host: ");
  sslClient.print(domainUrl);
  sslClient.print("\r\n");

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


std::pair<int, String> sendHttpsPostRequest(const char* domainUrl, const char* urlPath, const header_vector& headers, const uint8_t* tx_payload, size_t tx_size)
{
  WiFiClient Client;
  BearSSLClient sslClient(Client);

    Serial.println("Sending HTTPS POST request...");
    Serial.println(urlPath);

    if (!sslClient.connect(domainUrl, 443)) {
        Serial.println("Connection failed!");
        return {0, ""}; // Return an empty response with status code 0
    }

    // Construct the request with the provided URL path and headers
    sslClient.print("POST ");
    sslClient.print(urlPath);
    sslClient.print(" HTTP/1.1\r\n");
    sslClient.print("Host: ");
    sslClient.print(domainUrl);
    sslClient.print("\r\n");

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