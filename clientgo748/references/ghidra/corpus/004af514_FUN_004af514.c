// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004af514 | Name: FUN_004af514


void __thiscall FUN_004af514(undefined4 param_1,undefined1 param_2,undefined4 param_3)

{
  if (DAT_013b71e8 != (int *)0x0) {
    (**(code **)(*DAT_013b71e8 + 0x18))(param_2,param_3,param_1);
  }
  return;
}

