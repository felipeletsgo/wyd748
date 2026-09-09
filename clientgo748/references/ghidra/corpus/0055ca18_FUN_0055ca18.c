// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055ca18 | Name: FUN_0055ca18


undefined4 __fastcall FUN_0055ca18(int param_1)

{
  void *pvVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_64;
  undefined4 local_60;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2bb5;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  pvVar1 = operator_new(0x2b058);
  local_8 = 0;
  if (pvVar1 == (void *)0x0) {
    local_60 = 0;
  }
  else {
    local_60 = FUN_0042c160(*(undefined4 *)(param_1 + 0x10),*(undefined4 *)(param_1 + 0x14),
                            *(undefined4 *)(param_1 + 0x18),*(undefined4 *)(param_1 + 0x1c));
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0xe0) = local_60;
  iVar2 = (**(code **)(**(int **)(param_1 + 0xe0) + 4))(*(undefined4 *)(param_1 + 4));
  if (iVar2 == 0) {
    MessageBoxA(*(HWND *)(param_1 + 4),s_Initialize_Render_Failed__005c4dc4,s_Error_005c4dbc,0);
    FUN_0043125a(s_Initialize_Render_Failed_005c4de0);
    uVar3 = 0;
  }
  else {
    if (0 < *(int *)(param_1 + 0x104)) {
      FUN_0055baa8();
      pvVar1 = operator_new(0x21050);
      local_8 = 1;
      if (pvVar1 == (void *)0x0) {
        local_64 = 0;
      }
      else {
        local_64 = FUN_00429530();
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0xe4) = local_64;
      iVar2 = FUN_0042978d(*(undefined4 *)(param_1 + 4),2,2,0x5622,0x10);
      if (iVar2 < 0) {
        FUN_00431159(s_Initialize_Sound_Failed_005c4dfc);
        if (*(int *)(param_1 + 0xe4) != 0) {
          if (*(int *)(param_1 + 0xe4) != 0) {
            FUN_0055f410(1);
          }
          *(undefined4 *)(param_1 + 0xe4) = 0;
        }
      }
      else {
        local_18 = *(int *)(param_1 + 0x104) * 0x19 + -0x9c4;
        if (0 < local_18) {
          local_18 = 0;
        }
        if (local_18 < -0x9c4) {
          local_18 = -0x9c4;
        }
        FUN_004296c2(local_18);
        FUN_00429c74();
      }
    }
    if (0 < *(int *)(param_1 + 0x108)) {
      pvVar1 = operator_new(0x18);
      local_8 = 2;
      if (pvVar1 == (void *)0x0) {
        local_6c = 0;
      }
      else {
        local_6c = FUN_004287c2(1,*(int *)(param_1 + 0x108) * 0x1e + -3000);
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0xec) = local_6c;
      FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
    }
    pvVar1 = operator_new(0xa5c);
    local_8 = 3;
    if (pvVar1 == (void *)0x0) {
      local_70 = 0;
    }
    else {
      local_70 = FUN_004ae610();
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0xf0) = local_70;
    iVar2 = FUN_004ae744(*(undefined4 *)(param_1 + 4));
    if (iVar2 == 0) {
      FUN_00431159(s_Initialize_Interface_Failed_005c4e18);
      uVar3 = 0;
    }
    else {
      iVar2 = FUN_004ae776();
      if (iVar2 == 0) {
        FUN_00431159(s_Initialize_IME_Failed_005c4e34);
        uVar3 = 0;
      }
      else {
        pvVar1 = operator_new(0x70);
        local_8 = 4;
        if (pvVar1 == (void *)0x0) {
          local_74 = 0;
        }
        else {
          local_74 = FUN_004bac70();
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0xf4) = local_74;
        pvVar1 = operator_new(0x1e42c);
        local_8 = 5;
        if (pvVar1 == (void *)0x0) {
          local_78 = 0;
        }
        else {
          local_78 = FUN_004b1ea9();
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0xf8) = local_78;
        if (*(int *)(param_1 + 0xf8) != 0) {
          (**(code **)(**(int **)(param_1 + 0xf8) + 0x40))();
          (**(code **)(**(int **)(param_1 + 0xf8) + 0x3c))();
        }
        if (*(int *)(param_1 + 0xf4) != 0) {
          FUN_004bacff();
        }
        pvVar1 = operator_new(0x48);
        local_8 = 6;
        if (pvVar1 == (void *)0x0) {
          local_7c = 0;
        }
        else {
          local_7c = FUN_00423ca0();
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0xfc) = local_7c;
        iVar2 = FUN_00423dd8();
        if (iVar2 == 0) {
          FUN_00431159(s_Initialize_Socket_Failed_005c4e4c);
          uVar3 = 0;
        }
        else {
          DAT_013b71e4 = *(undefined4 *)(param_1 + 0xfc);
          (**(code **)(*DAT_0092e654 + 4))(0);
          *(undefined4 *)(param_1 + 8) = 1;
          uVar4 = (**(code **)(**(int **)(*(int *)(param_1 + 0xe0) + 0x2a3e4) + 0x10))
                            (*(undefined4 *)(*(int *)(param_1 + 0xe0) + 0x2a3e4));
          if (uVar4 < 0x41) {
            uVar4 = (**(code **)(**(int **)(*(int *)(param_1 + 0xe0) + 0x2a3e4) + 0x10))
                              (*(undefined4 *)(*(int *)(param_1 + 0xe0) + 0x2a3e4));
            if ((uVar4 < 0x39) || (DAT_005b4920 < 0x15)) {
              uVar4 = (**(code **)(**(int **)(*(int *)(param_1 + 0xe0) + 0x2a3e4) + 0x10))
                                (*(undefined4 *)(*(int *)(param_1 + 0xe0) + 0x2a3e4));
              if ((uVar4 < 0x31) || (DAT_005b4920 < 0xb)) {
                local_14 = 0;
              }
              else {
                local_14 = 10;
              }
            }
            else {
              local_14 = 0x14;
            }
          }
          else {
            local_14 = DAT_005b4920;
          }
          DAT_005b4920 = local_14;
          uVar3 = 1;
        }
      }
    }
  }
  ExceptionList = local_10;
  return uVar3;
}

