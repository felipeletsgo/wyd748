// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004851bc | Name: FUN_004851bc


undefined4 __thiscall FUN_004851bc(undefined4 param_1,int param_2)

{
  undefined4 local_8;
  
  FUN_0058f220(DAT_013b71e8 + 4,param_2 + 0xc,0x2e8,param_1);
  FUN_0058f220(DAT_013b71e8 + 0x2ec,param_2 + 0x2f4,0x400);
  *(undefined4 *)(DAT_013b71e8 + 0xc58) = *(undefined4 *)(param_2 + 0x6f4);
  _memset((void *)(DAT_013b71e8 + 0x1b094),0,0x3390);
  for (local_8 = 0; local_8 < 0x10; local_8 = local_8 + 1) {
    *(undefined1 *)(DAT_013b71e4 + local_8 + 0x20) = *(undefined1 *)(param_2 + local_8 + 0x708);
  }
  *(undefined4 *)(DAT_013b71e4 + 0x40) = 0;
  *(undefined4 *)(DAT_013b71e4 + 0x44) = 0;
  return 1;
}

