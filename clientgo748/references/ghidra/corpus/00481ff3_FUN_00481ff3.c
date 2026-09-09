// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00481ff3 | Name: FUN_00481ff3


undefined4 __thiscall FUN_00481ff3(int param_1,int param_2)

{
  bool bVar1;
  undefined4 uVar2;
  void *pvVar3;
  int local_180;
  int local_17c;
  int local_178;
  int local_174;
  _SYSTEMTIME local_148;
  undefined1 local_138 [128];
  int local_b8;
  int local_b4;
  int *local_b0;
  uint local_ac;
  int local_a8;
  undefined4 local_a4;
  int local_a0;
  undefined1 local_9c [128];
  undefined4 local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fa21;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(param_2 + 6));
  if (local_14 == 0) {
    local_a0 = param_2;
    local_b0 = DAT_013b71e8 + 0x1bb;
    local_a4 = *(undefined4 *)(param_1 + 0x27ae8);
    *(undefined1 *)(param_2 + 0x1b) = 0;
    *(undefined1 *)(param_2 + 0x7b) = 0;
    *(undefined1 *)(param_2 + 0x7a) = 0;
    local_ac = 0;
    local_1c = 0xffffff00;
    local_18 = 1;
    if ((*(char *)(param_2 + 0x1c) == '-') && (*(short *)(param_2 + 0x7c) == 3)) {
      if (((DAT_005b892c == 2) && (*(int *)(param_1 + 0x27b00) != 0)) &&
         (*(int *)(*(int *)(param_1 + 0x27b00) + 0x1e8) == 0)) {
        local_18 = 0;
      }
      local_1c = 0xffaaffff;
      local_ac = 1;
      if (*(char *)(param_2 + 0x1d) == '-') {
        local_1c = 0xff00ffff;
        local_ac = 2;
      }
      FUN_0058f078(local_9c,s___s_>__s_005b87fc,param_2 + 0xc,param_2 + 0x1c + local_ac);
    }
    else if (*(short *)(param_2 + 0x7c) == 7) {
      local_1c = 0xffbbbbbb;
      local_ac = 0;
      FUN_0058f078(local_9c,s___s_>__s_005b8808,param_2 + 0xc,param_2 + 0x1c);
    }
    else if (*(char *)(param_2 + 0x1c) == '=') {
      if ((DAT_005b892c == 2) && (*(int *)(*(int *)(param_1 + 0x27af8) + 0x1e8) == 0)) {
        local_18 = 0;
      }
      bVar1 = 1 < *(int *)(*(int *)(param_1 + 0x2872c) + 0x210);
      if (bVar1) {
        local_1c = 0xffff99ff;
      }
      local_ac = (uint)bVar1;
      FUN_0058f078(local_9c,s___s_>__s_005b8814,param_2 + 0xc,param_2 + 0x1c + local_ac);
    }
    else if (*(char *)(param_2 + 0x1c) == '!') {
      GetLocalTime(&local_148);
      pvVar3 = operator_new(0xe50);
      local_8 = 0;
      if (pvVar3 == (void *)0x0) {
        local_174 = 0;
      }
      else {
        local_174 = FUN_00407203(&DAT_005b8820,0xffffffff,0,0,0x43960000,0x41800000,0,0x77777777,1,0
                                );
      }
      local_8 = 0xffffffff;
      local_b8 = local_174;
      if (local_174 != 0) {
        FUN_00408d33(local_174);
      }
      FUN_0058f078(local_9c,&DAT_00a3b278,local_a0 + 0xc);
      FUN_0058f078(local_138,s__s___02d__02d__02d__005b8824,local_9c,local_148.wHour,
                   local_148.wMinute,local_148.wSecond);
      pvVar3 = operator_new(0xe50);
      local_8 = 1;
      if (pvVar3 == (void *)0x0) {
        local_178 = 0;
      }
      else {
        local_178 = FUN_00407203(local_138,0xffffffff,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
      }
      local_8 = 0xffffffff;
      if (local_178 != 0) {
        FUN_00408d33(local_178);
      }
      FUN_0058f078(local_9c,&DAT_005b8838,local_a0 + 0x1d + local_ac);
      pvVar3 = operator_new(0xe50);
      local_8 = 2;
      if (pvVar3 == (void *)0x0) {
        local_17c = 0;
      }
      else {
        local_17c = FUN_00407203(local_9c,0xffffffcc,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
      }
      local_8 = 0xffffffff;
      local_b4 = local_17c;
      if (local_17c != 0) {
        FUN_00408d33(local_17c);
      }
      if (*(int *)(param_1 + 0x27cf0) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27cf0) + 0x60))(1);
      }
      ExceptionList = local_10;
      return 1;
    }
    if (local_18 == 0) {
      uVar2 = 1;
    }
    else {
      pvVar3 = operator_new(0xe50);
      local_8 = 3;
      if (pvVar3 == (void *)0x0) {
        local_180 = 0;
      }
      else {
        local_180 = FUN_00407203(local_9c,local_1c,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
      }
      local_8 = 0xffffffff;
      local_a8 = local_180;
      if ((*(int *)(param_1 + 0x27ae8) != 0) && (local_180 != 0)) {
        FUN_00408d33(local_180);
      }
      uVar2 = (**(code **)(*DAT_0092e654 + 8))();
      *(undefined4 *)(param_1 + 0x27464) = uVar2;
      uVar2 = 1;
    }
  }
  else {
    uVar2 = 0;
  }
  ExceptionList = local_10;
  return uVar2;
}

