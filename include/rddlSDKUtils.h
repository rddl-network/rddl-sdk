#ifdef __cplusplus
extern "C" {
#endif

char* create_transaction( void* anyMsg, char* tokenAmount );
void storeSeed();
uint8_t* readSeed();
void getPlntmntKeys();
void signRDDLNetworkMessageContent( const char* data_str, size_t data_length, char* sig_out);
int registerMachine(void* anyMsg);
void setDenom( const char* denom, size_t len);
char* getDenom();
char* getChainID();
void setAccountID( const char* account, size_t len);
char* getAccountID();

#ifdef __cplusplus
}
#endif