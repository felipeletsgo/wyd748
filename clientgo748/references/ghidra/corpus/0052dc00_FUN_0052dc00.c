// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052dc00 | Name: FUN_0052dc00


undefined4 __thiscall FUN_0052dc00(int param_1,int param_2)

{
  int iVar1;
  
  if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
    *(undefined4 *)(DAT_013b71e8 + 0x704) = *(undefined4 *)(param_2 + 0xc);
    iVar1 = FUN_00494dcf();
    if (iVar1 == 30000) {
      FUN_004431e4(0,param_1,DAT_0067cf38);
    }
  }
  return 1;
}

