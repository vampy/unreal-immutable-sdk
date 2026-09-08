# Illuvium customizations to Immutable Unreal SDK 1.11.5

## Pinned source state

This branch is a clean semantic port to the official Immutable SDK 1.11.5 source. The historical
`reconcile/plastic-customizations-1.9.0` branch and commit
`48c6f55ad12d201402598d122cb93195cd4f4702` were used only as migration evidence; they are not the
current baseline and their patch was not applied.

| Item | Value |
| --- | --- |
| Branch | `illuvium` |
| Configured official remote | `upstream` = `git@github.com:immutable/unreal-immutable-sdk.git` |
| Canonical official URL | `https://github.com/immutable/unreal-immutable-sdk.git` |
| Pinned fetched `upstream/main` | `4973cee21bfaa53dadc9e4519d53a3ae2203bb29` |
| Matching official tag | `v1.11.5` |
| Original integration implementation commit | `7eb5b5b32704267692b0d668d2ef2fddde217f57` |
| Original Plastic integration source | `/main/UpdateImmutable_v1.11.5` at `cs:41211` |
| Original Plastic extraction state | Plugin scope clean; selected changeset and branch head both `cs:41211` |
| Plugin installation state | `Immutable.uplugin` has `"Installed": false` |

The authoritative marker at the pinned base, original implementation commit, and final local source tree is
`Source/Immutable/Public/Immutable/ImmutableDataTypes.h`:

```cpp
#define ENGINE_SDK_VERSION TEXT("1.11.5")
```

The original integration was a 31-file net delta with 839 insertions and 159 deletions.
The final local source is a 32-file net delta with 865 insertions and 160 deletions. No SDK 1.9.0
file was copied over a 1.11.5 file, no upstream-removed file was restored, and the old 1.9.0 patch
is absent.

After the 1.11.5 tree was synchronized to Plastic, two additional compile fixes were checked in at
`cs:41201`. A complete distributable-file comparison against Git commit
`49821c7d04a9aeff8975fbf28b85b6fa662a97f6` found no missing
files and only these two source differences. Their pinned Plastic SHA-256 values are:

- `ImmutableJSConnectorBrowserWidget.cpp`: `58477e214059cc41619af90ee471da0ea11d2910bf4ce05a9478f5f9a8cf8cdc`
- `ImmutablePassport.cpp`: `ac19468c16ddca5bcdb3812aad8e539093aed0abd29b5ea17c7c31138e1f36fb`

Those two files at compile-fix commit `5b5b28d2701e7fc9446dabeda672284b8d9b302f` matched those Plastic revisions
byte-for-byte. The later relogin change modifies `ImmutablePassport.cpp` again.

Plastic `cs:41211` then restored silent cached-session relogin and added its focused bridge-contract
test. The four resulting source/test files match the clean Plastic endpoint byte-for-byte.

## Subsequent local synchronization: 2026-09-08

The original `cs:41211` snapshot above remains historical provenance. This synchronization inspected
Plastic `/main@IlluviumGame@Illuvium@cloud`, loaded changeset and branch head `cs:41259`.
The callback file's last checked-in revision was `cs:41201` on `/main/UpdateImmutable_v1.11.5`;
`cm cat` at `cs:41259` matches Git HEAD's callback source after line-ending normalization.
The conhost implementation and subsequent explanatory comments are local changes over that baseline,
not a new Plastic changeset. No source behavior was changed during this synchronization.

| Item | Current local endpoint |
| --- | --- |
| Git branch and unchanged HEAD | `illuvium`, `1aa6864ad3295c701399d2bd66f678b8ad17c7bb` |
| Final source tree (not a commit) | `8762e9ca32fa2da840ea826fab9697ff821d76ea` |
| Plastic source | `/main`, `cs:41259` plus local `ImmutablePKCEWindows.cpp` changes |
| Callback file SHA-256 (copied bytes, LF) | `432a4b078a62b4643d2258c74f6ed5a350ba62e8becfc6d6a389b5538a683122` |
| Audit exclusions from source tree | `CUSTOM_MODIFICATIONS.md`, `illuvium-1.11.5-snapshot-customizations.patch` |

