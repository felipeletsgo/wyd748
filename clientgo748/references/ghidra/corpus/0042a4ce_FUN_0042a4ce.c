// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042a4ce | Name: FUN_0042a4ce


int __thiscall
FUN_0042a4ce(int *param_1,undefined4 *param_2,int param_3,uint param_4,undefined4 param_5,
            undefined4 param_6,undefined4 param_7,undefined4 param_8,uint param_9,int param_10,
            int param_11)

{
  void *pvVar1;
  undefined4 local_7c;
  undefined4 *local_78;
  undefined4 *local_50;
  int local_4c;
  undefined4 *local_48;
  int *local_44;
  uint local_40;
  int local_3c;
  void *local_38;
  undefined4 local_34;
  uint local_30;
  int local_2c;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f056;
  local_10 = ExceptionList;
  if (*param_1 == 0) {
    local_3c = -0x7ffbfe10;
  }
  else if (((param_3 == 0) || (param_2 == (undefined4 *)0x0)) || (param_11 == 0)) {
    local_3c = -0x7ff8ffa9;
  }
  else {
    local_50 = (undefined4 *)0x0;
    local_4c = 0;
    local_48 = (undefined4 *)0x0;
    local_38 = (void *)0x0;
    local_44 = (int *)0x0;
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x90);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_78 = (undefined4 *)0x0;
    }
    else {
      local_78 = (undefined4 *)FUN_0042b492();
    }
    local_8 = 0xffffffff;
    local_48 = local_78;
    FUN_0042b510(param_3,0,1);
    local_4c = param_10 * param_9;
    _memset(&local_34,0,0x24);
    local_34 = 0x24;
    local_30 = param_4 | 0x10100;
    local_2c = local_4c;
    local_20 = param_5;
    local_1c = param_6;
    local_18 = param_7;
    local_14 = param_8;
    local_24 = *local_48;
    local_3c = (**(code **)(*(int *)*param_1 + 0xc))(*param_1,&local_34,&local_50,0);
    if (-1 < local_3c) {
      local_3c = (**(code **)*local_50)(local_50,&DAT_005a9e8c,&local_44);
      if (local_3c < 0) {
        if (local_38 != (void *)0x0) {
          FUN_0058ea3a(local_38);
        }
      }
      else {
        local_38 = operator_new(param_9 << 3);
        if (local_38 == (void *)0x0) {
          local_3c = -0x7ff8fff2;
        }
        else {
          for (local_40 = 0; local_40 < param_9; local_40 = local_40 + 1) {
            *(uint *)((int)local_38 + local_40 * 8) = param_10 * local_40 + -1 + param_10;
            *(int *)((int)local_38 + local_40 * 8 + 4) = param_11;
          }
          local_3c = (**(code **)(*local_44 + 0xc))(local_44,param_9,local_38);
          if (local_3c < 0) {
            if (local_44 != (int *)0x0) {
              (**(code **)(*local_44 + 8))(local_44);
              local_44 = (int *)0x0;
            }
            if (local_38 != (void *)0x0) {
              FUN_0058ea3a(local_38);
            }
          }
          else {
            if (local_44 != (int *)0x0) {
              (**(code **)(*local_44 + 8))(local_44);
              local_44 = (int *)0x0;
            }
            if (local_38 != (void *)0x0) {
              FUN_0058ea3a(local_38);
              local_38 = (void *)0x0;
            }
            pvVar1 = operator_new(0x28);
            local_8 = 1;
            if (pvVar1 == (void *)0x0) {
              local_7c = 0;
            }
            else {
              local_7c = FUN_0042b05f(local_50,local_4c,local_48,param_10);
            }
            *param_2 = local_7c;
            local_3c = 0;
          }
        }
      }
    }
  }
  ExceptionList = local_10;
  return local_3c;
}

