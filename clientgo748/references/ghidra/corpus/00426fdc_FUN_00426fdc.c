// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00426fdc | Name: FUN_00426fdc


undefined4 FUN_00426fdc(DWORD param_1,DWORD param_2,DWORD param_3,DWORD param_4)

{
  LONG LVar1;
  undefined4 uVar2;
  DEVMODEA local_a4;
  
  FUN_00427048(&local_a4);
  local_a4.dmPelsWidth = param_1;
  local_a4.dmPelsHeight = param_2;
  local_a4.dmBitsPerPel = param_3;
  if (param_4 != 0) {
    local_a4.dmDisplayFrequency = param_4;
  }
  LVar1 = ChangeDisplaySettingsA(&local_a4,0);
  if ((LVar1 == 0) || (LVar1 == 1)) {
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

