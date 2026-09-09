// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042a232 | Name: FUN_0042a232


int __thiscall
FUN_0042a232(int *param_1,undefined4 *param_2,int param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
            undefined4 param_10,uint param_11)

{
  undefined4 *puVar1;
  void *pvVar2;
  int iVar3;
  undefined4 local_78;
  int local_74;
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
  puStack_c = &LAB_0059f036;
  local_10 = ExceptionList;
  if (*param_1 == 0) {
    local_38 = -0x7ffbfe10;
  }
  else if (((param_3 == 0) || (param_2 == (undefined4 *)0x0)) || (param_11 == 0)) {
    local_38 = -0x7ff8ffa9;
  }
  else {
    ExceptionList = &local_10;
    puVar1 = operator_new(param_11 << 2);
    if (puVar1 != (undefined4 *)0x0) {
      pvVar2 = operator_new(0x90);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_74 = 0;
      }
      else {
        local_74 = FUN_0042b492();
      }
      local_8 = 0xffffffff;
      if (local_74 != 0) {
        FUN_0042b74c(param_3,param_4,param_5,1);
        _memset(&local_34,0,0x24);
        local_34 = 0x24;
        local_30 = param_6;
        local_2c = param_4;
        local_20 = param_7;
        local_1c = param_8;
        local_18 = param_9;
        local_14 = param_10;
        local_24 = param_5;
        iVar3 = (**(code **)(*(int *)*param_1 + 0xc))(*param_1,&local_34,puVar1,0);
        if (iVar3 < 0) {
          if (puVar1 == (undefined4 *)0x0) {
            ExceptionList = local_10;
            return iVar3;
          }
          FUN_0058ea3a(puVar1);
          ExceptionList = local_10;
          return iVar3;
        }
        local_3c = 1;
        while( true ) {
          if (param_11 <= local_3c) {
            pvVar2 = operator_new(0x14);
            local_8 = 1;
            if (pvVar2 == (void *)0x0) {
              local_78 = 0;
            }
            else {
              local_78 = FUN_0042a7d1(puVar1,param_4,param_11,local_74);
            }
            local_8 = 0xffffffff;
            *param_2 = local_78;
            if (puVar1 != (undefined4 *)0x0) {
              FUN_0058ea3a(puVar1);
            }
            ExceptionList = local_10;
            return 0;
          }
          iVar3 = (**(code **)(*(int *)*param_1 + 0x14))(*param_1,*puVar1,puVar1 + local_3c);
          if (iVar3 < 0) break;
          local_3c = local_3c + 1;
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
    local_38 = -0x7ff8fff2;
    if (puVar1 != (undefined4 *)0x0) {
      FUN_0058ea3a(puVar1);
    }
  }
  ExceptionList = local_10;
  return local_38;
}

