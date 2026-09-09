// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040fc3e | Name: FUN_0040fc3e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_0040fc3e(int param_1,undefined4 param_2,float param_3,float param_4,int param_5,
            undefined4 param_6)

{
  int *piVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 uVar4;
  int local_18;
  int local_10;
  int local_c;
  int local_8;
  
  *(float *)(param_1 + 0x68) = param_3 + *(float *)(param_1 + 0x4c);
  *(float *)(param_1 + 0x6c) = param_4 + *(float *)(param_1 + 0x50);
  if (-1 < *(int *)(param_1 + 0xa0)) {
    FUN_00401637(param_2,param_3,param_4,param_5,param_6);
  }
  if (*(int *)(param_1 + 0x1f8) == 1) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x1e8); local_8 = local_8 + 1) {
      for (local_c = 0; local_c < *(int *)(param_1 + 0x1ec); local_c = local_c + 1) {
        if (*(int *)(*(int *)(param_1 + 0x1f0) + (local_c + local_8 * *(int *)(param_1 + 0x1ec)) * 4
                    ) == 0) {
          *(float *)(*(int *)(param_1 + 500) + 4 +
                    (local_c + local_8 * *(int *)(param_1 + 0x1ec)) * 0x16c) =
               (float)local_c *
               *(float *)(*(int *)(param_1 + 500) + 0xc +
                         (local_c + local_8 * *(int *)(param_1 + 0x1ec)) * 0x16c) +
               param_3 + *(float *)(param_1 + 0x4c);
          *(float *)(*(int *)(param_1 + 500) + 8 +
                    (local_c + local_8 * *(int *)(param_1 + 0x1ec)) * 0x16c) =
               (float)local_8 *
               *(float *)(*(int *)(param_1 + 500) + 0x10 +
                         (local_c + local_8 * *(int *)(param_1 + 0x1ec)) * 0x16c) +
               param_4 + *(float *)(param_1 + 0x50);
          *(int *)(*(int *)(param_1 + 500) + 0x148 +
                  (local_c + local_8 * *(int *)(param_1 + 0x1ec)) * 0x16c) = param_5;
          FUN_0040c26d(param_2,*(int *)(param_1 + 500) +
                               (local_c + local_8 * *(int *)(param_1 + 0x1ec)) * 0x16c,param_5);
        }
      }
    }
  }
  if (*(int *)(param_1 + 0x578) != 0) {
    if ((*(int *)(param_1 + 0x1e4) == 0) &&
       (((*(int *)(param_1 + 0x400) == 0 || (*(int *)(param_1 + 0x400) == 2)) ||
        (*(int *)(param_1 + 0x400) == 3)))) {
      *(float *)(param_1 + 0x410) =
           (float)*(int *)(param_1 + 0x57c) * *(float *)(*(int *)(param_1 + 500) + 0xc) +
           param_3 + *(float *)(param_1 + 0x4c);
      *(float *)(param_1 + 0x414) =
           (float)*(int *)(param_1 + 0x580) * *(float *)(*(int *)(param_1 + 500) + 0x10) +
           param_4 + *(float *)(param_1 + 0x50);
      if ((DAT_005b892c == 2) && (*(int *)(param_1 + 0x1f8) == 0)) {
        *(float *)(param_1 + 0x418) = (float)(*(int *)(param_1 + 0x584) * DAT_005b1094);
        *(float *)(param_1 + 0x41c) = (float)(*(int *)(param_1 + 0x588) * DAT_005b1094);
      }
      else if (DAT_005b892c == 2) {
        *(float *)(param_1 + 0x418) =
             (float)(*(int *)(param_1 + 0x584) * DAT_005b1094) / DAT_005b490c;
        *(float *)(param_1 + 0x41c) =
             (float)(*(int *)(param_1 + 0x588) * DAT_005b1094) / DAT_005b4910;
      }
      else {
        *(float *)(param_1 + 0x418) = (float)(*(int *)(param_1 + 0x584) * DAT_005b1094);
        *(float *)(param_1 + 0x41c) = (float)(*(int *)(param_1 + 0x588) * DAT_005b1094);
      }
    }
    else {
      *(float *)(param_1 + 0x410) = param_3 + *(float *)(param_1 + 0x4c);
      *(float *)(param_1 + 0x414) = param_4 + *(float *)(param_1 + 0x50);
      if (((DAT_005b892c == 2) && (*(int *)(param_1 + 0x1f8) == 0)) ||
         (*(int *)(param_1 + 0x400) == 1)) {
        *(undefined4 *)(param_1 + 0x418) = *(undefined4 *)(param_1 + 0x54);
        *(undefined4 *)(param_1 + 0x41c) = *(undefined4 *)(param_1 + 0x58);
      }
      else if (DAT_005b892c == 2) {
        *(float *)(param_1 + 0x418) = *(float *)(param_1 + 0x54) / DAT_005b490c;
        *(float *)(param_1 + 0x41c) = *(float *)(param_1 + 0x58) / DAT_005b4910;
      }
      else {
        *(undefined4 *)(param_1 + 0x418) = *(undefined4 *)(param_1 + 0x54);
        *(undefined4 *)(param_1 + 0x41c) = *(undefined4 *)(param_1 + 0x58);
      }
    }
    *(int *)(param_1 + 0x554) = param_5;
    *(undefined4 *)(param_1 + 0x43c) = *(undefined4 *)(param_1 + 0x578);
    FUN_0040c26d(param_2,param_1 + 0x40c,param_5);
  }
  if (*(int *)(param_1 + 0x400) == 5) {
    for (local_10 = 0; local_10 < *(int *)(param_1 + 0x1fc); local_10 = local_10 + 1) {
      piVar1 = *(int **)(param_1 + 0x200 + local_10 * 4);
      if (piVar1 != (int *)0x0) {
        uVar4 = 0;
        iVar3 = param_5;
        puVar2 = (undefined4 *)
                 FUN_00430f20(((float)piVar1[0x74] * *(float *)(param_1 + 0x54)) /
                              (float)*(int *)(param_1 + 0x1ec) +
                              param_3 + *(float *)(param_1 + 0x4c),
                              ((float)piVar1[0x75] * *(float *)(param_1 + 0x58)) /
                              (float)*(int *)(param_1 + 0x1e8) +
                              param_4 + *(float *)(param_1 + 0x50));
        (**(code **)(*piVar1 + 0x58))(param_2,*puVar2,puVar2[1],iVar3,uVar4);
        if ((_DAT_005a3534 < (float)piVar1[0x19d]) && ((float)piVar1[0x19d] < _DAT_005a3660)) {
          piVar1[0x7b] = (int)((float)piVar1[0x74] * *(float *)(*(int *)(param_1 + 500) + 0xc) +
                              param_3 + *(float *)(param_1 + 0x4c));
          piVar1[0x7c] = (int)((float)piVar1[0x75] * *(float *)(*(int *)(param_1 + 500) + 0x10) +
                               param_4 + *(float *)(param_1 + 0x50) + _DAT_005a3660);
          if (DAT_005b892c == 2) {
            piVar1[0x7d] = (int)(((_DAT_005a40f4 - (float)piVar1[0x19d]) *
                                 (float)(piVar1[0x76] * 0x15)) / DAT_005b490c);
            piVar1[0x7e] = (int)((float)(piVar1[0x77] * 0x15) / DAT_005b4910);
          }
          else {
            piVar1[0x7d] = (int)((_DAT_005a3660 - (float)piVar1[0x19d]) *
                                (float)(piVar1[0x76] * DAT_005b1094));
            piVar1[0x7e] = (int)(float)(piVar1[0x77] * DAT_005b1094);
          }
          piVar1[0xcc] = param_5;
          piVar1[0x86] = -0x56000000;
          FUN_0040c26d(param_2,piVar1 + 0x7a,param_5);
        }
      }
    }
  }
  else if (((((((*(int *)(param_1 + 0x1e4) == 0) && (*(int *)(param_1 + 0x400) != 8)) &&
              (*(int *)(param_1 + 0x400) != 9)) &&
             ((*(int *)(param_1 + 0x400) != 6 && (*(int *)(param_1 + 0x400) != 0xb)))) &&
            ((*(int *)(param_1 + 0x400) != 0xc &&
             ((*(int *)(param_1 + 0x400) != 0xe && (*(int *)(param_1 + 0x400) != 0x10)))))) &&
           (*(int *)(param_1 + 0x400) != 0x12)) &&
          ((*(int *)(param_1 + 0x400) != 0x14 && (*(int *)(param_1 + 0x400) != 0x16)))) {
    for (local_18 = 0; local_18 < *(int *)(param_1 + 0x1fc); local_18 = local_18 + 1) {
      piVar1 = *(int **)(param_1 + 0x200 + local_18 * 4);
      if (piVar1 != (int *)0x0) {
        uVar4 = 0;
        iVar3 = param_5;
        puVar2 = (undefined4 *)
                 FUN_00430f20(((float)piVar1[0x74] * *(float *)(param_1 + 0x54)) /
                              (float)*(int *)(param_1 + 0x1ec) +
                              param_3 + *(float *)(param_1 + 0x4c),
                              ((float)piVar1[0x75] * *(float *)(param_1 + 0x58)) /
                              (float)*(int *)(param_1 + 0x1e8) +
                              param_4 + *(float *)(param_1 + 0x50));
        (**(code **)(*piVar1 + 0x58))(param_2,*puVar2,puVar2[1],iVar3,uVar4);
        if (piVar1[0x25] == -0x10000) {
          piVar1[0x7b] = (int)((float)piVar1[0x74] * *(float *)(*(int *)(param_1 + 500) + 0xc) +
                              param_3 + *(float *)(param_1 + 0x4c));
          piVar1[0x7c] = (int)((float)piVar1[0x75] * *(float *)(*(int *)(param_1 + 500) + 0x10) +
                              param_4 + *(float *)(param_1 + 0x50));
          if ((DAT_005b892c == 2) && (*(int *)(param_1 + 0x1f8) == 0)) {
            piVar1[0x7d] = (int)(float)(piVar1[0x76] * DAT_005b1094);
            piVar1[0x7e] = (int)(float)(piVar1[0x77] * DAT_005b1094);
          }
          else if (DAT_005b892c == 2) {
            piVar1[0x7d] = (int)((float)(piVar1[0x76] * DAT_005b1094) / DAT_005b490c);
            piVar1[0x7e] = (int)((float)(piVar1[0x77] * DAT_005b1094) / DAT_005b4910);
          }
          else {
            piVar1[0x7d] = (int)(float)(piVar1[0x76] * DAT_005b1094);
            piVar1[0x7e] = (int)(float)(piVar1[0x77] * DAT_005b1094);
          }
          piVar1[0xcc] = param_5;
          piVar1[0x86] = 0x33ff0000;
          FUN_0040c26d(param_2,piVar1 + 0x7a,param_5);
        }
      }
    }
  }
  else if (*(int *)(param_1 + 0x200) != 0) {
    if (((*(int *)(param_1 + 0x400) == 9) || (*(int *)(param_1 + 0x400) == 6)) ||
       (*(int *)(param_1 + 0x400) == 0xb)) {
      *(float *)(*(int *)(param_1 + 0x200) + 0x1ec) = param_3 + *(float *)(param_1 + 0x4c);
      *(float *)(*(int *)(param_1 + 0x200) + 0x1f0) = param_4 + *(float *)(param_1 + 0x50);
      if ((DAT_005b892c == 2) && (*(int *)(param_1 + 0x1f8) == 0)) {
        *(float *)(*(int *)(param_1 + 0x200) + 500) = *(float *)(param_1 + 0x54) - _DAT_005a3834;
        *(float *)(*(int *)(param_1 + 0x200) + 0x1f8) = *(float *)(param_1 + 0x58) - _DAT_005a40f0;
      }
      else if (DAT_005b892c == 2) {
        *(float *)(*(int *)(param_1 + 0x200) + 500) =
             (*(float *)(param_1 + 0x54) - _DAT_005a3834) / DAT_005b490c;
        *(float *)(*(int *)(param_1 + 0x200) + 0x1f8) =
             (*(float *)(param_1 + 0x58) - _DAT_005a40f0) / DAT_005b4910;
      }
      else {
        *(float *)(*(int *)(param_1 + 0x200) + 500) = *(float *)(param_1 + 0x54) - _DAT_005a3834;
        *(float *)(*(int *)(param_1 + 0x200) + 0x1f8) = *(float *)(param_1 + 0x58) - _DAT_005a40f0;
      }
      *(int *)(*(int *)(param_1 + 0x200) + 0x330) = param_5;
      *(undefined4 *)(*(int *)(param_1 + 0x200) + 0x218) = 0x3300ff00;
      FUN_0040c26d(param_2,*(int *)(param_1 + 0x200) + 0x1e8,param_5);
    }
    uVar4 = 0;
    puVar2 = (undefined4 *)
             FUN_00430f20((*(float *)(param_1 + 0x54) - *(float *)(*(int *)(param_1 + 0x200) + 0x70)
                          ) / _DAT_005a365c + param_3 + *(float *)(param_1 + 0x4c),
                          (*(float *)(param_1 + 0x58) - *(float *)(*(int *)(param_1 + 0x200) + 0x74)
                          ) / _DAT_005a365c + param_4 + *(float *)(param_1 + 0x50));
    (**(code **)(**(int **)(param_1 + 0x200) + 0x58))(param_2,*puVar2,puVar2[1],param_5,uVar4);
  }
  return;
}

