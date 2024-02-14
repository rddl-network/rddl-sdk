
int arduinoSerialPrint(const char* msg);
bool getAccountInfoArduino( const char* account_address, uint64_t* account_id, uint64_t* sequence );
bool rddlWritefileArduino( const char* filename, uint8_t* content, size_t length);
int readfileArduino( const char* filename, uint8_t* content, size_t length);
