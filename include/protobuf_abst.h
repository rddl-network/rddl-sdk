void* fullfill_planetmintgo_machine_metadata(char* cid, char* gps, char* assetdef, char* device);
void* fullfill_planetmintgo_machine_machine(void* metadata_ptr, char* signature_hex);
void* fullfill_planetmintgo_machine_msgAttestMachine(void* machine_ptr);
int bind_msgAttestMachine_to_anyMsg(void* anyMsg, void* machineMsg);
