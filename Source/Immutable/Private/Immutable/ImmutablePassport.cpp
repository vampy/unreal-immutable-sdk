// Fill out your copyright notice in the Description page of Project Settings.

#include "Immutable/ImmutablePassport.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "ImmutableAnalytics.h"
#include "Immutable/Misc/ImtblLogging.h"
#include "Immutable/ImtblJSConnector.h"
#include "JsonObjectConverter.h"
#include "Immutable/ImmutableSaveGame.h"
#include "Immutable/ImmutableSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameInstanceSubsystem.h"

#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
#include "GenericPlatform/GenericPlatformHttp.h"
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/ImmutableAndroidJNI.h"
#elif PLATFORM_IOS
#include "IOS/ImmutableIOS.h"
#elif PLATFORM_MAC
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Mac/ImmutableMac.h"
#endif
#if PLATFORM_WINDOWS
#include "Immutable/Windows/ImmutablePKCEWindows.h"
#endif

#define PASSPORT_SAVE_GAME_SLOT_NAME TEXT("Immutable")

namespace
{
FString GetResponseError(const FImtblJSResponse& Response, const FString& Fallback)
{
	if (Response.Error.IsSet())
	{
		const FString StructuredError = Response.Error->ToString();
		if (!StructuredError.IsEmpty())
		{
			return StructuredError;
		}
	}

	FString JsonError;
	if (Response.JsonObject.IsValid())
	{
		Response.JsonObject->TryGetStringField(TEXT("error"), JsonError);
	}
	return JsonError.IsEmpty() ? Fallback : JsonError;
}
} // namespace

void UImmutablePassport::Initialize(const FImmutablePassportInitData& Data, const FImtblPassportResponseDelegate& ResponseDelegate)
{
	check(JSConnector.IsValid());
	
	LoadPassportSettings();
	// we check saved settings in case if player has not logged out properly
	if (Data.logoutRedirectUri.IsEmpty() && IsStateFlagsSet(IPS_PKCE))
	{
		IMTBL_ERR("Logout URI is empty. Previously logged in via PKCE.")
		ResetStateFlags(IPS_PKCE);
		SavePassportSettings();
	}

	InitData = Data;

	CallJS(ImmutablePassportAction::INIT, InitData.ToJsonString(), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnInitializeResponse), false);
}

void UImmutablePassport::Initialize(const FImtblPassportResponseDelegate& ResponseDelegate)
{
	check(JSConnector.IsValid());

	const UImmutableSettings* Settings = GetDefault<UImmutableSettings>();

	if (!Settings)
	{
		ResponseDelegate.ExecuteIfBound(FImmutablePassportResult{false, "Failed to find Immutable Settings"});

		return;
	}

	UApplicationConfig* ApplicationConfig = Settings->DefaultApplicationConfig.GetDefaultObject();

	if (!ApplicationConfig)
	{
		ResponseDelegate.ExecuteIfBound(FImmutablePassportResult{false, "Failed to retrieve default application configuration for Passport initialization"});

		return;
	}

	InitData.clientId = ApplicationConfig->GetClientID();
	InitData.environment = ApplicationConfig->GetEnvironmentString();
	InitData.redirectUri = ApplicationConfig->GetRedirectURL();
	InitData.logoutRedirectUri = ApplicationConfig->GetLogoutURL();

	LoadPassportSettings();
	// we check saved settings in case if player has not logged out properly
	if (InitData.logoutRedirectUri.IsEmpty() && IsStateFlagsSet(IPS_PKCE))
	{
		IMTBL_ERR("Logout URI is empty. Previously logged in via PKCE.")
		ResetStateFlags(IPS_PKCE);
		SavePassportSettings();
	}

	CallJS(ImmutablePassportAction::INIT, InitData.ToJsonString(), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnInitializeResponse), false);
}

