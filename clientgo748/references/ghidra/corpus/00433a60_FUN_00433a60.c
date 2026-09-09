// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00433a60 | Name: FUN_00433a60


undefined4 FUN_00433a60(int param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = (int)(param_2 + (param_2 >> 0x1f & 3U)) >> 2 & 0x800003ff;
  if ((int)uVar1 < 0) {
    uVar1 = (uVar1 - 1 | 0xfffffc00) + 1;
  }
  uVar2 = (int)(param_1 + (param_1 >> 0x1f & 3U)) >> 2 & 0x800003ff;
  if ((int)uVar2 < 0) {
    uVar2 = (uVar2 - 1 | 0xfffffc00) + 1;
  }
  return CONCAT31((int3)(uVar1 * 0x400 >> 8),(&DAT_00934178)[uVar2 + uVar1 * 0x400]);
}

