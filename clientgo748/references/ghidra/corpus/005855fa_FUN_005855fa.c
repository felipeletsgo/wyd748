// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005855fa | Name: FUN_005855fa


uint FUN_005855fa(int param_1,int param_2,int param_3,int param_4,int *param_5)

{
  char cVar1;
  uint uVar2;
  int local_8;
  
  local_8 = (int)param_5;
  if (param_3 < (int)param_5) {
    cVar1 = FUN_00585510(param_1,param_2,param_3,param_5);
    if (cVar1 == '\0') {
      return 0xffffffff;
    }
    param_2 = *(int *)(param_1 + 0xc);
    param_3 = *(int *)(param_1 + 0x18);
  }
  param_3 = param_3 - (int)param_5;
  uVar2 = param_2 >> ((byte)param_3 & 0x1f) & (1 << ((byte)param_5 & 0x1f)) - 1U;
  param_5 = (int *)(param_4 + 0x44 + (int)param_5 * 4);
  if (*param_5 < (int)uVar2) {
    do {
      if (param_3 < 1) {
        cVar1 = FUN_00585510(param_1,param_2,param_3,1);
        if (cVar1 == '\0') {
          return 0xffffffff;
        }
        param_2 = *(int *)(param_1 + 0xc);
        param_3 = *(int *)(param_1 + 0x18);
      }
      param_5 = param_5 + 1;
      param_3 = param_3 + -1;
      uVar2 = uVar2 << 1 | param_2 >> ((byte)param_3 & 0x1f) & 1U;
      local_8 = local_8 + 1;
    } while (*param_5 < (int)uVar2);
  }
  *(int *)(param_1 + 0x18) = param_3;
  *(int *)(param_1 + 0xc) = param_2;
  if (local_8 < 0x11) {
    uVar2 = (uint)*(byte *)((*(int *)(param_4 + 0x8c + local_8 * 4) -
                            *(int *)(param_4 + local_8 * 4)) + *(int *)(param_4 + 0xd0) + 0x11 +
                           uVar2);
  }
  else {
    *(undefined4 *)(**(int **)(param_1 + 0x1c) + 0x14) = 0x72;
    (**(code **)(**(int **)(param_1 + 0x1c) + 4))(*(int **)(param_1 + 0x1c),0xffffffff);
    uVar2 = 0;
  }
  return uVar2;
}

