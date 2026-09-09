// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00488ea9 | Name: FUN_00488ea9


undefined4 __thiscall FUN_00488ea9(undefined4 param_1,int param_2)

{
  *(int *)(DAT_013b71e8 + 0xc58) = *(int *)(DAT_013b71e8 + 0xc58) + *(int *)(param_2 + 0xc);
  *(int *)(DAT_013b71e8 + 0x704) = *(int *)(DAT_013b71e8 + 0x704) - *(int *)(param_2 + 0xc);
  FUN_004431e4(0,param_1);
  return 1;
}

