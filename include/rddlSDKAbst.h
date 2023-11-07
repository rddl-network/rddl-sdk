#include "stdbool.h"
#include "stddef.h"

#define PUB_KEY_SIZE 33
#define ADDRESS_HASH_SIZE 20
#define ADDRESS_TAIL 20

#define EXT_PUB_KEY_SIZE 112

/* MAKE IT GENERIC */
/* Cozemedim */
#define PSTR(s)   (s)


/* MAKE IT GENERIC */
#define printMsg printf


extern uint8_t g_priv_key_planetmint[32+1];
extern uint8_t g_priv_key_liquid[32+1];
extern uint8_t g_pub_key_planetmint[33+1];
extern uint8_t g_pub_key_liquid[33+1];
extern uint8_t g_machineid_public_key[33+1];


extern char g_address[64];
extern char g_ext_pub_key_planetmint[EXT_PUB_KEY_SIZE+1];
extern char g_ext_pub_key_liquid[EXT_PUB_KEY_SIZE+1];
extern char g_machineid_public_key_hex[33*2+1];


extern char g_accountid[20];
extern char g_planetmintapi[100];
extern char g_chainid[30];
extern char g_denom[20];

extern bool g_readSeed;

const char* getPlanetmintAPI();
bool hasMachineBeenAttested();
bool rddl_writefile( const char* filename, uint8_t* content, size_t length);
int readfile( const char* filename, uint8_t* content, size_t length);
int ResponseAppend_P(const char* format, ...);
int ResponseJsonEnd(void);
char* getGPSstring();
bool getAccountInfo( uint64_t* account_id, uint64_t* sequence );


