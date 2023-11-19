#include "stdbool.h"
#include "stddef.h"


#ifdef __cplusplus
extern "C" {
#endif


#define PUB_KEY_SIZE 33
#define ADDRESS_HASH_SIZE 20
#define ADDRESS_TAIL 20

#define EXT_PUB_KEY_SIZE 112

/* MAKE IT GENERIC */
/* Cozemedim */
#define PSTR(s)   (s)


/* MAKE IT GENERIC */
extern uint8_t sdk_priv_key_planetmint[32+1];
extern uint8_t sdk_priv_key_liquid[32+1];
extern uint8_t sdk_pub_key_planetmint[33+1];
extern uint8_t sdk_pub_key_liquid[33+1];
extern uint8_t sdk_machineid_public_key[33+1];


extern char sdk_address[64];
extern char sdk_ext_pub_key_planetmint[EXT_PUB_KEY_SIZE+1];
extern char sdk_ext_pub_key_liquid[EXT_PUB_KEY_SIZE+1];
extern char sdk_machineid_public_key_hex[33*2+1];


extern char sdk_accountid[20];
extern char sdk_planetmintapi[100];
extern char sdk_chainid[30];
extern char sdk_denom[20];

extern bool sdk_readSeed;
extern char responseArr[4096];

const char* getPlanetmintAPI();
bool hasMachineBeenAttested();
bool rddl_writefile( const char* filename, uint8_t* content, size_t length);
int readfile( const char* filename, uint8_t* content, size_t length);
int printMsg(const char* msg);
int ResponseAppendAbst(const char* msg);
int ResponseJsonEnd(void);
char* getGPSstring();
bool getAccountInfo( uint64_t* account_id, uint64_t* sequence );
int broadcast_transaction( char* tx_payload );

#ifdef __cplusplus
}
#endif
