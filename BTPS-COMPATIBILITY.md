# Standing Pickpocket 1.0.3 and BTPS

Install Better Third Person Selection separately. Standing Pickpocket uses Skyrim's active selected reference, which BTPS updates. No BTPS selection, UI, hook or asset code is included.

With BTPS installed, the pickpocket hotkey and existing pickpocket range logic enforce the configured physical reach even at a 1.0 distance multiplier. Existing lower multipliers and external distance overrides continue to apply. BTPS's selection distance can therefore exceed the distance at which the pickpocket key operates.

Standing Pickpocket's runtime filename, settings, API and normal activation path are preserved. No separate compatibility DLL is required. BTPS integration status is written to StandingPickpocket.log at startup.

BTPS source inspected for interoperability:
https://gitlab.com/Shrimperator/skyrim-mod-betterthirdpersonselection
Its README identifies GPL-3.0-or-later. Shrimperator retains ownership of BTPS. This package includes independently written Standing Pickpocket changes only.

Native build and existing tests must pass. Actual third-person selection and target switching require in-game verification.
