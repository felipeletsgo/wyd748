// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053277e | Name: FUN_0053277e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0053277e(int param_1,int param_2,int param_3)

{
  bool bVar1;
  
  *(int *)(param_1 + 0x20a20) = param_2;
  *(int *)(param_1 + 0x20a24) = param_3;
  *(float *)(param_1 + 0x20a28) =
       ((float)*(int *)(param_1 + 0x20a20) + (float)*(int *)(param_1 + 0x20a20)) * _DAT_005a5574;
  *(float *)(param_1 + 0x20a2c) =
       ((float)*(int *)(param_1 + 0x20a24) + (float)*(int *)(param_1 + 0x20a24)) * _DAT_005a5574;
  if (param_3 < 0x1a) {
    if ((((param_2 < 8) || (0xc < param_2)) || (param_3 < 0xb)) || (0xe < param_3)) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) {
      *(undefined4 *)(param_1 + 0xc4) = 3;
      DAT_005ccfa0 = 3;
    }
    else {
      if (((param_2 < 2) || (10 < param_2)) || (4 < param_3)) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if (bVar1) {
        *(undefined4 *)(param_1 + 0xc4) = 4;
        DAT_005ccfa0 = 4;
      }
      else if (((param_2 < 0x1a) || (0x1e < param_2)) || ((param_3 < 8 || (0xc < param_3)))) {
        *(undefined4 *)(param_1 + 0xc4) = 0;
        DAT_005ccfa0 = 0;
      }
      else {
        *(undefined4 *)(param_1 + 0xc4) = 5;
        DAT_005ccfa0 = 5;
      }
    }
  }
  else {
    if (((param_2 < 0x10) && (8 < param_2)) && (0x19 < param_3)) {
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    if (bVar1) {
      *(undefined4 *)(param_1 + 0xc4) = 2;
      DAT_005ccfa0 = 2;
    }
    else {
      *(undefined4 *)(param_1 + 0xc4) = 1;
      DAT_005ccfa0 = 1;
    }
  }
  return;
}

