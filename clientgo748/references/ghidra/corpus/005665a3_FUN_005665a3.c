// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005665a3 | Name: FUN_005665a3


undefined4 __thiscall FUN_005665a3(int *param_1,int *param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  void *pvVar3;
  int *piVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  int *piVar8;
  uint uVar9;
  int *piVar10;
  uint uVar11;
  uint uVar12;
  int *local_2c;
  uint local_24;
  int *local_1c;
  uint local_18;
  uint local_14;
  uint local_10;
  int *local_8;
  
  if (((param_3 < 4) || (*param_2 != 0x20534444)) || ((int)param_3 - 4U < 0x7c)) {
    return 0x80004005;
  }
  local_8 = param_2 + 0x20;
  param_1[3] = param_2[4];
  param_1[4] = param_2[3];
  if ((*(byte *)((int)param_2 + 10) & 0x80) == 0) {
    iVar1 = 1;
  }
  else {
    iVar1 = param_2[6];
  }
  param_1[5] = iVar1;
  if (iVar1 == 0) {
    param_1[5] = 1;
  }
  if ((param_2[0x1c] & 0xfc00U) == 0xfc00) {
    local_18 = 6;
  }
  else {
    if ((param_2[0x1c] & 0xfc00U) != 0) {
      return 0x80004005;
    }
    local_18 = 1;
  }
  local_24 = param_2[7];
  if (local_24 == 0) {
    local_24 = 1;
  }
  piVar10 = &DAT_005c52f8;
  iVar1 = DAT_005c52f8;
  while (iVar1 != 0) {
    if ((param_2[0x13] == 0x20) && (uVar11 = piVar10[2], param_2[0x14] == uVar11)) {
      if ((((((uVar11 & 4) == 0) || (param_2[0x15] == piVar10[3])) &&
           (((uVar11 & 0xc4462) == 0 || (param_2[0x16] == piVar10[4])))) &&
          (((uVar11 & 0xe4040) == 0 || (param_2[0x17] == piVar10[5])))) &&
         (((((uVar11 & 0xc4440) == 0 || (param_2[0x18] == piVar10[6])) &&
           (((uVar11 & 0xc4040) == 0 || (param_2[0x19] == piVar10[7])))) &&
          (((uVar11 & 0x80003) == 0 || (param_2[0x1a] == piVar10[8])))))) break;
    }
    piVar10 = piVar10 + 9;
    iVar1 = *piVar10;
  }
  iVar1 = *piVar10;
  if (iVar1 == 0) {
    return 0x80004005;
  }
  iVar2 = FUN_0055fab2(iVar1);
  if ((*(byte *)((int)param_2 + 10) & 0x80) == 0) {
    param_1[0x11] = (uint)(local_18 == 6) * 2 + 3;
  }
  else {
    param_1[0x11] = 4;
  }
  if (*(int *)(iVar2 + 4) == 1) {
    if ((int *)((int)param_3 - 0x80U) < (int *)0x400) {
      return 0x80004005;
    }
    param_3 = (int *)((int)param_3 - 0x480);
    local_2c = local_8;
    local_8 = param_2 + 0x120;
  }
  else {
    local_2c = (int *)0x0;
    param_3 = (int *)((int)param_3 - 0x80U);
  }
  local_14 = 0;
  piVar10 = param_3;
  if (local_18 != 0) {
    do {
      uVar11 = param_1[3];
      uVar9 = param_1[4];
      param_2 = (int *)param_1[5];
      piVar4 = param_1;
      if (local_14 != 0) {
        pvVar3 = operator_new(0x54);
        if (pvVar3 == (void *)0x0) {
          piVar4 = (int *)0x0;
        }
        else {
          piVar4 = (int *)FUN_00564a94();
        }
        if (piVar4 == (int *)0x0) {
          return 0x8007000e;
        }
        local_1c[0x14] = (int)piVar4;
      }
      local_10 = 0;
      piVar8 = piVar10;
      if (local_24 != 0) {
        do {
          piVar10 = piVar4;
          if (local_10 != 0) {
            pvVar3 = operator_new(0x54);
            if (pvVar3 == (void *)0x0) {
              piVar10 = (int *)0x0;
            }
            else {
              piVar10 = (int *)FUN_00564a94();
            }
            if (piVar10 == (int *)0x0) {
              return 0x8007000e;
            }
            piVar8[0x13] = (int)piVar10;
          }
          if (iVar1 == 0x31545844) {
            iVar5 = (uVar11 + 3 >> 2) << 3;
LAB_00566840:
            iVar7 = (uVar9 + 3 >> 2) * iVar5;
          }
          else {
            if (iVar1 == 0x32545844) {
LAB_0056682c:
              iVar5 = (uVar11 + 3 >> 2) << 4;
              goto LAB_00566840;
            }
            if (iVar1 == 0x32595559) {
LAB_0056681d:
              iVar5 = (uVar11 + 1 >> 1) << 2;
            }
            else {
              if (((iVar1 == 0x33545844) || (iVar1 == 0x34545844)) || (iVar1 == 0x35545844))
              goto LAB_0056682c;
              if (iVar1 == 0x59565955) goto LAB_0056681d;
              iVar5 = (*(uint *)(iVar2 + 8) >> 3) * uVar11;
            }
            iVar7 = iVar5 * uVar9;
          }
          piVar10[0xe] = 0;
          piVar10[0xf] = 0;
          piVar8 = (int *)(iVar7 * (int)param_2);
          *piVar10 = iVar1;
          piVar10[0xc] = iVar5;
          piVar10[1] = (int)local_8;
          piVar10[5] = (int)param_2;
          piVar10[0xd] = iVar7;
          piVar10[3] = uVar11;
          piVar10[4] = uVar9;
          piVar10[2] = (int)local_2c;
          if (param_3 < piVar8) {
            return 0x80004005;
          }
          local_8 = (int *)((int)local_8 + (int)piVar8);
          param_3 = (int *)((int)param_3 - (int)piVar8);
          uVar6 = 1;
          uVar12 = uVar6;
          if (uVar11 != 1) {
            uVar12 = uVar11 >> 1;
          }
          if (uVar9 != 1) {
            uVar6 = uVar9 >> 1;
          }
          if (param_2 == (int *)0x1) {
            param_2 = (int *)0x1;
          }
          else {
            param_2 = (int *)((uint)param_2 >> 1);
          }
          local_10 = local_10 + 1;
          uVar9 = uVar6;
          piVar8 = piVar10;
          uVar11 = uVar12;
        } while (local_10 < local_24);
      }
      local_14 = local_14 + 1;
      local_1c = piVar4;
    } while (local_14 < local_18);
  }
  return 0;
}