The tree object identifies every final tracked source path, mode, and blob without referencing a commit
that contains its own hash. It is a local object, not a published ref; recreate it from the synchronized
checkout using the instructions below if it has been pruned. No commit, check-in, or push was performed.
The local `origin/illuvium` tracking ref also names `1aa6864ad3295c701399d2bd66f678b8ad17c7bb`; a live remote
query failed with SSH `Permission denied (publickey)`, so the current remote tip was not verified.

A full filesystem comparison found only the callback source differed among shared plugin files before
synchronization. No Git-only SDK source required merging. Ten unchanged upstream `.github/` CI/template
files remain Git-only repository hosting metadata; they are retained in Git and in source-tree verification,
but excluded from Plastic plugin parity. `.git/`, `.plastic/`, and generated `Binaries/`, `Intermediate/`,
`Saved/`, and `DerivedDataCache/` output are excluded from filesystem comparison. All 114 distributable
source and audit files match byte-for-byte between the two locations after synchronization.

### Headless Windows callback customization

`Source/Immutable/Private/Immutable/Windows/ImmutablePKCEWindows.cpp` now registers:

```text
conhost.exe --headless powershell.exe -ExecutionPolicy Bypass -NoProfile -File "<script path>" "%1"
```

The C++ `FString::Printf` format uses `%%1` to emit Windows' `%1` placeholder. The script path and URI
are quoted; the URI remains data after `-File`. The existing generated PowerShell script, registry transport,
registry watcher, and Passport completion behavior are preserved. Direct PowerShell with `-WindowStyle Hidden`
still flashed because console creation can precede option handling. Comments preserve the callback flow,
quoting rationale, exit-code limitations, compatibility limits, and validation context. Callback delivery and
Passport results establish success; conhost's child exit-code propagation is not reliable.

Compatibility is deliberately limited: conhost is built into Windows 7 and later, but availability does not
prove support for this command. Windows 10 1809 is the documented ConPTY API baseline, not a guaranteed
minimum for this direct launcher. Microsoft treats direct conhost invocation as an unsupported interface.
Older conhost versions had argument-quoting issues; the oldest reliable Windows build for this exact command
remains unverified. There is no version gate or fallback. The integration document's Windows compatibility
section retains the primary-source references.

### Prior callback validation (retained, not rerun by this synchronization)

The IlluviumGame `Docs/Immutable/SDK1.11.5Integration.md` record for 2026-09-08 states:

- Development Editor and Win64 Shipping Game builds passed after the conhost implementation.
- Compiled Editor checks exercised real protocol registration and generated PowerShell callback delivery
  with synthetic login/logout URLs. Normal and spaced script paths passed all four checks, with zero visible
  callback-host windows sampled at approximately 5 ms intervals.
- The prior direct PowerShell launcher delivered the callback but failed the no-visible-window check.
- The user subsequently confirmed login and logout worked without flashing. No exact package/build ID was recorded.
- Later comment/documentation additions did not change executable code. The full Immutable suite was not
  rerun for the launcher change; older platform and suite results below remain historical.

These checks do not prove the absence of flashes shorter than the sampling interval or older-Windows
compatibility. Packaged-game testing belongs to the user; no packaged game was launched or tested here.
This synchronization reran source comparison, patch regeneration/application, tree/mode verification,
artifact hashes, and parity only; it did not rerun builds, protocol registration, or authentication.

## Customization review

