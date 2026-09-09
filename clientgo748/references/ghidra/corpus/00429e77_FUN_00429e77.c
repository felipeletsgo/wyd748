// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00429e77 | Name: FUN_00429e77


int __thiscall
FUN_00429e77(int *param_1,undefined4 *param_2,int param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6,undefined4 param_7,undefined4 param_8,uint param_9)

{
  undefined4 *puVar1;
  void *pvVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 local_9c;
  undefined4 *local_90;
  int local_4c;
  undefined4 *local_40;
  uint local_3c;
  int local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f016;
  local_10 = ExceptionList;
  local_4c = 0;
  local_40 = (undefined4 *)0x0;
  if (*param_1 == 0) {
    local_38 = -0x7ffbfe10;
  }
  else if (((param_3 == 0) || (param_2 == (undefined4 *)0x0)) || (param_9 == 0)) {
    local_38 = -0x7ff8ffa9;
  }
  else {
    ExceptionList = &local_10;
    puVar1 = operator_new(param_9 << 2);
    if (puVar1 == (undefined4 *)0x0) {
      local_38 = -0x7ff8fff2;
    }
    else {
      pvVar2 = operator_new(0x90);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_90 = (undefined4 *)0x0;
      }
      else {
        local_90 = (undefined4 *)FUN_0042b492();
      }
      local_8 = 0xffffffff;
      local_40 = local_90;
      if (local_90 == (undefined4 *)0x0) {
        local_38 = -0x7ff8fff2;
      }
      else {
        FUN_0042b510(param_3,0,1);
        iVar3 = FUN_0042b9d8();
        if (iVar3 != 0) {
          uVar4 = FUN_0042b9d8();
          _memset(&local_34,0,0x24);
          local_34 = 0x24;
          local_30 = param_4;
          local_20 = param_5;
          local_1c = param_6;
          local_18 = param_7;
          local_14 = param_8;
          local_24 = *local_90;
          local_2c = uVar4;
          iVar3 = (**(code **)(*(int *)*param_1 + 0xc))(*param_1,&local_34,puVar1,0);
          if (iVar3 == 0x878000a) {
            local_4c = 0x878000a;
          }
          if ((iVar3 < 0) && (iVar3 != -0x7787ff4c)) {
            if ((local_90 != (undefined4 *)0x0) && (local_90 != (undefined4 *)0x0)) {
              FUN_0042c0b0(1);
            }
            if (puVar1 == (undefined4 *)0x0) {
              ExceptionList = local_10;
              return iVar3;
            }
            FUN_0058ea3a(puVar1);
            ExceptionList = local_10;
            return iVar3;
          }
          for (local_3c = 1; local_3c < param_9; local_3c = local_3c + 1) {
            iVar3 = (**(code **)(*(int *)*param_1 + 0x14))(*param_1,*puVar1,puVar1 + local_3c);
            if (iVar3 < 0) {
              if ((local_90 != (undefined4 *)0x0) && (local_90 != (undefined4 *)0x0)) {
                FUN_0042c0b0(1);
              }
              if (puVar1 == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return iVar3;
              }
              FUN_0058ea3a(puVar1);
              ExceptionList = local_10;
              return iVar3;
            }
          }
          pvVar2 = operator_new(0x14);
          local_8 = 1;
          if (pvVar2 == (void *)0x0) {
            local_9c = 0;
          }
          else {
            local_9c = FUN_0042a7d1(puVar1,uVar4,param_9,local_90);
          }
          local_8 = 0xffffffff;
          *param_2 = local_9c;
          if (puVar1 == (undefined4 *)0x0) {
            ExceptionList = local_10;
            return local_4c;
          }
          FUN_0058ea3a(puVar1);
          ExceptionList = local_10;
          return local_4c;
        }
        local_38 = -0x7fffbffb;
      }
    }
    if ((local_40 != (undefined4 *)0x0) && (local_40 != (undefined4 *)0x0)) {
      FUN_0042c0b0(1);
    }
    if (puVar1 != (undefined4 *)0x0) {
      FUN_0058ea3a(puVar1);
    }
  }
  ExceptionList = local_10;
  return local_38;
}

