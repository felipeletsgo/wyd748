// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b2a50 | Name: FUN_004b2a50


void __fastcall FUN_004b2a50(undefined4 param_1)

{
  int iVar1;
  
  if (DAT_0067cf38 != (int *)0x0) {
    iVar1 = (**(code **)(*DAT_0067cf38 + 0x54))(param_1);
    if (iVar1 == 0) {
      (**(code **)(*DAT_013b71e0 + 0x1c))();
      FUN_004aeaeb();
    }
  }
  return;
}

