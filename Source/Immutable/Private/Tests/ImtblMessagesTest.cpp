// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreTypes.h"
#include "ImmutablePassport.h"
#include "Containers/UnrealString.h"

#include "Misc/AutomationTest.h"
#include "Stats/StatsMisc.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

#include "ImtblJSMessages.h"
#include "UObject/UObjectGlobals.h"


#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImtblMessagesTest, "Immutable.JSMessages",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ServerContext | EAutomationTestFlags::ProductFilter)
// EAutomationTestFlags::SmokeFilter -- note that SmokeFilter will run automatically during cooking


/**
 * This test suite follows the example from TimespanTest.cpp and DateTimeTest.cpp,
 * as recommended by the official Unreal docs.
 *
 * NB - must compile using "Development Editor" to recompile these tests.
 */
bool FImtblMessagesTest::RunTest(const FString& Parameters)
{
    const FString ClientId = "MyExampleClientId";
    const FString RedirectUri = "https://example.com";
    
	// an FImmutablePassportInitData should convert into an appropriate json string
	{
        const FImmutablePassportInitData InitData{ClientId, RedirectUri};
        const FString ExpectedJson = "{\"clientId\":\"MyExampleClientId\",\"redirectUri\":\"https://example.com\"}";
        const FString Result = InitData.ToJsonString();
        
		TestEqual("toJsonString() on FPassportInitData with clientId and redirectUri should produce valid JSON output", Result, ExpectedJson);
	}

    // an FImmutablePassportInitData with an empty redirectUri should leave the redirectUri field out of the json string when converted
    {
        const FImmutablePassportInitData InitData{ClientId, ""};
        const FString ExpectedJson = "{\"clientId\":\"MyExampleClientId\"}";
        const FString Result = InitData.ToJsonString();
        
		TestEqual("toJsonString() on FPassportInitData with an empty redirectUri should produce a valid JSON string with no redirectUri field", Result, ExpectedJson);
    }
	
	return true;
}


#endif //WITH_DEV_AUTOMATION_TESTS