#pragma once

#ifdef __cplusplus
extern "C" {
#endif

char* create_transaction( void* anyMsg, char* tokenAmount );
void storeSeed();
uint8_t* readSeed();
bool getPlntmntKeys();
void signRDDLNetworkMessageContent( const char* data_str, size_t data_length, char* sig_out);
int registerMachine(void* anyMsg, const char* machineCategory, const char* manufacturer, const char* cid);
int sendMessages( void* pAnyMsg);
int copyJsonValueString(char *buffer, size_t buffer_len, const char *json, const char *key);
int parseJsonBoolean(const char *json, const char *key, bool *result);
bool getPoPInfoFromJSON( const char* json);
bool convertStringToInt64( const char* valueString, int64_t* targetValue );
void checkNumOfCIDFiles(const char* path);
int GetRandomElementFromCIDJSONList(const char* json, char* cidBuffer, size_t bufferSize) ;

#ifdef __cplusplus
}
#endif