| Customization | Classification | SDK 1.11.5 result and relevant paths |
| --- | --- | --- |
| GC-safe Blueprint async world context | Dropped at user request | The retention customization was reverted. `UImtblBlueprintAsyncAction` again uses upstream's raw, non-`UPROPERTY` `UObject* WorldContextObject`, and current action factories use the upstream member/assignment pattern. This intentionally removes GC retention while leaving the separate subsystem safety checks intact. `Source/Immutable/Public/Immutable/Actions/ImtblBlueprintAsyncAction.h`; `Source/Immutable/Private/Immutable/Actions/*.cpp`. |
| Safe async subsystem lookup | Ported | The shared lookup validates the raw context object, world, and game instance. Each affected activation obtains one checked subsystem before `WhenReady`. `Source/Immutable/Private/Immutable/Actions/ImtblBlueprintAsyncAction.cpp`; current action implementations. |
| Removed IMX async actions | Obsolete | The 1.9.0 IMX action classes no longer exist in 1.11.5. They were not restored; the current direct/embedded login and zkEVM architecture was retained. |
| Per-PIE Passport save indexing | Ported | Save existence, load, and save use the owning game instance's PIE index; packaged/non-PIE use index `0`. `Source/Immutable/Private/Immutable/ImmutablePassport.cpp`. |
| Initialized Passport state values | Already upstream | `StateFlags` is initialized to `IPS_NONE`; `UImmutableSaveGame` initializes `bWasConnectedViaPKCEFlow` to `false`. No duplicate change was made. `Source/Immutable/Public/Immutable/ImmutablePassport.h`; `Source/Immutable/Private/Immutable/ImmutableSaveGame.cpp`. |
| Save-slot existence and safe load | Already upstream / adapted | Upstream already checks slot existence and validates the loaded save object. The port preserves that behavior, adds a create-save-object guard, and applies the per-PIE index consistently. `Source/Immutable/Private/Immutable/ImmutablePassport.cpp`. |
| Stable one-based PIE browser contexts | Adapted | The removed 1.9.0 browser widget was not restored. The 1.11.5 base widget now creates `ImmutablePassportPIEUser<N>` contexts using `PIEInstance + 1`, sharing within one PIE user and isolating simultaneous users. `Source/Immutable/Private/Immutable/Browser/ImmutableBaseBrowserWidget.cpp`. |
| Persistent Illuvium-engine CEF profiles | Adapted | Editor profiles are placed beside the global `Default` profile only when `[Browser] bUseExplicitCEFRootCachePath=true` and the runtime global path is usable. `Source/Immutable/Private/Immutable/Browser/ImmutableBaseBrowserWidget.cpp`. |
| Stock-engine isolated fallback | Adapted | When the explicit root behavior/path is unavailable, the named editor context has no cookie path and remains isolated in memory. All CEF-only types and helpers are protected by bundled-CEF/editor guards. |
| Packaged global browser profile | Already upstream / preserved | Non-editor builds continue using `SWebBrowser` and the engine global profile. The editor-only `SWebBrowserView` context path does not affect packaged games. |
| Bundled CEF selection | Already upstream | SDK 1.11.5 already selects the bundled WebBrowser path for UE 5.1+. Existing `USING_BUNDLED_CEF`/`USING_BLUI_CEF` definitions were retained. `Source/Immutable/Immutable.Build.cs`. |
| CEF3 desktop dependency | Ported | Added the engine CEF3 private static dependency for Win64, Mac, and Linux. `Source/Immutable/Immutable.Build.cs`. |
| CEF/Chromium diagnostics and include boundaries | Ported | Compile-time CEF/Chromium versions, CEF branch, global profile, requested profile, and PIE indices are logged. CEF headers use Unreal third-party and Windows type boundaries. `Source/Immutable/Private/Immutable/Browser/ImmutableBaseBrowserWidget.cpp`; `ImmutableJSConnectorBrowserWidget.cpp`. |
| Unity-safe JS connector CEF helpers | Ported from Plastic `cs:41201` | The JS connector's anonymous-namespace CEF version helpers now have connector-specific names. This avoids same-translation-unit redefinitions with the base browser widget when `bUseUnity = true`. `Source/Immutable/Private/Immutable/Browser/ImmutableJSConnectorBrowserWidget.cpp`. |
| Chromium 128+ JSConnector shim | Adapted | Bridge loading follows the 1.11.5 browser-created flow. Chromium 128+ receives a single guarded shim with a 256-callback maximum, oldest-entry eviction, 25 ms polling, and a 200-attempt (5 second) timeout before diagnostics. `Source/Immutable/Private/Immutable/Browser/ImmutableJSConnectorBrowserWidget.cpp`; `Source/Immutable/Private/Immutable/ImtblBrowserUserWidget.cpp`. |
| Bridge load/bind diagnostics | Ported | Bridge resource failures, load configuration, null widgets/objects, bind results, already-bound state, and missing connectors are logged. `Source/Immutable/Private/Immutable/Browser/ImmutableJSConnectorBrowserWidget.cpp`. |
| Headless PIE bridge creation | Ported | Non-BLUI game instances without a game viewport return before `bHasSetupGameBridge` is set, so a valid client instance can initialize later. `Source/Immutable/Private/Immutable/ImmutableSubsystem.cpp`. |
| Optional Passport response parsing | Adapted | Invalid JSON/conversion returns no response; absent result strings, booleans, and arrays are tolerated without unsafe getters. Structured errors are populated before the response is returned. `Source/Immutable/Public/Immutable/ImtblJSMessages.h`; `Source/Immutable/Private/Immutable/ImmutablePassport.cpp`. |
| Structured response error preference | Ported | Passport handlers prefer `FImtblResponseError`, fall back to the JSON `error` string, then use an action-specific fallback. Failed bridge requests include action, request ID, and error diagnostics. `Source/Immutable/Private/Immutable/ImmutablePassport.cpp`. |
| Cross-platform Passport compilation | Ported from Plastic `cs:41201` | Broad desktop/mobile guards were narrowed so common Passport response, state, save/load, and game-instance definitions compile on Linux. Deep-link delegates, PKCE completion, and platform-specific overloads remain restricted to Android, iOS, Mac, and Windows. `Source/Immutable/Private/Immutable/ImmutablePassport.cpp`. |
| Browser console severity mapping | Ported | Browser Fatal/Error map to Unreal Error, Warning to Warning, Debug/Verbose to Verbose, and remaining levels to Log. Browser Fatal never invokes Unreal Fatal; source and line are retained. `Source/Immutable/Private/Immutable/Browser/ImmutableBaseBrowserWidget.cpp`. |
| Custom URL launch delegate | Adapted | `FImtblPassportLaunchURLDelegate` and `SetCustomLaunchURLDelegate` are public Passport APIs. Windows PKCE auth and hard logout use the delegate when bound and otherwise use `FPlatformProcess::LaunchURL`; Android, iOS, and Mac retain their native paths. `Source/Immutable/Public/Immutable/ImmutablePassport.h`; `Source/Immutable/Private/Immutable/ImmutablePassport.cpp`. |
| Silent cached-session relogin | Ported as a compatibility wrapper | Native `UImmutablePassport::Relogin` invokes the existing `relogin` bridge action, restores `IPS_CONNECTED | IPS_PKCE`, persists state, tracks `COMPLETE_RELOGIN`, and returns failure without launching a browser. The caller owns interactive fallback. `Web/index.js` is unchanged. `Source/Immutable/Public/Immutable/ImmutableNames.h`; `Source/Immutable/Public/Immutable/ImmutablePassport.h`; `Source/Immutable/Private/Immutable/ImmutablePassport.cpp`. |
| Headless Windows callback launcher | Subsequent local customization over Plastic `cs:41259` | Shared login/logout protocol registration uses headless conhost, quoted script/URI arguments, and unchanged registry transport. See the implementation and validation limits above. `Source/Immutable/Private/Immutable/Windows/ImmutablePKCEWindows.cpp`. |
| Legacy desktop device flow | Obsolete | The old device-flow APIs are absent from SDK 1.11.5. They were not reintroduced; current PKCE desktop flows receive the launch override. |
| Immutable/ImmutableEditor module compatibility | Ported | `IWYUSupport.Full` and unity-build settings are retained with minimal Build.cs edits. `Source/Immutable/Immutable.Build.cs`; `Source/ImmutableEditor/ImmutableEditor.Build.cs`. |
| Forward declarations and implementation includes | Ported | Subsystem, analytics, game-instance, Slate browser, world, and CEF dependencies are declared/included at their use sites. Runtime dependency `Web/index.js` and the upstream module layout remain unchanged. |
| Plugin marked not installed | Ported for this fork | `Immutable.uplugin` now has `"Installed": false`. |
| Empty analytics JSON payload | Already upstream | SDK 1.11.5 already sends `{}`; `ImmutableAnalytics.cpp` is unchanged. |
| Automation flag spelling compatibility | Already upstream / preserved | The existing UE-version conditional for `EAutomationTestFlags_ApplicationContextMask` is retained. Focused response-handling and relogin bridge-contract tests live below it. `Source/Immutable/Private/Immutable/Tests/ImtblMessagesTest.cpp`. |
| Log category maximum verbosity | Already upstream | `LogImmutable` already declares maximum verbosity `All`; no duplicate change was made. `Source/Immutable/Public/Immutable/Misc/ImtblLogging.h`. |
| Call-before-bridge warning | Ported | Re-enabled the existing warning before queuing work for bridge readiness. `Source/Immutable/Private/Immutable/ImtblJSConnector.cpp`. |
| Initial browser throbber | Dropped | The 1.9.0 fork enabled it. SDK 1.11.5's browser is a hidden game-bridge widget and upstream defaults it off, so that UI-only change was not carried. |
| Extra save/load trace logs and restored comments | Dropped | They do not change behavior and would add noise/documentation churn to newer upstream code. Error paths remain logged. |
| Hard-coded `webcache_4430`, BLUI-era code, `version.txt`, and formatting-only changes | Obsolete | The profile path is replaced by explicit-root discovery; removed browser/BLUI files were not restored; the non-authoritative version marker and whitespace-only changes were not ported. |

