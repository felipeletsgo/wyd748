// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059d543 | Name: FUN_0059d543


void FUN_0059d543(int *param_1,int *param_2)

{
  short sVar1;
  int iVar2;
  ushort uVar3;
  int *piVar4;
  int *piVar5;
  ushort uVar6;
  int iVar7;
  ushort uVar8;
  ushort uVar9;
  byte local_28;
  undefined1 uStack_27;
  undefined2 uStack_26;
  short local_24;
  undefined2 uStack_22;
  undefined2 local_20;
  undefined1 uStack_1e;
  byte bStack_1d;
  int local_1c;
  int local_18;
  int local_14;
  ushort *local_10;
  ushort *local_c;
  short *local_8;
  
  piVar5 = param_2;
  piVar4 = param_1;
  local_18 = 0;
  local_28 = 0;
  uStack_27 = 0;
  uStack_26 = 0;
  local_24 = 0;
  uStack_22 = 0;
  local_20 = 0;
  uStack_1e = 0;
  bStack_1d = 0;
  uVar6 = *(ushort *)((int)param_1 + 10) & 0x7fff;
  uVar8 = *(ushort *)((int)param_2 + 10) & 0x7fff;
  uVar9 = (*(ushort *)((int)param_2 + 10) ^ *(ushort *)((int)param_1 + 10)) & 0x8000;
  uVar3 = uVar8 + uVar6;
  if (((uVar6 < 0x7fff) && (uVar8 < 0x7fff)) && (uVar3 < 0xbffe)) {
    if (uVar3 < 0x3fc0) {
LAB_0059d5e6:
      param_1[2] = 0;
      param_1[1] = 0;
      *param_1 = 0;
      return;
    }
    if (((uVar6 != 0) || (uVar3 = uVar3 + 1, (param_1[2] & 0x7fffffffU) != 0)) ||
       ((uVar6 = 0, param_1[1] != 0 || (*param_1 != 0)))) {
      if (((uVar8 == 0) && (uVar3 = uVar3 + 1, (param_2[2] & 0x7fffffffU) == 0)) &&
         ((param_2[1] == 0 && (*param_2 == 0)))) goto LAB_0059d5e6;
      local_14 = 0;
      local_8 = &local_24;
      param_2 = (int *)0x5;
      do {
        if (0 < (int)param_2) {
          local_c = (ushort *)(local_14 * 2 + (int)param_1);
          local_10 = (ushort *)(piVar5 + 2);
          local_1c = (int)param_2;
          do {
            iVar7 = FUN_0059cb91(*(undefined4 *)(local_8 + -2),(uint)*local_c * (uint)*local_10,
                                 local_8 + -2);
            if (iVar7 != 0) {
              *local_8 = *local_8 + 1;
            }
            local_c = local_c + 1;
            local_10 = local_10 + -1;
            local_1c = local_1c + -1;
          } while (local_1c != 0);
        }
        local_8 = local_8 + 1;
        local_14 = local_14 + 1;
        param_2 = (int *)((int)param_2 + -1);
      } while (0 < (int)param_2);
      param_1._0_2_ = uVar3 + 0xc002;
      if ((short)(ushort)param_1 < 1) {
LAB_0059d69a:
        param_1._0_2_ = (ushort)param_1 - 1;
        if ((short)(ushort)param_1 < 0) {
          iVar7 = -(int)(short)(ushort)param_1;
          param_1._0_2_ = (ushort)param_1 + (short)iVar7;
          do {
            if ((local_28 & 1) != 0) {
              local_18 = local_18 + 1;
            }
            FUN_0059cc3e(&local_28);
            iVar7 = iVar7 + -1;
          } while (iVar7 != 0);
          if (local_18 != 0) {
            local_28 = local_28 | 1;
          }
        }
      }
      else {
        do {
          if ((bStack_1d & 0x80) != 0) break;
          FUN_0059cc10(&local_28);
          param_1._0_2_ = (ushort)param_1 - 1;
        } while (0 < (short)(ushort)param_1);
        if ((short)(ushort)param_1 < 1) goto LAB_0059d69a;
      }
      if ((0x8000 < CONCAT11(uStack_27,local_28)) ||
         (sVar1 = CONCAT11(bStack_1d,uStack_1e), iVar2 = CONCAT22(local_20,uStack_22),
         iVar7 = CONCAT22(local_24,uStack_26),
         (CONCAT22(uStack_26,CONCAT11(uStack_27,local_28)) & 0x1ffff) == 0x18000)) {
        if (CONCAT22(local_24,uStack_26) == -1) {
          iVar7 = 0;
          if (CONCAT22(local_20,uStack_22) == -1) {
            if (CONCAT11(bStack_1d,uStack_1e) == -1) {
              param_1._0_2_ = (ushort)param_1 + 1;
              sVar1 = -0x8000;
              iVar2 = 0;
              iVar7 = 0;
            }
            else {
              sVar1 = CONCAT11(bStack_1d,uStack_1e) + 1;
              iVar2 = 0;
              iVar7 = 0;
            }
          }
          else {
            sVar1 = CONCAT11(bStack_1d,uStack_1e);
            iVar2 = CONCAT22(local_20,uStack_22) + 1;
          }
        }
        else {
          iVar7 = CONCAT22(local_24,uStack_26) + 1;
          sVar1 = CONCAT11(bStack_1d,uStack_1e);
          iVar2 = CONCAT22(local_20,uStack_22);
        }
      }
      local_24 = (short)((uint)iVar7 >> 0x10);
      uStack_26 = (undefined2)iVar7;
      local_20 = (undefined2)((uint)iVar2 >> 0x10);
      uStack_22 = (undefined2)iVar2;
      bStack_1d = (byte)((ushort)sVar1 >> 8);
      uStack_1e = (undefined1)sVar1;
      if (0x7ffe < (ushort)param_1) goto LAB_0059d743;
      uVar6 = (ushort)param_1 | uVar9;
      *(undefined2 *)piVar4 = uStack_26;
      *(uint *)((int)piVar4 + 2) = CONCAT22(uStack_22,local_24);
      *(uint *)((int)piVar4 + 6) = CONCAT13(bStack_1d,CONCAT12(uStack_1e,local_20));
    }
    *(ushort *)((int)piVar4 + 10) = uVar6;
  }
  else {
LAB_0059d743:
    piVar4[1] = 0;
    *piVar4 = 0;
    piVar4[2] = (-(uint)(uVar9 != 0) & 0x80000000) + 0x7fff8000;
  }
  return;
}

