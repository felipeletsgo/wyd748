// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042b4ca | Name: FUN_0042b4ca


void __fastcall FUN_0042b4ca(int *param_1)

{
  FUN_0042bcb3();
  if ((param_1[0x20] == 0) && (*param_1 != 0)) {
    FUN_0058ea3a(*param_1);
    *param_1 = 0;
  }
  return;
}