## Differences from the SDK 1.9.0 customization set

- The port starts at official commit `4973cee...`, not the historical unpublished 1.9.0 snapshot.
- The 1.11.5 `UImmutableBaseBrowserWidget` / `UImmutableJSConnectorBrowserWidget` architecture is extended instead of restoring `ImtblBrowserWidget`.
- Removed IMX and legacy device-flow APIs are not recreated; current direct login, embedded login, PKCE, and zkEVM paths are covered.
- The removed C++ relogin option is restored only as a thin native wrapper over the retained SDK 1.11.5 `relogin` bridge action; no legacy
  device flow or new JavaScript is reintroduced.
- The historical GC-retained world-context customization was explicitly reverted; current actions use upstream's raw `WorldContextObject` member while retaining null-safe subsystem lookup.
- The Chromium bridge queue is explicitly bounded at 256 callbacks while retaining the historical 5-second polling ceiling.
- SDK 1.11.5 behavior already covers save existence checks, state initialization, analytics `{}`, automation flag spelling, and log-category maximum verbosity.
- The old cache directory, initial-throbber change, local version marker, restored comments, and formatting-only deltas are dropped.

## Build and test record

Validation used Unreal Engine 5.8.2 (`5.8.2-56702186`) and disposable output outside the Git repository.
The compile-fix transfer was validated on 2026-09-03 after copying the two pinned Plastic files
into the existing disposable host plugin.

