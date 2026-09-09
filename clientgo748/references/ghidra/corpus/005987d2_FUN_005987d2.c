// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005987d2 | Name: FUN_005987d2


void FUN_005987d2(int param_1)

{
  undefined4 *puVar1;
  
  if (param_1 == 1) {
    puVar1 = (undefined4 *)FUN_00594c1d();
    *puVar1 = 0x21;
  }
  else if ((1 < param_1) && (param_1 < 4)) {
    puVar1 = (undefined4 *)FUN_00594c1d();
    *puVar1 = 0x22;
    return;
  }
  return;
}

