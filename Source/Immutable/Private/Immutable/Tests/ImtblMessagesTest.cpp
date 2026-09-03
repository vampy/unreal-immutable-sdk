// Fill out your copyright notice in the Description page of Project Settings.

#include "Containers/UnrealString.h"
#include "CoreTypes.h"
#include "Interfaces/IPluginManager.h"
#include "Runtime/Launch/Resources/Version.h"

#include "Immutable/ImtblJSConnector.h"
#include "Immutable/ImmutableNames.h"
#include "Immutable/ImmutablePassport.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
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

#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC | PLATFORM_WINDOWS

struct FImtblPassportReloginTestAccessor
{
	static void Setup(UImmutablePassport& Passport, UImtblJSConnector* Connector)
	{
		Passport.Setup(MakeWeakObjectPtr(Connector));
	}

	static void MarkInitialized(UImmutablePassport& Passport)
	{
		Passport.SetStateFlags(UImmutablePassport::IPS_INITIALIZED);
	}

	static bool IsConnecting(const UImmutablePassport& Passport)
	{
		return Passport.IsStateFlagsSet(UImmutablePassport::IPS_CONNECTING);
	}

	static bool IsConnected(const UImmutablePassport& Passport)
	{
		return Passport.IsStateFlagsSet(UImmutablePassport::IPS_CONNECTED);
	}
};

#if ((ENGINE_MAJOR_VERSION <= 4) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 4))
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImtblIlluviumReloginBridgeContractTest, "Immutable.Illuvium.ReloginBridgeContract", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)
#else
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImtblIlluviumReloginBridgeContractTest, "Immutable.Illuvium.ReloginBridgeContract", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
#endif

bool FImtblIlluviumReloginBridgeContractTest::RunTest(const FString& Parameters)
{
	const TSharedPtr<IPlugin> ImmutablePlugin = IPluginManager::Get().FindPlugin(TEXT("Immutable"));
	TestTrue(TEXT("Immutable plugin is available"), ImmutablePlugin.IsValid());
	if (!ImmutablePlugin.IsValid())
	{
		return false;
	}

	FString BridgeSource;
	const FString BridgePath = FPaths::Combine(ImmutablePlugin->GetBaseDir(), TEXT("Web/index.js"));
	TestTrue(TEXT("Bundled bridge can be read"), FFileHelper::LoadFileToString(BridgeSource, *BridgePath));
	TestTrue(TEXT("Bundled bridge exposes relogin"), BridgeSource.Contains(TEXT("case\"relogin\"")));
	TestTrue(TEXT("Bundled relogin uses the cached session"), BridgeSource.Contains(TEXT("useCachedSession")));

	UImtblJSConnector* Connector = NewObject<UImtblJSConnector>();
	Connector->Init(true);

	FString ExecutedJS;
	Connector->ExecuteJs.BindLambda([&ExecutedJS](const FString& JS)
	{
		ExecutedJS = JS;
	});

	UImmutablePassport* Passport = NewObject<UImmutablePassport>();
	FImtblPassportReloginTestAccessor::Setup(*Passport, Connector);
	bool bUninitializedResponseReceived = false;
	Passport->Relogin(UImmutablePassport::FImtblPassportResponseDelegate::CreateLambda(
		[&bUninitializedResponseReceived](const FImmutablePassportResult Result)
		{
			bUninitializedResponseReceived = !Result.Success;
		}));

	TestTrue(TEXT("Uninitialized relogin reports failure"), bUninitializedResponseReceived);
	TestFalse(TEXT("Uninitialized relogin does not enter connecting state"), FImtblPassportReloginTestAccessor::IsConnecting(*Passport));
	TestTrue(TEXT("Uninitialized relogin does not call the bridge"), ExecutedJS.IsEmpty());

	FImtblPassportReloginTestAccessor::MarkInitialized(*Passport);

	bool bResponseReceived = false;
	bool bReloginSucceeded = true;
	Passport->Relogin(UImmutablePassport::FImtblPassportResponseDelegate::CreateLambda(
		[&bResponseReceived, &bReloginSucceeded](const FImmutablePassportResult Result)
		{
			bResponseReceived = true;
			bReloginSucceeded = Result.Success;
		}));

	TestTrue(TEXT("Relogin enters connecting state"), FImtblPassportReloginTestAccessor::IsConnecting(*Passport));
	TestTrue(TEXT("Relogin emits the bridge action"), ExecutedJS.Contains(TEXT("\\\"fxName\\\":\\\"relogin\\\"")));

	const FString RequestIdPrefix = TEXT("\\\"requestId\\\":\\\"");
	const int32 RequestIdStart = ExecutedJS.Find(RequestIdPrefix);
	TestTrue(TEXT("Relogin request includes an id"), RequestIdStart != INDEX_NONE);
	if (RequestIdStart == INDEX_NONE)
	{
		return false;
	}

	const FString RequestIdTail = ExecutedJS.Mid(RequestIdStart + RequestIdPrefix.Len());
	const int32 RequestIdEnd = RequestIdTail.Find(TEXT("\\\""));
	TestTrue(TEXT("Relogin request id is terminated"), RequestIdEnd != INDEX_NONE);
	if (RequestIdEnd == INDEX_NONE)
	{
		return false;
	}

	const FString RequestId = RequestIdTail.Left(RequestIdEnd);
	Connector->SendToGame(FString::Printf(
		TEXT("{\"responseFor\":\"relogin\",\"requestId\":\"%s\",\"success\":false,\"error\":\"Cached session unavailable\"}"),
		*RequestId));

	TestTrue(TEXT("Relogin forwards the bridge response"), bResponseReceived);
	TestFalse(TEXT("Failed cached session remains failed"), bReloginSucceeded);
	TestFalse(TEXT("Failed relogin clears connecting state"), FImtblPassportReloginTestAccessor::IsConnecting(*Passport));
	TestFalse(TEXT("Failed relogin remains disconnected"), FImtblPassportReloginTestAccessor::IsConnected(*Passport));

	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS
