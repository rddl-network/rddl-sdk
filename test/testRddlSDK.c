#include "stdio.h"
#include "stdint.h"
#include <stdbool.h>
#include <string.h>
// #include "rddlSDKAbst.h"
// #include "rddlSDKUtils.h"
#include "rddlSDKAPI.h"
#include "unity.h"


// void testGetPlntmntKeys(void){

//     getPlntmntKeys();

//     TEST_ASSERT_TRUE(true);
// }


// void testAttestedTrue(void){ 

//     memcpy( g_ext_pub_key_planetmint, "pmpb7veeZqNgEJHYVbYJePCCXQZAJrVZ27vd8iuVKcFLLJZrkw57qgNmJxQktvSLNFY1j56S3HDGc21Rgx96n8Jac7hKPTphAYUk5NwwVcBWrX5", 
//             sizeof("pmpb7veeZqNgEJHYVbYJePCCXQZAJrVZ27vd8iuVKcFLLJZrkw57qgNmJxQktvSLNFY1j56S3HDGc21Rgx96n8Jac7hKPTphAYUk5NwwVcBWrX5"));

//     bool status = hasMachineBeenAttested();

//     TEST_ASSERT_TRUE(status == true);
// }


// void testAttestedFalse(void){ 

//     memcpy( g_ext_pub_key_planetmint, "xpub6G8TnTyKFoBR5hqZXW8HDFEZoG83gzrXcifjtSpPf1qzkcfTGD3gSbD7rF3xRBy9RKz53AuRuBaJTok3mFNFRKEBpZHWRHzjg1111111111", 
//             sizeof("xpub6G8TnTyKFoBR5hqZXW8HDFEZoG83gzrXcifjtSpPf1qzkcfTGD3gSbD7rF3xRBy9RKz53AuRuBaJTok3mFNFRKEBpZHWRHzjg4NW5UmUjc7"));

//     bool status = hasMachineBeenAttested();

//     TEST_ASSERT_TRUE(status == false);
// }


void testNotarizationFlow(void){ 

    runRDDLSDKNotarizationWorkflow("FATIH", sizeof("FATIH"));

    TEST_ASSERT_TRUE(true);
}


int main()
{
    UNITY_BEGIN();
    //RUN_TEST(getPlntmntKeys);
    //RUN_TEST(testAttestedTrue);
    //RUN_TEST(testAttestedFalse);
    RUN_TEST(testNotarizationFlow);
    return UNITY_END();
}
