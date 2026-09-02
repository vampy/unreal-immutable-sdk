// Fill out your copyright notice in the Description page of Project Settings.

#include "ImtblBrowserWidget.h"

#include "Immutable/ImmutableUtilities.h"
#include "Immutable/ImtblJSConnector.h"
#include "Immutable/Misc/ImtblLogging.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#if USING_BUNDLED_CEF
#include "SWebBrowser.h"
#include "WebBrowserModule.h"
#if WITH_CEF3
// CEF headers include Windows types and define macros which conflict with Unreal's sanitized platform headers, so use the same include
// boundary as the engine WebBrowser module. Omitting these guards causes errors such as CEF's TRUE constant being undefined.
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif
#pragma push_macro("OVERRIDE")
#undef OVERRIDE
THIRD_PARTY_INCLUDES_START
#include "include/cef_request_context.h"
#include "include/cef_version.h"
THIRD_PARTY_INCLUDES_END
#pragma pop_macro("OVERRIDE")
#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#endif
#endif

namespace
{
const TCHAR* ImmutableBridgeUrl = TEXT("file:///immutable/index.html");

const TCHAR* ImmutableCefBindingShim = TEXT(R"JS(
(function() {
	if (window.__immutableUeConnectorShimInstalled) {
		return;
	}

	window.__immutableUeConnectorShimInstalled = true;
	window.ue = window.ue || {};

	var queuedMessages = [];
	var proxyConnector = {
		sendtogame: function(message) {
			queuedMessages.push(message);
			console.log("[ImmutableBridgeDebug] Unreal JSConnector pending; queuedCallbacks=" + queuedMessages.length);
		}
	};

	if (!window.ue.jsconnector) {
		window.ue.jsconnector = proxyConnector;
	}

	function getRealConnector() {
		if (!window.ue || !window.ue.jsconnector || window.ue.jsconnector === proxyConnector) {
			return null;
		}

		return window.ue.jsconnector;
	}

	function flushQueuedMessages() {
		var connector = getRealConnector();
		if (!connector || typeof connector.sendtogame !== "function") {
			return false;
		}

		if (queuedMessages.length > 0) {
			console.log("[ImmutableBridgeDebug] Unreal JSConnector ready; flushingCallbacks=" + queuedMessages.length);
		}

		while (queuedMessages.length > 0) {
			connector.sendtogame(queuedMessages.shift());
		}

		return true;
	}

	var attempts = 0;
	var pollHandle = setInterval(function() {
		attempts++;
		if (flushQueuedMessages() || attempts >= 200) {
			clearInterval(pollHandle);
			if (queuedMessages.length > 0) {
				console.error("[ImmutableBridgeDebug] Unreal JSConnector not defined after wait; queuedCallbacks=" + queuedMessages.length);
			}
		}
	}, 25);
})();
)JS");

#if USING_BUNDLED_CEF
// Illuvium custom engine integration: this helper checks a WebBrowser setting added by our engine patch; stock Unreal Engine lacks it.
// Keep this check aligned with the custom engine's [Browser] bUseExplicitCEFRootCachePath option.
// A stock engine can still read this project setting as true, but it will not apply the required root_cache_path behavior inside CEF.
bool IsExplicitCEFRootCachePathEnabled()
{
	bool bEnabled = false;
	if (GConfig)
	{
		GConfig->GetBool(TEXT("Browser"), TEXT("bUseExplicitCEFRootCachePath"), bEnabled, GEngineIni);
	}
	return bEnabled;
}

const ANSICHAR* GetCompileTimeCefVersion()
{
#if WITH_CEF3 && defined(CEF_VERSION)
	return CEF_VERSION;
#else
	return "unknown";
#endif
}

int32 GetCompileTimeChromeVersionMajor()
{
#if WITH_CEF3 && defined(CHROME_VERSION_MAJOR)
	return CHROME_VERSION_MAJOR;
#else
	return 0;
#endif
}

int32 GetCompileTimeChromeVersionMinor()
{
#if WITH_CEF3 && defined(CHROME_VERSION_MINOR)
	return CHROME_VERSION_MINOR;
#else
	return 0;
#endif
}

int32 GetCompileTimeChromeVersionBuild()
{
#if WITH_CEF3 && defined(CHROME_VERSION_BUILD)
	return CHROME_VERSION_BUILD;
#else
	return 0;
#endif
}

int32 GetCompileTimeChromeVersionPatch()
{
#if WITH_CEF3 && defined(CHROME_VERSION_PATCH)
	return CHROME_VERSION_PATCH;
#else
	return 0;
#endif
}

const TCHAR* GetCefBranchName()
{
#if WITH_CEF3 && defined(CEF3_USE_EXPERIMENTAL_VERSION)
	return CEF3_USE_EXPERIMENTAL_VERSION ? TEXT("experimental") : TEXT("legacy");
#elif WITH_CEF3
	return TEXT("unknown-cef3");
#else
	return TEXT("no-cef3");
#endif
}

FString GetGlobalCefProfilePath()
{
#if WITH_CEF3
	IWebBrowserModule::Get().GetSingleton();
	const CefRefPtr<CefRequestContext> GlobalContext = CefRequestContext::GetGlobalContext();
	if (GlobalContext)
	{
		const FString ProfilePath = WCHAR_TO_TCHAR(GlobalContext->GetCachePath().ToWString().c_str());
		return ProfilePath.IsEmpty() ? TEXT("<in-memory>") : FPaths::ConvertRelativePathToFull(ProfilePath);
	}
#endif

	return TEXT("<unavailable>");
}

bool ShouldUseImmutableCefBindingShim()
{
	return GetCompileTimeChromeVersionMajor() >= 128;
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
const TCHAR* GetConsoleLogSeverityName(const EWebBrowserConsoleLogSeverity Severity)
{
	switch (Severity)
	{
		case EWebBrowserConsoleLogSeverity::Default:
			return TEXT("Default");
		case EWebBrowserConsoleLogSeverity::Verbose:
			return TEXT("Verbose");
		case EWebBrowserConsoleLogSeverity::Debug:
			return TEXT("Debug");
		case EWebBrowserConsoleLogSeverity::Info:
			return TEXT("Info");
		case EWebBrowserConsoleLogSeverity::Warning:
			return TEXT("Warning");
		case EWebBrowserConsoleLogSeverity::Error:
			return TEXT("Error");
		case EWebBrowserConsoleLogSeverity::Fatal:
			return TEXT("Fatal");
		default:
			return TEXT("Unknown");
	}
}
#endif
#endif
} // namespace


UImtblBrowserWidget::UImtblBrowserWidget()
{
	IMTBL_LOG_FUNCSIG

	JSConnector = NewObject<UImtblJSConnector>(this, "JSConnector");
	JSConnector->ExecuteJs = UImtblJSConnector::FOnExecuteJsDelegate::CreateUObject(this, &UImtblBrowserWidget::ExecuteJS);

	InitialURL = TEXT("about:blank");
}

void UImtblBrowserWidget::BindConnector()
{
	if (JSConnector && JSConnector->IsBound())
	{
		IMTBL_LOG_FUNC("JSConnector already bound; PageLoaded=%d", IsPageLoaded())
		return;
	}

	IMTBL_LOG("Setting up %s...", *UImtblJSConnector::StaticClass()->GetName())

	if (JSConnector)
	{
		const bool bBindSucceeded = BindUObject(UImtblJSConnector::JSObjectName(), JSConnector);
		IMTBL_LOG_FUNC("BindUObject result=%d; PageLoaded=%d", bBindSucceeded, IsPageLoaded())
		if (bBindSucceeded)
		{
			JSConnector->Init(IsPageLoaded());
		}
	}
	else
	{
		IMTBL_ERR_FUNC("JSConnector is null")
	}
}

TWeakObjectPtr<UImtblJSConnector> UImtblBrowserWidget::GetJSConnector() const
{
	return JSConnector;
}

bool UImtblBrowserWidget::IsPageLoaded() const
{
#if USING_BUNDLED_CEF
	return WebBrowserWidget.IsValid() && WebBrowserWidget->IsLoaded();
#else
	return false;
#endif
}

void UImtblBrowserWidget::ExecuteJS(const FString& ScriptText) const
{
#if USING_BUNDLED_CEF
	if (WebBrowserWidget.IsValid())
	{
		WebBrowserWidget->ExecuteJavascript(ScriptText);
	}
#endif
}

UGameInstance* UImtblBrowserWidget::GetGameInstance() const
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetGameInstance();
	}

	return nullptr;
}

