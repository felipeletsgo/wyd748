// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00567cf8 | Name: FUN_00567cf8


void FUN_00567cf8(void)

{
  int iVar1;
  
  iVar1 = FUN_00571112();
  if (iVar1 == 0) {
    PTR_FUN_005c57c0 = FUN_005679a3;
    PTR_FUN_005c57c4 = FUN_00567afa;
  }
  else {
    PTR_FUN_005c57c0 = FUN_00567be2;
    PTR_FUN_005c57c4 = FUN_00567be2;
  }
                    /* WARNING: Could not recover jumptable at 0x00567d2a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)PTR_FUN_005c57c0)();
  return;
}

