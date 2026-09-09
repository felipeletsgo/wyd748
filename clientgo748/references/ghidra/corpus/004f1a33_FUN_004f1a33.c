// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f1a33 | Name: FUN_004f1a33


void __thiscall FUN_004f1a33(int param_1,undefined4 param_2,undefined4 param_3)

{
  void *pvVar1;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a168e;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *(undefined4 *)(param_1 + 0x2e8) = 2;
  *(undefined4 *)(param_1 + 0x2ec) = 0;
  *(undefined4 *)(param_1 + 0x2e4) = 0;
  *(undefined4 *)(param_1 + 0x2fc) = 0x3f99999a;
  *(undefined4 *)(param_1 + 0x304) = 0;
  *(undefined4 *)(param_1 + 0x308) = 0x40000000;
  *(undefined4 *)(param_1 + 0x30c) = 0;
  *(undefined4 *)(param_1 + 0x310) = 0x40000000;
  if ((-1 < *(short *)(param_1 + 0x160)) && (*(short *)(param_1 + 0x160) < 0x1965)) {
    *(undefined2 *)(param_1 + 0x2f0) = (undefined2)param_2;
    *(int *)(param_1 + 0x2f4) = (int)*(short *)(&DAT_00d44990 + *(short *)(param_1 + 0x2f0) * 0x8c);
    *(int *)(param_1 + 0x2f8) = (int)*(short *)(&DAT_00d44990 + *(short *)(param_1 + 0x2f0) * 0x8c);
    *(undefined4 *)(param_1 + 0x160) = param_2;
    *(undefined4 *)(param_1 + 0x164) = param_3;
    if (*(int *)(param_1 + 0x2f8) == 0x79) {
      *(undefined4 *)(param_1 + 0x2fc) = 0x3f99999a;
      *(undefined4 *)(param_1 + 0x304) = 0x3f800000;
      *(undefined4 *)(param_1 + 0x308) = 0x40400000;
      *(undefined4 *)(param_1 + 0x30c) = 0x3f800000;
      *(undefined4 *)(param_1 + 0x310) = 0x40400000;
    }
    else if (*(int *)(param_1 + 0x2f8) == 0x88) {
      *(undefined4 *)(param_1 + 0x2f4) = 0x89;
      *(undefined4 *)(param_1 + 0x2fc) = 0x3f8ccccd;
      *(undefined4 *)(param_1 + 0x304) = 0x3fa66666;
      *(undefined4 *)(param_1 + 0x308) = 0x4019999a;
      *(undefined4 *)(param_1 + 0x30c) = 0x3fa66666;
      *(undefined4 *)(param_1 + 0x310) = 0x4019999a;
    }
    else if (*(int *)(param_1 + 0x2f8) == 0x8d) {
      *(undefined4 *)(param_1 + 0x304) = 0x3f800000;
      *(undefined4 *)(param_1 + 0x308) = 0x3f800000;
      *(undefined4 *)(param_1 + 0x30c) = 0x3f800000;
      *(undefined4 *)(param_1 + 0x310) = 0x3f800000;
    }
    else if ((*(int *)(param_1 + 0x2f8) < 0x3dd) || (0x3e0 < *(int *)(param_1 + 0x2f8))) {
      if (*(int *)(param_1 + 0x2f8) == 0x94) {
        *(undefined4 *)(param_1 + 0x304) = 0;
        *(undefined4 *)(param_1 + 0x308) = 0;
        *(undefined4 *)(param_1 + 0x30c) = 0;
        *(undefined4 *)(param_1 + 0x310) = 0;
      }
      else if (*(int *)(param_1 + 0x2f8) == 0x1e0) {
        *(undefined4 *)(param_1 + 0x2f4) = 0x1e0;
        *(undefined4 *)(param_1 + 0x2fc) = 0x3f91eb85;
        *(undefined4 *)(param_1 + 0x304) = 0x3fa66666;
        *(undefined4 *)(param_1 + 0x308) = 0x4019999a;
        *(undefined4 *)(param_1 + 0x30c) = 0x3fa66666;
        *(undefined4 *)(param_1 + 0x310) = 0x4019999a;
      }
      else if (*(int *)(param_1 + 0x2f8) == 0x6d5) {
        *(undefined4 *)(param_1 + 0x2fc) = 0x3fd33333;
      }
      else if (*(int *)(param_1 + 0x2f8) == 0xae0) {
        *(undefined4 *)(param_1 + 0x2fc) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x304) = 0x41200000;
        *(undefined4 *)(param_1 + 0x308) = 0x41200000;
        *(undefined4 *)(param_1 + 0x30c) = 0x41200000;
        *(undefined4 *)(param_1 + 0x310) = 0x41200000;
      }
      else {
        *(undefined4 *)(param_1 + 0x304) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x308) = 0x40400000;
        *(undefined4 *)(param_1 + 0x30c) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x310) = 0x40400000;
      }
    }
    else {
      *(undefined4 *)(param_1 + 0x304) = 0x3f800000;
      *(undefined4 *)(param_1 + 0x308) = 0x3f800000;
      *(undefined4 *)(param_1 + 0x30c) = 0x3f800000;
      *(undefined4 *)(param_1 + 0x310) = 0x3f800000;
      if (*(int *)(param_1 + 0x2e0) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x2e0));
        *(undefined4 *)(param_1 + 0x2e0) = 0;
      }
      pvVar1 = operator_new(0x124);
      local_8 = 0;
      if (pvVar1 == (void *)0x0) {
        local_20 = 0;
      }
      else {
        local_20 = FUN_004cb900(2,0,0x3fe66666,0x3fe66666,0x3fe66666,0,0);
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x2e0) = local_20;
      if (*(int *)(param_1 + 0x2e0) != 0) {
        *(int *)(*(int *)(param_1 + 0x2e0) + 0x1c) = param_1;
        *(undefined4 *)(*(int *)(param_1 + 0x2e0) + 0x108) = 0;
        *(undefined4 *)(*(int *)(param_1 + 0x2e0) + 0x6c) = 1;
        *(undefined4 *)(*(int *)(param_1 + 0x2e0) + 0x104) = 2;
        *(undefined4 *)(*(int *)(param_1 + 0x2e0) + 0x10c) = 0x3e99999a;
        if (*(int *)(param_1 + 0x2f8) == 0x3dd) {
          (**(code **)(**(int **)(param_1 + 0x2e0) + 0x3c))(0xff004488);
        }
        if (*(int *)(param_1 + 0x2f8) == 0x3de) {
          (**(code **)(**(int **)(param_1 + 0x2e0) + 0x3c))(0xff666600);
        }
        if (*(int *)(param_1 + 0x2f8) == 0x3df) {
          (**(code **)(**(int **)(param_1 + 0x2e0) + 0x3c))(0xff008800);
        }
        if (*(int *)(param_1 + 0x2f8) == 0x3e0) {
          (**(code **)(**(int **)(param_1 + 0x2e0) + 0x3c))(0xff880000);
        }
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x2e0));
      }
    }
    if ((*(int *)(param_1 + 0x2f4) == 0x94) || (*(int *)(param_1 + 0x2f4) == 0x96)) {
      *(undefined2 *)(param_1 + 0x302) = 1;
    }
    else {
      *(undefined2 *)(param_1 + 0x302) = 0;
    }
    if (*(int *)(param_1 + 0x2f4) == 0x8d) {
      *(undefined2 *)(param_1 + 0x302) = 2;
    }
    if ((0x3dc < *(int *)(param_1 + 0x2f8)) && (*(int *)(param_1 + 0x2f8) < 0x3e1)) {
      *(undefined2 *)(param_1 + 0x302) = 1;
    }
    if (*(int *)(param_1 + 0x2f8) == 0xae0) {
      *(undefined2 *)(param_1 + 0x302) = 1;
    }
  }
  ExceptionList = local_10;
  return;
}

