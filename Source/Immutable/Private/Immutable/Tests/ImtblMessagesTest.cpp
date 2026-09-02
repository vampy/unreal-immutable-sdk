// Fill out your copyright notice in the Description page of Project Settings.

#include "Containers/UnrealString.h"
#include "CoreTypes.h"
#include "Runtime/Launch/Resources/Version.h"

#include "Immutable/ImmutablePassport.h"

#include "Misc/AutomationTest.h"
#include "Stats/StatsMisc.h"
#include "Tests/AutomationCommon.h"
#if WITH_EDITOR
#include "Tests/AutomationEditorCommon.h"
#endif

#include "UObject/UObjectGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS

#if ((ENGINE_MAJOR_VERSION <= 4) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 4))
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImtblMessagesTest, "Immutable.JSMessages", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)
#else
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImtblMessagesTest, "Immutable.JSMessages", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
#endif

// EAutomationTestFlags::SmokeFilter -- note that SmokeFilter will run
// automatically during cooking

/**
 * This test suite follows the example from TimespanTest.cpp and
 * DateTimeTest.cpp, as recommended by the official Unreal docs.
 *
 * NB - must compile using "Development Editor" to recompile these tests.
 */
bool FImtblMessagesTest::RunTest(const FString& Parameters)
{
	const FString ClientId = "MyExampleClientId";

	// an FImmutablePassportInitData should convert into an appropriate json
	// string
	{
		const FString RedirectUri = "https://example.com";
		const FImmutablePassportInitData InitData { ClientId, RedirectUri, ImmutablePassportEnvironmentConstants::EnvironmentSandbox };
		FString ExpectedJson = "{\"clientId\":\"MyExampleClientId\",\"redirectUri\":\"https://" "example.com\",\"environment\":\"sandbox\"";
		ExpectedJson += ",\"engineVersion\":{";
		ExpectedJson += "\"engine\":\"unreal\"";
		// example:
		// engineVersion":"5.2.1-26001984+++UE5+Release-5.2","platform":"Mac","platformVersion":"13.5.2"
		ExpectedJson += ",\"engineVersion\":\"" + FEngineVersion::Current().ToString().Replace(TEXT(" "), TEXT("_")) + "\"";
		ExpectedJson += FString(",\"platform\":\"") + FString(FPlatformProperties::IniPlatformName()).Replace(TEXT(" "), TEXT("_")) + "\"";
		ExpectedJson += FString(",\"platformVersion\":\"") + FPlatformMisc::GetOSVersion().Replace(TEXT(" "), TEXT("_")) + "\"";
		ExpectedJson += "}}";
		const FString Result = InitData.ToJsonString();
		TestEqual("toJsonString() on FPassportInitData with clientId and " "redirectUri should produce valid JSON output", Result, ExpectedJson);
	}

	// an FImmutablePassportInitData with an empty redirectUri should leave the
	// redirectUri field out of the json string when converted
	{
		const FImmutablePassportInitData InitData { ClientId, "", ImmutablePassportEnvironmentConstants::EnvironmentSandbox };
		FString ExpectedJson = "{\"clientId\":\"MyExampleClientId\",\"environment\":\"sandbox\"";
		ExpectedJson += ",\"engineVersion\":{";
		ExpectedJson += "\"engine\":\"unreal\"";
		// example:
		// engineVersion":"5.2.1-26001984+++UE5+Release-5.2","platform":"Mac","platformVersion":"13.5.2"
		ExpectedJson += ",\"engineVersion\":\"" + FEngineVersion::Current().ToString().Replace(TEXT(" "), TEXT("_")) + "\"";
		ExpectedJson += FString(",\"platform\":\"") + FString(FPlatformProperties::IniPlatformName()).Replace(TEXT(" "), TEXT("_")) + "\"";
		ExpectedJson += FString(",\"platformVersion\":\"") + FPlatformMisc::GetOSVersion().Replace(TEXT(" "), TEXT("_")) + "\"";
		ExpectedJson += "}}";
		const FString Result = InitData.ToJsonString();
		TestEqual("toJsonString() on FPassportInitData with an empty redirectUri " "should produce a valid JSON string with no redirectUri field", Result, ExpectedJson);
	}

	return true;
}

#if ((ENGINE_MAJOR_VERSION <= 4) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 4))
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImtblIlluviumResponseHandlingTest, "Immutable.Illuvium.ResponseHandling", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)
#else
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImtblIlluviumResponseHandlingTest, "Immutable.Illuvium.ResponseHandling", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
#endif

bool FImtblIlluviumResponseHandlingTest::RunTest(const FString& Parameters)
{
	const TOptional<FImtblJSResponse> Response = FImtblJSResponse::FromJsonString(
		TEXT("{\"responseFor\":\"getToken\",\"requestId\":\"42\",\"success\":false,\"errorType\":0,\"error\":\"Denied\"}"));

	TestTrue(TEXT("A response with optional result data parses"), Response.IsSet());
	if (!Response.IsSet())
	{
		return false;
	}

	TestTrue(TEXT("Structured response errors are retained"), Response->Error.IsSet());
	if (Response->Error.IsSet())
	{
		TestEqual(TEXT("Structured response errors include their type"), Response->Error->ToString(), FString(TEXT("AuthenticationError: Denied")));
	}
	TestEqual(TEXT("A missing string result is tolerated"), UImmutablePassport::GetResponseResultAsString(*Response), FString());
	TestFalse(TEXT("A missing bool result is tolerated"), UImmutablePassport::GetResponseResultAsBool(*Response));
	TestTrue(TEXT("A missing array result is tolerated"), UImmutablePassport::GetResponseResultAsStringArray(*Response).IsEmpty());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
