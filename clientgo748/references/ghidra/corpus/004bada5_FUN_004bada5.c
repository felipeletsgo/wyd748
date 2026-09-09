// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bada5 | Name: FUN_004bada5


void __thiscall FUN_004bada5(int param_1,undefined4 param_2)

{
  DWORD DVar1;
  
  *(undefined4 *)(param_1 + 0x54) = param_2;
  *(undefined4 *)(param_1 + 0x50) = param_2;
  DVar1 = timeGetTime();
  *(DWORD *)(param_1 + 0x58) = DVar1;
  FUN_005913b8(param_1 + 100);
  return;
}