### Standalone BuildPlugin

```powershell
& 'D:\unreal\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat' BuildPlugin `
  -Plugin='D:\dev\illuvium\unreal-immutable-sdk\Immutable.uplugin' `
  -Package='C:\Users\vampy\AppData\Local\Temp\i115pkg-final' `
  -TargetPlatforms=Win64 -Rocket
```

Result: failed in UHT before C++ compilation. UE 5.8 classified the packaged foreign plugin as an
engine module and required explicit Blueprint categories on numerous unchanged SDK 1.11.5
declarations, principally in `ImmutableDataTypes.h` and `ImmutableRequests.h`. This is an upstream
1.11.5 / UE 5.8 standalone-packaging compatibility issue, not introduced by this port. The port does
not add unrelated category metadata to dozens of upstream declarations. UAT's generated, untracked
`Config/FilterPlugin.ini` was removed after the attempt.

### Disposable host builds

A minimal source host project was created under
`C:\Users\vampy\AppData\Local\Temp\i115h`; the plugin was not copied into IlluviumGame.

```powershell
& 'D:\unreal\UE_5.8\Engine\Build\BatchFiles\Build.bat' HostProject Win64 Development `
  -Project='C:\Users\vampy\AppData\Local\Temp\i115h\HostProject.uproject' `
  -WaitMutex -NoEngineChanges -NoHotReloadFromIDE

& 'D:\unreal\UE_5.8\Engine\Build\BatchFiles\Build.bat' HostProjectEditor Win64 Development `
  -Project='C:\Users\vampy\AppData\Local\Temp\i115h\HostProject.uproject' `
  -WaitMutex -NoEngineChanges -NoHotReloadFromIDE -ForceUnity -DisableAdaptiveUnity

$env:LINUX_MULTIARCH_ROOT = 'D:\unreal\Linux_CrossCompileToolChain\v26_clang-20.1.8-rockylinux8\'
& 'D:\unreal\UE_5.8\Engine\Build\BatchFiles\Build.bat' HostProject Linux Development `
  -Project='C:\Users\vampy\AppData\Local\Temp\i115h\HostProject.uproject' `
  -WaitMutex -NoEngineChanges -NoHotReloadFromIDE -ForceUnity -DisableAdaptiveUnity
