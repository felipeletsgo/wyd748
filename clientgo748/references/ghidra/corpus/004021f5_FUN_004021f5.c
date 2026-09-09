// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004021f5 | Name: FUN_004021f5


void __thiscall FUN_004021f5(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0xcd8) = param_2;
  if (*(int *)(param_1 + 0xcd8) == 0) {
    *(undefined4 *)(param_1 + 0x724) = 0;
    *(undefined4 *)(param_1 + 0x890) = 0;
    *(undefined4 *)(param_1 + 0x9fc) = 0;
    *(undefined4 *)(param_1 + 0xb68) = 0;
  }
  else if (*(int *)(param_1 + 0xcd8) == 1) {
    *(undefined4 *)(param_1 + 0x724) = 1;
    *(undefined4 *)(param_1 + 0x890) = 1;
    *(undefined4 *)(param_1 + 0x9fc) = 1;
    *(undefined4 *)(param_1 + 0xb68) = 1;
  }
  else if (*(int *)(param_1 + 0xcd8) == 2) {
    *(undefined4 *)(param_1 + 0x724) = 1;
    *(undefined4 *)(param_1 + 0x890) = 1;
    *(undefined4 *)(param_1 + 0x9fc) = 1;
    *(undefined4 *)(param_1 + 0xb68) = 1;
    *(undefined4 *)(param_1 + 0xd1c) = 0xfffffffe;
    *(undefined4 *)(param_1 + 0x8cc) = 0xfffffffe;
    *(undefined4 *)(param_1 + 0xa38) = 0xfffffffe;
    *(undefined4 *)(param_1 + 0xba4) = 0xfffffffe;
  }
  return;
}

