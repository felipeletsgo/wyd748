// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054b256 | Name: FUN_0054b256


bool FUN_0054b256(int param_1,uint param_2)

{
  uint uVar1;
  
  uVar1 = param_2 & 0x80000007;
  if ((int)uVar1 < 0) {
    uVar1 = (uVar1 - 1 | 0xfffffff8) + 1;
  }
  return ((int)*(char *)(param_1 + ((int)(param_2 + ((int)param_2 >> 0x1f & 7U)) >> 3)) &
         1 << ((byte)uVar1 & 0x1f)) != 0;
}

