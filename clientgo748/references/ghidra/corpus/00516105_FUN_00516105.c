// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00516105 | Name: FUN_00516105


undefined4 __fastcall FUN_00516105(int param_1)

{
  undefined4 uVar1;
  
  if (((*(byte *)(param_1 + 0x44a) & 0xf) == 0) || (0xe < (*(byte *)(param_1 + 0x44a) & 0xf))) {
    if (((*(short *)(param_1 + 0x250) == 0x36) ||
        (((*(short *)(param_1 + 0x250) == 0x37 || (*(short *)(param_1 + 0x250) == 0x38)) ||
         (*(short *)(param_1 + 0x250) == 0x39)))) ||
       (((*(short *)(param_1 + 0x250) == 0x33 || (*(short *)(param_1 + 0x250) == 0x44)) ||
        (*(short *)(param_1 + 0x250) == 0x43)))) {
      uVar1 = 1;
    }
    else {
      uVar1 = 0;
    }
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

