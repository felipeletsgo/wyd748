// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e7f8 | Name: FUN_0059e7f8


void __fastcall FUN_0059e7f8(undefined4 *param_1)

{
  *param_1 = &type_info::vftable;
  FUN_00596aad(0x1b);
  if (param_1[1] != 0) {
    FUN_0058ffd8(param_1[1]);
  }
  FUN_00596b0e(0x1b);
  return;
}

