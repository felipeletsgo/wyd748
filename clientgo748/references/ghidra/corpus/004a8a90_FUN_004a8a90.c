// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a8a90 | Name: FUN_004a8a90


/* WARNING: Removing unreachable block (ram,0x004a8ada) */

undefined4 FUN_004a8a90(uint param_1)

{
  uint uVar1;
  
  FUN_005913b8(&DAT_00e22f00);
  uVar1 = FUN_0055888e();
  if (uVar1 % 7 == 0) {
    param_1 = param_1 & 0x80000001;
    if ((int)param_1 < 0) {
      param_1 = (param_1 - 1 | 0xfffffffe) + 1;
    }
    if ((uVar1 / 7 & 0x80000001) != param_1) {
      return 1;
    }
  }
  return 0;
}

