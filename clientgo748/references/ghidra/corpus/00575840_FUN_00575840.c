// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00575840 | Name: FUN_00575840


void FUN_00575840(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0057ad9a(0,0,0);
  *(undefined4 *)(param_1 + 0x100) = uVar1;
  return;
}

