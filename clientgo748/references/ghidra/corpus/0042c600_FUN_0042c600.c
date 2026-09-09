// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042c600 | Name: FUN_0042c600


undefined4 __thiscall FUN_0042c600(int *param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  void *pvVar3;
  HDC hdc;
  int local_670;
  int local_66c;
  int local_61c;
  undefined2 local_618 [256];
  undefined2 auStack_418 [256];
  undefined2 auStack_218 [256];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f0b7;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00425b1b(param_2);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    if (param_1[0xa9c3] == 0) {
      pvVar3 = operator_new(0xe845c);
      local_8 = 0;
      if (pvVar3 == (void *)0x0) {
        local_66c = 0;
      }
      else {
        local_66c = FUN_004b7780();
      }
      local_8 = 0xffffffff;
      param_1[0xa9c3] = local_66c;
      if ((param_1[0xa9c3] != 0) && (iVar1 = FUN_004b7be5(), iVar1 == 0)) {
        FUN_00431159(s_Initialize_Texture_Manager_Faile_005b4950);
        ExceptionList = local_10;
        return 0;
      }
    }
    if (param_1[0xa9c2] == 0) {
      pvVar3 = operator_new(0x1b0);
      local_8 = 1;
      if (pvVar3 == (void *)0x0) {
        local_670 = 0;
      }
      else {
        local_670 = FUN_004219a0();
      }
      local_8 = 0xffffffff;
      param_1[0xa9c2] = local_670;
    }
    if ((param_1[0xa9c1] == 0) &&
       (local_14 = FUN_00560f07(param_1[0xa8f9],param_1 + 0xa9c1), local_14 < 0)) {
      ExceptionList = local_10;
      return 0;
    }
    iVar1 = (**(code **)(*param_1 + 0x3c))();
    if (iVar1 == 0) {
      uVar2 = 0;
    }
    else {
      uVar2 = FUN_00430f50(0,0,0x3c23d70a);
      FUN_004310a0(uVar2);
      param_1[0xa9c8] = 0x3f800000;
      param_1[0xa9c9] = -0x55555556;
      param_1[0xa9ca] = 0x3c23d70a;
      param_1[0xa9cb] = 0x3c23d70a;
      uVar2 = FUN_00430f50(0,0,0x3c23d70a);
      FUN_004310a0(uVar2);
      param_1[0xa9cf] = 0x3f800000;
      param_1[0xa9d0] = -0x55555556;
      param_1[0xa9d1] = 0x3f7d70a4;
      param_1[0xa9d2] = 0x3c23d70a;
      uVar2 = FUN_00430f50(0,0,0x3c23d70a);
      FUN_004310a0(uVar2);
      param_1[0xa9d6] = 0x3f800000;
      param_1[0xa9d7] = -0x55555556;
      param_1[0xa9d8] = 0x3f7d70a4;
      param_1[0xa9d9] = 0x3f7d70a4;
      uVar2 = FUN_00430f50(0,0,0x3c23d70a);
      FUN_004310a0(uVar2);
      param_1[0xa9dd] = 0x3f800000;
      param_1[0xa9de] = -0x55555556;
      param_1[0xa9df] = 0x3c23d70a;
      param_1[0xa9e0] = 0x3f7d70a4;
      if (DAT_005b491c != 0x32) {
        _memset(local_618,0,0x600);
        (**(code **)(*(int *)param_1[0xa8f9] + 0x4c))(param_1[0xa8f9],local_618);
        for (local_18 = 0; local_18 < 0x100; local_18 = local_18 + 1) {
          local_61c = __ftol();
          if (0xffff < local_61c) {
            local_61c = 0xffff;
          }
          local_618[local_18] = (undefined2)local_61c;
          auStack_418[local_18] = (undefined2)local_61c;
          auStack_218[local_18] = (undefined2)local_61c;
        }
        (**(code **)(*(int *)param_1[0xa8f9] + 0x48))(param_1[0xa8f9],0,local_618);
        if (param_1[0xa8f0] != 0) {
          hdc = GetDC(*(HWND *)(DAT_013b71e0 + 4));
          SetDeviceGammaRamp(hdc,local_618);
          ReleaseDC(*(HWND *)(DAT_013b71e0 + 4),hdc);
        }
      }
      uVar2 = 1;
    }
  }
  ExceptionList = local_10;
  return uVar2;
}

