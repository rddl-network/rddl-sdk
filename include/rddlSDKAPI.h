#ifdef __cplusplus
extern "C" {
#endif

char* sdkSetSeed(char* pMnemonic, size_t len);
void sdkStoreSeed(char* new_seed);
void sdkReadSeed(char* seed_arr, int* seed_size);
void runRDDLSDKNotarizationWorkflow(const char* data_str, size_t data_length);

#ifdef __cplusplus
}
#endif