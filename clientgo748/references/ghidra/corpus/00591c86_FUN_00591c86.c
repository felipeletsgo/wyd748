// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591c86 | Name: FUN_00591c86


int FUN_00591c86(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_00591cb4(param_1);
  if (iVar1 != 0) {
    return -1;
  }
  if ((*(byte *)(param_1 + 0xd) & 0x40) != 0) {
    iVar1 = FUN_00590ae5(*(undefined4 *)(param_1 + 0x10));
    return -(uint)(iVar1 != 0);
  }
  return 0;
}

