// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a74d | Name: FUN_0055a74d


void FUN_0055a74d(void *param_1,int param_2,int param_3)

{
  undefined4 local_8;
  
  _memset(param_1,0,param_3 >> 3);
  for (local_8 = 0; local_8 < param_3; local_8 = local_8 + 1) {
    *(byte *)((int)param_1 + (local_8 >> 3)) =
         *(byte *)((int)param_1 + (local_8 >> 3)) |
         *(char *)(param_2 + local_8) << ((byte)local_8 & 7);
  }
  return;
}