#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC | PLATFORM_WINDOWS
void UImmutablePassport::Connect(const FImtblPassportResponseDelegate& ResponseDelegate, const FImmutableDirectLoginOptions& DirectLoginOptions)
{
	SetStateFlags(IPS_CONNECTING | IPS_PKCE);

#if PLATFORM_WINDOWS
	// Verify PKCEData is null before initializing to ensure we're not overriding an active PKCE operation.
	// A non-null value indicates another PKCE operation is already in progress.
	PKCEData = UImmutablePKCEWindows::Initialise(InitData);
	if (PKCEData)
	{
		PKCEData->DynamicMulticastDelegate_DeepLinkCallback.AddDynamic(this, &ThisClass::OnDeepLinkActivated);
	}
#endif

	PKCEResponseDelegate = ResponseDelegate;
	Analytics->Track(UImmutableAnalytics::EEventName::START_LOGIN_PKCE);

	TSharedPtr<FJsonObject> RequestObject = MakeShareable(new FJsonObject);

	TSharedPtr<FJsonObject> DirectLoginOptionsObject = DirectLoginOptions.ToJsonObject();
	if (DirectLoginOptionsObject.IsValid())
	{
		RequestObject->SetObjectField(TEXT("directLoginOptions"), DirectLoginOptionsObject);
	}

	RequestObject->SetStringField(TEXT("imPassportTraceId"), DirectLoginOptions.ImPassportTraceId);

	// Convert to JSON string
	FString PKCERequestJson;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PKCERequestJson);
	if (!FJsonSerializer::Serialize(RequestObject.ToSharedRef(), Writer))
	{
		IMTBL_ERR("Failed to serialize PKCE request to JSON");
		FImmutablePassportResult Result;
		Result.Success = false;
		Result.Error = TEXT("Failed to serialize authentication request");
		ResponseDelegate.ExecuteIfBound(Result);
		return;
	}

	CallJS(ImmutablePassportAction::GetPKCEAuthUrl, PKCERequestJson, PKCEResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnGetAuthUrlResponse));
}
#endif

void UImmutablePassport::Logout(bool DoHardLogout, const FImtblPassportResponseDelegate& ResponseDelegate)
{
#if PLATFORM_WINDOWS
	// Verify PKCEData is null before initializing to ensure we're not overriding an active PKCE operation.
	// A non-null value indicates another PKCE operation is already in progress.
	ensureAlways(!PKCEData);
	PKCEData = UImmutablePKCEWindows::Initialise(InitData);
	if (PKCEData)
	{
		PKCEData->DynamicMulticastDelegate_DeepLinkCallback.AddDynamic(this, &ThisClass::OnDeepLinkActivated);
	}
#endif
#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC | PLATFORM_WINDOWS
	if (IsStateFlagsSet(IPS_PKCE))
	{
		PKCELogoutResponseDelegate = ResponseDelegate;
	}
#endif
	if (IsStateFlagsSet(IPS_CONNECTED))
	{
		if (DoHardLogout)
		{
			SetStateFlags(IPS_HARDLOGOUT);
		}
		CallJS(ImmutablePassportAction::Logout, TEXT(""), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnLogoutResponse));
	}
	else
	{
		IMTBL_WARN("Passport is not connected to execute logout.");
	}
}

void UImmutablePassport::ConnectEvm(const FImtblPassportResponseDelegate& ResponseDelegate)
{
	CallJS(ImmutablePassportAction::ConnectEvm, TEXT(""), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnBridgeCallbackResponse));
}

void UImmutablePassport::ZkEvmRequestAccounts(const FImtblPassportResponseDelegate& ResponseDelegate)
{
	CallJS(ImmutablePassportAction::ZkEvmRequestAccounts, TEXT(""), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnBridgeCallbackResponse));
}

void UImmutablePassport::ZkEvmGetBalance(const FImmutablePassportZkEvmGetBalanceData& Data, const FImtblPassportResponseDelegate& ResponseDelegate)
{
	CallJS(ImmutablePassportAction::ZkEvmGetBalance, Data.ToJsonString(), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnBridgeCallbackResponse));
}

void UImmutablePassport::ZkEvmSendTransaction(const FImtblTransactionRequest& Request, const FImtblPassportResponseDelegate& ResponseDelegate)
{
	CallJS(ImmutablePassportAction::ZkEvmSendTransaction, UStructToJsonString(Request), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnBridgeCallbackResponse));
}

