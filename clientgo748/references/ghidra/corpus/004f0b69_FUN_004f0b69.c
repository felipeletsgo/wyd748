// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f0b69 | Name: FUN_004f0b69


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004f0b69(int param_1,float param_2,float param_3)

{
  *(float *)(param_1 + 0x28) = param_2 + *(float *)(param_1 + 0x28);
  *(float *)(param_1 + 0x2c) = param_3 + *(float *)(param_1 + 0x2c);
  if (*(float *)(param_1 + 0x28) <= _DAT_005a4290) {
    if (*(float *)(param_1 + 0x28) < _DAT_005a3534) {
      *(float *)(param_1 + 0x28) = *(float *)(param_1 + 0x28) + _DAT_005a4290;
    }
  }
  else {
    *(float *)(param_1 + 0x28) = *(float *)(param_1 + 0x28) - _DAT_005a4290;
  }
  if (*(float *)(param_1 + 0x2c) < _DAT_005a4380) {
    if (*(float *)(param_1 + 0x2c) <= _DAT_005a46bc) {
      *(undefined4 *)(param_1 + 0x2c) = 0xbfc8ef16;
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x2c) = 0x3fc8ef16;
  }
  return;
}

