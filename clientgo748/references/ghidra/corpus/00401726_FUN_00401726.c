// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00401726 | Name: FUN_00401726


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_00401726(int param_1,undefined4 param_2,uint param_3,int param_4,int param_5)

{
  undefined4 uVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 0x3c) == 0) {
    uVar1 = 0;
  }
  else {
    iVar2 = FUN_00401000(param_4,param_5,*(undefined4 *)(param_1 + 0x4c),
                         *(undefined4 *)(param_1 + 0x50),*(undefined4 *)(param_1 + 0x54),0x41c00000)
    ;
    uVar1 = FUN_00401000(param_4,param_5,*(undefined4 *)(param_1 + 0x4c),
                         *(undefined4 *)(param_1 + 0x50),*(undefined4 *)(param_1 + 0x54),
                         *(undefined4 *)(param_1 + 0x58));
    *(undefined4 *)(param_1 + 0x34) = uVar1;
    if ((*(int *)(param_1 + 0x34) == 0) && (*(int *)(param_1 + 0x1d0) != 0)) {
      (**(code **)(**(int **)(param_1 + 0x1d0) + 0x60))(0);
    }
    switch(param_2) {
    case 0x200:
      if ((*(int *)(param_1 + 0x1e0) != 0) && (*(int *)(param_1 + 0x1dc) != 0)) {
        *(float *)(param_1 + 0x4c) =
             (float)(param_4 - *(int *)(param_1 + 0x1d4)) + *(float *)(param_1 + 0x4c);
        *(float *)(param_1 + 0x50) =
             (float)(param_5 - *(int *)(param_1 + 0x1d8)) + *(float *)(param_1 + 0x50);
        if (*(float *)(param_1 + 0x4c) < _DAT_005a3534) {
          *(undefined4 *)(param_1 + 0x4c) = 0;
        }
        if (*(float *)(param_1 + 0x50) < _DAT_005a3534) {
          *(undefined4 *)(param_1 + 0x50) = 0;
        }
        if ((float)*(uint *)(DAT_013b71e0 + 0x10) <
            *(float *)(param_1 + 0x4c) + *(float *)(param_1 + 0x54)) {
          *(float *)(param_1 + 0x4c) =
               (float)*(uint *)(DAT_013b71e0 + 0x10) - *(float *)(param_1 + 0x54);
        }
        if ((float)*(uint *)(DAT_013b71e0 + 0x14) <
            *(float *)(param_1 + 0x50) + *(float *)(param_1 + 0x58)) {
          *(float *)(param_1 + 0x50) =
               (float)*(uint *)(DAT_013b71e0 + 0x14) - *(float *)(param_1 + 0x58);
        }
        *(int *)(param_1 + 0x1d4) = param_4;
        *(int *)(param_1 + 0x1d8) = param_5;
      }
      if ((*(int *)(param_1 + 0x34) == 1) && ((param_3 & 1) == 1)) {
        return 1;
      }
      if ((*(int *)(param_1 + 0x34) == 1) && (*(int *)(param_1 + 0x1d0) != 0)) {
        (**(code **)(**(int **)(param_1 + 0x1d0) + 0x60))(1);
      }
      break;
    case 0x201:
      if (((*(int *)(param_1 + 0x1dc) != 0) && (iVar2 != 0)) &&
         (*(int *)(*(int *)(DAT_0067cf38 + 0x28) + 0x130) == 0)) {
        *(int *)(*(int *)(DAT_0067cf38 + 0x28) + 0x130) = param_1;
        *(undefined4 *)(param_1 + 0x1e0) = 1;
        *(int *)(param_1 + 0x1d4) = param_4;
        *(int *)(param_1 + 0x1d8) = param_5;
      }
      break;
    case 0x202:
      if (param_1 == *(int *)(*(int *)(DAT_0067cf38 + 0x28) + 0x130)) {
        *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x28) + 0x130) = 0;
      }
      if (*(int *)(param_1 + 0x1dc) != 0) {
        *(undefined4 *)(param_1 + 0x1e0) = 0;
      }
      break;
    default:
      if ((*(int *)(param_1 + 0x60) == 1) && (*(int *)(param_1 + 0x34) == 1)) {
        return 1;
      }
      uVar1 = FUN_0040bf90(param_2,param_3,param_4,param_5);
      return uVar1;
    case 0x204:
      if (*(int *)(param_1 + 0x34) == 1) {
        return 1;
      }
    }
    if ((DAT_005b892c == 2) && (*(int *)(param_1 + 0x34) == 1)) {
      uVar1 = 1;
    }
    else if ((*(int *)(param_1 + 0x60) == 1) && (*(int *)(param_1 + 0x34) == 1)) {
      uVar1 = 1;
    }
    else {
      uVar1 = 0;
    }
  }
  return uVar1;
}

