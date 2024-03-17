
int arduinoSerialPrint(const char* msg);
bool getAccountInfoArduino( const char* account_address, uint64_t* account_id, uint64_t* sequence, const char* api_url);
bool writefileArduino( const char* filename, uint8_t* content, size_t length);
int readfileArduino( const char* filename, uint8_t* content, size_t length);
bool hasMachineBeenAttestedArduino(const char* g_ext_pub_key_planetmint, const char* api_url);  
int broadcastTransactionArduino( char* tx_payload, char *http_answ, const char* api_url);
bool getPoPInfoArduino( const char* blockHeight, const char* api_url);
char* getCIDsArduino( const char* address,  const char* api_url);