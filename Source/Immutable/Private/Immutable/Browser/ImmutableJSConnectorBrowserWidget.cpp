#include "Immutable/Browser/ImmutableJSConnectorBrowserWidget.h"

#include "Immutable/ImtblJSConnector.h"
#include "Immutable/ImmutableUtilities.h"
#include "Immutable/Misc/ImtblLogging.h"

#if USING_BUNDLED_CEF && WITH_CEF3
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif
#pragma push_macro("OVERRIDE")
#undef OVERRIDE
THIRD_PARTY_INCLUDES_START
#include "include/cef_version.h"
THIRD_PARTY_INCLUDES_END
#pragma pop_macro("OVERRIDE")
#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
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
	var maxQueuedMessages = 256;
	var proxyConnector = {
		sendtogame: function(message) {
			if (queuedMessages.length >= maxQueuedMessages) {
				queuedMessages.shift();
				console.error("[ImmutableBridgeDebug] Unreal JSConnector queue full; dropped oldest callback");
			}
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

int32 GetCompileTimeChromeVersionMajor()
{
#if WITH_CEF3 && defined(CHROME_VERSION_MAJOR)
	return CHROME_VERSION_MAJOR;
#else
	return 0;
#endif
}

const ANSICHAR* GetCompileTimeCefVersion()
{
#if WITH_CEF3 && defined(CEF_VERSION)
	return CEF_VERSION;
#else
	return "unknown";
#endif
}
} // namespace

void UImmutableJSConnectorBrowserWidget::PostInitProperties()
{
	Super::PostInitProperties();

	if (!IsTemplate())
	{
		JSConnector = NewObject<UImtblJSConnector>(this);
		JSConnector->ExecuteJs.BindUObject(this, &ThisClass::ExecuteJavaScript);
	}
}

UImtblJSConnector* UImmutableJSConnectorBrowserWidget::GetJSConnector() const
{
	return JSConnector;
}

void UImmutableJSConnectorBrowserWidget::LoadGameBridge()
{
#if USING_BUNDLED_CEF
	FString JavaScript;
	if (!FImmutableUtilities::LoadGameBridge(JavaScript))
	{
		IMTBL_ERR_FUNC("Failed to load Immutable game bridge JavaScript")
		return;
	}

	const bool bUseBindingShim = GetCompileTimeChromeVersionMajor() >= 128;
	const FString IndexHtml = FString(
		"<!doctype html><html lang='en'><head><meta charset='utf-8'><title>GameSDK Bridge</title><script>")
		+ (bUseBindingShim ? FString(ImmutableCefBindingShim) : FString()) + JavaScript
		+ FString("</script></head><body><h1>Bridge Running</h1></body></html>");
	IMTBL_LOG_FUNC("Loading Immutable bridge; Url=%s, CEFVersion=%hs, ChromeMajor=%d, BindingShim=%d, JavaScriptLength=%d",
		ImmutableBridgeUrl, GetCompileTimeCefVersion(), GetCompileTimeChromeVersionMajor(), bUseBindingShim, JavaScript.Len())
	LoadString(IndexHtml, ImmutableBridgeUrl);
#endif
}

void UImmutableJSConnectorBrowserWidget::ExecuteJavaScript(const FString& ScriptText) const
{
#if USING_BUNDLED_CEF
	if (WebBrowserWidget.IsValid())
	{
		WebBrowserWidget->ExecuteJavascript(ScriptText);
	}
#endif
}

bool UImmutableJSConnectorBrowserWidget::BindUObject(const FString& Name, UObject* Object, bool bIsPermanent) const
{
#if USING_BUNDLED_CEF
	if (!WebBrowserWidget.IsValid())
	{
		IMTBL_WARN("Could not bind UObject because WebBrowserWidget is null")
		return false;
	}
	if (!Object)
	{
		IMTBL_WARN("Could not bind null UObject to browser")
		return false;
	}

	WebBrowserWidget->BindUObject(Name, Object, bIsPermanent);
	return true;
#else
	return false;
#endif
}

void UImmutableJSConnectorBrowserWidget::OnBrowserCreated()
{
	Super::OnBrowserCreated();

	SetupJavaScriptBindings();
}

void UImmutableJSConnectorBrowserWidget::SetupJavaScriptBindings()
{
	if (JSConnector && JSConnector->IsBound())
	{
		IMTBL_LOG_FUNC("JSConnector already bound; PageLoaded=%d", IsPageLoaded())
		return;
	}

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
