#include "stdio.h"
#include "stdint.h"
#include <stdbool.h>
#include <string.h>
#include "rddlSDKAPI.h"
#include "unity.h"

char TEST_SEED[]        = "12345d8b5ee5bcefd523ee4d4340a8956affbef5bb1978eb1e3f640318f87f4b";
char TEST_MNEMONIC[]    = "victory cancel radio rotate all depart mind become axis boost tent ensure";

void testMnemonic(void){
    char* mnemonic = sdkSetSeed(TEST_MNEMONIC, sizeof(TEST_MNEMONIC));

    TEST_ASSERT_TRUE(!strcmp(mnemonic, TEST_MNEMONIC));
}


void testSeedOperation(void){ 
    sdkStoreSeed(TEST_SEED);

    char seed_arr[128] = {0};
    int  seed_size = sizeof(seed_arr);
    sdkReadSeed(seed_arr, &seed_size);

    TEST_ASSERT_TRUE(!strcmp(seed_arr, TEST_SEED));
}


void testNotarizationFlow(void){ 

    runRDDLSDKNotarizationWorkflow("TESTDATA", sizeof("TESTDATA"));

    TEST_ASSERT_TRUE(true);
}


int main()
{
    UNITY_BEGIN();
    RUN_TEST(testMnemonic);
    RUN_TEST(testSeedOperation);
    RUN_TEST(testNotarizationFlow);
    return UNITY_END();
}
