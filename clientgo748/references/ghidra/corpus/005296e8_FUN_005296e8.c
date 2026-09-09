// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005296e8 | Name: FUN_005296e8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_005296e8(int param_1,int param_2)

{
  void *pvVar1;
  undefined4 *puVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_54;
  int local_50;
  int local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a22c9;
  local_10 = ExceptionList;
  if (*(short *)(param_2 + 0xc) == 100) {
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x1eac);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_50 = 0;
    }
    else {
      iVar4 = (int)*(short *)(param_2 + 0xe);
      puVar2 = (undefined4 *)
               FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                            *(float *)(param_1 + 0x30) + _DAT_005a4104,
                            *(undefined4 *)(param_1 + 0x2c));
      local_50 = FUN_004d01b0(*puVar2,puVar2[1],puVar2[2],iVar4);
    }
    local_8 = 0xffffffff;
    if (local_50 != 0) {
      FUN_0054ac09(local_50);
    }
  }
  else if (*(short *)(param_2 + 0xe) == 1) {
    if ((*(int *)(param_1 + 0x24c) != 0x24) && (*(int *)(param_1 + 0x24c) != 0x25)) {
      if (*(int *)(param_1 + 0x24c) == 0x27) {
        ExceptionList = &local_10;
        uVar3 = _rand();
        uVar3 = uVar3 & 0x80000001;
        if ((int)uVar3 < 0) {
          uVar3 = (uVar3 - 1 | 0xfffffffe) + 1;
        }
        if ((((uVar3 == 0) && (*(undefined2 *)(param_2 + 0xc) = 0x10, *(int *)(param_1 + 0x48) == 1)
             ) && (DAT_005ccf98 != 0)) && (iVar4 = FUN_00429a6d(0x10b), iVar4 != 0)) {
          uVar6 = 0;
          uVar5 = 0;
          FUN_00429a6d(0x10b);
          FUN_0042ad2b(uVar5,uVar6);
        }
        FUN_00523533((int)*(short *)(param_2 + 0xc),*(undefined4 *)(param_2 + 0x10));
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x38) && (*(short *)(param_1 + 0x1d2) == 0)) {
        ExceptionList = &local_10;
        FUN_00523533((int)*(short *)(param_2 + 0xc),*(undefined4 *)(param_2 + 0x10));
      }
      else if (((*(int *)(param_1 + 0x5c) == 0) ||
               (((*(int *)(param_1 + 0x5c) == 1 || (*(int *)(param_1 + 0x5c) == 0x15)) ||
                (*(int *)(param_1 + 0x5c) == 3)))) &&
              ((((ExceptionList = &local_10,
                 FUN_00523533((int)*(short *)(param_2 + 0xc),*(undefined4 *)(param_2 + 0x10)),
                 *(int *)(param_1 + 0x24c) == 4 && (*(short *)(param_1 + 0x1d2) == 0xf)) &&
                (*(int *)(param_1 + 0x48) == 1)) &&
               ((DAT_005ccf98 != 0 && (iVar4 = FUN_00429a6d(300), iVar4 != 0)))))) {
        uVar6 = 0;
        uVar5 = 0;
        FUN_00429a6d(300);
        FUN_0042ad2b(uVar5,uVar6);
      }
    }
  }
  else {
    ExceptionList = &local_10;
    if (*(short *)(param_2 + 0xe) == 2) {
      ExceptionList = &local_10;
      *(undefined1 *)(param_1 + 0x245) = 0;
    }
    if (*(short *)(param_2 + 0xe) == 3) {
      pvVar1 = operator_new(0x90);
      local_8 = 1;
      if (pvVar1 == (void *)0x0) {
        local_54 = 0;
      }
      else {
        uVar5 = 0;
        puVar2 = (undefined4 *)
                 FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                              *(undefined4 *)(param_1 + 0x2c));
        local_54 = FUN_004d19a0(*puVar2,puVar2[1],puVar2[2],uVar5);
      }
      local_8 = 0xffffffff;
      if (local_54 != 0) {
        FUN_0054ac09(local_54);
      }
    }
    if (*(short *)(param_2 + 0xc) < 0x100) {
      if (*(short *)(DAT_013b71e8 + 0xc5c) == *(short *)(param_2 + 6)) {
        *(undefined4 *)(param_1 + 0x73c) = 0xffffffff;
      }
      local_20 = (int)*(short *)(param_2 + 0xc);
      if (((*(uint *)(param_1 + 0x20) < 1000) &&
          ((((*(int *)(param_1 + 0x5c) == 3 || (*(int *)(param_1 + 0x5c) == 8)) ||
            (*(int *)(param_1 + 0x5c) == 7)) ||
           ((*(int *)(param_1 + 0x5c) == 0x19 || (*(int *)(param_1 + 0x5c) == 0x1c)))))) &&
         (((0xe < local_20 && (local_20 < 0x19)) || (local_20 == 0xd)))) {
        local_20 = local_20 + -0xe;
      }
      FUN_00523533(local_20,*(undefined4 *)(param_2 + 0x10));
    }
  }
  ExceptionList = local_10;
  return 1;
}

