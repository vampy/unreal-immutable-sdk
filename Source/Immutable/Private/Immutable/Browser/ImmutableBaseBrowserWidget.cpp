#include "Immutable/Browser/ImmutableBaseBrowserWidget.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Immutable/Misc/ImtblLogging.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"

#if USING_BUNDLED_CEF
#include "SWebBrowser.h"
#include "SWebBrowserView.h"
#include "WebBrowserModule.h"
#if WITH_CEF3
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

#define LOCTEXT_NAMESPACE "BaseBrowserWidget"

namespace
{
#if USING_BUNDLED_CEF
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
#endif

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
} // namespace

void UImmutableBaseBrowserWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

#if USING_BUNDLED_CEF
	WebBrowserWidget.Reset();
#endif
}

bool UImmutableBaseBrowserWidget::IsPageLoaded() const
{
#if USING_BUNDLED_CEF
	return WebBrowserWidget.IsValid() && WebBrowserWidget->IsLoaded();
#else
	return false;
#endif
}

FString UImmutableBaseBrowserWidget::GetUrl() const
{
#if USING_BUNDLED_CEF
	if (WebBrowserWidget.IsValid())
	{
		return WebBrowserWidget->GetUrl();
	}
#endif
	return FString();
}

void UImmutableBaseBrowserWidget::LoadURL(FString NewURL) const
{
#if USING_BUNDLED_CEF
	if (WebBrowserWidget.IsValid())
	{
		return WebBrowserWidget->LoadURL(NewURL);
	}
#endif
}

void UImmutableBaseBrowserWidget::LoadString(FString Contents, FString DummyURL)
{
#if USING_BUNDLED_CEF
	if (WebBrowserWidget.IsValid())
	{
		WebBrowserWidget->LoadString(Contents, DummyURL);
	}
#endif
}

FImmutableBrowserConsoleMessageDynamicMulticastDelegate* UImmutableBaseBrowserWidget::DynamicMulticastDelegate_OnConsoleMessage()
{
	return &Internal_DynamicMulticastDelegate_OnConsoleMessage;
}

FSimpleMulticastDelegate* UImmutableBaseBrowserWidget::MulticastDelegate_OnLoadCompleted()
{
	return &Internal_MulticastDelegate_OnLoadCompleted;
}

FSimpleMulticastDelegate* UImmutableBaseBrowserWidget::MulticastDelegate_OnBrowserCreated()
{
	return &Internal_MulticastDelegate_OnBrowserCreated;
}

