// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e8a95 | Name: FUN_004e8a95


void __fastcall FUN_004e8a95(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x94) = uVar1;
  *(undefined4 *)(param_1 + 0x9c) = 1;
  return;
}

