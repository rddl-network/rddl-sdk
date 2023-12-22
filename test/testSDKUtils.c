
#include "stdio.h"
#include "stdint.h"
#include <stdbool.h> 
#include <string.h>
#include "rddlSDKUtils.h"
#include "rddlSDKAbst.h"
#include "rddl.h"
#include "unity.h"
#include "google/protobuf/any.pb-c.h"


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
  char initiator[60] = {0};

  int result = copyJsonValueString( initiator, 60, testPoPInfo, "initiator");
  TEST_ASSERT_EQUAL_INT32( 0, result);
  TEST_ASSERT_EQUAL_STRING("6a1afc01df7d2b778cb094de94a5f14d5bb8d970", initiator);

  result = copyJsonValueString( challenger, 60, testPoPInfo, "challenger");
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
  TEST_ASSERT_EQUAL_STRING( "6a1afc01df7d2b778cb094de94a5f14d5bb8d970", popParticipation.initiator );
  TEST_ASSERT_EQUAL_STRING( "plmnt1w2aatad7ecm05a3yx2s2k0zjttkcp6hr08enpl", popParticipation.challenger );
  TEST_ASSERT_EQUAL_STRING( "plmnt14jsrvk25p9vgq2d9unmvgmqd80qvywu84tl0vf", popParticipation.challengee );
  TEST_ASSERT_TRUE( !popParticipation.finished );
}

const char* jsonCIDList = "{\"cids\": [\"bafkreig3ougm2x3agi3xsrhe6zxqzorhylmji2s5nxsbha7aoe37vm7o24\", \
  \"bafkreig3ougm2x3agi3xsrhe9zxqzorhylmji2s5nxsbha7aoe37vm7o24\", \
  \"bafkreig3ougm2x3ag77xsrhe9zxqzorhylmji2s5nxsbha7aoe37vm7o24\"]}";

const char* stringList[3]= {
"bafkreig3ougm2x3agi3xsrhe6zxqzorhylmji2s5nxsbha7aoe37vm7o24",
"bafkreig3ougm2x3agi3xsrhe9zxqzorhylmji2s5nxsbha7aoe37vm7o24",
"bafkreig3ougm2x3ag77xsrhe9zxqzorhylmji2s5nxsbha7aoe37vm7o24"
};

void testGetRandomElementFromCIDJSONList(void){
  int elementId;
  
  for( int i = 0 ; i < 10; ++i ){
    char cidBuffer[100] = {0};
    elementId = GetRandomElementFromCIDJSONList(jsonCIDList, cidBuffer, 100) ;
    TEST_ASSERT_TRUE( elementId <3 );
    TEST_ASSERT_TRUE( elementId >= 0 );
    TEST_ASSERT_EQUAL_STRING( stringList[elementId], cidBuffer);
  }
}

void testGetRandomElementFromCIDJSONListLarge(void){
  abstClearStack();
  memset(challengedCID, 0, 64 );
  const char* jsonObject = "{\"cids\":[\"bafkreihbe67fuhb3og2h5dipi6kmjcvvemdga5rwlpvi3bwm7fwbesli4m\",\"bafkreih4eliviytdxhawg6xthbuprez7qhqwnbhti7fbdj3g4sq67qituu\",\"bafkreigl3nq6lq2raxqy67pwl6eh4w2geykojrovkkvjb3wpuj7djm2ghm\",\"bafkreigh7echtazdeim653teotnszb4keq7qytqvhoh7fq5t6ygf6ttotm\",\"bafkreifjsi7xa3upxh4wnlu6xbqpkfggj7lxe3y7sintlvugdg73vj5kzi\",\"bafkreifdhwlmhmauhrq6fpf5qa6tjnpyvroalylvyzifidwc2juzwddp6q\",\"bafkreifcgn7oq2vt5wqm6yp6opm3bmgubxjxvp6vkezd6jp3wtuagomiua\",\"bafkreiev4n4hds5ggi7kt3wysvcqf7z4uywknue465otlv47fzev5w2i4a\",\"bafkreieifwvhyxjz7h7a4vfyu6bydbr475vyp4ysibpycx7lmbv4xzqdt4\",\"bafkreido2aoilxbip67gu4fqeuf4mqbqfsur4dmn4ilqmvcnhzdlzc6vpm\",\"bafkreidklqutytliiodwx24tfkwbqwmcsf3iun57noymx3ltqbbgvjftni\",\"bafkreidjbvkx4xzzpqyaca2gs4erdbd63rdka75vrtpoc4ydt53x4kbycq\",\"bafkreidhjkuy2lid4idg5g3ua55te7t4fx32kr2irgocf4ai6emntfic2i\",\"bafkreiccgmnpcumjaubc437iiy2whn3vukriinvc2l5tkk4fbdazowfpje\",\"bafkreibsjaa5fpivnjqcuhyeo2qdyhk2owdzgxf3spsnz7h6kpnltkki3u\",\"bafkreibjotiyft4um2unyfkips4u22dxlgi3ose65yxid24mv4ahsgxlxa\",\"bafkreib43nsaegfktqctamknsoyafengj6lvxklakkuoh7dfurkx6k37om\"],\"pagination\":null}";
  int cidIndex = GetRandomElementFromCIDJSONList(jsonObject, challengedCID, 64);
  int32_t size = (int32_t)strlen( challengedCID);
  TEST_ASSERT_TRUE( cidIndex <16 );
  TEST_ASSERT_TRUE( cidIndex >= 0 );
  TEST_ASSERT_EQUAL_INT32( 59, size);
}


