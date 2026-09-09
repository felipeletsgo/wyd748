// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00452631 | Name: FUN_00452631


undefined4 __thiscall FUN_00452631(undefined4 param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == 'w') || (param_2 == 'W')) {
    FUN_0044f46b(param_1);
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

