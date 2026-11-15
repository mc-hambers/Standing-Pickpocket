Scriptname SP_MCM extends SKI_ConfigBase
Int keyOption
Int disableOption
Int followerOption
Int horseOption
Int distanceOption
Int Function GetVersion()
    Return 12
EndFunction
Event OnConfigInit()
    ModName = "Standing Pickpocket"
EndEvent
Event OnPageReset(String page)
    SetCursorFillMode(TOP_TO_BOTTOM)
    AddHeaderOption("Standing Pickpocket")
    keyOption = AddKeyMapOption("Pickpocket key", SP_Native.GetKey(), OPTION_FLAG_WITH_UNMAP)
    disableOption = AddToggleOption("Disable vanilla pickpocket", SP_Native.GetDisableVanilla())
    followerOption = AddToggleOption("Open follower inventory", SP_Native.GetFollowerInventory())
    horseOption = AddToggleOption("Open horse inventory", SP_Native.GetHorseInventory())
    Int distanceFlags = 0
    If SP_Native.IsDistanceOverridden()
        distanceFlags = OPTION_FLAG_DISABLED
    EndIf
    distanceOption = AddSliderOption("Pickpocket distance multiplier", SP_Native.GetDistanceMultiplier(), "{2}", distanceFlags)
EndEvent
Event OnOptionKeyMapChange(Int option, Int keyCode, String conflictControl, String conflictName)
    If option == keyOption
        If keyCode == 1 || keyCode == -1
            SP_Native.SetKey(-1)
            SetKeyMapOptionValue(keyOption, -1)
            Return
        EndIf
        If conflictControl != "" && !ShowMessage("This key conflicts with " + conflictName + ": " + conflictControl + ". Use it anyway?")
            Return
        EndIf
        SP_Native.SetKey(keyCode)
        SetKeyMapOptionValue(keyOption, SP_Native.GetKey())
    EndIf
EndEvent
Event OnOptionSelect(Int option)
    If option == followerOption
        SP_Native.SetFollowerInventory(!SP_Native.GetFollowerInventory())
        SetToggleOptionValue(followerOption, SP_Native.GetFollowerInventory())
    ElseIf option == horseOption
        SP_Native.SetHorseInventory(!SP_Native.GetHorseInventory())
        SetToggleOptionValue(horseOption, SP_Native.GetHorseInventory())
    ElseIf option == disableOption
        SP_Native.SetDisableVanilla(!SP_Native.GetDisableVanilla())
        SetToggleOptionValue(disableOption, SP_Native.GetDisableVanilla())
    EndIf
EndEvent
Event OnOptionSliderOpen(Int option)
    If option == distanceOption && !SP_Native.IsDistanceOverridden()
        SetSliderDialogStartValue(SP_Native.GetDistanceMultiplier())
        SetSliderDialogDefaultValue(1.0)
        SetSliderDialogRange(0.1, 1.0)
        SetSliderDialogInterval(0.05)
    EndIf
EndEvent
Event OnOptionSliderAccept(Int option, Float value)
    If option == distanceOption && !SP_Native.IsDistanceOverridden()
        SP_Native.SetDistanceMultiplier(value)
        SetSliderOptionValue(distanceOption, SP_Native.GetDistanceMultiplier(), "{2}")
    EndIf
EndEvent
Event OnOptionDefault(Int option)
    If option == keyOption
        SP_Native.SetKey(34)
        SetKeyMapOptionValue(keyOption, 34)
    ElseIf option == followerOption
        SP_Native.SetFollowerInventory(false)
        SetToggleOptionValue(followerOption, false)
    ElseIf option == horseOption
        SP_Native.SetHorseInventory(false)
        SetToggleOptionValue(horseOption, false)
    ElseIf option == disableOption
        SP_Native.SetDisableVanilla(false)
        SetToggleOptionValue(disableOption, false)
    ElseIf option == distanceOption && !SP_Native.IsDistanceOverridden()
        SP_Native.SetDistanceMultiplier(1.0)
        SetSliderOptionValue(distanceOption, 1.0, "{2}")
    EndIf
EndEvent
Event OnOptionHighlight(Int option)
    If option == keyOption
        SetInfoText("Default: G. Press Escape while binding to unbind. Aim at a living NPC and press this key to pickpocket while standing or sneaking. Uses the current game activation path and installed pickpocket rules.")
    ElseIf option == followerOption
        SetInfoText("Use the pickpocket key to open a current follower's existing inventory for trading, not pickpocketing.")
    ElseIf option == horseOption
        SetInfoText("Use the pickpocket key to open your owned horse's existing actor inventory, not pickpocketing. No new container is created.")
    ElseIf option == disableOption
        SetInfoText("Use normal talk/interaction and its HUD prompt while sneaking. Only your separate pickpocket key opens pickpocketing. Off restores vanilla activation. Saved across game restarts.")
    ElseIf option == distanceOption
        If SP_Native.IsDistanceOverridden()
            SetInfoText("Distance is controlled by another mod. Its override applies to pickpocket activation and the HUD.")
            Return
        EndIf
        SetInfoText("1.0 keeps vanilla reach. Lower values shorten pickpocket reach for the separate key, normal sneaking activation and the HUD prompt. Ordinary talking stays available.")
    EndIf
EndEvent