TSharedRef<SWidget> UImmutableBaseBrowserWidget::RebuildWidget()
{
	if (IsDesignTime())
	{
		// Show placeholder in editor
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("BrowserPlaceholder", "Browser Widget"))
			];
	}
	else
	{
#if USING_BUNDLED_CEF
		// Every PIE user receives a stable one-based context. Empty storage keeps stock Unreal contexts isolated in memory.
#if WITH_EDITOR
		const int32 PIEInstanceIndex = GetWorldUserIndex();
		const int32 BrowserProfileIndex = PIEInstanceIndex + 1;
		const FString ContextId = FString::Printf(TEXT("ImmutablePassportPIEUser%d"), BrowserProfileIndex);
		TOptional<FBrowserContextSettings> BrowserContextSettings;
		BrowserContextSettings.Emplace(ContextId);

		const FString GlobalProfilePath = GetGlobalCefProfilePath();
		const FString CEFRootPath = FPaths::GetPath(GlobalProfilePath);
		const bool bHasUsableRoot = IsExplicitCEFRootCachePathEnabled()
			&& FPaths::GetCleanFilename(GlobalProfilePath).Equals(TEXT("Default"), ESearchCase::IgnoreCase)
			&& !CEFRootPath.IsEmpty() && !FPaths::IsRelative(CEFRootPath);
		if (bHasUsableRoot)
		{
			BrowserContextSettings->CookieStorageLocation = FPaths::Combine(CEFRootPath, ContextId);
			IMTBL_LOG_FUNC(
				"Editor browser using persistent isolated CEF profile; ContextId=%s, GlobalProfile=%s, RequestedProfile=%s, "
				"CEFVersion=%hs, ChromeVersion=%d.%d.%d.%d, CEFBranch=%s, PIEInstanceIndex=%d, BrowserProfileIndex=%d",
				*ContextId, *GlobalProfilePath, *BrowserContextSettings->CookieStorageLocation, GetCompileTimeCefVersion(),
				GetCompileTimeChromeVersionMajor(), GetCompileTimeChromeVersionMinor(), GetCompileTimeChromeVersionBuild(),
				GetCompileTimeChromeVersionPatch(), GetCefBranchName(), PIEInstanceIndex, BrowserProfileIndex)
		}
		else
		{
			IMTBL_WARN_FUNC(
				"Editor browser using isolated in-memory CEF profile; ContextId=%s, GlobalProfile=%s, PIEInstanceIndex=%d, "
				"BrowserProfileIndex=%d",
				*ContextId, *GlobalProfilePath, PIEInstanceIndex, BrowserProfileIndex)
		}

		WebBrowserWidget = SNew(SWebBrowserView)
			.InitialURL(InitialURL)
			.SupportsTransparency(bSupportsTransparency)
			.ContextSettings(BrowserContextSettings)
			.OnConsoleMessage(BIND_UOBJECT_DELEGATE(FOnConsoleMessageDelegate, HandleOnConsoleMessage));
#else
		WebBrowserWidget = SNew(SWebBrowser)
			.InitialURL(InitialURL)
			.ShowControls(false)
			.SupportsTransparency(bSupportsTransparency)
			.ShowInitialThrobber(bShowInitialThrobber)
#if PLATFORM_ANDROID || PLATFORM_IOS
			.OnLoadCompleted(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnLoadCompleted))
#endif
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
			.OnConsoleMessage(BIND_UOBJECT_DELEGATE(FOnConsoleMessageDelegate, HandleOnConsoleMessage))
#endif
			;
#endif

		return WebBrowserWidget.ToSharedRef();
#else
		// Fallback for non-CEF builds
		return
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoCEF", "Browser Not Available"))
			];
#endif
	}
}

void UImmutableBaseBrowserWidget::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	OnBrowserCreated();
}

#if PLATFORM_ANDROID || PLATFORM_IOS
void UImmutableBaseBrowserWidget::HandleOnLoadCompleted()
{
	// Default mobile load handling
	HandleLoadCompleted();
}
#endif

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
void UImmutableBaseBrowserWidget::HandleOnConsoleMessage(const FString& Message, const FString& Source, int32 Line, EWebBrowserConsoleLogSeverity Severity)
{
	const TCHAR* SeverityName = GetConsoleLogSeverityName(Severity);
	switch (Severity)
	{
		case EWebBrowserConsoleLogSeverity::Fatal:
			// Browser Fatal describes JavaScript severity; Unreal Fatal would terminate the process.
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
	HandleConsoleMessage(Message, Source, Line, static_cast<int32>(Severity));
}
#endif

void UImmutableBaseBrowserWidget::HandleLoadCompleted()
{
	Internal_MulticastDelegate_OnLoadCompleted.Broadcast();
}

void UImmutableBaseBrowserWidget::HandleConsoleMessage(const FString& Message, const FString& Source, int32 Line, int32 Severity)
{
	Internal_DynamicMulticastDelegate_OnConsoleMessage.Broadcast(Message, Source, Line, Severity);
}

void UImmutableBaseBrowserWidget::OnBrowserCreated()
{
	Internal_MulticastDelegate_OnBrowserCreated.Broadcast();
}

int32 UImmutableBaseBrowserWidget::GetWorldUserIndex() const
{
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (const FWorldContext* WorldContext = GameInstance->GetWorldContext())
			{
				return WorldContext->PIEInstance == INDEX_NONE ? 0 : WorldContext->PIEInstance;
			}
		}
	}
#endif
	return 0;
}

#undef LOCTEXT_NAMESPACE
