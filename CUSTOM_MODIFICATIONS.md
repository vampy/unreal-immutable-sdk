# Illuvium customizations to the Immutable Unreal SDK

## Baseline determination

This fork is based on an **upstream 1.9.0 snapshot**, not directly on the published `v1.5.0` tag.

| Item | Git object | Meaning |
| --- | --- | --- |
| Illuvium merge | `6a232d5193f737983953790e4bc9f3ae41af22e8` | Merged upstream `main` into the Illuvium branch. |
| Illuvium parent | `90a5ddb47099175a16439032b78574f6b929147c` | Illuvium state before that merge. |
| Merged upstream parent | [`31ce0f28ad20d3cbb26b17c1a52ac9a201ed5a17`](https://github.com/immutable/unreal-immutable-sdk/commit/31ce0f28ad20d3cbb26b17c1a52ac9a201ed5a17) | Exact historical baseline used for this patch. |
| Upstream-reachable equivalent | [`d032141a4809a326120e90efe223ccabbf04852c`](https://github.com/immutable/unreal-immutable-sdk/commit/d032141a4809a326120e90efe223ccabbf04852c) | Has the same tree, `c6ba836f247e1ee8d223517fbee99f2f90844fd4`. |
| Fork endpoint | `dc7bd8ab51345dad995384485589fcc08bb8d58c` | Illuvium snapshot represented by the patch. |

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

The net Illuvium patch changes 39 files with 376 insertions and 254 deletions. Ignoring whitespace and blank-line-only changes, 32 files contain semantic changes and 7 are formatting-only.

### GC-safe Blueprint async world contexts

- Replace the raw `WorldContextObject` member in `UImtblBlueprintAsyncAction` with an initialized `UPROPERTY(Transient)` named `SavedWorldContextObject` so Unreal's garbage collector retains the context for the lifetime of an asynchronous action.
- Update subsystem lookup, validation, and every affected async-action factory to use the retained member.
- Add the implementation-side subsystem include and a public forward declaration.

This affects the shared base plus 18 concrete action source files, covering login/connect, Passport initialization and logout, token and address queries, IMX operations, and zkEVM operations.

### Per-PIE state and browser isolation

- Derive a user index from the owning game instance's `PIEInstance` while running in the editor, falling back to index `0` outside PIE.
- Use that index for Passport save-slot existence checks, loads, and saves so simultaneous PIE sessions do not share PKCE state.
- Initialize `bWasConnectedViaPKCEFlow` to `false`.
- Give editor browser instances separate `FBrowserContextSettings` and cookie storage under `webcache_4430/immutable_<PIE instance>`.
- Use `SWebBrowserView` in the editor so custom context settings can be supplied; keep `SWebBrowser` for packaged runtime builds.

The cache path deliberately contains the Chromium build directory `webcache_4430`; it may need revisiting if Unreal's bundled CEF build changes.

### Browser behavior

- Enable the initial throbber for the packaged `SWebBrowser` path.
- Bind browser console messages without the previous UE 5.1-or-newer guard at the construction site.
- Add the WebBrowser module include required for browser context access.
- Clarify several preprocessor closing comments.

### Custom external URL launching

- Add `FImtblPassportLaunchURLDelegate` and `SetCustomLaunchURLDelegate` to `UImmutablePassport`.
- Route device-flow and desktop hard-logout URL launches through the custom delegate when bound.
- Fall back to `FPlatformProcess::LaunchURL` when no override is installed.

Platform-specific Android, iOS, and macOS PKCE launch paths remain unchanged.

### Build and include compatibility

- Enable `IWYUSupport.Full` and unity builds for the `Immutable` and `ImmutableEditor` modules.
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

- Size: 70,088 bytes
- SHA-256: `3c058d497610e9193891627119725f3c339dff04eee4c1632b29fe26a7ba8360`
- Base: `31ce0f28ad20d3cbb26b17c1a52ac9a201ed5a17`
- Result: `dc7bd8ab51345dad995384485589fcc08bb8d58c`

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
  31ce0f28ad20d3cbb26b17c1a52ac9a201ed5a17 `
  dc7bd8ab51345dad995384485589fcc08bb8d58c -- .
```
