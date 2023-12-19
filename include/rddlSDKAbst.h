#pragma once

#include "stdbool.h"
#include "stddef.h"

 
#ifdef __cplusplus
extern "C" {
#endif


#define PUB_KEY_SIZE 33
#define ADDRESS_HASH_SIZE 20
#define ADDRESS_TAIL 20

#define EXT_PUB_KEY_SIZE 112 

#define MAX_CID_FILE_SIZE (24*42)   // One every hour, 42 days in total

/* MAKE IT GENERIC */
/* Cozemedim */
#define PSTR(s)   (s)

#ifdef LINUX_MACHINE
    #define CID_FILE_DIRECTORY "./"
#else
    #define CID_FILE_DIRECTORY "/"
#endif


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


extern char sdk_planetmintapi[100];
extern char sdk_chainid[30];
extern char sdk_denom[20];

extern bool sdk_readSeed;
extern char responseArr[4096];

extern uint32_t num_of_cid_files;

typedef struct PoPInfo {
    int64_t blockHeight;
    char challenger[64];
    char challengee[64];
    bool finished;
} PoPInfo;

extern PoPInfo popParticipation;

void resetPopInfo();
bool hasMachineBeenAttested();
bool rddl_writefile( const char* filename, uint8_t* content, size_t length);
int readfile( const char* filename, uint8_t* content, size_t length);
int printMsg(const char* msg);
void AddLogLineAbst(const char* msg, ...);
void vAddLogLineAbst(const char* msg, va_list args);
char* getGPSstring();
bool getAccountInfo( uint64_t* account_id, uint64_t* sequence );
bool getPoPInfo( const char* blockHeight);
int broadcast_transaction( char* tx_payload );
char* getSetting(uint32_t index);
bool setSetting(uint32_t index, const char* replacementText);

uint8_t* abstGetStack( size_t size );
void abstClearStack();
int abstGetNumOfCIDFiles(const char* path);
int abstDeleteOldestCIDFile(const char* path);


#ifdef __cplusplus
}
#endif