int32 UImtblBrowserWidget::GetWorldUserIndex() const
{
	if (WITH_EDITOR)
	{
		UGameInstance* GameInstance = GetGameInstance();
		if (!GameInstance)
		{
			return 0;
		}

		const FWorldContext* WorldContext = GameInstance->GetWorldContext();
		const int32 InstanceID = WorldContext ? WorldContext->PIEInstance : INDEX_NONE;
		return InstanceID == INDEX_NONE ? 0 : InstanceID;
	}

	return 0;
}

void UImtblBrowserWidget::SetBrowserContent()
{
#if USING_BUNDLED_CEF
	if (!WebBrowserWidget.IsValid())
	{
		IMTBL_ERR("Browser widget is not valid")
		return;
	}

	FString JavaScript;

	if (FImmutableUtilities::LoadGameBridge(JavaScript))
	{
		const bool bUseBindingShim = ShouldUseImmutableCefBindingShim();
		FString IndexHtml = FString(
								"<!doctype html><html lang='en'><head><meta "
								"charset='utf-8'><title>GameSDK Bridge</title><script>") +
							(bUseBindingShim ? FString(ImmutableCefBindingShim) : FString()) + JavaScript +
							FString("</script></head><body><h1>Bridge Running</h1></body></html>");

		IMTBL_LOG_FUNC(
			"Loading Immutable bridge; Url=%s, CEFVersion=%hs, ChromeVersion=%d.%d.%d.%d, CEFBranch=%s, BindingShim=%s, "
			"JavaScriptLength=%d",
			ImmutableBridgeUrl,
			GetCompileTimeCefVersion(),
			GetCompileTimeChromeVersionMajor(),
			GetCompileTimeChromeVersionMinor(),
			GetCompileTimeChromeVersionBuild(),
			GetCompileTimeChromeVersionPatch(),
			GetCefBranchName(),
			bUseBindingShim ? TEXT("enabled") : TEXT("disabled"),
			JavaScript.Len())

		WebBrowserWidget->LoadString(IndexHtml, ImmutableBridgeUrl);
	}
	else
	{
		IMTBL_ERR_FUNC("Failed to load Immutable game bridge JavaScript")
	}
#endif
}

