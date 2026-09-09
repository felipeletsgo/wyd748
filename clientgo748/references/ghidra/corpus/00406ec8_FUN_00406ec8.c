// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00406ec8 | Name: FUN_00406ec8


undefined4 __fastcall FUN_00406ec8(int param_1)

{
  undefined4 uVar1;
  
  if ((*(int *)(param_1 + 0x30) == 1) && (*(int *)(param_1 + 0x105c) == 0)) {
    if (DAT_005d0504 != 0) {
      uVar1 = FUN_004aea2c();
      *(undefined4 *)(param_1 + 0x1060) = uVar1;
      if (DAT_0067cf38 != 0) {
        if (*(int *)(param_1 + 0x1060) == 0) {
          (**(code **)(**(int **)(DAT_0067cf38 + 0x60) + 0x88))(&DAT_005b10b4);
        }
        else {
          (**(code **)(**(int **)(DAT_0067cf38 + 0x60) + 0x88))(&DAT_005b10b0);
        }
      }
    }
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

