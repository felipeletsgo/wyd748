// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00545838 | Name: FUN_00545838


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00545838(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  float10 fVar3;
  
  if (*(int *)(DAT_0067cf38 + 0x24) == 0x7534) {
    uVar1 = 1;
  }
  else {
    iVar2 = FUN_0053ea45();
    if (iVar2 == 1) {
      uVar1 = 1;
    }
    else {
      if ((*(int *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54) != 0) &&
         (fVar3 = (float10)FUN_004d0150(param_1 + 0x28), (float)fVar3 < _DAT_005a5608)) {
        *(undefined4 *)(param_1 + 0x48) = 1;
        return 1;
      }
      *(undefined4 *)(param_1 + 0x48) = 0;
      uVar1 = 0;
    }
  }
  return uVar1;
}

