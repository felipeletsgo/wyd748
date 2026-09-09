// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004fa8e3 | Name: FUN_004fa8e3


void FUN_004fa8e3(void)

{
  int unaff_EBP;
  bool in_ZF;
  
  if ((((in_ZF) || (*(int *)(*(int *)(unaff_EBP + -4) + 0x7a0) == 0x28)) ||
      ((*(int *)(*(int *)(unaff_EBP + -4) + 0x7a0) == 0x14 &&
       (*(short *)(*(int *)(unaff_EBP + -4) + 0x1a2) != 7)))) ||
     (*(int *)(*(int *)(unaff_EBP + -4) + 0x7a0) == 0x27)) {
    if (*(char *)(*(int *)(unaff_EBP + -4) + 0x77f) == '\x01') {
      if (DAT_005d0408 == 1) {
        *(undefined4 *)(*(int *)(unaff_EBP + -4) + 0x220) = 0x40a00000;
      }
      else {
        *(undefined4 *)(*(int *)(unaff_EBP + -4) + 0x220) = 0x40000000;
      }
    }
    else if (DAT_005d0408 == 1) {
      *(undefined4 *)(*(int *)(unaff_EBP + -4) + 0x220) = 0x40c00000;
    }
    else {
      *(undefined4 *)(*(int *)(unaff_EBP + -4) + 0x220) = 0x40400000;
    }
  }
  else {
    *(int *)(unaff_EBP + -8) =
         *(int *)(&DAT_005c07e8 + *(short *)(*(int *)(unaff_EBP + -4) + 0x1a0) * 0x18) -
         (int)*(char *)(*(int *)(unaff_EBP + -4) + 0x77f);
    *(float *)(*(int *)(unaff_EBP + -4) + 0x220) = (float)*(int *)(unaff_EBP + -8);
  }
  DAT_005bb418 = __ftol();
  return;
}

