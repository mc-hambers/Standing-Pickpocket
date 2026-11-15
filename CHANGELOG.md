# v1.0.2
- Remove artificial sneak-state changes from G-key pickpocketing to prevent stealth HUD/follower state leaking.
- Use a scoped target request and real-stance engine pickpocket menu instead.
- Preserve actual sneaking and separate follower/horse trade paths.
- Limit install ZIP to gameplay files, SEQ and LICENSE; API header stays in workspace.

# v1.0.1
- Separate follower and owned-horse inventory MCM toggles.
- Migrate the old combined preference to both toggles and retire the old setting.
- Horse teammates follow only the horse toggle.

# v1.0.0 (release numbering reset)
- Mark StandingPickpocket.esp as ESL/light; retain filename and local quest ID 0x800.
- Reset native metadata and package version to 1.0.0; include prior feature and crash-repair work.

## Earlier development version history

# v1.6.2
- Remove last-ridden-mount virtual lookup previously executed for every hotkey target.
- Only check horse ownership for non-teammate mounts.
- Flush hotkey stage logging to preserve the final checkpoint on crashes.
- Runtime crash resolution remains unconfirmed without game reproduction.

# v1.6.1
- Replace direct companion kNPCMode opening with engine Actor.OpenInventory dispatch to perform normal inventory setup.
- Fail without opening a menu if VM dispatch is unavailable; log rejected requests.

# v1.6.0
- Route hotkey pickpocket requests through current NPC activation processing rather than directly opening a menu.
- Add optional existing follower/owned-horse inventory access in NPC trade mode.

# v1.5.0
- Default binding G; Escape during rebinding unbinds. Preserve existing saved bindings.
- Add versioned native owner-controlled distance API and disabled MCM slider while overridden.

# v1.4.1
- Move Default: P from its separate MCM row into the pickpocket key description.

# v1.4.0
- Remove Dynamic Activation Key integration, mode settings, signal logging and rear-cone targeting.
- Keep native dedicated hotkey, vanilla activation toggle and distance multiplier.
- Automatically retire the saved DAK mode entry without resetting other preferences.

# v1.3.3
- Correct v1.3.2 dependency detection to support Papyrus DAK without its native DLL.
- Consume the same shared held-state global for both editions, keeping exclusive mode and distance rules.
- Add MCM key-state readout and gameplay key-transition logging.

# v1.3.2
- Require both the DAK native plugin and held-state global before enabling integration.
- Explain incomplete DAK installation in MCM and log a warning on save/new-game load when selected but unavailable.
- Log native dependency status alongside global resolution; preserve exclusive mode until explicitly disabled.

# Changelog

## v1.3.1
- Make DAK mode exclusive in native input, queued key actions and vanilla pickpocket activation.
- Grey out separate keybind and vanilla controls while DAK mode is selected, and restore previous preferences when disabled.
- Remove the HUD eligibility check against the previous crosshair handle, which could reject NPCs during prompt rebuilds.
- Refresh the HUD through the UI queue, matching current DAK's integration path.
- Preserve queued-key target revalidation separately from HUD eligibility.
- Allow disabling DAK mode even when the dependency is missing.
- Add native mode-policy regression assertions and DAK resolution logging.


## v1.3.0
- Add a native pickpocket distance multiplier and MCM slider (0.1-1.0; default/max 1.0).
- Apply the range to the separate key, ordinary sneaking pickpocket activation, and DAK activation/HUD.
- Preserve vanilla crosshair reach at 1.0 and ordinary talk reach at reduced settings.
- Replace the former 200-unit hardcoded cap with engine-targeted default reach and live activation-setting scaling.


## v1.2.0
- Add an optional native adapter for Dynamic Activation Key's held-modifier global.
- Holding DAK behind an eligible NPC changes normal Activate to Pickpocket in either stance.
- Use the same rear-cone/range conditions for HUD and activation; restore ordinary interaction on release or leaving the cone.
- Refresh the native crosshair on eligibility changes, including NPC turning.
- Add MCM mode/availability controls and behind-position regression checks; keep the separate key and previous modes.


## v1.1.1
- Correct the native plugin metadata to CommonLib's runtime-independent structure declaration, fixing SKSE's newer-AE startup rejection.
- Preserve the dedicated key, native interaction/HUD toggle and MCM options.

## v1.1.0
- Add the MCM "Disable vanilla pickpocket" option, off by default.
- Native NPC activation and HUD hooks use talk/ordinary interaction while sneaking when enabled.
- Keep the dedicated pickpocket key active in either stance.
- Persist the setting in the INI and add scoped-state restoration checks.

## v1.0.0
- Initial native standing/sneaking pickpocket key and MCM binding.






