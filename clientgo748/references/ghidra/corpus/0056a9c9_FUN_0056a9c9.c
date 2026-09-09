// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056a9c9 | Name: FUN_0056a9c9


/* WARNING: Type propagation algorithm not settling */

int FUN_0056a9c9(undefined4 *param_1,int *param_2,undefined4 param_3,uint *param_4,
                undefined4 param_5,uint param_6)

{
  uint *puVar1;
  bool bVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint *puVar7;
  int local_64 [2];
  uint local_5c;
  int local_58;
  uint local_4c;
  uint local_48;
  uint local_44;
  uint local_40;
  uint local_3c;
  uint local_38;
  uint local_34;
  uint local_30;
  uint local_2c;
  uint local_28;
  undefined4 local_24;
  undefined4 local_20;
  uint *local_1c;
  int *local_18;
  int *local_14;
  int *local_10;
  uint local_c;
  uint local_8;
  
  FUN_0056a93f();
  (**(code **)(*param_2 + 0x20))(param_2,local_64);
  if (param_4 == (uint *)0x0) {
    local_2c = local_4c;
    local_28 = local_48;
    local_c = ~param_6 & 1;
    local_34 = 0;
    local_30 = 0;
    bVar2 = false;
  }
  else {
    local_34 = *param_4;
    local_30 = param_4[1];
    local_2c = param_4[2];
    local_28 = param_4[3];
    if (((((int)local_34 < 0) || (local_4c < local_2c)) || ((int)local_2c < (int)local_34)) ||
       ((((int)local_30 < 0 || (local_48 < local_28)) || ((int)local_28 < (int)local_30)))) {
      return -0x7789f794;
    }
    if (((local_34 != 0) || (local_2c != local_4c)) ||
       ((local_30 != 0 || (bVar2 = false, local_28 != local_48)))) {
      bVar2 = true;
    }
    if (((param_6 & 1) != 0) || (bVar2)) {
      local_c = 0;
    }
    else {
      local_c = 1;
    }
  }
  local_8 = param_6 & 0x10000;
  if (local_8 == 0) {
    if ((local_58 == 0) && ((local_5c & 0x200) == 0)) {
      FUN_00564a04(1);
      iVar5 = (**(code **)(*param_2 + 0x1c))(param_2,&DAT_005a6d40,&local_18);
      if (iVar5 < 0) {
        local_5c = param_2[4];
        local_8 = ~(local_5c >> 0x1a) & 1;
      }
      else {
        local_8 = 1;
        if (local_18 != (int *)0x0) {
          (**(code **)(*local_18 + 8))(local_18);
          local_18 = (int *)0x0;
        }
      }
      FUN_00564a04(0);
      if (local_8 != 0) goto LAB_0056aaf1;
    }
    local_14 = param_2;
  }
  else {
LAB_0056aaf1:
    puVar7 = local_1c;
    if ((((local_64[0] == 0x31545844) || (local_64[0] == 0x32545844)) || (local_64[0] == 0x33545844)
        ) || ((local_64[0] == 0x34545844 ||
              (local_14 = (int *)local_48, uVar6 = local_4c, local_64[0] == 0x35545844)))) {
      local_14 = (int *)(local_48 + 3 & 0xfffffffc);
      uVar6 = local_4c + 3 & 0xfffffffc;
    }
    puVar1 = local_1c + 3;
    (**(code **)(*param_2 + 0xc))(param_2,puVar1);
    puVar7 = puVar7 + 2;
    iVar5 = (**(code **)(*(int *)*puVar1 + 0x6c))((int *)*puVar1,uVar6,local_14,local_64[0],puVar7);
    if (iVar5 < 0) {
      return iVar5;
    }
    if ((local_c == 0) &&
       (iVar5 = (**(code **)(*(int *)*puVar1 + 0x70))((int *)*puVar1,param_2,0,0,*puVar7,0),
       iVar5 < 0)) {
      return iVar5;
    }
    local_14 = (int *)*puVar7;
  }
  uVar6 = (param_6 & 1 | 0x80) << 4;
  if (((local_c == 0) || (local_8 != 0)) || ((local_5c & 0x200) == 0)) {
LAB_0056ac0e:
    uVar3 = local_44;
    uVar4 = local_40;
    if (bVar2) {
      if ((local_64[0] == 0x31545844) || (local_64[0] == 0x32545844)) {
LAB_0056ac96:
        local_44 = local_34 & 0xfffffffc;
        local_3c = local_2c + 3 & 0xfffffffc;
        local_40 = local_30 & 0xfffffffc;
        local_38 = local_28 + 3 & 0xfffffffc;
        if (local_8 == 0) {
          if (local_4c < local_3c) {
            local_3c = local_4c;
          }
          if (local_48 < local_38) {
            local_38 = local_48;
          }
joined_r0x0056ace1:
          if (((local_44 == 0) && (local_3c == local_4c)) && (local_40 == 0)) {
            uVar3 = 0;
            uVar4 = 0;
            if (local_38 == local_48) goto LAB_0056adc1;
          }
        }
        else if (((local_44 == 0) && (local_3c == (local_4c + 3 & 0xfffffffc))) && (local_40 == 0))
        {
          uVar3 = 0;
          uVar4 = 0;
          if (local_38 == (local_48 + 3 & 0xfffffffc)) goto LAB_0056adc1;
        }
      }
      else {
        if (local_64[0] == 0x32595559) {
LAB_0056ac5a:
          local_44 = local_34 & 0xfffffffe;
          local_3c = local_2c + 1 & 0xfffffffe;
          local_40 = local_30;
          local_38 = local_28;
          if (local_4c < local_3c) {
            local_3c = local_4c;
          }
          goto joined_r0x0056ace1;
        }
        if (((local_64[0] == 0x33545844) || (local_64[0] == 0x34545844)) ||
           (local_64[0] == 0x35545844)) goto LAB_0056ac96;
        if (local_64[0] == 0x59565955) goto LAB_0056ac5a;
        local_44 = local_34;
        local_40 = local_30;
        local_3c = local_2c;
        local_38 = local_28;
      }
      iVar5 = (**(code **)(*local_14 + 0x24))(local_14,&local_24,&local_44,uVar6);
      if (iVar5 < 0) {
        return iVar5;
      }
      local_34 = local_34 - local_44;
      local_2c = local_2c - local_44;
      local_30 = local_30 - local_40;
      local_28 = local_28 - local_40;
      goto LAB_0056ad1f;
    }
  }
  else {
    if ((param_6 & 0x20000) == 0) {
      FUN_00564a04(1);
      iVar5 = (**(code **)(*param_2 + 0x1c))(param_2,&DAT_005a6d30,&local_10);
      if (-1 < iVar5) {
        iVar5 = (**(code **)(*local_10 + 0x34))(local_10);
        if (iVar5 == 1) {
          uVar6 = uVar6 | 0x2000;
          bVar2 = false;
        }
        if (local_10 != (int *)0x0) {
          (**(code **)(*local_10 + 8))(local_10);
          local_10 = (int *)0x0;
        }
      }
      FUN_00564a04(0);
      goto LAB_0056ac0e;
    }
    uVar6 = uVar6 | 0x2000;
    uVar3 = local_44;
    uVar4 = local_40;
  }
LAB_0056adc1:
  local_40 = uVar4;
  local_44 = uVar3;
  iVar5 = (**(code **)(*local_14 + 0x24))(local_14,&local_24,0,uVar6);
  if (iVar5 < 0) {
    return iVar5;
  }
LAB_0056ad1f:
  param_1[3] = 0;
  param_1[4] = 0;
  param_1[5] = 0;
  param_1[8] = 0;
  param_1[0xe] = 0;
  *param_1 = local_20;
  param_1[1] = local_64[0];
  param_1[10] = local_34;
  param_1[0xf] = 1;
  param_1[0x10] = 1;
  param_1[2] = local_24;
  param_1[0xb] = local_30;
  param_1[0x11] = param_5;
  param_1[6] = local_4c;
  param_1[0xc] = local_2c;
  param_1[0x12] = param_3;
  param_1[7] = local_48;
  param_1[9] = 1;
  param_1[0xd] = local_28;
  local_1c[1] = (uint)param_2;
  *local_1c = param_6;
  return 0;
}

