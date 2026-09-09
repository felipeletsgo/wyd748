// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054b2fd | Name: FUN_0054b2fd


void FUN_0054b2fd(int param_1,uint param_2)

{
  int iVar1;
  
  iVar1 = (int)(param_2 + ((int)param_2 >> 0x1f & 7U)) >> 3;
  param_2 = param_2 & 0x80000007;
  if ((int)param_2 < 0) {
    param_2 = (param_2 - 1 | 0xfffffff8) + 1;
  }
  *(byte *)(param_1 + iVar1) =
       *(byte *)(param_1 + iVar1) & ((byte)(1 << ((byte)param_2 & 0x1f)) ^ 0xff);
  return;
}

