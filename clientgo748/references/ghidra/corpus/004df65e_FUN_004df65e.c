// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004df65e | Name: FUN_004df65e


void __thiscall FUN_004df65e(int param_1,uint param_2)

{
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x84) != 0) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x8c); local_8 = local_8 + 1) {
      *(uint *)(*(int *)(param_1 + 0x84) + 0xc + local_8 * 0x18) = param_2;
    }
    *(uint *)(param_1 + 0xa4) = param_2 >> 0x18;
    *(uint *)(param_1 + 0xa8) = (param_2 & 0xff0000) >> 0x10;
    *(uint *)(param_1 + 0xac) = (param_2 & 0xff00) >> 8;
    *(uint *)(param_1 + 0xb0) = param_2 & 0xff;
  }
  return;
}

