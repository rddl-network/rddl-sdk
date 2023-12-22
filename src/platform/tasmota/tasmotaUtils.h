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
bool getPoPInfoTasmota( const char* blockHeight);
void AddLogLineTasmota(const char* msg, ...);
void vAddLogLineTasmota(const char* msg, va_list args);
int tasmotaSerialPrint(const char* msg);
char* tasmotaGetSetting(uint32_t index);
bool tasmotaSetSetting(uint32_t index, const char* replacementText);
void SubscribeTasmota( const char *topic );
void PublishPayloadTasmota(const char* topic, const char* payload);
char* getCIDsTasmota( const char* address );



#ifdef __cplusplus
}
#endif


#endif // TASMOTAUTILS_H