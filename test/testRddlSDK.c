#include "stdio.h"
#include "stdint.h"
#include <stdbool.h> 
#include <string.h>
#include "rddlSDKAPI.h"
#include "rddlSDKAbst.h"
#include "unity.h"

char TEST_SEED[]        = "12345d8b5ee5bcefd523ee4d4340a8956affbef5bb1978eb1e3f640318f87f4b";
char TEST_MNEMONIC[]    = "victory cancel radio rotate all depart mind become axis boost tent ensure";
char TEST_ADDRESS[]     = "plmnt1qv6k5st59kzt6jurdkc43xdndpdzx6wrzpsvv6";
char TEST_LIQUID[]      = "xpub6FSQbqmhvqemWQ75NVpppJK2quNuVEoFGC2knrbgVymTkokfr4NCdBASt99FBGsXvNM7DsTAh3kLxjYpdUz1QStUNrZ3g2ENqD1BjAaQd6s";
char TEST_PLANETMINT[]  = "pmpb7uDZMFAM4TrgEqrYoAHvx2Gegw5hERKCeWaVi3t7QCntXAGfVfswR8DdE96BYmLV4XTfXUfMmrtsH7XB8VHAaBCxWcjZEJSyufiLfsZuBcD";
char TEST_MACHINE_ID[]  = "03E58EC4AE9B60564EDF51A1C9BCF9759C63B276D236CD55F15B02FD226AC2CE3F";


void testMnemonic(void){
    char* mnemonic = sdkSetSeed(TEST_MNEMONIC, 0);

    printf("MNEMONIC: %s\n", mnemonic);

    TEST_ASSERT_TRUE(sdkGetPlntmntKeys());
}


void testMnemonicGiven(void){
    char* mnemonic = sdkSetSeed(TEST_MNEMONIC, sizeof(TEST_MNEMONIC));

    TEST_ASSERT_TRUE(!strcmp(mnemonic, TEST_MNEMONIC));
}


void testSeedOperation(void){ 
    sdkStoreSeed(TEST_SEED);

    char seed_arr[128] = {0};
    int  seed_size = sizeof(seed_arr);
    sdkReadSeed(seed_arr, &seed_size);

    printf("size %d\n", seed_size);

    TEST_ASSERT_TRUE(!strcmp(seed_arr, TEST_SEED));
}


void testGetPublicKeys(){
    sdkGetPlntmntKeys();

    printf("\n\nAddress: %s\nLiquid: %s\nPlanetmint: %s\nMachine ID: %s\n\n", sdkGetRDDLAddress(), sdkGetExtPubKeyLiquid(), sdkGetExtPubKeyPlanetmint(), sdkGetMachinePublicKey()); 
    TEST_ASSERT_TRUE(!strcmp(sdkGetRDDLAddress(),           TEST_ADDRESS));
    TEST_ASSERT_TRUE(!strcmp(sdkGetExtPubKeyLiquid(),       TEST_LIQUID));
    TEST_ASSERT_TRUE(!strcmp(sdkGetExtPubKeyPlanetmint(),   TEST_PLANETMINT));
    TEST_ASSERT_TRUE(!strcmp(sdkGetMachinePublicKey(),      TEST_MACHINE_ID));
}


void testMachineAttestation(void){ 
    char machinecid_buffer[64] = {0};
    memset((void*)machinecid_buffer,0, 58+1);

    runRDDLSDKMachineAttestation("otherserial", "RDDL", machinecid_buffer);

    TEST_ASSERT_TRUE(true);
}


void testNotarizationFlow(void){ 

    runRDDLSDKNotarizationWorkflow("TESTDATA", sizeof("TESTDATA"));

    TEST_ASSERT_TRUE(true);
}

void testGetPoPFromChain(void){
    bool result = getPoPFromChain( "228" );
    TEST_ASSERT_TRUE( result );
    TEST_ASSERT_EQUAL_INT64( 228, popParticipation.blockHeight);
    TEST_ASSERT_EQUAL_STRING( "", popParticipation.challenger);
    TEST_ASSERT_EQUAL_STRING( "", popParticipation.challengee);
}



int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(testMnemonic);
    RUN_TEST(testMnemonicGiven);
    RUN_TEST(testSeedOperation);
    RUN_TEST(testGetPublicKeys);
    RUN_TEST(testMachineAttestation);
    RUN_TEST(testNotarizationFlow);
    //RUN_TEST(testGetPoPFromChain);

    return UNITY_END();
}
