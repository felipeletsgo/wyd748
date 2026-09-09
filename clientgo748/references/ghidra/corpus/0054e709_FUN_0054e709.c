// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054e709 | Name: FUN_0054e709


undefined4 FUN_0054e709(short *param_1)

{
  undefined4 uVar1;
  
  if ((*param_1 < 0x938) && (0x955 < *param_1)) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(undefined4 *)(&DAT_005be3a0 + ((int)(char)param_1[2] / 10) * 4);
  }
  return uVar1;
}

