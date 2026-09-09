// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00403683 | Name: FUN_00403683


int __thiscall
FUN_00403683(int param_1,undefined4 param_2,float param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6)

{
  float fVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  undefined1 local_18 [16];
  uint local_8;
  
  if (*(int *)(param_1 + 0x44) == 0x1214) {
    *(undefined4 *)(param_1 + 0x94) = 0xaa000077;
  }
  if (*(char *)(param_1 + 0x1ed) != '\0') {
    local_8 = (**(code **)(*DAT_0092e654 + 8))();
    if (DAT_005b892c == 2) {
      if ((int)(((ulonglong)local_8 / 0x2ee) % 2) == 0) {
        *(undefined4 *)(param_1 + 0xa4) = 0;
      }
      else {
        *(undefined4 *)(param_1 + 0xa4) = 1;
      }
    }
    else if ((int)(((ulonglong)local_8 / 0x2ee) % 2) != 0) {
      return (int)(((ulonglong)local_8 / 0x2ee) / 2);
    }
  }
  iVar2 = FUN_00401637(param_2,param_3,param_4,param_5,param_6);
  if (*(int *)(param_1 + 0x1f0) != 0) {
    iVar3 = FUN_0040c0f0();
    iVar2 = 0;
    if ((iVar3 != 0) &&
       ((0 < *(int *)(param_1 + 0xa0) || (iVar2 = param_1, *(int *)(param_1 + 0xa0) < -2)))) {
      uVar4 = FUN_00430f20(*(undefined4 *)(param_1 + 0x4c),*(undefined4 *)(param_1 + 0x50));
      uVar4 = FUN_0040bd60(local_18,uVar4);
      FUN_0040bd30(uVar4);
      if ((float)*(uint *)(DAT_005ccf9c + 0x2a504) <
          param_3 + *(float *)(*(int *)(param_1 + 0x1f0) + 0x4c) +
          *(float *)(*(int *)(param_1 + 0x1f0) + 0x54)) {
        fVar1 = *(float *)(*(int *)(param_1 + 0x1f0) + 0x4c);
        param_3 = ((float)*(uint *)(DAT_005ccf9c + 0x2a504) - (fVar1 + fVar1)) -
                  *(float *)(*(int *)(param_1 + 0x1f0) + 0x54);
      }
      iVar2 = (**(code **)(**(int **)(param_1 + 0x1f0) + 0x58))
                        (param_2,param_3,param_4,0x1c,param_6);
    }
  }
  return iVar2;
}