void UImmutablePassport::ZkEvmSendTransactionWithConfirmation(const FImtblTransactionRequest& Request,
	const FImtblPassportResponseDelegate& ResponseDelegate)
{
	CallJS(ImmutablePassportAction::zkEvmSendTransactionWithConfirmation, UStructToJsonString(Request), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnBridgeCallbackResponse));
}

void UImmutablePassport::ZkEvmGetTransactionReceipt(const FZkEvmTransactionReceiptRequest& Request, const FImtblPassportResponseDelegate& ResponseDelegate)
{
	CallJS(ImmutablePassportAction::ZkEvmGetTransactionReceipt, UStructToJsonString(Request), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnBridgeCallbackResponse));
}

void UImmutablePassport::ZkEvmSignTypedDataV4(const FString& RequestJsonString, const FImtblPassportResponseDelegate& ResponseDelegate)
{
	CallJS(ImmutablePassportAction::ZkEvmSignTypedDataV4, RequestJsonString, ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnBridgeCallbackResponse));
}



void UImmutablePassport::GetIdToken(const FImtblPassportResponseDelegate& ResponseDelegate)
{
	CallJS(ImmutablePassportAction::GetIdToken, TEXT(""), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnBridgeCallbackResponse));
}

void UImmutablePassport::GetAccessToken(const FImtblPassportResponseDelegate& ResponseDelegate)
{
	CallJS(ImmutablePassportAction::GetAccessToken, TEXT(""), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnBridgeCallbackResponse));
}

void UImmutablePassport::GetEmail(const FImtblPassportResponseDelegate& ResponseDelegate)
{
	CallJS(ImmutablePassportAction::GetEmail, TEXT(""), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnBridgeCallbackResponse));
}

void UImmutablePassport::GetLinkedAddresses(const FImtblPassportResponseDelegate& ResponseDelegate)
{
	CallJS(ImmutablePassportAction::GetLinkedAddresses, TEXT(""), ResponseDelegate, FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnBridgeCallbackResponse));
}

void UImmutablePassport::HasStoredCredentials(const FImtblPassportResponseDelegate& ResponseDelegate)
{
	// we do check credentials into two steps, we check accessToken and then IdToken
	// check access token
	CallJS(ImmutablePassportAction::GetAccessToken, TEXT(""), ResponseDelegate, FImtblJSResponseDelegate::CreateLambda([this, ResponseDelegate](FImtblJSResponse Response)
	{
		const FString AccessToken = GetResponseResultAsString(Response);
		if (!Response.success || AccessToken.IsEmpty())
		{
			ResponseDelegate.ExecuteIfBound(FImmutablePassportResult{
				false, GetResponseError(Response, TEXT("Failed to retrieve Access Token.")), Response});
		}
		else
		{
			// check for id token
			CallJS(ImmutablePassportAction::GetIdToken, TEXT(""), ResponseDelegate, FImtblJSResponseDelegate::CreateLambda([ResponseDelegate](FImtblJSResponse Response)
			{
				const FString IdToken = GetResponseResultAsString(Response);
				if (!Response.success || IdToken.IsEmpty())
				{
					ResponseDelegate.ExecuteIfBound(FImmutablePassportResult{
						false, GetResponseError(Response, TEXT("Failed to retrieve Id Token.")), Response});
				}
				else
				{
					ResponseDelegate.ExecuteIfBound(FImmutablePassportResult{Response.success, "", Response});
				}
			}));
		}
	}));
}

FString UImmutablePassport::GetResponseResultAsString(const FImtblJSResponse& Response)
{
	if (!Response.JsonObject.IsValid())
	{
		IMTBL_ERR("Response JSON data for %s is not valid", *Response.responseFor)
		
		return "";	
	}

	FString Result;
	Response.JsonObject->TryGetStringField(TEXT("result"), Result);
	return Result;
}