```

Results:

- Win64 Development game target: passed; both transferred files compiled and `HostProject.exe` linked.
- Win64 Development editor target: passed with adaptive unity disabled and unity forced. `Module.Immutable.cpp` compiled and linked, directly validating that the connector-specific helper names eliminate the unity translation-unit collision.
- Linux x86_64 Development game target: passed with Clang 20.1.8. `Module.Immutable.cpp` compiled and `HostProject` linked, validating that common Passport definitions remain available when the desktop/mobile deep-link macros are false.

### Automation tests

```powershell
& 'D:\unreal\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\Users\vampy\AppData\Local\Temp\i115h\HostProject.uproject' `
  -unattended -stdout -FullStdoutLogOutput -NoSplash -NullRHI -SKIPCOMPILE `
  '-ExecCmds=Automation RunTests Immutable; Quit' `
  '-TestExit=Automation Test Queue Empty'
```

The full suite run during the initial port discovered two tests:

- `Immutable.Illuvium.ResponseHandling`: passed. It verifies structured errors and missing optional string, boolean, and array results.
- `Immutable.JSMessages`: failed its two pre-existing exact-JSON expectations. The actual upstream 1.11.5 payload contains `logoutRedirectUri`, `isSilentLogout`, `engineSdkVersion`, and `deviceModel`, while the unchanged expected strings omit them. Serialization code was not changed by this port, so this is an upstream stale-test issue.

The focused `Immutable.Illuvium.ResponseHandling` invocation was rerun after the Plastic compile-fix
transfer. Unreal discovered one matching test, reported `Result={Success}`, and exited with code `0`.

The clean IlluviumGame Plastic endpoint at `cs:41211` was built as Win64 Development Editor, Win64
Shipping Game, and Linux Development Server. Its focused `Immutable.Illuvium` run discovered
`Immutable.Illuvium.ReloginBridgeContract` and `Immutable.Illuvium.ResponseHandling`; both passed and
the automation process exited with code `0`.

## Pending manual validation

Local Windows login/logout has the callback checks and user confirmation recorded above. Live silent relogin
after restart, unusable-session fallback, native Android/iOS/Mac paths, simultaneous multi-user PIE persistence,
headless multiplayer PIE, browser console mapping, CEF profile persistence under the Illuvium engine, and
stock-engine in-memory fallback still need their target environments. Packaged-game testing belongs to the
user; no package/build identifier or oldest-reliable-Windows result was established. Original integration
build and automation results below their historical headings must not be interpreted as rerun on this endpoint.

## Patch artifact

| Item | Value |
| --- | --- |
| File | `illuvium-1.11.5-snapshot-customizations.patch` |
| Base commit | `4973cee21bfaa53dadc9e4519d53a3ae2203bb29` |
| Final source endpoint tree | `8762e9ca32fa2da840ea826fab9697ff821d76ea` |
| Git HEAD before/after local synchronization | `1aa6864ad3295c701399d2bd66f678b8ad17c7bb` |
| Size | 79,014 bytes |
| SHA-256 | `41b1bde2cc6a917556c8476d63210165aadb319a2e6108adc5509a537f87682d` |
| Changed files | 32 modified, 0 added/deleted/renamed |
| Insertions | 865 |
| Deletions | 160 |

The full SDK customization patch excludes `CUSTOM_MODIFICATIONS.md` and the patch file itself.
It includes the original integration customizations plus the subsequent callback change.

### Changed-file inventory against upstream

