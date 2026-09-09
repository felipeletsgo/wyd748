// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00421a4b | Name: FUN_00421a4b


void __fastcall FUN_00421a4b(undefined4 *param_1)

{
  *param_1 = &PTR_FUN_005a4100;
  if (param_1[0x67] != 0) {
    (**(code **)(*(int *)param_1[0x67] + 8))(param_1[0x67]);
    param_1[0x67] = 0;
  }
  return;
}