bool UImmutablePassport::GetResponseResultAsBool(const FImtblJSResponse& Response)
{
	if (!Response.JsonObject.IsValid())
	{
		IMTBL_ERR("Response JSON data for %s is not valid", *Response.responseFor)
		
		return false;	
	}

	bool bResult = false;
	Response.JsonObject->TryGetBoolField(TEXT("result"), bResult);
	return bResult;
}

TArray<FString> UImmutablePassport::GetResponseResultAsStringArray(const FImtblJSResponse& Response)
{
	if (!Response.JsonObject.IsValid())
	{
		IMTBL_ERR("Response JSON data for %s is not valid", *Response.responseFor)
		
		return TArray<FString>();	
	}
	
	TArray<FString> StringArray;
	const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
	if (!Response.JsonObject->TryGetArrayField(TEXT("result"), Values) || !Values)
	{
		return StringArray;
	}

	for (const TSharedPtr<FJsonValue>& Value : *Values)
	{
		FString StringValue;
		if (Value.IsValid() && Value->TryGetString(StringValue))
		{
			StringArray.Add(MoveTemp(StringValue));
		}
	}

	return StringArray;
}


void UImmutablePassport::Setup(const TWeakObjectPtr<UImtblJSConnector> Connector)
{
	IMTBL_LOG_FUNCSIG

	if (!Connector.IsValid())
	{
		IMTBL_ERR("Invalid JSConnector passed to UImmutablePassport::Setup.")
		return;
	}

	JSConnector = Connector.Get();

	// Analytics
	Analytics = NewObject<UImmutableAnalytics>(this);
	Analytics->Setup(Connector);
}



bool UImmutablePassport::CheckIsInitialized(const FString& Action, const FImtblPassportResponseDelegate& ResponseDelegate) const
{
	const bool IsInitialized = IsStateFlagsSet(IPS_INITIALIZED);

	if (!IsInitialized)
	{
		IMTBL_WARN("Attempting action '%s' before Passport is initialized", *Action)
		ResponseDelegate.ExecuteIfBound(FImmutablePassportResult{false, "Passport is not initialized"});
	}

	return IsInitialized;
}

void UImmutablePassport::CallJS(const FString& Action, const FString& Data, const FImtblPassportResponseDelegate& ClientResponseDelegate, const FImtblJSResponseDelegate& HandleJSResponse, const bool bCheckInitialized /*= true*/)
{
	if (bCheckInitialized && !CheckIsInitialized(Action, ClientResponseDelegate)) { return; }

	check(JSConnector.IsValid());
	const FString Guid = JSConnector->CallJS(Action, Data, HandleJSResponse);
	ResponseDelegates.Add(Guid, ClientResponseDelegate);
}

TOptional<UImmutablePassport::FImtblPassportResponseDelegate> UImmutablePassport::GetResponseDelegate(const FImtblJSResponse& Response)
{
	FImtblPassportResponseDelegate ResponseDelegate;
	if (!ResponseDelegates.RemoveAndCopyValue(Response.requestId, ResponseDelegate))
	{
		IMTBL_WARN("Couldn't find delegate; Action=%s, RequestId=%s", *Response.responseFor, *Response.requestId)
		return TOptional<FImtblPassportResponseDelegate>();
	}
	return ResponseDelegate;
}

void UImmutablePassport::OnInitializeResponse(FImtblJSResponse Response)
{
	if (TOptional<FImtblPassportResponseDelegate> ResponseDelegate = GetResponseDelegate(Response))
	{
		FString Error;
		
		if (Response.success)
		{
			SetStateFlags(IPS_INITIALIZED);
			IMTBL_LOG("Passport initialization succeeded.")
		}
		else
		{
			IMTBL_ERR("Passport initialization failed.")
			Error = GetResponseError(Response, TEXT("Passport initialization failed."));
		}
		Analytics->Track(UImmutableAnalytics::EEventName::INIT_PASSPORT, Response.success);
		ResponseDelegate->ExecuteIfBound(FImmutablePassportResult { Response.success, Error, Response });
	}
}