| Path | Insertions | Deletions |
| --- | ---: | ---: |
| `Immutable.uplugin` | 2 | 2 |
| `Source/Immutable/Immutable.Build.cs` | 9 | 1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblBlueprintAsyncAction.cpp` | 7 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblConnectionAsyncAction.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblEmbeddedLoginAsyncAction.cpp` | 1 | 1 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportConnectEvmAsyncAction.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportGetEmailAsyncAction.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportGetLinkedAddressesAsyncAction.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportGetTokenAsyncAction.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportHasStoredCredentialsAsyncAction.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportInitializationAsyncAction.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportLogoutAsyncAction.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmGetBalanceAsyncAction.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmGetTransactionReceiptAA.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmRequestAccountsAsyncAction.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmSendTransactionAsyncAction.cpp` | 6 | 4 |
| `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmSignTypedDataV4AsyncAction.cpp` | 3 | 2 |
| `Source/Immutable/Private/Immutable/Browser/ImmutableBaseBrowserWidget.cpp` | 219 | 9 |
| `Source/Immutable/Private/Immutable/Browser/ImmutableJSConnectorBrowserWidget.cpp` | 144 | 5 |
| `Source/Immutable/Private/Immutable/ImmutablePassport.cpp` | 193 | 85 |
| `Source/Immutable/Private/Immutable/ImmutableSubsystem.cpp` | 13 | 1 |
| `Source/Immutable/Private/Immutable/ImtblBrowserUserWidget.cpp` | 2 | 8 |
| `Source/Immutable/Private/Immutable/ImtblJSConnector.cpp` | 2 | 2 |
| `Source/Immutable/Private/Immutable/Tests/ImtblMessagesTest.cpp` | 147 | 0 |
| `Source/Immutable/Private/Immutable/Windows/ImmutablePKCEWindows.cpp` | 26 | 1 |
| `Source/Immutable/Public/Immutable/Actions/ImtblBlueprintAsyncAction.h` | 2 | 1 |
| `Source/Immutable/Public/Immutable/Browser/ImmutableBaseBrowserWidget.h` | 9 | 1 |
| `Source/Immutable/Public/Immutable/Browser/ImmutableJSConnectorBrowserWidget.h` | 2 | 1 |
| `Source/Immutable/Public/Immutable/ImmutableNames.h` | 2 | 1 |
| `Source/Immutable/Public/Immutable/ImmutablePassport.h` | 32 | 2 |
| `Source/Immutable/Public/Immutable/ImtblJSMessages.h` | 8 | 8 |
| `Source/ImmutableEditor/ImmutableEditor.Build.cs` | 3 | 1 |

### Reproduce

Run in PowerShell 7 from the SDK Git checkout. The final source tree can be reconstructed from the synchronized working
files without changing the main index or creating a commit. The inspected checkout had no untracked SDK
source; `add -u` deliberately includes its complete tracked source state. The temporary index preserves
tracked modes; `core.autocrlf=input` uses the same clean-filter behavior as this checkout.

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
  if ($SourceTree -ne '8762e9ca32fa2da840ea826fab9697ff821d76ea') { throw 'Source endpoint differs' }
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
if ($Hash -ne '41b1bde2cc6a917556c8476d63210165aadb319a2e6108adc5509a537f87682d') { throw 'Patch hash differs' }
```

### Apply and compare at the exact base

The isolated checkout is created inside the SDK's Git metadata directory, outside the source payload.
Resolve the patch first. Every command must succeed before continuing.

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
if ($AppliedTree -ne '8762e9ca32fa2da840ea826fab9697ff821d76ea') { throw 'Applied source differs' }
# Remove only this newly created disposable checkout after successful comparison.
git worktree remove --force $ApplyTree
```

### Reproduction verification

On 2026-09-08, `apply --check`, application, and exact Git tree comparison passed in an isolated checkout
at the pinned base. The applied tree was `8762e9ca32fa2da840ea826fab9697ff821d76ea`, matching the final
source endpoint across all 122 tracked paths, modes, and blob hashes. The earlier original-integration
verification produced `e0389a693f2b8a186b0d24dedd24737916aa30fd`; that remains a historical result.

There are no mode changes relative to upstream. The upstream `.github/scripts/check_team_membership.sh`
retains mode `100755`; the other 121 files are `100644`. Windows filesystem parity cannot independently
represent POSIX executable bits, so modes are verified through Git trees. The ten unchanged `.github/`
files participate in patch reproduction but are excluded from the 114-file distributable parity check.

The callback file retains its existing LF bytes; the audit Markdown uses LF. No blanket source conversion
was performed. Verification uses `core.autocrlf=false` to preserve exact blobs on checkout/application.
The patch preserves ten CRLF lines from existing diff content; do not normalize the patch as ordinary text,
as that changes its hash and can change its payload. Tree equality covers exact committed blob line endings;
separate filesystem SHA-256 comparison verifies byte-for-byte parity of the two distributable plugin trees.
The audit artifacts are excluded from both endpoint and patch payload, then compared separately after copying.
