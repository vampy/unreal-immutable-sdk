# Illuvium customizations to Immutable Unreal SDK 1.11.5

This branch (`illuvium`) is Illuvium's fork of the official Immutable Unreal SDK, tag `v1.11.5`
(commit `4973cee21bfaa53dadc9e4519d53a3ae2203bb29`). IlluviumGame ships it as `Plugins/Immutable`, and the
distributable files of both copies are byte-identical. This file lists every change the fork makes to
upstream, the upstream behavior it reviewed and left alone, and how to reproduce and verify the patch that
captures the changes.

Every claim in the customization sections was re-verified on 2026-10-26 by diffing the base commit against
the working tree, file by file.

## Contents

- [Custom modifications at a glance](#custom-modifications-at-a-glance)
- [Custom modifications in detail](#custom-modifications-in-detail)
  - [Editor browser profiles and CEF](#editor-browser-profiles-and-cef)
  - [Game bridge loading](#game-bridge-loading)
  - [Passport login, logout and responses](#passport-login-logout-and-responses)
  - [Windows login callback](#windows-login-callback)
  - [Blueprint async actions](#blueprint-async-actions)
  - [Build and plugin settings](#build-and-plugin-settings)
  - [Tests](#tests)
  - [Formatting-only differences](#formatting-only-differences)
- [Reviewed and not carried over](#reviewed-and-not-carried-over)
- [Known limitations](#known-limitations)
- [Source state and sync history](#source-state-and-sync-history)
- [Patch artifact](#patch-artifact)
  - [Changed files](#changed-files)
  - [Reproduce the patch](#reproduce-the-patch)
  - [Apply and compare at the exact base](#apply-and-compare-at-the-exact-base)
  - [Reproduction verification](#reproduction-verification)
- [Build and test record](#build-and-test-record)
- [Pending manual validation](#pending-manual-validation)

## Custom modifications at a glance

Origin says where a change comes from: carried over from the earlier SDK 1.9.0 fork as is (ported) or
re-implemented for the 1.11.5 code (adapted), new in the 1.11.5 port, or a later Plastic change mirrored here.

| ID | Modification | Origin | Main files |
| --- | --- | --- | --- |
| B1 | Separate browser profile per PIE player in editor builds | 1.9.0 fork, adapted | `ImmutableBaseBrowserWidget.cpp/.h` |
| B2 | Editor profiles persist on disk with the Illuvium engine, in memory otherwise | 1.9.0 fork, adapted | `ImmutableBaseBrowserWidget.cpp` |
| B3 | CEF, Chromium and profile diagnostics in editor builds | 1.9.0 fork, ported | `ImmutableBaseBrowserWidget.cpp` |
| B4 | CEF headers included behind Unreal's third-party and Windows type guards | 1.9.0 fork, ported | both browser widget `.cpp` files |
| B5 | Browser console messages mapped to matching Unreal log levels | 1.9.0 fork, ported | `ImmutableBaseBrowserWidget.cpp` |
| G1 | Bridge loading moved into the JS connector widget, with a Chromium 128+ binding shim | 1.9.0 fork, adapted | `ImmutableJSConnectorBrowserWidget.cpp/.h`, `ImtblBrowserUserWidget.cpp` |
| G2 | Unity-build-safe names for the JS connector's CEF helpers | Plastic `cs:41201` | `ImmutableJSConnectorBrowserWidget.cpp` |
| G3 | Bind diagnostics, and a fix for a null-pointer crash in `BindUObject` | 1.9.0 fork, ported | `ImmutableJSConnectorBrowserWidget.cpp` |
| G4 | No game bridge for a game instance without a viewport (headless PIE servers) | 1.9.0 fork, ported | `ImmutableSubsystem.cpp` |
| G5 | Warning when JavaScript is called before the bridge is ready | 1.9.0 fork, ported | `ImtblJSConnector.cpp` |
| P1 | Structured bridge errors preferred, and failed bridge requests logged as errors | 1.9.0 fork, ported | `ImmutablePassport.cpp` |
| P2 | Structured response errors reach callers; a missing `result` field is tolerated | 1.9.0 fork, adapted | `ImtblJSMessages.h`, `ImmutablePassport.cpp` |
| P3 | A failed login-URL request now answers the caller instead of hanging | New in 1.11.5 port | `ImmutablePassport.cpp` |
| P4 | Custom browser-launch delegate for Windows login and logout, and a Windows hard-logout fix | 1.9.0 fork, adapted | `ImmutablePassport.cpp/.h` |
| P5 | Silent login from a cached session (`Relogin`) | New in 1.11.5 port | `ImmutablePassport.cpp/.h`, `ImmutableNames.h` |
| P6 | Passport save slot per PIE player | 1.9.0 fork, ported | `ImmutablePassport.cpp/.h` |
| P7 | Guard against a failed save-object creation | 1.9.0 fork, adapted | `ImmutablePassport.cpp` |
| P8 | Passport compiles on Linux and other platforms without deep links | Plastic `cs:41201` | `ImmutablePassport.cpp` |
| W1 | Windows login callback launched through headless `conhost`, so no console window flashes | Plastic, 2026-09-08 | `ImmutablePKCEWindows.cpp` |
| A1 | Async actions check the context, world and game instance before using the subsystem | 1.9.0 fork, ported | `ImtblBlueprintAsyncAction.cpp/.h`, 13 action `.cpp` files |
| D1 | CEF3 build dependency on Win64, Mac and Linux | 1.9.0 fork, ported | `Immutable.Build.cs` |
| D2 | Explicit include-what-you-use and unity-build settings in both modules | 1.9.0 fork, ported | `Immutable.Build.cs`, `ImmutableEditor.Build.cs` |
| D3 | Plugin marked not installed | New in 1.11.5 port | `Immutable.uplugin` |
| D4 | Forward declarations and includes at their use sites | 1.9.0 fork, ported | several headers and `.cpp` files |
| T1 | `Immutable.Illuvium.ResponseHandling` test | New in 1.11.5 port | `ImtblMessagesTest.cpp` |
| T2 | `Immutable.Illuvium.ReloginBridgeContract` test | New in 1.11.5 port | `ImtblMessagesTest.cpp` |
| T3 | Upstream's stale `Immutable.JSMessages` expectations fixed | Plastic, 2026-10-06 | `ImtblMessagesTest.cpp` |

Paths are under `Source/Immutable/Private/Immutable/` or `Source/Immutable/Public/Immutable/` unless the
[changed-file inventory](#changed-files) says otherwise. `Web/index.js` is unchanged.

## Custom modifications in detail

### Editor browser profiles and CEF

**B1. Separate browser profile per PIE player.** In `WITH_EDITOR` builds `UImmutableBaseBrowserWidget::RebuildWidget`
creates an `SWebBrowserView` instead of upstream's `SWebBrowser`, because only `SWebBrowserView` accepts browser
context settings. Each widget uses the context `ImmutablePassportPIEUser<PIEInstance + 1>`, from `GetWorldUserIndex`;
a world outside PIE gets `ImmutablePassportPIEUser1`. The engine shares one context per ID, so the widgets of one PIE
player share cookies and different players stay apart. The switch depends on the compile flag, so `-game` runs of
editor binaries also use `ImmutablePassportPIEUser1`. Non-editor builds keep upstream's `SWebBrowser` with the same
arguments and the engine's global profile. The header declares `GetWorldUserIndex` and switches the `WebBrowserWidget`
type under `WITH_EDITOR`.

**B2. Persistent editor profiles.** The context gets a disk cookie path,
`<parent of the global profile>/ImmutablePassportPIEUser<N>`, only when all of these hold:

- `[Browser] bUseExplicitCEFRootCachePath=true` in the Engine config (`IsExplicitCEFRootCachePathEnabled`). The Illuvium
  engine reads this setting; stock UE 5.8 does not have it. IlluviumGame sets it in `Config/DefaultEngine.ini`.
- The global CEF profile folder (`GetGlobalCefProfilePath`) is named `Default`, case-insensitively.
- Its parent folder is an absolute path.

Otherwise the context has no cookie path: it lives in memory only, so editor logins do not survive an editor restart,
and a Warning is logged. The engine appends `_<Chrome build number>` to the cookie path, so the logged `RequestedProfile`
is the path before that suffix.

**B3. CEF, Chromium and profile diagnostics.** Helpers read the compile-time CEF version, Chrome
major/minor/build/patch, and CEF branch (experimental or legacy) from `CEF_VERSION`, `CHROME_VERSION_*` and
`CEF3_USE_EXPERIMENTAL_VERSION`. The persistent-profile branch of B2 logs them at Log level with the global profile,
requested profile and PIE and profile indices. The in-memory branch logs only the context ID, global profile and
indices, at Warning level. All of it is editor-only.

**B4. Safe CEF includes.** CEF headers (`cef_request_context.h`, `cef_version.h`) are included inside
`AllowWindowsPlatformTypes`, a saved and undefined `OVERRIDE` macro, and `THIRD_PARTY_INCLUDES_START/END`, guarded by
`USING_BUNDLED_CEF` and `WITH_CEF3`, in both browser widget `.cpp` files.

**B5. Browser console severity mapping.** Upstream `HandleOnConsoleMessage` logged every browser console message at Log
level, printing the severity as a number. Now browser Fatal and Error map to Unreal Error, Warning to Warning, Verbose and
Debug to Verbose, and Default, Info and anything else to Log. Each line carries a `[Severity]` tag, the source and the
line number. Browser Fatal never becomes Unreal Fatal, which would terminate the process. Browser Debug and Verbose
messages are hidden at `LogImmutable`'s default Log verbosity.

### Game bridge loading

**G1. Bridge loading and the Chromium 128+ binding shim.** `UImtblBrowserUserWidget` now calls
`UImmutableJSConnectorBrowserWidget::LoadGameBridge` instead of building the bridge page itself. `LoadGameBridge` builds
the same page and URL as upstream, logs an Error when `index.js` cannot be read (upstream was silent), and logs the load
configuration. When the compile-time Chrome major version is 128 or newer, it prepends `ImmutableCefBindingShim` to the
bridge script:

- The shim installs once, guarded by `window.__immutableUeConnectorShimInstalled`.
- It creates `window.ue` and a stand-in `ue.jsconnector` that queues `sendtogame` calls: at most 256, dropping the oldest
  with a `console.error`.
- It polls every 25 ms and flushes the queue to the real connector once Unreal binds it. After 200 attempts (5 s) it stops,
  with a `console.error` if anything is still queued.

The shim stops `index.js` throwing "Unreal JSConnector not defined" when it runs before Unreal binds the connector. With
the Illuvium engine's CEF 128 it is active on Win64, Mac and Linux; Android and iOS have no CEF and report Chrome version 0.

**G2. Unity-build-safe helper names.** The JS connector's anonymous-namespace helpers are named
`GetJSConnectorCompileTimeChromeVersionMajor` and `GetJSConnectorCompileTimeCefVersion`, so they no longer collide with
the base widget's helpers when `bUseUnity` puts both files into one translation unit.

**G3. Bind diagnostics and a crash fix.** Upstream `BindUObject` logged `Object->GetName()` inside the branch that runs
when `Object` is null, which crashes. The check is now two warnings: one for a missing browser widget, one for a null
object. `SetupJavaScriptBindings` logs when the connector is already bound, logs the bind result with the page-loaded
state, and logs an Error when `JSConnector` is null.

**G4. No game bridge without a viewport.** On non-BLUI builds `UImmutableSubsystem::SetupGameBridge` returns before
setting `bHasSetupGameBridge` when the world is null or has no game viewport. The viewport-created delegate is global, so
in multiplayer PIE it also fires for headless dedicated-server instances. The delegate stays bound, so a client instance
sets the bridge up later.

**G5. Early-call warning.** The commented-out warning in `UImtblJSConnector` for JavaScript calls made before the bridge
is ready is enabled again; the call is still queued.

### Passport login, logout and responses

**P1. Structured bridge errors and failure logs.** A file-local `GetResponseError` picks the error text in order: the
structured `FImtblResponseError`, then the JSON `error` string, then an action-specific fallback. The initialize, logout,
connect, `HasStoredCredentials` and generic bridge-callback handlers use it; upstream read only the JSON string in some of
them. A failed bridge request is logged at Error with its action, request ID and error, where upstream logged at Log level.
The missing-delegate warning includes the request ID.

**P2. Response parsing.** Upstream `FImtblJSResponse::FromJsonString` copied the response into its return value before
setting `Error`, so callers never saw structured errors. The fork returns the response after setting `Error`, and sets it
only when the response carries an `errorType` of 0 or more or a non-empty `error`. The `GetResponseResultAsString`,
`GetResponseResultAsBool` and `GetResponseResultAsStringArray` getters use `TryGet*`, so a missing `result` returns the
default without LogJson warnings, and the array getter skips non-string elements. Invalid JSON returning no response is
upstream behavior, not a change.

**P3. Failed login-URL request.** When the bridge's `getPKCEAuthUrl` call failed, upstream `OnGetAuthUrlResponse` only
logged it: the caller never got an answer and `IPS_CONNECTING | IPS_PKCE` stayed set. The fork logs an Error with the
action and request ID, answers the caller with the failure, unbinds the delegate and clears both flags.

**P4. Custom browser-launch delegate.** `FImtblPassportLaunchURLDelegate`, the public `SetCustomLaunchURLDelegate` and a
private `LaunchURL` let the game open Passport URLs itself; IlluviumGame uses it to open a browser profile per PIE window.
`LaunchURL` runs the delegate when bound, otherwise `FPlatformProcess::LaunchURL`. Only the Windows login and hard-logout
paths use it; Android, iOS and Mac keep their native launchers. The rewrite also fixes upstream's Windows hard logout,
which looked up its response delegate a second time after the first lookup had removed it. Every Windows hard logout
logged "Couldn't find delegate", and a browser launch failure then dereferenced an empty `TOptional`, a check failure.

**P5. Silent cached-session login.** `ImmutablePassportAction::Relogin` and `UImmutablePassport::Relogin` call the
`relogin` action that the bundled bridge already has (`login({ useCachedSession: true })`). It fails without calling the
bridge when Passport is not initialized. While pending it sets `IPS_CONNECTING`. `OnReloginResponse` clears that flag, and
on success sets `IPS_CONNECTED | IPS_PKCE` and saves the Passport state. It always tracks `COMPLETE_RELOGIN`. It never
opens a browser; the caller decides whether to fall back to an interactive login. A test-only friend,
`FImtblPassportReloginTestAccessor`, supports T2. This restores, as a thin wrapper, the relogin option that SDK 1.11.5
removed from `Connect`.

**P6. Passport save slot per PIE player.** `GetGameInstance` and `GetWorldUserIndex` give the PIE instance index in
editor builds and 0 elsewhere. Saving, checking and loading the `Immutable` save slot use that index; upstream always used
0. The first PIE window (instance 0) therefore shares its slot with non-PIE runs.

**P7. Save-object guard.** `SavePassportSettings` logs an Error and returns when `CreateSaveGameObject` fails; upstream
dereferenced the result unchecked.

**P8. Builds without deep links.** Upstream's desktop and mobile guard wrapped the bridge-callback handler, state flags,
save and load, and `OnDeepLinkActivated`, so Passport failed to compile on Linux. The guard now ends after the relogin
response. `OnDeepLinkActivated` stays declared everywhere (it is a `UFUNCTION`) with its body guarded, `CompleteLoginFlow`
has its own guard, and `HandleDeepLink` no longer leaves a function body at file scope on other platforms.

### Windows login callback

**W1. Headless callback launcher.** `UImmutablePKCEWindows::RegisterProtocolHandler` registers:

```text
conhost.exe --headless powershell.exe -ExecutionPolicy Bypass -NoProfile -File "<script path>" "%1"
```

The C++ format string writes `%%1` to emit Windows' `%1` placeholder. Both the script path and the callback URI are
quoted, and the URI stays script data after `-File`. Upstream launched `powershell.exe` directly, which flashes a
console window; `-WindowStyle Hidden` still flashed, because Windows can show the console before PowerShell reads the
option. The generated `PKCE.ps1`, the registry transport, the registry watcher and Passport's completion handling are
unchanged. Callback delivery and Passport's result decide success, because conhost does not reliably pass on the child's
exit code.

Compatibility is limited: conhost ships with Windows 7 and later, but that does not prove this command works. Windows 10
1809 is the documented ConPTY API baseline, not a guaranteed minimum for this launcher, and Microsoft treats direct
conhost invocation as unsupported. Older conhost versions had argument-quoting bugs. There is no version check or
fallback, and the oldest Windows build that runs the command reliably is unverified. IlluviumGame's
`Docs/Immutable/SDK1.11.5Integration.md` keeps the primary-source references.

### Blueprint async actions

**A1. Safe subsystem lookup.** Upstream `UImtblBlueprintAsyncAction::GetSubsystem` checked only the context and world,
then dereferenced the game instance, which can be null. The fork checks `IsValid(WorldContextObject)` (which also rejects
destroyed objects), the world and the game instance, and returns null when any is missing. Each of the 14 `Activate`
functions in 13 action files fetches the subsystem once, fails when it is null and calls `WhenReady` on it. The 13 files
are the Connection, ConnectEvm, GetEmail, GetLinkedAddresses, GetToken, HasStoredCredentials, Initialization, Logout,
ZkEvmGetBalance, ZkEvmGetTransactionReceipt, ZkEvmRequestAccounts, ZkEvmSendTransaction (two actions) and
ZkEvmSignTypedDataV4 actions. `UImtblEmbeddedLoginAsyncAction::Activate` does not use the subsystem and is unchanged.
The failure messages still say "missing world or world context object", although they now also cover a missing game
instance or subsystem. The header forward-declares `UImmutableSubsystem` instead of including it; the `.cpp` includes
it. `WorldContextObject` itself stays upstream's raw, non-`UPROPERTY` pointer (see
[reviewed and not carried over](#reviewed-and-not-carried-over)).

### Build and plugin settings

**D1. CEF3 dependency.** `Immutable.Build.cs` adds `AddEngineThirdPartyPrivateStaticDependencies(Target, "CEF3")` for
Win64, Mac and Linux, the same platforms as UE's WebBrowser module. It supplies the CEF include path and the
`CEF3_USE_EXPERIMENTAL_VERSION` definition that B3, B4 and `CefRequestContext::GetGlobalContext` need.

**D2. Explicit IWYU and unity settings.** Both `Immutable.Build.cs` and `ImmutableEditor.Build.cs` set
`IWYUSupport = IWYUSupport.Full` and `bUseUnity = true`. Both equal the Illuvium engine's defaults, so the only effect is
overriding a target that disables unity builds for these modules. Upstream 1.11.5 does not set them.

**D3. Plugin marked not installed.** `Immutable.uplugin` sets `"Installed": false`; upstream has `true`. It is new in the
1.11.5 port: the 1.9.0 fork and Plastic before `cs:41201` had `true`. No record says why.

**D4. Includes and forward declarations.** Added where the code above needs them: the subsystem forward declaration in
the async action header, `UImmutableAnalytics` in `ImmutablePassport.h`, game instance and world includes, the Slate
browser types in the base widget, and the CEF headers.

### Tests

All three live in `Private/Immutable/Tests/ImtblMessagesTest.cpp`.

- **T1. `Immutable.Illuvium.ResponseHandling`** parses a failed response with `errorType` 0 and `error` "Denied". It
  checks that the structured error survives as "AuthenticationError: Denied" and that the string, bool and array getters
  tolerate a missing `result`. On upstream code the structured-error check fails (P2).
- **T2. `Immutable.Illuvium.ReloginBridgeContract`** (Android, iOS, Mac and Windows) checks that the bundled
  `Web/index.js` still has the `relogin` action with `useCachedSession`, that `Relogin` fails without a bridge call
  before initialization, and that afterwards it sends `relogin`, sets `IPS_CONNECTING`, and on a failure reply answers
  the caller and clears the flag. It does not cover the success path.
- **T3. `Immutable.JSMessages`** is upstream's test with fixed expectations. Upstream built
  `FImmutablePassportInitData { ClientId, RedirectUri, Environment }` positionally, which put the environment into
  `logoutRedirectUri`, and its expected JSON lacked `logoutRedirectUri`, `isSilentLogout`, `engineSdkVersion` and
  `deviceModel`. The fork passes an empty `logoutRedirectUri` and expects all four fields, so the test passes.

### Formatting-only differences

These change no behavior:

- Every changed file gains a final newline, which upstream lacked.
- `Immutable.uplugin` now mixes line endings: upstream is all CRLF, but the edited lines 17 and 41 end in LF.
- `ImmutableEditor.Build.cs` indents its two added lines with tabs, in a file indented with spaces.
- Re-indented code: the `SWebBrowser` builder in the base widget, the Windows launch block in `OnGetAuthUrlResponse`
  (P3), and the re-wrapped `CreateAndDispatchWhenReady` arguments in `OnDeepLinkActivated`.
- `IsPageLoaded` and `BindUObject` use `#else` instead of an unreachable `return false;` after the bundled-CEF branch.
- `ImmutableJSConnectorBrowserWidget.cpp` closes its namespace with `}<tab> // namespace`.

## Reviewed and not carried over

The 1.9.0 fork's customizations were reviewed against 1.11.5. These were not carried over:

| Item | Result | Evidence or reason |
| --- | --- | --- |
| Async action world context kept alive for garbage collection | Dropped at user request | Reverted in `707cb3f`. `WorldContextObject` is upstream's raw, non-`UPROPERTY` `UObject*`; A1's checks remain. |
| IMX async actions (`ConnectImx`, `ImxTransfer`, `ImxBatchNftTransfer`, `ImxRegisterOffchain`, `ImxIsRegisteredOffchain`) | Obsolete | 1.11.5 has no IMX code; the direct, embedded, PKCE and zkEVM flows replace them. |
| Legacy desktop device-code flow | Obsolete | Absent from 1.11.5. Only the relogin option returns, as P5. |
| Passport state initial values | Already upstream | `StateFlags = IPS_NONE` in `ImmutablePassport.h`; `bWasConnectedViaPKCEFlow = false` in `ImmutableSaveGame.cpp`. |
| Save-slot existence check and validated load | Already upstream | `LoadPassportSettings` already checks `DoesSaveGameExist` and casts the loaded object; P6 and P7 add the rest. |
| Bundled CEF selection | Already upstream | `Immutable.Build.cs` already sets `USING_BUNDLED_CEF=1` and `USING_BLUI_CEF=0` for UE 5.1+. |
| Packaged-game global browser profile | Already upstream | Non-editor builds keep upstream's `SWebBrowser` and arguments (B1). |
| Empty analytics JSON payload | Already upstream | `ImmutableAnalytics.cpp` already sends `{}`. |
| Automation flag spelling | Already upstream | The UE-version check for `EAutomationTestFlags_ApplicationContextMask` is upstream; the new tests reuse it. |
| `LogImmutable` maximum verbosity `All` | Already upstream | `ImtblLogging.h` is unchanged. |
| Initial browser throbber | Dropped | The 1.9.0 fork enabled it; 1.11.5's browser is a hidden bridge widget and upstream defaults it off. |
| Extra save and load trace logs, restored comments | Dropped | No behavior change. P7's error log is the only new save log. |
| Hard-coded `webcache_4430`, BLUI-era code, `version.txt` | Obsolete | B2's explicit-root discovery replaces the path; the removed browser files were not restored. |
| Login callback routing for several PIE windows | Dropped, 2026-10-26 | Written and tested, then dropped to keep the diff small; see [known limitations](#known-limitations). |

## Known limitations

An editor process completes only one browser login at a time. Every login or logout in
`UImmutablePKCEWindows::Initialise` stops the protocol's registry watcher and starts one for itself, and when a
finished login's `UImmutablePKCEData` is garbage collected, its `BeginDestroy` stops the watcher again. When two PIE windows
need a browser login together, as at PIE start with no saved sessions, one of them is left without a listener and stays
on its loading screen. IlluviumGame's `Docs/Immutable/SDK1.11.5Integration.md`, section "Logging in from several PIE
windows", describes the failure and the workaround: log in with one PIE player first, then add one player per run.

## Source state and sync history

| Item | Value |
| --- | --- |
| Branch | `illuvium` |
| Official remote | `upstream` = `git@github.com:immutable/unreal-immutable-sdk.git` |
| Base commit and tag | `4973cee21bfaa53dadc9e4519d53a3ae2203bb29`, `v1.11.5` |
| Version marker | `#define ENGINE_SDK_VERSION TEXT("1.11.5")` in `Source/Immutable/Public/Immutable/ImmutableDataTypes.h` |
| Git commit | The commit that adds this revision of this file, on top of `7d04bb17c625f99c78ff4d87169ff75ca01885af` |
| Source endpoint tree | `5bf1b2e3c68eacc0a474ca87823f7e3b881d6248` (excludes this file and the patch) |
| Plastic source | IlluviumGame `/main` at `cs:41970`; plugin source unchanged there, this file and the patch updated |

The port is a clean re-implementation on the official 1.11.5 source. The historical branch
`reconcile/plastic-customizations-1.9.0` (commit `48c6f55ad12d201402598d122cb93195cd4f4702`) served only as migration
evidence; its patch was not applied, and no 1.9.0 file replaced a 1.11.5 file.

1. **2026-09-02 to 09-03, original integration.** Ported the customizations (`60128d0`, `a962022`), reverted the retained
   world context (`707cb3f`), and mirrored the two Plastic `cs:41201` compile fixes byte-for-byte (`5b5b28d`): G2 in
   `ImmutableJSConnectorBrowserWidget.cpp` and P8 in `ImmutablePassport.cpp`. Restored silent relogin with T2 (`7eb5b5b`),
   matching the Plastic `/main/UpdateImmutable_v1.11.5` endpoint `cs:41211`: 31 files, 839 insertions, 159 deletions.
2. **2026-09-08, headless callback launcher.** W1 was written in Plastic over `/main` `cs:41259`, mirrored here, and
   committed as `7d04bb1`, source tree `8762e9ca32fa2da840ea826fab9697ff821d76ea`: 32 files, 865 insertions, 160 deletions.
3. **2026-10-26, test fix.** A file-by-file comparison against Plastic `/main` `cs:41970` found one difference: T3, made
   in Plastic on 2026-10-06. It was copied byte-for-byte and committed with this revision of this file. All 114
   distributable files of the two copies now match by SHA-256.

The parity check excludes `.git/`, `.plastic/`, generated `Binaries/`, `Intermediate/`, `Saved/` and `DerivedDataCache/`,
and the ten unchanged upstream `.github/` CI and template files, which exist only in Git. No push was performed.

## Patch artifact

| Item | Value |
| --- | --- |
| File | `illuvium-1.11.5-snapshot-customizations.patch` |
| Base commit | `4973cee21bfaa53dadc9e4519d53a3ae2203bb29` |
| Source endpoint tree | `5bf1b2e3c68eacc0a474ca87823f7e3b881d6248` |
| Size | 82,463 bytes |
| SHA-256 | `d5671422aa8b598fe4db8e06808002d1d5b16b51f39e83c51d1d3696cea09520` |
| Changed files | 32 modified, none added, deleted or renamed |
| Insertions / deletions | 873 / 164 |

The patch covers every customization above and excludes this file and the patch itself.

### Changed files

| Path | Insertions | Deletions | Modifications |
| --- | ---: | ---: | --- |
| `Immutable.uplugin` | 2 | 2 | D3 |
| `Source/Immutable/Immutable.Build.cs` | 9 | 1 | D1, D2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblBlueprintAsyncAction.cpp` | 7 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblConnectionAsyncAction.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblEmbeddedLoginAsyncAction.cpp` | 1 | 1 | Final newline only |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportConnectEvmAsyncAction.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportGetEmailAsyncAction.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportGetLinkedAddressesAsyncAction.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportGetTokenAsyncAction.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportHasStoredCredentialsAsyncAction.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportInitializationAsyncAction.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportLogoutAsyncAction.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmGetBalanceAsyncAction.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmGetTransactionReceiptAA.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmRequestAccountsAsyncAction.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmSendTransactionAsyncAction.cpp` | 6 | 4 | A1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmSignTypedDataV4AsyncAction.cpp` | 3 | 2 | A1 |
| `Source/Immutable/Private/Immutable/Browser/ImmutableBaseBrowserWidget.cpp` | 219 | 9 | B1-B5, D4 |
| `Source/Immutable/Private/Immutable/Browser/ImmutableJSConnectorBrowserWidget.cpp` | 144 | 5 | B4, G1-G3 |
| `Source/Immutable/Private/Immutable/ImmutablePassport.cpp` | 193 | 85 | P1-P8 |
| `Source/Immutable/Private/Immutable/ImmutableSubsystem.cpp` | 13 | 1 | G4 |
| `Source/Immutable/Private/Immutable/ImtblBrowserUserWidget.cpp` | 2 | 8 | G1 |
| `Source/Immutable/Private/Immutable/ImtblJSConnector.cpp` | 2 | 2 | G5 |
| `Source/Immutable/Private/Immutable/Tests/ImtblMessagesTest.cpp` | 155 | 4 | T1-T3 |
| `Source/Immutable/Private/Immutable/Windows/ImmutablePKCEWindows.cpp` | 26 | 1 | W1 |
| `Source/Immutable/Public/Immutable/Actions/ImtblBlueprintAsyncAction.h` | 2 | 1 | A1 |
| `Source/Immutable/Public/Immutable/Browser/ImmutableBaseBrowserWidget.h` | 9 | 1 | B1 |
| `Source/Immutable/Public/Immutable/Browser/ImmutableJSConnectorBrowserWidget.h` | 2 | 1 | G1 |
| `Source/Immutable/Public/Immutable/ImmutableNames.h` | 2 | 1 | P5 |
| `Source/Immutable/Public/Immutable/ImmutablePassport.h` | 32 | 2 | P4-P6, D4 |
| `Source/Immutable/Public/Immutable/ImtblJSMessages.h` | 8 | 8 | P2 |
| `Source/ImmutableEditor/ImmutableEditor.Build.cs` | 3 | 1 | D2 |

### Reproduce the patch

Run in PowerShell 7 from this checkout. It rebuilds the source tree from the working files in a temporary index, without
touching the main index or creating a commit; `add -u` takes the complete tracked source state. `core.autocrlf=input`
matches this checkout's clean filter.

```powershell
$PreviousIndex = $env:GIT_INDEX_FILE
$SyncIndex = Join-Path $env:TEMP ([guid]::NewGuid().ToString() + '.index')
try {
  $env:GIT_INDEX_FILE = $SyncIndex
  git read-tree HEAD
  if ($LASTEXITCODE -ne 0) { throw 'read-tree failed' }
  git -c core.autocrlf=input -c core.filemode=false add -u -- .
  if ($LASTEXITCODE -ne 0) { throw 'index source failed' }
  git update-index --force-remove -- CUSTOM_MODIFICATIONS.md illuvium-1.11.5-snapshot-customizations.patch
  if ($LASTEXITCODE -ne 0) { throw 'exclude audits failed' }
  $SourceTree = git write-tree
  if ($LASTEXITCODE -ne 0) { throw 'write-tree failed' }
  if ($SourceTree -ne '5bf1b2e3c68eacc0a474ca87823f7e3b881d6248') { throw 'Source endpoint differs' }
} finally {
  $env:GIT_INDEX_FILE = $PreviousIndex
  Remove-Item -LiteralPath $SyncIndex -ErrorAction SilentlyContinue
}

git -c diff.algorithm=myers diff --no-ext-diff --no-textconv --binary --full-index --find-renames `
  --src-prefix=a/ --dst-prefix=b/ --no-color `
  --output=illuvium-1.11.5-snapshot-customizations.patch `
  4973cee21bfaa53dadc9e4519d53a3ae2203bb29 $SourceTree -- . `
  ':(exclude)CUSTOM_MODIFICATIONS.md' `
  ':(exclude)illuvium-1.11.5-snapshot-customizations.patch'
if ($LASTEXITCODE -ne 0) { throw 'patch generation failed' }
$Hash = (Get-FileHash .\illuvium-1.11.5-snapshot-customizations.patch -Algorithm SHA256).Hash.ToLowerInvariant()
if ($Hash -ne 'd5671422aa8b598fe4db8e06808002d1d5b16b51f39e83c51d1d3696cea09520') { throw 'Patch hash differs' }
```

### Apply and compare at the exact base

The disposable checkout is created inside `.git`, outside the source payload. Every command must succeed before the
next one runs.

```powershell
$PatchPath = (Resolve-Path '.\illuvium-1.11.5-snapshot-customizations.patch').Path
$ApplyTree = Join-Path (Resolve-Path '.git').Path ('patch-check-' + [guid]::NewGuid())
git -c core.autocrlf=false worktree add --detach $ApplyTree 4973cee21bfaa53dadc9e4519d53a3ae2203bb29
if ($LASTEXITCODE -ne 0) { throw 'base checkout failed' }
git -C $ApplyTree -c core.autocrlf=false apply --binary --check $PatchPath
if ($LASTEXITCODE -ne 0) { throw 'apply check failed' }
git -C $ApplyTree -c core.autocrlf=false apply --binary $PatchPath
if ($LASTEXITCODE -ne 0) { throw 'apply failed' }
git -C $ApplyTree -c core.autocrlf=false add -A
if ($LASTEXITCODE -ne 0) { throw 'index applied result failed' }
$AppliedTree = git -C $ApplyTree write-tree
if ($LASTEXITCODE -ne 0) { throw 'applied write-tree failed' }
if ($AppliedTree -ne '5bf1b2e3c68eacc0a474ca87823f7e3b881d6248') { throw 'Applied source differs' }
# Remove only this newly created disposable checkout after a successful comparison.
git worktree remove --force $ApplyTree
```

### Reproduction verification

On 2026-10-26 the patch was regenerated with the commands above. `apply --check`, application and the tree comparison
passed in a disposable checkout at the base: the applied tree was `5bf1b2e3c68eacc0a474ca87823f7e3b881d6248`, matching
the source endpoint across all 122 tracked paths, modes and blobs. Earlier endpoints were
`e0389a693f2b8a186b0d24dedd24737916aa30fd` (original integration) and `8762e9ca32fa2da840ea826fab9697ff821d76ea`
(2026-09-08).

No file mode changes from upstream: `.github/scripts/check_team_membership.sh` keeps `100755` and the other 121 files
are `100644`. Windows file systems cannot represent the executable bit, so Git trees verify modes. The ten `.github/`
files take part in patch reproduction but not in the 114-file parity check.

Verification uses `core.autocrlf=false` to keep exact blobs. The patch carries ten CRLF lines from existing diff content;
do not normalize it as ordinary text, which changes its hash and can change its payload. This file uses LF.

## Build and test record

**2026-10-26, IlluviumGame at `cs:41970`.** The Win64 Development Editor and Development Game targets built, and a
headless `StartsWith:Immutable` automation run passed all 7 tests: T1, T2, T3 and the four `Immutable.Illuvium.Transactions`
tests of IlluviumGame's zkEVM helpers. T2's expected failure path logs the only warnings.

**2026-09-03 and 2026-09-08, IlluviumGame.** The `cs:41211` endpoint built as Win64 Development Editor, Win64 Shipping
Game and Linux Development Server; T1 and T2 passed. After W1, the Win64 Development Editor and Shipping Game builds
passed. Compiled-editor checks drove the real protocol registration and the generated script with synthetic login and
logout URLs, with normal and space-containing script paths: all four passed with no visible callback window, sampled
about every 5 ms. The earlier direct PowerShell launcher delivered the callback but showed a window. The user then
confirmed login and logout worked without flashing. These checks cannot rule out flashes shorter than the sampling
interval.

**2026-09-02 to 09-03, standalone checks with UE 5.8.2 (`5.8.2-56702186`).** A standalone `BuildPlugin` failed in UHT
before compiling C++: UE 5.8 treated the packaged plugin as an engine module and required Blueprint categories on many
unchanged upstream declarations, mainly in `ImmutableDataTypes.h` and `ImmutableRequests.h`. That is an upstream 1.11.5
packaging issue the port does not fix. A disposable host project then passed these builds:

- Win64 Development game target.
- Win64 Development editor target with forced unity and adaptive unity off, which confirms G2 removes the unity collision.
- Linux x86_64 Development game target with Clang 20.1.8, which confirms P8.

```powershell
& 'D:\unreal\UE_5.8\Engine\Build\BatchFiles\Build.bat' HostProjectEditor Win64 Development `
  -Project='C:\Users\vampy\AppData\Local\Temp\i115h\HostProject.uproject' `
  -WaitMutex -NoEngineChanges -NoHotReloadFromIDE -ForceUnity -DisableAdaptiveUnity

$env:LINUX_MULTIARCH_ROOT = 'D:\unreal\Linux_CrossCompileToolChain\v26_clang-20.1.8-rockylinux8\'
& 'D:\unreal\UE_5.8\Engine\Build\BatchFiles\Build.bat' HostProject Linux Development `
  -Project='C:\Users\vampy\AppData\Local\Temp\i115h\HostProject.uproject' `
  -WaitMutex -NoEngineChanges -NoHotReloadFromIDE -ForceUnity -DisableAdaptiveUnity
```

In that host, T1 passed and upstream's `Immutable.JSMessages` failed on its stale expectations, which T3 has since fixed.

## Pending manual validation

These need their target environments and have not been run:

- Silent relogin after an editor restart, and the interactive fallback after an unusable cached session.
- The several-PIE-window login workaround in [known limitations](#known-limitations).
- CEF profile persistence with the Illuvium engine (B2), and the in-memory fallback on a stock engine.
- Browser console severity mapping (B5) with live messages.
- The no-viewport bridge guard (G4) in headless multiplayer PIE, and on Android and iOS, where setup runs once at engine
  start with no retry.
- Native Android, iOS and Mac login paths.
- W1 on the oldest Windows build the game supports, including installation paths with spaces.
- Packaged-game login and logout; no package or build identifier has been recorded.
