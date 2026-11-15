; Compile-only SkyUI API declarations. Never install or compile this file to Data.
Scriptname SKI_ConfigBase extends Quest Hidden
String Property ModName Auto
Int Property TOP_TO_BOTTOM = 2 AutoReadOnly
Int Property OPTION_FLAG_WITH_UNMAP = 4 AutoReadOnly
Function SetCursorFillMode(Int fillMode)
EndFunction
Int Function AddHeaderOption(String text, Int flags = 0)
EndFunction
Int Function AddKeyMapOption(String text, Int keyCode, Int flags = 0)
EndFunction
Int Function AddTextOption(String text, String value, Int flags = 0)
EndFunction
Bool Function ShowMessage(String message, Bool withCancel = true, String acceptLabel = "$Accept", String cancelLabel = "$Cancel")
EndFunction
Function SetKeyMapOptionValue(Int option, Int keyCode, Bool noUpdate = false)
EndFunction
Function SetInfoText(String text)
EndFunction
Event OnConfigInit()
EndEvent
Event OnPageReset(String page)
EndEvent
Event OnOptionKeyMapChange(Int option, Int keyCode, String conflictControl, String conflictName)
EndEvent
Event OnOptionDefault(Int option)
EndEvent
Event OnOptionHighlight(Int option)
EndEvent
Int Function GetVersion()
EndFunction
Int Function AddToggleOption(String text, Bool checked, Int flags = 0)
EndFunction
Function SetToggleOptionValue(Int option, Bool checked, Bool noUpdate = false)
EndFunction
Event OnOptionSelect(Int option)
EndEvent
Int Function AddSliderOption(String text, Float value, String format = "{0}", Int flags = 0)
EndFunction
Function SetSliderOptionValue(Int option, Float value, String format = "{0}", Bool noUpdate = false)
EndFunction
Function SetSliderDialogStartValue(Float value)
EndFunction
Function SetSliderDialogDefaultValue(Float value)
EndFunction
Function SetSliderDialogRange(Float minimum, Float maximum)
EndFunction
Function SetSliderDialogInterval(Float value)
EndFunction
Event OnOptionSliderOpen(Int option)
EndEvent
Event OnOptionSliderAccept(Int option, Float value)
EndEvent
Int Property OPTION_FLAG_DISABLED = 1 AutoReadOnly
Function ForcePageReset()
EndFunction
