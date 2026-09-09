// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d56eb | Name: FUN_004d56eb


void __fastcall FUN_004d56eb(undefined4 *param_1)

{
  void *pvVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int *local_4c;
  int local_48;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_005a0a92;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a4a60;
  local_8 = 0;
  if (param_1[0x87] != 0) {
    if (param_1[0x87] != 0) {
      FUN_00493d20(1);
    }
    param_1[0x87] = 0;
  }
  if ((param_1[0x7c] == 0) && (param_1[0x67] == 2)) {
    pvVar1 = operator_new(0xa0);
    local_8._0_1_ = 1;
    if (pvVar1 == (void *)0x0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_004e2a30(param_1[0x79],param_1[0x7a],param_1[0x7b],0,0,0xffffffff,0x22331100);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    if (local_48 != 0) {
      FUN_0054ac09(local_48);
    }
  }
  else if (param_1[0x7c] == 1) {
    pvVar1 = operator_new(0x22c);
    local_8._0_1_ = 2;
    if (pvVar1 == (void *)0x0) {
      local_4c = (int *)0x0;
    }
    else {
      uVar4 = param_1[7];
      uVar3 = 0;
      puVar2 = (undefined4 *)FUN_00430f50(0,0,0);
      local_4c = (int *)FUN_004d5480(0x14,param_1[0x79],param_1[0x7a],param_1[0x7b],*puVar2,
                                     puVar2[1],puVar2[2],uVar3,uVar4);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    if (local_4c != (int *)0x0) {
      FUN_0058f220(local_4c + 0x58,param_1 + 0x58,0x20);
      (**(code **)(*local_4c + 0x78))(0);
      local_4c[0x65] = 3000;
      *(undefined4 *)(local_4c[0x18] + 0x24) = 0x3e4ccccd;
      *(undefined4 *)(local_4c[0x18] + 0x28) = 0x3e4ccccd;
      *(undefined4 *)(local_4c[0x18] + 0x2c) = 0x3e4ccccd;
      local_4c[0x66] = 0;
      local_4c[0x69] = 0x3f800000;
      local_4c[0x6a] = 0x3f800000;
      local_4c[0x6b] = 0x3f800000;
      local_4c[0x75] = 0;
      local_4c[0x68] = param_1[0x7d];
      local_4c[0x67] = 3;
      FUN_0054ac09(local_4c);
    }
  }
  local_8 = 0xffffffff;
  FUN_0053e26b();
  ExceptionList = local_10;
  return;
}