void UImmutablePassport::OnLogoutResponse(FImtblJSResponse Response)
{
	auto ResponseDelegate = GetResponseDelegate(Response);
	
	if (!ResponseDelegate)
	{
		return;
	}

	FString Message;

	if (!Response.success)
	{
		Message = GetResponseError(Response, TEXT("Passport logout failed."));

		IMTBL_ERR("%s", *Message)
		ResponseDelegate->ExecuteIfBound(FImmutablePassportResult{ Response.success, Message, Response });

		return;
	}

	if (!IsStateFlagsSet(IPS_HARDLOGOUT))
	{
		IMTBL_LOG("Logged out without clearing browser session")
		ResponseDelegate->ExecuteIfBound(FImmutablePassportResult{ true });
		
		return;
	}

	ResetStateFlags(IPS_HARDLOGOUT);

	const FString Url = GetResponseResultAsString(Response);

	if (!Url.IsEmpty())
	{
#if PLATFORM_ANDROID || PLATFORM_IOS || PLATFORM_MAC || PLATFORM_WINDOWS
		OnHandleDeepLink.AddUObject(this, &UImmutablePassport::OnDeepLinkActivated);
#endif
#if PLATFORM_ANDROID
		LaunchAndroidUrl(Url);
#elif PLATFORM_IOS
		[[ImmutableIOS instance] launchUrl:TCHAR_TO_ANSI(*Url)];
#elif PLATFORM_MAC
		[[ImmutableMac instance] launchUrl:TCHAR_TO_ANSI(*Url) forRedirectUri:TCHAR_TO_ANSI(*InitData.logoutRedirectUri)];
#elif PLATFORM_WINDOWS
		FString ErrorMessage;
		LaunchURL(Url, TEXT(""), ErrorMessage);
		if (!ErrorMessage.IsEmpty())
		{
			ErrorMessage = "Failed to launch browser: " + ErrorMessage;
			IMTBL_ERR("%s", *ErrorMessage)
			ResponseDelegate->ExecuteIfBound(FImmutablePassportResult{false, ErrorMessage, Response});
		}
#endif
	}
	else
	{
		ResponseDelegate->ExecuteIfBound(FImmutablePassportResult{false, "Logout Url is empty", Response});
	}
	ResetStateFlags(IPS_CONNECTED);
}

#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC | PLATFORM_WINDOWS
void UImmutablePassport::OnGetAuthUrlResponse(FImtblJSResponse Response)
{
	if (PKCEResponseDelegate.IsBound())
	{
		FString Msg = GetResponseResultAsString(Response);
		if (!Response.success || Msg.IsEmpty())
		{
			const FString Error = GetResponseError(Response, TEXT("Could not get PKCE auth URL from Passport."));
			IMTBL_ERR("Bridge request failed; Action=%s, RequestId=%s, Error=%s", *Response.responseFor, *Response.requestId, *Error)
			PKCEResponseDelegate.ExecuteIfBound(FImmutablePassportResult{false, Error, Response});
			PKCEResponseDelegate.Unbind();
			ResetStateFlags(IPS_PKCE | IPS_CONNECTING);
			return;
		}

		// Handle deeplink calls
		OnHandleDeepLink.AddUObject(this, &UImmutablePassport::OnDeepLinkActivated);

		Msg = Msg.Replace(TEXT(" "), TEXT("+"));
#if PLATFORM_ANDROID
		OnPKCEDismissed = FImtblPassportOnPKCEDismissedDelegate::CreateUObject(this, &UImmutablePassport::HandleOnLoginPKCEDismissed);
		LaunchAndroidUrl(Msg);
#elif PLATFORM_IOS
		[[ImmutableIOS instance] launchUrl:TCHAR_TO_ANSI(*Msg)];
#elif PLATFORM_MAC
		[[ImmutableMac instance] launchUrl:TCHAR_TO_ANSI(*Msg) forRedirectUri:TCHAR_TO_ANSI(*InitData.redirectUri)];
#elif PLATFORM_WINDOWS
		FString ErrorMessage;
		LaunchURL(Msg, TEXT(""), ErrorMessage);
		if (!ErrorMessage.IsEmpty())
		{
			ErrorMessage = "Failed to launch browser: " + ErrorMessage;
			IMTBL_ERR("%s", *ErrorMessage);
			PKCEResponseDelegate.ExecuteIfBound(FImmutablePassportResult{false, ErrorMessage});
			PKCEResponseDelegate.Unbind();
			ResetStateFlags(IPS_PKCE | IPS_CONNECTING);
		}
#endif
	}
	else
	{
		IMTBL_ERR("Unable to return a response for Connect PKCE.");
	}
}

