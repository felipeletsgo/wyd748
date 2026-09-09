// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00481c77 | Name: FUN_00481c77


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00481c77(int param_1,int param_2)

{
  size_t sVar1;
  undefined4 uVar2;
  undefined1 local_14 [8];
  float local_c;
  undefined4 local_8;
  
  if ((*(int *)(param_1 + 0x27c70) != 0) && (param_2 != 0)) {
    FUN_00430f10();
    FUN_0040c110(local_14);
    sVar1 = _strlen((char *)(*(int *)(param_2 + 0x1b0) + 0x18));
    if (DAT_005b490c == 0x3f4ccccd) {
      local_c = _DAT_005a4370 - (float)(int)(sVar1 - 1) * _DAT_005a40f0;
      local_8 = 0x40000000;
    }
    else if (DAT_005b490c == 0x3fa3d70a) {
      local_c = _DAT_005a4410 - (float)(int)(sVar1 - 1) * _DAT_005a3834;
      local_8 = 0x40a00000;
    }
    else if (DAT_005b490c == 0x3fcccccd) {
      local_c = _DAT_005a440c - (float)(int)(sVar1 - 1) * _DAT_005a4104;
      local_8 = 0x40a00000;
    }
    else if (DAT_005b490c == 0x40000000) {
      local_c = _DAT_005a4408 - (float)(int)(sVar1 - 1) * _DAT_005a4104;
      local_8 = 0x40a00000;
    }
    uVar2 = __ftol();
    *(undefined4 *)(*(int *)(param_2 + 0x1b0) + 0x1a0) = uVar2;
    uVar2 = __ftol();
    *(undefined4 *)(*(int *)(param_2 + 0x1b0) + 0x1a4) = uVar2;
  }
  return;
}

