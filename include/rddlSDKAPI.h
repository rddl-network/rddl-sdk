#ifdef __cplusplus
extern "C" {
#endif
#define SDK_STACK_LIMIT 7168


const char* sdkGetRDDLAddress();
const char* sdkGetExtPubKeyLiquid();
const char* sdkGetExtPubKeyPlanetmint();
const uint8_t* sdkGetPrivKeyLiquid();
const uint8_t* sdkGetPrivKeyPlanetmint();
const char* sdkGetMachinePublicKey();

bool  sdkGetPlntmntKeys();
char* sdkSetSeed(char* pMnemonic, size_t len);

void sdkStoreSeed(char* new_seed);
uint8_t* sdkReadSeed(char* seed_arr, int* seed_size);

int sdkReadFile( const char* filename, uint8_t* content, size_t length);
bool sdkGetAccountInfo( uint64_t* account_id, uint64_t* sequence );

char* sdkGetSetting(uint32_t index);
bool sdkSetSetting(uint32_t index, const char* replacementText);

uint8_t* sdkGetStack( size_t size );
void sdkClearStack();


void runRDDLSDKMachineAttestation(const char* machineCategory, const char* manufacturer, const char* cid );
void runRDDLSDKNotarizationWorkflow(const char* data_str, size_t data_length);


#ifdef __cplusplus
}
#endif