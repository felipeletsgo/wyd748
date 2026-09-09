// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052e2f6 | Name: FUN_0052e2f6


undefined4 __fastcall FUN_0052e2f6(int param_1)

{
  int iVar1;
  int iVar2;
  
  if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
    *(undefined2 *)(DAT_013b71e8 + 0xcfa) = 0;
    *(undefined1 *)(DAT_013b71e8 + 0xcf8) = 0;
    DAT_005b12bc = 0xffff;
    iVar1 = FUN_00494dcf();
    iVar2 = DAT_0067cf38;
    if (iVar1 == 30000) {
      iVar1 = (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x48))(0x240);
      iVar2 = (**(code **)(**(int **)(iVar2 + 0x28) + 0x48))(0x286);
      if (iVar1 != 0) {
        iVar1 = FUN_0040c0f0();
        if (iVar1 == 1) {
          FUN_0044b890(0);
        }
      }
      if (iVar2 != 0) {
        iVar2 = FUN_0040c0f0();
        if (iVar2 == 1) {
          FUN_0044ae38(0,0);
        }
      }
    }
  }
  return 1;
}

