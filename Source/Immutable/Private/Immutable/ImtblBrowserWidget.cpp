// Fill out your copyright notice in the Description page of Project Settings.

#include "ImtblBrowserWidget.h"

#include "Immutable/Misc/ImtblLogging.h"
#include "Immutable/ImtblJSConnector.h"
#if USING_BUNDLED_CEF
#include "SWebBrowser.h"
#include "WebBrowserModule.h"
#endif
#include "Immutable/Assets/ImtblSDKResource.h"
#include "Immutable/ImmutableSubsystem.h"
#include "Interfaces/IPluginManager.h"

UImtblBrowserWidget::UImtblBrowserWidget()
{
	IMTBL_LOG_FUNCSIG

	JSConnector = NewObject<UImtblJSConnector>(this, "JSConnector");
	JSConnector->ExecuteJs = UImtblJSConnector::FOnExecuteJsDelegate::CreateUObject(this, &UImtblBrowserWidget::ExecuteJS);

	// WebBrowserWidget->LoadString("<html><head><title>Test</title></head><body><h1>Test</h1></body></html>",
	// TEXT("http://www.google.com")); InitialURL = TEXT("http://www.google.com");
	// InitialURL = TEXT("chrome://version");
	// IPluginManager& PluginManager = IPluginManager::Get();
	// if (const TSharedPtr<IPlugin> Plugin =
	// PluginManager.FindPlugin("Immutable"))
	// {
	//     InitialURL = FString::Printf(TEXT("%s%s"), TEXT("file:///"),
	//     *FPaths::ConvertRelativePathToFull(FPaths::Combine(Plugin->GetContentDir(),
	//     TEXT("index.html")))); IMTBL_LOG("Loading initial url: %s",
	//     *InitialURL)
	// }
	InitialURL = TEXT("about:blank");
}

void UImtblBrowserWidget::BindConnector()
{
	if (JSConnector && JSConnector->IsBound())
	{
		return;
	}

	IMTBL_LOG("Setting up %s...", *UImtblJSConnector::StaticClass()->GetName())

	if (JSConnector)
	{
		if (BindUObject(UImtblJSConnector::JSObjectName(), JSConnector))
		{
			JSConnector->Init(IsPageLoaded());
		}
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
		const FWorldContext* WorldContext = GetGameInstance()->GetWorldContext();
		const int32 InstanceID = WorldContext ? WorldContext->PIEInstance : INDEX_NONE;
		return InstanceID == INDEX_NONE ? 0 : InstanceID;
	}

	return 0;
}

void UImtblBrowserWidget::SetBrowserContent()
{
#if USING_BUNDLED_CEF
	FSoftObjectPath AssetRef(TEXT("/Script/Immutable.ImtblSDKResource'/Immutable/" "PackagedResources/index.index'"));
	if (UObject* LoadedAsset = AssetRef.TryLoad())
	{
		if (auto Resource = Cast<UImtblSDKResource>(LoadedAsset))
		{
			if (!WebBrowserWidget.IsValid())
			{
				IMTBL_ERR("no browser")
				return;
			}

			const FString IndexHtml = FString("<!doctype html><html lang='en'><head><meta " "charset='utf-8'><title>GameSDK Bridge</title><script>") + Resource->Js + FString("</script></head><body><h1>Bridge Running</h1></body></html>");

			// IMTBL_LOG("Loaded resource: %s", *Resource->GetName())
			WebBrowserWidget->LoadString(IndexHtml, TEXT("file:///immutable/index.html"));
			// WebBrowserWidget->LoadURL(FString::Printf(TEXT("%s%s"),
			// TEXT("file:///"),
			// *FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(),
			// TEXT("html"), TEXT("index.html")))));
		}
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
		return SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)[SNew(STextBlock).Text(NSLOCTEXT("Immutable", "Immutable Web Browser", "Immutable Web Browser"))];
	}
	else
	{
		FBrowserContextSettings BrowserContextSettings{FGuid::NewGuid().ToString()};

		// Set per PIE window web cache
		if (WITH_EDITOR)
		{
			// BrowserContextSettings.bPersistSessionCookies = false;
			const IWebBrowserSingleton* WebBrowserSingleton = IWebBrowserModule::Get().GetSingleton();

			// TODO: We can't make a custom name because GenerateWebCacheFolderName is ran on the CookieStorageLocation
			// The cache directory must always live under the Saved/webcache_<CHROME_VERSION_BUILD>
			const FString DirectoryName = FString::Printf(TEXT("webcache_4430/immutable_%d"), GetWorldUserIndex());
			const FString RelativeCachePath(FPaths::Combine(WebBrowserSingleton->ApplicationCacheDir(), DirectoryName));
			BrowserContextSettings.CookieStorageLocation = FPaths::ConvertRelativePathToFull(RelativeCachePath);
			IMTBL_LOG("CookieStorageLocation: %s", *BrowserContextSettings.CookieStorageLocation);
		}

#if USING_BUNDLED_CEF
		WebBrowserWidget = SNew(SWebBrowserView).InitialURL(InitialURL).SupportsTransparency(bSupportsTransparency)
#if PLATFORM_ANDROID | PLATFORM_IOS
            .OnLoadCompleted(
                BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnLoadCompleted))
#endif
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
			.OnConsoleMessage(BIND_UOBJECT_DELEGATE(FOnConsoleMessageDelegate, HandleOnConsoleMessage))
			.ContextSettings(BrowserContextSettings)
#endif
			;

		return WebBrowserWidget.ToSharedRef();
#else
    return SNew(SBox)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)[SNew(STextBlock)
                                   .Text(NSLOCTEXT("Immutable",
                                                   "Immutable Web Browser",
                                                   "Immutable Web Browser"))];
#endif
	}
}

#if PLATFORM_ANDROID | PLATFORM_IOS
void UImtblBrowserWidget::HandleOnLoadCompleted() {
#if PLATFORM_ANDROID
  FString indexUrl = "file://immutable/index.html";
#else
  FString indexUrl = "file:///index.html";
#endif

#if USING_BUNDLED_CEF
  if (WebBrowserWidget->GetUrl() == indexUrl) {
    JSConnector->SetMobileBridgeReady();
  }
#endif
}
#endif

void UImtblBrowserWidget::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	BindConnector();
	SetBrowserContent();
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
void UImtblBrowserWidget::HandleOnConsoleMessage(const FString& Message, const FString& Source, int32 Line, EWebBrowserConsoleLogSeverity Severity)
{
	// TODO: add severity to log and callback
	IMTBL_LOG("Browser console message: %s, Source: %s, Line: %d", *Message, *Source, Line);
	OnConsoleMessage.Broadcast(Message, Source, Line);
}
#endif
