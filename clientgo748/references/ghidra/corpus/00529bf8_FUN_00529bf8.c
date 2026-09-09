// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00529bf8 | Name: FUN_00529bf8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00529bf8(int param_1,int param_2)

{
  int iVar1;
  void *pvVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int local_cc;
  int local_c4;
  int local_c0;
  int local_bc;
  int local_b8;
  undefined4 local_5c;
  int local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  int local_38;
  int local_34;
  int local_30;
  int *local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2328;
  local_10 = ExceptionList;
  local_18 = param_2;
  if (*(int *)(DAT_0067cf38 + 0x4c) == param_1) {
    if (*(short *)(param_1 + 0x450) < 1) {
      ExceptionList = &local_10;
      FUN_0052355c();
    }
  }
  else {
    local_14 = DAT_0067cf38;
    local_1c = DAT_0067cf38;
    ExceptionList = &local_10;
    if ((((DAT_0067cf38 != 0) && (ExceptionList = &local_10, iVar1 = FUN_0040c0f0(), iVar1 == 1)) &&
        (*(int *)(local_1c + 0x26e94) != 0)) &&
       (*(int *)(param_1 + 0x20) == *(int *)(*(int *)(local_1c + 0x26e94) + 0x20))) {
      (**(code **)(**(int **)(local_1c + 0x27c74) + 0x60))(0);
    }
    if ((local_14 != 0) && (*(int *)(local_14 + 0x24) == 30000)) {
      if ((*(int *)(param_1 + 0x20) == *(int *)(*(int *)(local_1c + 0x28730) + 0x404)) &&
         (*(int *)(*(int *)(local_1c + 0x285d4) + 0x28) == 1)) {
        FUN_004481c5(0);
      }
      if ((*(int *)(param_1 + 0x20) == *(int *)(local_1c + 0x275d8)) &&
         (*(int *)(*(int *)(local_1c + 0x27bdc) + 0x28) == 1)) {
        FUN_0044a3a9(0);
      }
      if (((*(uint *)(param_1 + 0x20) == (uint)*(ushort *)(local_1c + 0x275ae)) &&
          (local_24 = (**(code **)(**(int **)(local_1c + 0x28) + 0x48))(0x286), local_24 != 0)) &&
         (iVar1 = FUN_0040c0f0(), iVar1 == 1)) {
        FUN_0044ae38(0,0);
      }
      local_20 = *(int *)(local_1c + 0x2872c);
      if (((local_20 == 0) || (1 < *(int *)(local_20 + 0x210))) ||
         (*(char *)(param_1 + 0x248) != '\x01')) {
        if (local_20 != 0) {
          for (local_30 = 0; local_30 < *(int *)(local_20 + 0x210); local_30 = local_30 + 1) {
            local_34 = *(int *)(local_20 + 0x214 + local_30 * 4);
            if (*(int *)(local_34 + 0xe50) == *(int *)(param_1 + 0x20)) {
              local_38 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined4 *)(local_34 + 0xe50));
              if ((local_38 != 0) &&
                 (*(undefined1 *)(local_38 + 0x248) = 0, *(int *)(param_1 + 0x430) != 0)) {
                if (*(undefined4 **)(param_1 + 0x430) != (undefined4 *)0x0) {
                  (**(code **)**(undefined4 **)(param_1 + 0x430))(1);
                }
                *(undefined4 *)(param_1 + 0x430) = 0;
              }
              if (*(int *)(local_34 + 0xe5c) == 2) {
                *(undefined4 *)(local_34 + 0xe5c) = 4;
                *(undefined4 *)(local_34 + 0x754) = 0xff777777;
                FUN_00421b8f(local_34 + 0x76a,*(undefined4 *)(local_34 + 0x754),0);
              }
              else if (*(int *)(local_34 + 0xe5c) == 0) {
                *(undefined4 *)(local_34 + 0xe5c) = 3;
                *(undefined4 *)(local_34 + 0x754) = 0xff777777;
                FUN_00421b8f(local_34 + 0x76a,*(undefined4 *)(local_34 + 0x754),0);
              }
              break;
            }
          }
        }
      }
      else {
        FUN_004091ab();
        *(undefined1 *)(param_1 + 0x248) = 0;
        if (*(int *)(param_1 + 0x430) != 0) {
          if (*(undefined4 **)(param_1 + 0x430) != (undefined4 *)0x0) {
            (**(code **)**(undefined4 **)(param_1 + 0x430))(1);
          }
          *(undefined4 *)(param_1 + 0x430) = 0;
        }
        *(undefined1 *)(*(int *)(local_1c + 0x4c) + 0x248) = 0;
        if (DAT_005b892c != 2) {
          local_28 = 0;
          local_2c = *(int **)(local_1c + 0x27d8c);
          (**(code **)(*local_2c + 0x74))(0x42e40000,_DAT_005a4374 * 0.0 + _DAT_005a4370);
          (**(code **)(*local_2c + 0x70))(0,_DAT_005a436c - (float)local_28 * _DAT_005a4374);
          FUN_0040981b(0x42e40000,(float)local_28 * _DAT_005a4374);
        }
      }
    }
    if (*(int *)(param_1 + 0x254) == -1) {
      if (*(int *)(local_18 + 0xc) == 0) {
        if (param_1 != *(int *)(DAT_0067cf38 + 0x4c)) {
          *(undefined4 *)(param_1 + 0x254) = *(undefined4 *)(local_18 + 0xc);
        }
      }
      else if ((*(int *)(local_18 + 0xc) == 1) || (*(int *)(param_1 + 0x5c) == 0x25)) {
        local_3c = (**(code **)(*DAT_0092e654 + 8))();
        *(undefined4 *)(param_1 + 0x254) = *(undefined4 *)(local_18 + 0xc);
        if (*(int *)(param_1 + 0x5f8) == 0) {
          *(undefined4 *)(param_1 + 0x5f8) = local_3c;
        }
        *(undefined2 *)(param_1 + 0x450) = 0;
        FUN_0052355c();
      }
      else if (*(int *)(local_18 + 0xc) == 2) {
        *(undefined4 *)(param_1 + 0x254) = *(undefined4 *)(local_18 + 0xc);
        local_4c = 0;
        uVar4 = *(undefined4 *)(param_1 + 0x2c);
        iVar1 = FUN_0049c70f(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c));
        FUN_00430f50(*(undefined4 *)(param_1 + 0x28),(float)iVar1 * _DAT_005a4214 + _DAT_005a439c,
                     uVar4);
        if (*(char *)(param_1 + 0x795) == '\0') {
          if ((((*(int *)(param_1 + 0x24c) == 1) || (*(int *)(param_1 + 0x24c) == 2)) ||
              (*(int *)(param_1 + 0x24c) == 4)) ||
             ((*(int *)(param_1 + 0x24c) == 8 || (*(int *)(param_1 + 0x24c) == 0x1a)))) {
            pvVar2 = operator_new(0x94);
            local_8 = 0;
            if (pvVar2 == (void *)0x0) {
              local_b8 = 0;
            }
            else {
              local_b8 = FUN_004d8ef0(local_48,local_44,local_40,0,0);
            }
            local_8 = 0xffffffff;
            if (local_b8 != 0) {
              FUN_0054ac09(local_b8);
            }
            pvVar2 = operator_new(0x124);
            local_8 = 1;
            if (pvVar2 == (void *)0x0) {
              local_bc = 0;
            }
            else {
              local_bc = FUN_004cb900(1,2000,0x3f000000,0x3f000000,0x3f000000,0x3b03126f,0);
            }
            local_8 = 0xffffffff;
            local_4c = local_bc;
            if (local_bc != 0) {
              *(undefined4 *)(local_bc + 0x6c) = 1;
            }
          }
          else if ((((*(int *)(param_1 + 0x5c) == 0x15) || (*(int *)(param_1 + 0x5c) == 0x16)) ||
                   ((*(int *)(param_1 + 0x5c) == 0x17 ||
                    (((*(int *)(param_1 + 0x5c) == 0x18 || (*(int *)(param_1 + 0x5c) == 2)) ||
                     (*(int *)(param_1 + 0x5c) == 3)))))) || (*(int *)(param_1 + 0x5c) == 4)) {
            pvVar2 = operator_new(0x94);
            local_8 = 2;
            if (pvVar2 == (void *)0x0) {
              local_c0 = 0;
            }
            else {
              local_c0 = FUN_004d8ef0(local_48,local_44,local_40,1,0);
            }
            local_8 = 0xffffffff;
            if (local_c0 != 0) {
              FUN_0054ac09(local_c0);
            }
          }
          else if (*(int *)(param_1 + 0x5c) == 0x14) {
            pvVar2 = operator_new(0x94);
            local_8 = 3;
            if (pvVar2 == (void *)0x0) {
              local_c4 = 0;
            }
            else {
              local_c4 = FUN_004d8ef0(local_48,local_44,local_40,1,0);
            }
            local_8 = 0xffffffff;
            if (local_c4 != 0) {
              FUN_0054ac09(local_c4);
            }
          }
          if (local_4c != 0) {
            FUN_004310a0(&local_48);
            FUN_0054ac09(local_4c);
          }
        }
        if (*(int *)(param_1 + 0x5f8) == 0) {
          iVar1 = (**(code **)(*DAT_0092e654 + 8))();
          *(int *)(param_1 + 0x5f8) = iVar1 + 1000;
        }
        if (((DAT_005ccf98 != 0) && (*(int *)(DAT_013b71e8[0x6c21] + 0x54) == param_1)) &&
           (iVar1 = FUN_00429a6d(4), iVar1 != 0)) {
          uVar5 = 0;
          uVar4 = 0;
          FUN_00429a6d(4);
          FUN_0042ad2b(uVar4,uVar5);
        }
      }
      else if (*(int *)(param_1 + 0x254) == 3) {
        if (param_1 != *(int *)(DAT_0067cf38 + 0x4c)) {
          FUN_004f9b6f();
        }
      }
      else {
        local_5c = 1;
        if (999 < *(uint *)(param_1 + 0x20)) {
          local_5c = 3;
        }
        if (*(char *)(param_1 + 0x795) == '\0') {
          pvVar2 = operator_new(0xac);
          local_8 = 4;
          if (pvVar2 == (void *)0x0) {
            local_cc = 0;
          }
          else {
            puVar3 = (undefined4 *)
                     FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                                  *(undefined4 *)(param_1 + 0x2c));
            local_cc = FUN_004ee730(*puVar3,puVar3[1],puVar3[2],local_5c);
          }
          local_8 = 0xffffffff;
          if (local_cc != 0) {
            FUN_0054ac09(local_cc);
          }
        }
        if (param_1 != *(int *)(DAT_0067cf38 + 0x4c)) {
          FUN_004f9b6f();
        }
      }
    }
    else if (*(int *)(param_1 + 0x254) == 1) {
      if ((*(int *)(param_1 + 0x254) == 1) && (*(int *)(param_1 + 0x5f8) == 0)) {
        *(undefined1 *)(param_1 + 0x245) = 0;
        FUN_0052355c();
        if (*(int *)(param_1 + 0x5f8) == 0) {
          iVar1 = (**(code **)(*DAT_0092e654 + 8))();
          *(int *)(param_1 + 0x5f8) = iVar1 + 1000;
        }
      }
    }
    else if (param_1 != *(int *)(DAT_0067cf38 + 0x4c)) {
      FUN_004f9b6f();
    }
  }
  ExceptionList = local_10;
  return 1;
}

