// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f3d1a | Name: FUN_004f3d1a


void __thiscall FUN_004f3d1a(int param_1,undefined4 param_2,undefined4 param_3)

{
  byte bVar1;
  undefined2 uVar2;
  ushort uVar3;
  undefined4 uVar4;
  void *pvVar5;
  undefined4 local_30;
  undefined4 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a16c9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *(undefined4 *)(param_1 + 0x20) = 0xffffffff;
  *(undefined2 *)(param_1 + 0x1bc) = 0;
  *(undefined2 *)(param_1 + 0x1be) = 0;
  *(undefined4 *)(param_1 + 0x1b4) = 0;
  *(undefined4 *)(param_1 + 0x1c4) = 0;
  *(undefined4 *)(param_1 + 0x1b8) = 0;
  *(undefined4 *)(param_1 + 0x160) = param_2;
  *(undefined4 *)(param_1 + 0x164) = param_3;
  if ((-1 < *(short *)(param_1 + 0x160)) && (*(short *)(param_1 + 0x160) < 0x1965)) {
    uVar4 = FUN_0040cea0(*(undefined2 *)(param_1 + 0x160));
    *(undefined4 *)(param_1 + 0x24) = uVar4;
    uVar2 = FUN_0054e06c(&param_2);
    *(undefined2 *)(param_1 + 0x1bc) = uVar2;
    if (0xc < *(short *)(param_1 + 0x1bc)) {
      *(undefined2 *)(param_1 + 0x1bc) = 0xc;
    }
    *(undefined2 *)(param_1 + 0x1be) =
         *(undefined2 *)(&DAT_00d449da + *(short *)(param_1 + 0x160) * 0x8c);
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x1ac));
    *(undefined4 *)(param_1 + 0x1ac) = 0;
    pvVar5 = operator_new(0xe4c);
    local_8 = 0;
    if (pvVar5 == (void *)0x0) {
      local_28 = 0;
    }
    else {
      local_28 = FUN_00401d03(0xffffffff,s_NoName_005bb40c,0xffaaaaff,0,0,0x43000000,0x41800000,0,0,
                              1,1);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x1ac) = local_28;
    (**(code **)(**(int **)(param_1 + 0x1ac) + 0x60))(0);
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x1ac));
    if (*(int *)(param_1 + 0x1c0) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x1c0));
    }
    *(undefined4 *)(param_1 + 0x1c0) = 0;
    *(undefined4 *)(param_1 + 0x1c8) = 0;
    if ((((((*(short *)(param_1 + 0x160) == 0x19c) || (*(short *)(param_1 + 0x160) == 0x19d)) ||
          (*(short *)(param_1 + 0x160) == 0x102d)) ||
         ((*(short *)(param_1 + 0x160) == 0x1a3 || (*(short *)(param_1 + 0x160) == 0x1a4)))) ||
        ((*(short *)(param_1 + 0x160) == 0x2f1 ||
         ((*(short *)(param_1 + 0x160) == 0x301 || (*(short *)(param_1 + 0x160) == 0x6be)))))) ||
       ((0x13f5 < *(short *)(param_1 + 0x160) && (*(short *)(param_1 + 0x160) < 0x140e)))) {
      *(undefined4 *)(param_1 + 0x1c8) = 1;
    }
    uVar4 = FUN_0054df49(&param_2);
    switch(uVar4) {
    case 0x74:
      *(undefined2 *)(param_1 + 0x1be) = 0x74;
      break;
    case 0x75:
      *(undefined2 *)(param_1 + 0x1be) = 0x75;
      break;
    case 0x76:
      *(undefined2 *)(param_1 + 0x1be) = 0x76;
      break;
    case 0x77:
      *(undefined2 *)(param_1 + 0x1be) = 0x77;
      break;
    case 0x78:
      *(undefined2 *)(param_1 + 0x1be) = 0x78;
      break;
    case 0x79:
      *(undefined2 *)(param_1 + 0x1be) = 0x79;
      break;
    case 0x7a:
      *(undefined2 *)(param_1 + 0x1be) = 0x7a;
      break;
    case 0x7b:
      *(undefined2 *)(param_1 + 0x1be) = 0x7b;
      break;
    case 0x7c:
      *(undefined2 *)(param_1 + 0x1be) = 0x7c;
      break;
    case 0x7d:
      *(undefined2 *)(param_1 + 0x1be) = 0x7d;
    }
    if ((((*(short *)(&DAT_00d449d4 + *(short *)(param_1 + 0x160) * 0x8c) == 0x33) ||
         (*(short *)(param_1 + 0x1be) != 0)) && (*(short *)(param_1 + 0x1be) < 5)) &&
       (9 < *(short *)(param_1 + 0x1bc))) {
      uVar3 = FUN_004173cf(&param_2);
      *(ushort *)(param_1 + 0x1be) = (uVar3 & 0xff) + 4;
    }
    else if ((*(short *)(param_1 + 0x1be) == 4) && (9 < *(short *)(param_1 + 0x1bc))) {
      bVar1 = FUN_004173cf(&param_2);
      *(ushort *)(param_1 + 0x1be) = (ushort)bVar1;
    }
    *(undefined4 *)(param_1 + 0x23c) = 0;
    if ((DAT_0067cf38 != 0) && (*(int *)(DAT_0067cf38 + 0x28) != 0)) {
      pvVar5 = operator_new(0x1e4);
      local_8 = 1;
      if (pvVar5 == (void *)0x0) {
        local_30 = 0;
      }
      else {
        local_30 = FUN_00401466(0xfffffffe,0,0,0x41200000,0x41200000,0x77777777,4);
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x23c) = local_30;
      if (*(int *)(param_1 + 0x23c) != 0) {
        (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x23c));
      }
    }
    *(undefined4 *)(param_1 + 0x1c0) = 0;
  }
  ExceptionList = local_10;
  return;
}