void UImmutablePassport::OnConnectResponse(FImtblJSResponse Response)
{
	if (PKCEResponseDelegate.IsBound())
	{
		FString Msg;

		if (Response.success)
		{
			IMTBL_LOG("Successfully connected via PKCE")
			SetStateFlags(IPS_CONNECTED);
		}
		else
		{
			IMTBL_WARN("Connect PKCE attempt failed.");
			ResetStateFlags(IPS_PKCE);
			Msg = GetResponseError(Response, TEXT("Passport connection failed."));
		}
		Analytics->Track(UImmutableAnalytics::EEventName::COMPLETE_LOGIN_PKCE, Response.success);
		PKCEResponseDelegate.ExecuteIfBound(FImmutablePassportResult{Response.success, Msg, Response});
		PKCEResponseDelegate = nullptr;

		// we save passport state for PKCE flow in case if we decide to close a game
		// and reopen it later with Passport is still being connected and we decided to logout.
		// In this case, we logout using PKCE flow
		SavePassportSettings();
	}
	else
	{
		IMTBL_ERR("Unable to return a response for Connect PKCE.");
	}
	ResetStateFlags(IPS_COMPLETING_PKCE);
}
#endif

void UImmutablePassport::OnBridgeCallbackResponse(FImtblJSResponse Response)
{
	auto ResponseDelegate = GetResponseDelegate(Response);

	if (!ResponseDelegate)
	{
		IMTBL_ERR("Response delegate is not assigned for %s", *Response.responseFor);
		return;
	}

	const FString Error = GetResponseError(Response, TEXT(""));
	if (!Response.success)
	{
		const FString ErrorMessage = Error.IsEmpty() ? TEXT("<none provided>") : Error;
		IMTBL_ERR_FUNC("Bridge request failed; Action=%s, RequestId=%s, Error=%s", *Response.responseFor, *Response.requestId, *ErrorMessage);
	}

	ResponseDelegate->ExecuteIfBound(FImmutablePassportResult{ Response.success, Error, Response });
}

void UImmutablePassport::SetStateFlags(uint8 StateIn)
{
	StateFlags |= StateIn;
}

void UImmutablePassport::ResetStateFlags(uint8 StateIn)
{
	StateFlags &= ~StateIn;
}

bool UImmutablePassport::IsStateFlagsSet(uint8 StateIn) const
{
	return (StateFlags & StateIn) == StateIn;
}

void UImmutablePassport::SavePassportSettings()
{
	UImmutableSaveGame* SaveGameInstance = Cast<UImmutableSaveGame>(UGameplayStatics::CreateSaveGameObject(UImmutableSaveGame::StaticClass()));
	if (!SaveGameInstance)
	{
		IMTBL_ERR("Failed to create Passport save game object")
		return;
	}
	SaveGameInstance->bWasConnectedViaPKCEFlow = IsStateFlagsSet(IPS_PKCE | IPS_CONNECTED);

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, PASSPORT_SAVE_GAME_SLOT_NAME, GetWorldUserIndex());
}

void UImmutablePassport::LoadPassportSettings()
{
	const int32 UserIndex = GetWorldUserIndex();
	if (UGameplayStatics::DoesSaveGameExist(PASSPORT_SAVE_GAME_SLOT_NAME, UserIndex))
	{
		if (const UImmutableSaveGame* SaveGameInstance = Cast<UImmutableSaveGame>(
			UGameplayStatics::LoadGameFromSlot(PASSPORT_SAVE_GAME_SLOT_NAME, UserIndex)))
		{
			SaveGameInstance->bWasConnectedViaPKCEFlow ? SetStateFlags(IPS_PKCE) : ResetStateFlags(IPS_PKCE);
		}
	}
}

