#ifdef __cplusplus
extern "C" {
#endif

const char* sdkGetRDDLAddress();
const char* sdkGetExtPubKeyLiquid();
const char* sdkGetExtPubKeyPlanetmint();
const uint8_t* sdkGetPrivKeyLiquid();
const uint8_t* sdkGetPrivKeyPlanetmint();
const char* sdkGetMachinePublicKey();
char  sdkGetPlntmntKeys();
char* sdkSetSeed(char* pMnemonic, size_t len);
void sdkStoreSeed(char* new_seed);
void sdkReadSeed(char* seed_arr, int* seed_size);
void runRDDLSDKMachineAttestation(const char* machineCategory, const char* manufacturer, const char* cid );
void runRDDLSDKNotarizationWorkflow(const char* data_str, size_t data_length);

#ifdef __cplusplus
}
#endif