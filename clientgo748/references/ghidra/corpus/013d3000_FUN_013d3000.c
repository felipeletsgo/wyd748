// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013d3000 | Name: FUN_013d3000


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_013d3000(void)

{
  int iVar1;
  int iVar2;
  int unaff_EBP;
  
  iVar1 = *(int *)(unaff_EBP + -0x68);
  iVar2 = *(int *)(iVar1 + 0x7a0);
  if (iVar2 == 0x1d) {
    if ((*(short *)(iVar1 + 0x1a6) == 5) || (*(short *)(iVar1 + 0x1a6) == 10)) {
      *(undefined4 *)(unaff_EBP + -0x44) = 0xbe4ccccd;
      *(float *)(unaff_EBP + -0x40) = 1.0 / *(float *)(iVar1 + 0x7a4);
    }
  }
  else if (iVar2 == 0x1f) {
    if (*(short *)(iVar1 + 0x1a6) == 0x11) {
      *(undefined4 *)(unaff_EBP + -0x44) = 0xbf19999a;
    }
  }
  else if (iVar2 == 0x30) {
    *(float *)(unaff_EBP + -0x44) = *(float *)(unaff_EBP + -0x44) - _DAT_013d3208;
    *(float *)(unaff_EBP + -0x3c) = *(float *)(unaff_EBP + -0x3c) + _DAT_013d3200;
  }
  else if (iVar2 == 0x31) {
    *(float *)(unaff_EBP + -0x44) = *(float *)(unaff_EBP + -0x44) - _DAT_013d3200;
    if (*(short *)(iVar1 + 0x1a6) != 0x18) {
      *(float *)(unaff_EBP + -0x3c) = *(float *)(unaff_EBP + -0x3c) - _DAT_013d3200;
    }
  }
  else if (iVar2 == 0x32) {
    *(float *)(unaff_EBP + -0x44) = *(float *)(unaff_EBP + -0x44) - _DAT_013d3204;
    *(float *)(unaff_EBP + -0x3c) = *(float *)(unaff_EBP + -0x3c) - _DAT_013d3200;
  }
  else if (iVar2 == 0x33) {
    *(undefined4 *)(unaff_EBP + -0x44) = 0xbec28f5c;
    *(undefined4 *)(unaff_EBP + -0x3c) = 0x3e99999a;
  }
  else if (iVar2 == 0x3b) {
    *(undefined4 *)(unaff_EBP + -0x44) = 0xbe3851ec;
    *(undefined4 *)(unaff_EBP + -0x3c) = 0xbe4ccccd;
    *(undefined4 *)(unaff_EBP + -0x40) = 0x3f800000;
  }
  FUN_005042c7();
  return;
}

