// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a709 | Name: FUN_0055a709


void FUN_0055a709(int param_1,int param_2,int param_3)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < param_3; local_8 = local_8 + 1) {
    *(byte *)(param_1 + local_8) =
         (byte)((int)*(char *)(param_2 + (local_8 >> 3)) >> ((byte)local_8 & 7)) & 1;
  }
  return;
}

