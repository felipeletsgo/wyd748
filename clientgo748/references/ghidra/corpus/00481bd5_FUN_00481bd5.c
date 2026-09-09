// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00481bd5 | Name: FUN_00481bd5


void __thiscall FUN_00481bd5(int param_1,char param_2)

{
  if (((*(int *)(param_1 + 0x28e34) != 0) && (*(int *)(param_1 + 0x28e38) != 0)) &&
     (*(int *)(param_1 + 0x28e3c) != 0)) {
    if (param_2 == '\x01') {
      *(undefined4 *)(*(int *)(param_1 + 0x28e34) + 0x30) = 0xc104cccd;
      *(undefined4 *)(*(int *)(param_1 + 0x28e38) + 0x78) = 0xc0975c29;
      *(undefined4 *)(*(int *)(param_1 + 0x28e3c) + 0x78) = 0xc0f9999a;
    }
    else {
      *(undefined4 *)(*(int *)(param_1 + 0x28e34) + 0x30) = 0xc1a00000;
      *(undefined4 *)(*(int *)(param_1 + 0x28e38) + 0x78) = 0xc1a00000;
      *(undefined4 *)(*(int *)(param_1 + 0x28e3c) + 0x78) = 0xc1a00000;
    }
  }
  return;
}

