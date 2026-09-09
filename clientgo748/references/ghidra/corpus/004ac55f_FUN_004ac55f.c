// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ac55f | Name: FUN_004ac55f


void __fastcall FUN_004ac55f(int param_1)

{
  undefined4 uVar1;
  
  *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x8c) = 1;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x2009c) = uVar1;
  if (DAT_005c4958 == 1) {
    if (*(int *)(param_1 + 0x27d30) == 0) {
      FUN_0049e1e3(s_UI_DemoCamAction_005b9784);
    }
    else if (*(int *)(param_1 + 0x27d30) == 1) {
      FUN_0049e1e3(s_UI_DemoCamAction2_005b9798);
    }
    else if (*(int *)(param_1 + 0x27d30) == 2) {
      FUN_0049e1e3(s_UI_DemoCamAction3_005b97ac);
    }
    else if (*(int *)(param_1 + 0x27d30) == 3) {
      FUN_0049e1e3(s_UI_DemoCamAction4_005b97c0);
    }
    else if (*(int *)(param_1 + 0x27d30) == 4) {
      FUN_0049e1e3(s_UI_DemoCamAction5_005b97d4);
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x200b0) = 1;
    _memset((void *)(param_1 + 0x200b4),0,28000);
    *(undefined4 *)(param_1 + 0x200b4) = 1;
    *(undefined2 *)(param_1 + 0x200b8) = 0;
    if (*(int *)(param_1 + 0x27d30) == 0) {
      *(undefined4 *)(param_1 + 0x200bc) = 0x455d68cd;
      *(undefined4 *)(param_1 + 0x200c0) = 0xc0a00000;
      *(undefined4 *)(param_1 + 0x200c4) = 0x453a6000;
      *(undefined4 *)(param_1 + 0x200c8) = 0x3fdf66f3;
      *(undefined4 *)(param_1 + 0x200cc) = 0x3e860a92;
    }
    else if (*(int *)(param_1 + 0x27d30) == 1) {
      *(undefined4 *)(param_1 + 0x200bc) = 0x45068000;
      *(undefined4 *)(param_1 + 0x200c0) = 0x3fb33333;
      *(undefined4 *)(param_1 + 0x200c4) = 0x45043000;
      *(undefined4 *)(param_1 + 0x200c8) = 0x3fc90fdb;
      *(undefined4 *)(param_1 + 0x200cc) = 0;
    }
    else if (*(int *)(param_1 + 0x27d30) == 2) {
      *(undefined4 *)(param_1 + 0x200bc) = 0x448ec000;
      *(undefined4 *)(param_1 + 0x200c0) = 0x41933333;
      *(undefined4 *)(param_1 + 0x200c4) = 0x44d5299a;
      *(undefined4 *)(param_1 + 0x200c8) = 0x3fc90fdb;
      *(undefined4 *)(param_1 + 0x200cc) = 0xbfbc430e;
    }
    else if (*(int *)(param_1 + 0x27d30) == 3) {
      *(undefined4 *)(param_1 + 0x200bc) = 0x44860000;
      *(undefined4 *)(param_1 + 0x200c0) = 0x40600000;
      *(undefined4 *)(param_1 + 0x200c4) = 0x44d34000;
      *(undefined4 *)(param_1 + 0x200c8) = 0xc0490fdb;
      *(undefined4 *)(param_1 + 0x200cc) = 0;
    }
  }
  *(undefined2 *)(param_1 + 0x20098) = 1;
  return;
}

