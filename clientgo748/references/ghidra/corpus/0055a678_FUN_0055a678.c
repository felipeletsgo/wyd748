// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a678 | Name: FUN_0055a678


void FUN_0055a678(int param_1,int param_2,int param_3)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < param_3; local_8 = local_8 + 1) {
    *(byte *)(param_1 + local_8) = *(byte *)(param_1 + local_8) ^ *(byte *)(param_2 + local_8);
  }
  return;
}

