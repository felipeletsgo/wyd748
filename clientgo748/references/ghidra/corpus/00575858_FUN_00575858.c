// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00575858 | Name: FUN_00575858


void FUN_00575858(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  
  if ((*(byte *)(param_1 + 0x10c) & 0x20) == 0) {
    if ((*(byte *)(param_1 + 0x5d) & 8) != 0) {
      return;
    }
  }
  else if ((*(uint *)(param_1 + 0x5c) & 0x300) == 0x300) {
    return;
  }
  uVar1 = FUN_0057ad9a(*(undefined4 *)(param_1 + 0x100),param_2,param_3);
  *(undefined4 *)(param_1 + 0x100) = uVar1;
  return;
}

