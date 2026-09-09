// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f0d01 | Name: FUN_004f0d01


void __thiscall FUN_004f0d01(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  if (*(int *)(param_1 + 0x24) == 0x195) {
    *(undefined4 *)(param_1 + 0x2f4) = 0x3ff33333;
    *(undefined4 *)(param_1 + 0x2f8) = 0x3f800000;
  }
  if (*(int *)(param_1 + 0x24) == 0x647) {
    *(undefined4 *)(param_1 + 0x2f4) = 0x3f800000;
    *(undefined4 *)(param_1 + 0x2f8) = 0x3f99999a;
  }
  FUN_0053e435(param_2,param_3,param_4);
  *(undefined4 *)(param_1 + 0x2e0) = param_2;
  *(undefined4 *)(param_1 + 0x2e4) = param_4;
  return;
}

