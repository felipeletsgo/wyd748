// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004afa19 | Name: FUN_004afa19


void __thiscall
FUN_004afa19(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            )

{
  *(undefined4 *)(param_1 + 0x44c) = param_3;
  if (DAT_005ccec0 != (int *)0x0) {
    (**(code **)(*DAT_005ccec0 + 0x88))(param_4,param_5);
  }
  if (DAT_013b71e8 != (int *)0x0) {
    (**(code **)(*DAT_013b71e8 + 0xc))(param_2,param_3,param_4,param_5);
  }
  return;
}

