// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004064de | Name: FUN_004064de


void __fastcall FUN_004064de(int *param_1)

{
  int iVar1;
  
  if (DAT_005b892c != 2) {
    iVar1 = (**(code **)(*(int *)param_1[0x23] + 0x84))();
    *(undefined4 *)(iVar1 + 0x40) = 1;
  }
  if (param_1[0x1b] < param_1[0x1c] + -5) {
    param_1[0x1b] = param_1[0x1b] + 5;
  }
  else if (param_1[0x1b] < param_1[0x1c]) {
    param_1[0x1b] = param_1[0x1c];
  }
  (**(code **)(*param_1 + 0x54))();
  return;
}

