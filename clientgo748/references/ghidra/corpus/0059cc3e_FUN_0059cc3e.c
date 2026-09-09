// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059cc3e | Name: FUN_0059cc3e


void FUN_0059cc3e(uint *param_1)

{
  uint uVar1;
  
  uVar1 = param_1[1];
  param_1[1] = uVar1 >> 1 | param_1[2] << 0x1f;
  param_1[2] = param_1[2] >> 1;
  *param_1 = *param_1 >> 1 | uVar1 << 0x1f;
  return;
}

