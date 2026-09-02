# Illuvium customizations to the Immutable Unreal SDK

## Baseline determination

This fork is based on an **upstream 1.9.0 snapshot**, not directly on the published `v1.5.0` tag.

| Item | Git object | Meaning |
| --- | --- | --- |
| Illuvium merge | `6a232d5193f737983953790e4bc9f3ae41af22e8` | Merged upstream `main` into the Illuvium branch. |
| Illuvium parent | `90a5ddb47099175a16439032b78574f6b929147c` | Illuvium state before that merge. |
| Merged upstream parent | [`31ce0f28ad20d3cbb26b17c1a52ac9a201ed5a17`](https://github.com/immutable/unreal-immutable-sdk/commit/31ce0f28ad20d3cbb26b17c1a52ac9a201ed5a17) | Exact historical baseline used for this patch. |
| Upstream-reachable equivalent | [`d032141a4809a326120e90efe223ccabbf04852c`](https://github.com/immutable/unreal-immutable-sdk/commit/d032141a4809a326120e90efe223ccabbf04852c) | Has the same tree, `c6ba836f247e1ee8d223517fbee99f2f90844fd4`. |
| Reconciliation start | `a165dabe319e6790285de9b8b30dc852a9da49ae` | Exact Illuvium commit from which `reconcile/plastic-customizations-1.9.0` was created. |

The baseline's `Source/Immutable/Public/Immutable/ImmutableDataTypes.h` declares:

```cpp
#define ENGINE_SDK_VERSION TEXT("1.9.0")
```

There is no published `v1.9.0` release. The upstream release history jumps from [`v1.5.0`](https://github.com/immutable/unreal-immutable-sdk/releases/tag/v1.5.0) to [`v1.10.0`](https://github.com/immutable/unreal-immutable-sdk/releases/tag/v1.10.0), so `v1.5.0` is the nearest published release while `31ce0f2` is the exact source baseline.

The following labels are not authoritative:

- `Illuvium_1.5` is the fork branch name, not the commit that was merged from upstream.
- `version.txt` is local to the fork, absent from the upstream baseline, and still contains `v1.5.0`.
- `Immutable.uplugin` says `1.3.0.alpha` in both the upstream snapshot and this fork; upstream's release workflow instead reads `ENGINE_SDK_VERSION` from `ImmutableDataTypes.h`.

## Upstream changes after `v1.5.0`

The exact baseline is close to `v1.5.0`: the upstream-only delta is 7 files, 23 insertions, and 190 deletions. These are upstream changes and therefore are not included as Illuvium customizations in this report.

| Path | Upstream change after `v1.5.0` |
| --- | --- |
| `.github/workflows/release.yml` | Read the engine SDK version separately, retain the TypeScript SDK version, and create `v`-prefixed release tags. |
| `Immutable.uplugin` | Remove the plugin-manager module and switch the default browser integration from BLUI to WebBrowserWidget. |
| `Source/Immutable/Private/Immutable/ImmutablePassport.cpp` | Check that the Passport save slot exists before loading it. |
| `Source/Immutable/Public/Immutable/ImmutableDataTypes.h` | Advance `ENGINE_SDK_VERSION` from `1.5.0` through the unpublished bumps to `1.9.0`. |
| `Source/ImmutablePluginManager/ImmutablePluginManager.Build.cs` | Remove the upstream plugin-manager module. |
| `Source/ImmutablePluginManager/Private/ImmutablePluginManager.cpp` | Remove the plugin-manager implementation. |
| `Source/ImmutablePluginManager/Public/ImmutablePluginManager.h` | Remove the plugin-manager public interface. |

Using `v1.5.0` as the patch base would incorrectly mix these upstream changes into the Illuvium patch.

## Illuvium modifications

The reconciled Illuvium patch changes 39 tracked SDK files with 720 insertions and 279 deletions. `CUSTOM_MODIFICATIONS.md` and the patch file are audit artifacts and are deliberately excluded from those totals and from the patch payload.

### GC-safe Blueprint async world contexts

- Replace the raw `WorldContextObject` member in `UImtblBlueprintAsyncAction` with an initialized `UPROPERTY(Transient)` named `SavedWorldContextObject` so Unreal's garbage collector retains the context for the lifetime of an asynchronous action.
- Update subsystem lookup, validation, and every affected async-action factory to use the retained member.
- Add the implementation-side subsystem include and a public forward declaration.

This affects the shared base plus 18 concrete action source files, covering login/connect, Passport initialization and logout, token and address queries, IMX operations, and zkEVM operations.

### Per-PIE state and browser isolation

- Derive a user index from the owning game instance's `PIEInstance` while running in the editor, falling back to index `0` outside PIE.
- Use that index for Passport save-slot existence checks, loads, and saves so simultaneous PIE sessions do not share PKCE state.
- Initialize `bWasConnectedViaPKCEFlow` to `false`.
- Give editor browser instances stable, one-based `ImmutablePassportPIEUser<N>` contexts.
- Persist those contexts beside the global `Default` profile only when `[Browser] bUseExplicitCEFRootCachePath=true`; otherwise use isolated in-memory contexts.
- Use `SWebBrowserView` in the editor so custom context settings can be supplied; keep `SWebBrowser` for packaged runtime builds.

This supersedes the obsolete Chromium-build-specific `webcache_4430/immutable_<index>` path.

### Browser behavior

- Enable the initial throbber for the packaged `SWebBrowser` path.
- Bind browser console messages without the previous UE 5.1-or-newer guard at the construction site.
- Add the WebBrowser module include required for browser context access.
- For Chromium 128 and newer, install a guarded bridge binding shim that queues callbacks every 25 ms for up to 200 attempts and flushes once the real `JSConnector` appears.
- Log compile-time CEF/Chromium/branch information, the global browser profile, bridge load failures, and bridge bind failures when bundled CEF headers are available.
- Map browser Fatal/Error to Unreal Error, Warning to Warning, Debug/Verbose to Verbose, and all other severities to Log; browser messages never invoke Unreal Fatal.
- Clarify several preprocessor closing comments.

### Plastic reconciliation provenance

The behavior source was the pinned Plastic comparison `cs:29001` through `cs:41193`. The clean Plastic workspace was inspected read-only and not updated, switched, modified, checked in, or synchronized. No SDK 1.11.x source was copied into this Git fork.

| Behavior | Status on SDK 1.9.0 | Provenance |
| --- | --- | --- |
| CEF module definitions and bundled/BLUI selection | Already present; retained the UE 5.1+ structure. | `39809`, `39837` |
| Desktop CEF3 static dependency | Ported for Win64, Mac, and Linux. | `39809`, `39837` |
| Chromium 128+ bridge shim | Ported with the bounded 25 ms / 200-attempt queue. | `40356`, `40359`, `40362` |
| CEF and bridge diagnostics | Ported behind bundled-CEF/header guards. | `40760` |
| PIE browser profiles | Adapted to stable one-based names and explicit-root persistence; the hard-coded Chromium-build cache directory is superseded. | `40356`, `40359`, `40362`, `41184` |
| Headless PIE bridge setup | Ported by requiring a game viewport before setting `bHasSetupGameBridge` on the non-BLUI path. | `40454` |
| Passport response parsing | Ported with tolerant fields, structured-error preference, JSON fallback, and non-empty action/request diagnostics. | `40760` |
| Browser console severity mapping | Ported without mapping browser Fatal to Unreal Fatal. | `40760` |
| Empty analytics JSON | Already present as `{}`; `ImmutableAnalytics.cpp` remains unchanged. | Superseded by the 1.9.0 baseline |
| Automation flag spelling | Already contains the UE-version conditional; `ImtblMessagesTest.cpp` remains unchanged. | Superseded by the 1.9.0 baseline |

### Custom external URL launching

- Add `FImtblPassportLaunchURLDelegate` and `SetCustomLaunchURLDelegate` to `UImmutablePassport`.
- Route device-flow and desktop hard-logout URL launches through the custom delegate when bound.
- Fall back to `FPlatformProcess::LaunchURL` when no override is installed.

Platform-specific Android, iOS, and macOS PKCE launch paths remain unchanged.

### Build and include compatibility

- Enable `IWYUSupport.Full` and unity builds for the `Immutable` and `ImmutableEditor` modules.
- Add the engine `CEF3` private static dependency for Win64, Mac, and Linux when using the UE 5.1+ bundled browser path.
- Normalize the editor PCH configuration syntax.
- Add forward declarations for analytics and browser classes, move the required subsystem include into the implementation, and remove an unused BLUI include.
- Preserve the existing runtime dependencies and module layout.

### Diagnostics, initialization, and documentation

- Change `LogImmutable`'s compile-time verbosity from `Log` to `All`.
- Re-enable the warning emitted when JavaScript is called before the bridge is ready.
- Add function-level logging around Passport settings saves and loads.
- Restore explanatory `CallJS` comments in `ImmutablePassport.h`.
- Add the local `version.txt` marker containing `v1.5.0`; as noted above, this marker does not describe the actual upstream baseline.

### Formatting-only files

The exact patch retains non-functional whitespace, BOM, or blank-line differences in these seven files:

- `Immutable.uplugin`
- `Source/Immutable/Private/Immutable/ImmutableDataTypes.cpp`
- `Source/Immutable/Private/Immutable/ImmutableRequests.cpp`
- `Source/Immutable/Private/Immutable/ImtblBlui.cpp`
- `Source/Immutable/Public/Immutable/ImmutableDataTypes.h`
- `Source/Immutable/Public/Immutable/ImmutableRequests.h`
- `Source/Immutable/Public/Immutable/ImtblJSConnector.h`

## Complete changed-file inventory

Every path in the exact baseline-to-fork diff appears once below.

| Primary category | Path |
| --- | --- |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblBlueprintAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblConnectImxAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportConnectEvmAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportGetAddressAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportGetEmailAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportGetLinkedAddressesAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportGetTokenAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportHasStoredCredentialsAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportImxBatchNftTransferAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportImxIsRegisteredOffchainAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportImxRegisterOffchainAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportImxTransferAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportInitializationAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportLogoutAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmGetBalanceAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmGetTransactionReceiptAA.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmRequestAccountsAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmSendTransactionAsyncAction.cpp` |
| Async world context | `Source/Immutable/Private/Immutable/Actions/ImtblPassportZkEvmSignTypedDataV4AsyncAction.cpp` |
| Async world context | `Source/Immutable/Public/Immutable/Actions/ImtblBlueprintAsyncAction.h` |
| PIE, URL, and Passport state | `Source/Immutable/Private/Immutable/ImmutablePassport.cpp` |
| PIE, URL, and Passport state | `Source/Immutable/Private/Immutable/ImtblBrowserWidget.cpp` |
| PIE, URL, and Passport state | `Source/Immutable/Private/Immutable/ImtblBrowserWidget.h` |
| PIE, URL, and Passport state | `Source/Immutable/Public/Immutable/ImmutablePassport.h` |
| PIE, URL, and Passport state | `Source/Immutable/Public/Immutable/ImmutableSaveGame.h` |
| Build and includes | `Source/Immutable/Immutable.Build.cs` |
| Build and includes | `Source/Immutable/Private/Immutable/ImmutableSubsystem.cpp` |
| Build and includes | `Source/Immutable/Public/Immutable/ImmutableSubsystem.h` |
| Build and includes | `Source/ImmutableEditor/ImmutableEditor.Build.cs` |
| Diagnostics | `Source/Immutable/Private/Immutable/ImtblJSConnector.cpp` |
| Diagnostics | `Source/Immutable/Public/Immutable/Misc/ImtblLogging.h` |
| Local metadata | `version.txt` |
| Formatting only | `Immutable.uplugin` |
| Formatting only | `Source/Immutable/Private/Immutable/ImmutableDataTypes.cpp` |
| Formatting only | `Source/Immutable/Private/Immutable/ImmutableRequests.cpp` |
| Formatting only | `Source/Immutable/Private/Immutable/ImtblBlui.cpp` |
| Formatting only | `Source/Immutable/Public/Immutable/ImmutableDataTypes.h` |
| Formatting only | `Source/Immutable/Public/Immutable/ImmutableRequests.h` |
| Formatting only | `Source/Immutable/Public/Immutable/ImtblJSConnector.h` |

## Patch usage

Patch file: `illuvium-1.9.0-snapshot-customizations.patch`

- Size: 84,193 bytes
- SHA-256: `006c13fb56be0600bb3e51c00ea743dfc59d458e0a1d86189d7ab2274b175a1b`
- Base: `31ce0f28ad20d3cbb26b17c1a52ac9a201ed5a17`
- Equivalent base: `d032141a4809a326120e90efe223ccabbf04852c` (tree-identical only; not the authoritative comparison base)
- Reconciliation branch: `reconcile/plastic-customizations-1.9.0`
- Result: staged reconciled tree; the final commit was withheld because the required full package build and automation test did not pass.

Apply it to a clean checkout of the baseline:

```powershell
git checkout --detach 31ce0f28ad20d3cbb26b17c1a52ac9a201ed5a17
git apply --check illuvium-1.9.0-snapshot-customizations.patch
git apply illuvium-1.9.0-snapshot-customizations.patch
```

The same patch applies to `d032141a4809a326120e90efe223ccabbf04852c` because that commit has the identical tree.

Regenerate the patch from the pinned snapshots:

```powershell
git diff --binary --full-index --find-renames `
  --output=illuvium-1.9.0-snapshot-customizations.patch `
  31ce0f28ad20d3cbb26b17c1a52ac9a201ed5a17 -- . `
  ':(exclude)CUSTOM_MODIFICATIONS.md' `
  ':(exclude)illuvium-1.9.0-snapshot-customizations.patch'
```

## Validation record

| Check | Result |
| --- | --- |
| Source whitespace | Passed: `git diff --check` over tracked SDK paths with the two audit artifacts excluded. |
| Patch application | Passed: `git apply --binary --check` and `git apply --binary` in a disposable detached worktree at `31ce0f28...`. |
| Mode/blob manifest | Passed: all 675 non-audit index entries matched exactly, with no extra or missing files. |
| UE 5.8 `RunUAT BuildPlugin` | Failed before reconciliation code compilation because SDK 1.9.0 lacks UE 5.8-required reflection categories when UHT classifies the foreign plugin as an engine module. |
| UE 5.8 local-plugin build | The changed `Immutable` module and its `ImmutableCore`, `ImmutableEditor`, and `ImmutableMarketplace` companions compiled and linked successfully. The full plugin still failed in unchanged `ImmutableOrderbook` and `ImmutablezkEVMAPI` sources because UE 5.8 removed `FHttpRetrySystem::FManager::Update()`. |
| `Immutable.JSMessages` | Ran one test and failed two existing expected-JSON assertions. Actual 1.9.0 fork payloads include the already-customized `logoutRedirectUri`, `isSilentLogout`, `engineSdkVersion`, and `deviceModel` fields; the plan requires `ImtblMessagesTest.cpp` to remain unchanged. |

Passport login, live bridge initialization, multi-user PIE persistence, headless multiplayer PIE, and runtime browser logging were not performed because no disposable full game host and credentials were available. The Plastic 1.11.5 plugin was not substituted for any check.
