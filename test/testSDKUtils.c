
#include "stdio.h"
#include "stdint.h"
#include <stdbool.h> 
#include <string.h>
#include "rddlSDKUtils.h"
#include "rddlSDKAbst.h"
#include "unity.h"

const char testPoPInfo[]="{\
  \"challenge\": {\
    \"initiator\": \"6a1afc01df7d2b778cb094de94a5f14d5bb8d970\",\
    \"challenger\": \"plmnt1w2aatad7ecm05a3yx2s2k0zjttkcp6hr08enpl\",\
    \"challengee\": \"plmnt14jsrvk25p9vgq2d9unmvgmqd80qvywu84tl0vf\",\
    \"height\": \"3\",\
    \"success\": false,\
    \"finished\": false\
  }\
}";

const char testPoPInfoEmpty[]="{\"challenge\":{\"initiator\":\"6a1afc01df7d2b778cb094de94a5f14d5bb8d970\",\"challenger\":\"\",\"challengee\":\"\",\"height\":\"228\",\"success\":false,\"finished\":false}}";
const char testPoPInfoMalformed[]="{\"challenge\": \"testlkj}";

void testCopyJsonValueString(void){
  char challenger[60] = {0};
  char challengee[60] = {0};

  int result = copyJsonValueString( challenger, 60, testPoPInfo, "challenger");
  TEST_ASSERT_EQUAL_INT32( 0, result);
  TEST_ASSERT_EQUAL_STRING("plmnt1w2aatad7ecm05a3yx2s2k0zjttkcp6hr08enpl", challenger);

  result = copyJsonValueString( challengee, 60, testPoPInfo, "challengee");
  TEST_ASSERT_EQUAL_INT32( 0, result);
  TEST_ASSERT_EQUAL_STRING("plmnt14jsrvk25p9vgq2d9unmvgmqd80qvywu84tl0vf", challengee);

  result = copyJsonValueString( challengee, 60, testPoPInfo, "123456");
  TEST_ASSERT_EQUAL_INT32( -1, result);

  result = copyJsonValueString( challengee, 60, testPoPInfoMalformed, "challenge");
  TEST_ASSERT_EQUAL_INT32( -2, result);

  result = copyJsonValueString( challengee, 5, testPoPInfo, "challengee");
  TEST_ASSERT_EQUAL_INT32( -3, result);

  result = copyJsonValueString( challengee, 60, testPoPInfoEmpty, "challengee");
  TEST_ASSERT_EQUAL_INT32( 0, result);
  TEST_ASSERT_EQUAL_STRING( "", challengee);
}


void testGetPoPInfoFromJson(void){
  resetPopInfo();
  bool result = getPoPInfoFromJSON( testPoPInfo );
  TEST_ASSERT_TRUE( result );
  TEST_ASSERT_EQUAL_INT64( 3, popParticipation.blockHeight );
  TEST_ASSERT_EQUAL_STRING( "plmnt1w2aatad7ecm05a3yx2s2k0zjttkcp6hr08enpl", popParticipation.challenger );
  TEST_ASSERT_EQUAL_STRING( "plmnt14jsrvk25p9vgq2d9unmvgmqd80qvywu84tl0vf", popParticipation.challengee );
  TEST_ASSERT_TRUE( !popParticipation.finished );
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(testCopyJsonValueString);
    RUN_TEST(testGetPoPInfoFromJson);

    return UNITY_END();
}



