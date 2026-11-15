# v1.4.0 validation
Build checks cover native stance restoration, vanilla toggle/range policy, compiled MCM and install archive structure.
Gameplay checks (not run):
- Upgrade with DynamicActivationKeyMode=1: dedicated key must work, MCM has no DAK options, and other preferences survive.
- P opens pickpocket while standing/sneaking; unbind and alternate key work.
- Disable vanilla pickpocket makes normal Activate talk with the matching HUD while sneaking.
- Distance multiplier affects dedicated key, vanilla pickpocket and HUD; normal talking remains available.
- Verify MCM update on existing saves, crime/success rules and SE/AE runtimes.

 v1.5.0: regression tests cover override ownership, invalid values, update and release. Gameplay still needs Escape/unbinding, fresh G versus saved bindings, MCM locking and another SKSE plugin calling the API.

 v1.6.0 gameplay checks pending: installed pickpocket overhaul restrictions/chances, standing and sneaking activation with vanilla toggle on/off, teammate trade and owned/last-ridden horse inventory with option on/off, unrelated horses excluded, no duplicate inventory, range still enforced.

 v1.6.1 crash-repair candidate: direct kNPCMode opening removed. Verify repeated follower/horse opens and closes, item transfers, empty inventories and installed inventory mods in game. No crash report located; runtime fix unconfirmed.

 v1.6.2: regression candidate removes QLastRiddenMount from the shared G-key path. Gameplay validation pending. Repeat G on NPCs/followers/owned horses and inspect flushed Inventory hotkey checkpoints if crash persists.

 v1.0.0 reset: verify TES4 ESL flag 0x200, quest local ID within 0x800-0xFFF, unchanged VMAD/SEQ and native version 1.0.0. In-game light-load order, existing saves and G-key crash resolution remain unverified.

 v1.0.1: user confirmed preceding G-key build works. Validate independent follower/horse toggles on/off, horse teammates using horse toggle, migration from combined setting and existing independent preferences. New split behavior not yet tested in game.

 v1.0.2: verify repeated standing G-key pickpocket opens/closes, stolen attempts, detection failure, cancel and save/load with a follower. No stealth icon or follower sneaking should begin from the hotkey. Genuine sneak before/during/after pickpocket must stay intact; follower/horse trade should never change it. Check ordinary Activate and Disable vanilla pickpocket toggle. Request-scope tests cover nesting, exceptions and both real stances. Runtime confirmation pending.