UGameInstance* UImmutablePassport::GetGameInstance() const
{
	if (const UGameInstanceSubsystem* Subsystem = Cast<UGameInstanceSubsystem>(GetOuter()))
	{
		return Subsystem->GetGameInstance();
	}
	return nullptr;
}

int32 UImmutablePassport::GetWorldUserIndex() const
{
#if WITH_EDITOR
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const FWorldContext* WorldContext = GameInstance->GetWorldContext())
		{
			return WorldContext->PIEInstance == INDEX_NONE ? 0 : WorldContext->PIEInstance;
		}
	}
#endif
	return 0;
}

void UImmutablePassport::LaunchURL(const FString& URL, const FString& Params, FString& OutError)
{
	if (CustomLaunchURLDelegate.IsBound())
	{
		CustomLaunchURLDelegate.Execute(URL, Params, OutError);
		return;
	}
	FPlatformProcess::LaunchURL(*URL, Params.IsEmpty() ? nullptr : *Params, &OutError);
}

void UImmutablePassport::OnDeepLinkActivated(const FString& DeepLink)
{
#if PLATFORM_ANDROID || PLATFORM_IOS || PLATFORM_MAC || PLATFORM_WINDOWS
	OnHandleDeepLink.Clear();
	if (DeepLink.StartsWith(InitData.logoutRedirectUri))
	{
		// execute on game thread to prevent call to Passport instance from another thread
		if (FTaskGraphInterface::IsRunning())
		{
			FGraphEventRef GameThreadTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
				[this]()
				{
					Analytics->Track(UImmutableAnalytics::EEventName::COMPLETE_LOGOUT_PKCE);
					IMTBL_LOG("Complete Logout PKCE")
					PKCELogoutResponseDelegate.ExecuteIfBound(FImmutablePassportResult{true, "Logged out"});
					PKCELogoutResponseDelegate = nullptr;
					ResetStateFlags(IPS_CONNECTED | IPS_PKCE);
					SavePassportSettings();
				},
				TStatId(),
				nullptr,
				ENamedThreads::GameThread);
		}
	}
	else if (DeepLink.StartsWith(InitData.redirectUri))
	{
		CompleteLoginFlow(DeepLink);
	}

	PKCEData = nullptr;
#endif
}

#if PLATFORM_ANDROID || PLATFORM_IOS || PLATFORM_MAC || PLATFORM_WINDOWS
void UImmutablePassport::CompleteLoginFlow(FString Url)
{
	// Required mainly for Android to detect when Chrome Custom tabs is dismissed
	// See HandleOnLoginPKCEDismissed
	SetStateFlags(IPS_COMPLETING_PKCE);

	// Get code and state from deeplink URL
	TOptional<FString> Code, State;
	FString Endpoint, Params;
	Url.Split(TEXT("?"), &Endpoint, &Params);
	TArray<FString> ParamsArray;

	Params.ParseIntoArray(ParamsArray, TEXT("&"));
	for (FString Param : ParamsArray)
	{
		FString Key, Value;

		if (Param.StartsWith("code"))
		{
			Param.Split(TEXT("="), &Key, &Value);
			Code = Value;
		}
		else if (Param.StartsWith("state"))
		{
			Param.Split(TEXT("="), &Key, &Value);
			State = Value;
		}
	}

	if (!Code.IsSet() || !State.IsSet())
	{
		const FString ErrorMsg = "Uri was missing state and/or code. Please call ConnectPKCE() again";

		IMTBL_ERR("%s", *ErrorMsg);
		PKCEResponseDelegate.ExecuteIfBound(FImmutablePassportResult{false, ErrorMsg});
		PKCEResponseDelegate = nullptr;
		ResetStateFlags(IPS_PKCE | IPS_CONNECTING | IPS_COMPLETING_PKCE);
		SavePassportSettings();
	}
	else
	{
		FImmutablePassportConnectData Data = FImmutablePassportConnectData{Code.GetValue(), State.GetValue()};

		CallJS(ImmutablePassportAction::LOGIN_PKCE, UStructToJsonString(Data), PKCEResponseDelegate,
		       FImtblJSResponseDelegate::CreateUObject(this, &UImmutablePassport::OnConnectResponse));
	}
}
#endif