bool UImtblBrowserWidget::BindUObject(const FString& Name, UObject* Object, const bool bIsPermanent) const
{
#if USING_BUNDLED_CEF
	if (!WebBrowserWidget)
	{
		IMTBL_WARN_FUNC("Could not bind UObject '%s' to browser, WebBrowserWidget is null", *Object->GetName())
		return false;
	}

	WebBrowserWidget->BindUObject(Name, Object, bIsPermanent);
#endif
	return true;
}

void UImtblBrowserWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
#if USING_BUNDLED_CEF
	WebBrowserWidget.Reset();
#endif
}

TSharedRef<SWidget> UImtblBrowserWidget::RebuildWidget()
{
	if (IsDesignTime())
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)[SNew(STextBlock).Text(NSLOCTEXT("Immutable", "Immutable Web Browser", "Immutable Web Browser"))];
	}
	else
	{
#if WITH_EDITOR
		const int32 PIEInstanceIndex = GetWorldUserIndex();
		const int32 BrowserProfileIndex = PIEInstanceIndex + 1;
		const FString GlobalProfilePath = GetGlobalCefProfilePath();
		TOptional<FBrowserContextSettings> BrowserContextSettings;

		// Every PIE user receives a stable, isolated, one-based context so editor browsers never use the global profile shared with packaged
		// games. All browser widgets belonging to one PIE user share a session, while different PIE users remain isolated from each other.
		//
		// The Illuvium engine sets CEF's explicit root_cache_path and normalizes Windows paths when bUseExplicitCEFRootCachePath is enabled.
		// Build each editor profile beside the global Default profile so CEF 128 accepts it as an immediate child of the same root.
		// Without those engine modifications, CEF rejects the persistent child path and defaults the editor context to in-memory storage.
		// The resulting "not a child of the root_cache_path" and "cache_path is invalid" errors in cef3.log are expected on a stock engine;
		// they also mean that the PIE user's browser state will not persist across editor restarts.
		const FString ContextId = FString::Printf(TEXT("ImmutablePassportPIEUser%d"), BrowserProfileIndex);
		BrowserContextSettings.Emplace(ContextId);
		const FString CEFRootPath = FPaths::GetPath(GlobalProfilePath);
		const bool bHasUsableRoot = IsExplicitCEFRootCachePathEnabled() &&
			FPaths::GetCleanFilename(GlobalProfilePath).Equals(TEXT("Default"), ESearchCase::IgnoreCase) && !CEFRootPath.IsEmpty() &&
			!FPaths::IsRelative(CEFRootPath);

		if (bHasUsableRoot)
		{
			BrowserContextSettings->CookieStorageLocation = FPaths::Combine(CEFRootPath, ContextId);
			IMTBL_LOG_FUNC(
				"Editor browser using persistent isolated CEF profile; ContextId=%s, GlobalProfile=%s, RequestedProfile=%s, "
				"CEFVersion=%hs, ChromeVersion=%d.%d.%d.%d, CEFBranch=%s, PIEInstanceIndex=%d, BrowserProfileIndex=%d",
				*ContextId,
				*GlobalProfilePath,
				*BrowserContextSettings->CookieStorageLocation,
				GetCompileTimeCefVersion(),
				GetCompileTimeChromeVersionMajor(),
				GetCompileTimeChromeVersionMinor(),
				GetCompileTimeChromeVersionBuild(),
				GetCompileTimeChromeVersionPatch(),
				GetCefBranchName(),
				PIEInstanceIndex,
				BrowserProfileIndex)
		}
		else
		{
			IMTBL_WARN_FUNC(
				"Editor browser falling back to isolated in-memory CEF profile; ContextId=%s, GlobalProfile=%s, PIEInstanceIndex=%d, "
				"BrowserProfileIndex=%d",
				*ContextId,
				*GlobalProfilePath,
				PIEInstanceIndex,
				BrowserProfileIndex)
		}
#endif // WITH_EDITOR

#if USING_BUNDLED_CEF

#if WITH_EDITOR
		// Every editor browser receives the isolated, one-based context configured above.
		WebBrowserWidget = SNew(SWebBrowserView)
							   .InitialURL(InitialURL)
							   .SupportsTransparency(bSupportsTransparency)
							   .ContextSettings(BrowserContextSettings)
#else
		// The global packaged-game profile already persists Passport state and does not need the editor's per-PIE isolation machinery.
		WebBrowserWidget = SNew(SWebBrowser)
							   .InitialURL(InitialURL)
							   .ShowControls(false)
							   .SupportsTransparency(bSupportsTransparency)
							   .ShowInitialThrobber(true)
#endif // WITH_EDITOR

#if PLATFORM_ANDROID | PLATFORM_IOS
							   .OnLoadCompleted(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnLoadCompleted))
