// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00406471 | Name: FUN_00406471


void __fastcall FUN_00406471(int *param_1)

{
  int iVar1;
  
  if (DAT_005b892c != 2) {
    iVar1 = (**(code **)(*(int *)param_1[0x22] + 0x84))();
    *(undefined4 *)(iVar1 + 0x40) = 1;
  }
  if (param_1[0x1b] < 6) {
    if (0 < param_1[0x1b]) {
      param_1[0x1b] = 0;
    }
  }
  else {
    param_1[0x1b] = param_1[0x1b] + -5;
  }
  (**(code **)(*param_1 + 0x54))();
  return;
}