#if PLATFORM_ANDROID || PLATFORM_IOS || PLATFORM_MAC || PLATFORM_WINDOWS
#if PLATFORM_ANDROID || PLATFORM_WINDOWS
// Called from Android JNI
void UImmutablePassport::HandleDeepLink(FString DeepLink) const
#else
// Called from iOS Objective C
void UImmutablePassport::HandleDeepLink(NSString* sDeepLink) const
#endif
{
#if PLATFORM_IOS | PLATFORM_MAC
	FString DeepLink = FString(UTF8_TO_TCHAR([sDeepLink UTF8String]));
	IMTBL_LOG("Handle Deep Link: %s", *DeepLink);
#endif
#if PLATFORM_WINDOWS
	if (PKCEData)
	{
		UImmutablePKCEWindows::HandleDeepLink(PKCEData, DeepLink);
	}
#endif

	OnHandleDeepLink.Broadcast(DeepLink);
}
#endif

#if PLATFORM_ANDROID
void UImmutablePassport::HandleOnLoginPKCEDismissed()
{
	IMTBL_LOG("Handle On Login PKCE Dismissed");
	OnPKCEDismissed = nullptr;

	// If the second part of PKCE (CompleteLoginPKCEFlow) has not started yet and custom tabs is dismissed,
	// this means the user manually dismissed the custom tabs before entering all
	// all required details (e.g. email address) into Passport
	// Cannot use IPS_CONNECTING as that is set when PKCE flow is initiated. Here we are checking against the second
	// half of the PKCE flow.
	if (!IsStateFlagsSet(IPS_COMPLETING_PKCE))
	{
		// User hasn't entered all required details (e.g. email address) into
		// Passport yet
		IMTBL_LOG("Login PKCE dismissed before completing the flow");
		if (FTaskGraphInterface::IsRunning())
		{
			FGraphEventRef GameThreadTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this]()
				{
					if (!PKCEResponseDelegate.ExecuteIfBound(FImmutablePassportResult{ false, "Cancelled" }))
					{
						IMTBL_WARN("Login PKCEResponseDelegate delegate was not called");
					}
					PKCEResponseDelegate = nullptr;
				}, TStatId(), nullptr, ENamedThreads::GameThread);
		}
	}
	else
	{
		IMTBL_LOG("PKCE dismissed by user or SDK");
	}
}

void UImmutablePassport::HandleCustomTabsDismissed(FString Url)
{
	IMTBL_LOG("On PKCE Dismissed");

	if (!OnPKCEDismissed.ExecuteIfBound())
	{
		IMTBL_WARN("OnPKCEDismissed delegate was not called");
	}
}

void UImmutablePassport::CallJniStaticVoidMethod(JNIEnv *Env, const jclass Class, jmethodID Method, ...)
{
	va_list Args;

	va_start(Args, Method);
	Env->CallStaticVoidMethodV(Class, Method, Args);
	va_end(Args);
	Env->DeleteLocalRef(Class);
}

void UImmutablePassport::LaunchAndroidUrl(FString Url)
{
	if (JNIEnv *Env = FAndroidApplication::GetJavaEnv())
	{
		jstring jurl = Env->NewStringUTF(TCHAR_TO_UTF8(*Url));
		jclass jimmutableAndroidClass = FAndroidApplication::FindJavaClass("com/immutable/unreal/ImmutableActivity");
		static jmethodID jlaunchUrl = FJavaWrapper::FindStaticMethod(Env, jimmutableAndroidClass, "startActivity", "(Landroid/app/Activity;Ljava/lang/String;)V", false);
					
		CallJniStaticVoidMethod(Env, jimmutableAndroidClass, jlaunchUrl, FJavaWrapper::GameActivityThis, jurl);
	}
}
#endif
