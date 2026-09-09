// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f58e8 | Name: FUN_004f58e8


void __thiscall FUN_004f58e8(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int local_8;
  
  FUN_0053e47d(param_2,param_3,param_4);
  if (*(short *)(param_1 + 0x160) == 0x6bf) {
    for (local_8 = 0; local_8 < 0x1c; local_8 = local_8 + 1) {
      if (*(int *)(param_1 + 0x1cc + local_8 * 4) != 0) {
        *(float *)(*(int *)(param_1 + 0x1cc + local_8 * 4) + 0x74) =
             *(float *)(&DAT_0092e678 + local_8 * 0xc) + *(float *)(param_1 + 0x28);
        *(float *)(*(int *)(param_1 + 0x1cc + local_8 * 4) + 0x78) =
             *(float *)(&DAT_0092e67c + local_8 * 0xc) + *(float *)(param_1 + 0x30);
        *(float *)(*(int *)(param_1 + 0x1cc + local_8 * 4) + 0x7c) =
             *(float *)(&DAT_0092e680 + local_8 * 0xc) + *(float *)(param_1 + 0x2c);
      }
    }
  }
  return;
}

