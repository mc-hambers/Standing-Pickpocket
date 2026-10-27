# Standing QuickPocket Compatibility 0.1.1 diagnostic

Optional, separate SKSE DLL for Standing Pickpocket 1.0.2 and QuickPocket 1.0.0.
It does not replace or edit either mod's installed files. No PCR dependency or
chance/Karma integration is included.

## Behavior

- Press the existing Standing Pickpocket hotkey while aiming at an eligible NPC
  to open QuickPocket's real-time menu while standing, without a rear-position
  requirement. Press the same hotkey again on that NPC to close the session.
- With the adapter active, QuickPocket opens through the explicit hotkey request
  rather than automatically opening when crouching over a target.
- Range follows Standing Pickpocket's effective distance API, including an
  external distance override if another mod supplies one.
- QuickPocket continues to handle the menu, perks, theft/plant actions, chance
  display, rolls and crime behavior. This adapter does not calculate any of them.
- QuickPocket's own environment and optional close-on-detection checks remain.
- Target loss/change, invalid range, combat, unloaded/dead actors, blocked menus
  and loading a game end the adapter's access. The player's sneak state is never
  spoofed. Companion and horse inventory paths are not redirected.
- When QuickPocket is disabled or its QuickLoot API is unavailable, an explicit
  Standing Pickpocket request falls back to the normal inventory menu.

## Requirements and installation

Standing Pickpocket 1.0.2; the supplied QuickPocket 1.0.0 release; QuickLoot IE
4.1.1+ and those mods' dependencies, including matching SKSE and Address Library.
This build targets flat Skyrim SE/AE, not VR. Native use of 1.7.104 is not yet
verified in game.

Install the optional ZIP alongside those mods. It adds only
`SKSE/Plugins/StandingQuickPocket.dll` plus license notices. No ESP, Papyrus,
MCM, or packaged INI is added. Configure the hotkey in Standing Pickpocket.
Restart Skyrim after installing/removing the adapter. Removing it restores the
other mods' normal code on the next process launch. No co-save data is written.

## Supported binaries and safety

Exact SHA-256 fingerprints are in profiles.json. They match the supplied
QuickPocket 1.0.0 DLL and the existing Standing Pickpocket 1.0.2 release package.
The QuickPocket PDB was inspected to identify its native routines; no PDB or
QuickPocket implementation/assets are distributed here. Profiles describe ABI
addresses and minimal patch-site bytes only. Runtime hash and byte checks run
before writes. Unsupported or already modified sites leave the adapter inactive.
All writes affect process memory only, before QuickPocket's targeting worker
starts at DataLoaded. Companions' files are never patched on disk.

Log: Documents/My Games/Skyrim Special Edition/SKSE/StandingQuickPocket.log.
Look for 'Adapter hook installation passed' or an explicit inactive reason.

## Validation and limits

Build and native/session tests passed. A native smoke executable exercises the
same patch/trampoline helpers, mismatch rejection and restoration. Binary audits
verify released profiles and relocation-free prologue boundaries. Loader/export
and ZIP checks also run. None of these establish successful Skyrim gameplay.

Pending: in-game hotkey open/close while standing; loss/change of target; range;
QuickPocket disable/detection options; planting/theft; companion inventory;
save/load; unsupported-version fallback. PCR is outside this adapter's scope.

## Build and source

Full adapter and CommonLib source are in the companion source ZIP. Standard
CMake + Ninja/MSVC + Windows SDK and vcpkg dependencies are needed. Configure
vcpkg.json with your own vcpkg toolchain and build Release. The optional
COMMONLIB_PREBUILT_LIBRARY CMake argument can reuse a matching SE/AE CommonLib
library; it is not needed for rebuilding from the included vendor source.
The author's scripts/build.ps1 additionally uses the existing portable toolchain
from ../Pickpocketing-Loot; this is development tooling, not a runtime requirement.

GPL-3.0-or-later. See LICENSE.txt and dependency license notices.

## Empty-session hostility investigation

This diagnostic build records hotkey-session state, QuickPocket eligibility rejection, close requests, item transfers and combat events for two minutes after opening. It does not change detection, theft rolls, crime reporting or combat. It is not a confirmed fix. Reproduce an empty-session close/walk-away and inspect StandingQuickPocket.log. A combat event with zero recorded transfers distinguishes empty-session hostility from an actual item transfer, but does not itself identify which mod initiated combat.