const char* expContentBytes = "{\"Time\":\"2023-12-18T09:03:50\",\"ANALOG\":{\"Temperature1\":9.0},\"ENERGY\":{\"TotalStartTime\":\"2023-10-16T11:51:45\",\"Total\":0.000,\"Yesterday\":0.000,\"Today\":0.000,\"Power\":0,\"ApparentPower\":0,\"ReactivePower\":0,\"Factor\":0.00,\"Voltage\":265,\"Current\":0.000},\"TempUnit\":\"C\"}";
const char* popChallengeStr =  "{ \"PoPChallenge\": {\"cid\": \"bafkreig3ougm2x3agi3xsrhe6zxqzorhylmji2s5nxsbha7aoe37vm7o24\",\"encoding\": \"hex\",\"data\": \"7B2254696D65223A22323032332D31322D31385430393A30333A3530222C22414E414C4F47223A7B2254656D706572617475726531223A392E307D2C22454E45524759223A7B22546F74616C537461727454696D65223A22323032332D31302D31365431313A35313A3435222C22546F74616C223A302E3030302C22596573746572646179223A302E3030302C22546F646179223A302E3030302C22506F776572223A302C224170706172656E74506F776572223A302C225265616374697665506F776572223A302C22466163746F72223A302E30302C22566F6C74616765223A3236352C2243757272656E74223A302E3030307D2C2254656D70556E6974223A2243227D\"} }";
void testFromChallengeToContent(void){
  char buffer[1024] = {0};
  char bufferEncoding[20] = {0};
  char bufferCID[100] = {0};
  int result = copyJsonValueString(buffer, 1024, popChallengeStr, "data");
  TEST_ASSERT_EQUAL_INT32( 0, result);
  TEST_ASSERT_EQUAL_STRING( "7B2254696D65223A22323032332D31322D31385430393A30333A3530222C22414E414C4F47223A7B2254656D706572617475726531223A392E307D2C22454E45524759223A7B22546F74616C537461727454696D65223A22323032332D31302D31365431313A35313A3435222C22546F74616C223A302E3030302C22596573746572646179223A302E3030302C22546F646179223A302E3030302C22506F776572223A302C224170706172656E74506F776572223A302C225265616374697665506F776572223A302C22466163746F72223A302E30302C22566F6C74616765223A3236352C2243757272656E74223A302E3030307D2C2254656D70556E6974223A2243227D", buffer);

  result = copyJsonValueString(bufferEncoding, 20, popChallengeStr, "encoding");
  TEST_ASSERT_EQUAL_INT32( 0, result);
  TEST_ASSERT_EQUAL_STRING( "hex", bufferEncoding);

  result = copyJsonValueString(bufferCID, 100, popChallengeStr, "cid");
  TEST_ASSERT_EQUAL_INT32( 0, result);
  TEST_ASSERT_EQUAL_STRING( "bafkreig3ougm2x3agi3xsrhe6zxqzorhylmji2s5nxsbha7aoe37vm7o24", bufferCID);

  const uint8_t* contentBytes = fromHexString(buffer);
  TEST_ASSERT_EQUAL_STRING( expContentBytes, (const char*) contentBytes);

}

void testTokenAssumption(void){
  const char *str = "Split this string by spaces";
  char buffer[200]= {0};
  strcpy( buffer, str);
  char* token = strtok(buffer, " ");
  TEST_ASSERT_EQUAL_STRING( "Split",  token);

  token = strtok( NULL, " ");
  TEST_ASSERT_EQUAL_STRING( "this",  token);

}

void testCreatePopResult(void){
  Google__Protobuf__Any anyMsg = GOOGLE__PROTOBUF__ANY__INIT;
  abstClearStack();
  resetPopInfo();
  bool result = getPoPInfoFromJSON( testPoPInfo );
  TEST_ASSERT_TRUE( result );
  bool dataIntegrity = true;
  int32_t res = CreatePoPResult( &anyMsg, dataIntegrity);
  TEST_ASSERT_EQUAL_INT32( 0, res );
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(testCopyJsonValueString);
    RUN_TEST(testGetPoPInfoFromJson);
    RUN_TEST(testGetRandomElementFromCIDJSONList);
    RUN_TEST(testGetRandomElementFromCIDJSONListLarge);
    RUN_TEST(testFromChallengeToContent);
    RUN_TEST(testTokenAssumption);
    RUN_TEST(testCreatePopResult);

    return UNITY_END();
}



