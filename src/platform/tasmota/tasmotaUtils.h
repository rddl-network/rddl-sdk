#ifndef TASMOTAUTILS_H
#define TASMOTAUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

bool hasMachineBeenAttestedTasmota(const char* g_ext_pub_key_planetmint);
bool rddlWritefileTasmota( const char* filename, uint8_t* content, size_t length);
int readfileTasmota( const char* filename, uint8_t* content, size_t length);
char* getGPSstringTasmota();
int broadcastTransactionTasmota( char* tx_payload, char *http_answ );
bool getAccountInfoTasmota( const char* account_address, uint64_t* account_id, uint64_t* sequence );
int ResponseAppendAbstTasmota(const char* msg);

#ifdef __cplusplus
}
#endif


#endif // TASMOTAUTILS_H