#endif
							   .OnConsoleMessage(BIND_UOBJECT_DELEGATE(FOnConsoleMessageDelegate, HandleOnConsoleMessage));

		return WebBrowserWidget.ToSharedRef();

#else
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)[SNew(STextBlock).Text(NSLOCTEXT("Immutable", "Immutable Web Browser", "Immutable Web Browser"))];
#endif // USING_BUNDLED_CEF
	}
}

#if PLATFORM_ANDROID | PLATFORM_IOS
void UImtblBrowserWidget::HandleOnLoadCompleted()
{
	FString indexUrl = "file:///immutable/index.html";

#if USING_BUNDLED_CEF
	if (WebBrowserWidget->GetUrl() == indexUrl)
	{
		IMTBL_LOG_FUNC("Mobile bridge loaded expected URL: %s", *indexUrl)
		JSConnector->SetMobileBridgeReady();
	}
	else
	{
		IMTBL_ERR("Immutable Browser Widget Url don't match: (loaded : %s, required: %s)", *WebBrowserWidget->GetUrl(), *indexUrl);
	}
#endif // USING_BUNDLED_CEF
}
#endif // PLATFORM_ANDROID | PLATFORM_IOS

void UImtblBrowserWidget::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	IMTBL_LOG_FUNC("Widget rebuilt; InitialURL=%s", *InitialURL)
	BindConnector();
	SetBrowserContent();
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
void UImtblBrowserWidget::HandleOnConsoleMessage(
	const FString& Message, const FString& Source, int32 Line, EWebBrowserConsoleLogSeverity Severity)
{
	const TCHAR* SeverityName = GetConsoleLogSeverityName(Severity);
	switch (Severity)
	{
		case EWebBrowserConsoleLogSeverity::Fatal:
			// Browser fatal severity describes the JavaScript console message; Unreal Fatal would terminate the process.
		case EWebBrowserConsoleLogSeverity::Error:
			IMTBL_ERR("Browser console message [%s]: %s, Source: %s, Line: %d", SeverityName, *Message, *Source, Line)
			break;
		case EWebBrowserConsoleLogSeverity::Warning:
			IMTBL_WARN("Browser console message [%s]: %s, Source: %s, Line: %d", SeverityName, *Message, *Source, Line)
			break;
		case EWebBrowserConsoleLogSeverity::Verbose:
		case EWebBrowserConsoleLogSeverity::Debug:
			IMTBL_VERBOSE("Browser console message [%s]: %s, Source: %s, Line: %d", SeverityName, *Message, *Source, Line)
			break;
		case EWebBrowserConsoleLogSeverity::Default:
		case EWebBrowserConsoleLogSeverity::Info:
		default:
			IMTBL_LOG("Browser console message [%s]: %s, Source: %s, Line: %d", SeverityName, *Message, *Source, Line)
			break;
	}
	OnConsoleMessage.Broadcast(Message, Source, Line);
}
#endif
