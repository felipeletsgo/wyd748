// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00428344 | Name: FUN_00428344


void __fastcall FUN_00428344(int *param_1)

{
  FUN_00428412();
  if (param_1[4] != 0) {
    (**(code **)(*(int *)param_1[4] + 8))(param_1[4]);
    param_1[4] = 0;
  }
  if (param_1[3] != 0) {
    (**(code **)(*(int *)param_1[3] + 8))(param_1[3]);
    param_1[3] = 0;
  }
  if (param_1[2] != 0) {
    (**(code **)(*(int *)param_1[2] + 8))(param_1[2]);
    param_1[2] = 0;
  }
  if (param_1[1] != 0) {
    (**(code **)(*(int *)param_1[1] + 8))(param_1[1]);
    param_1[1] = 0;
  }
  if (*param_1 != 0) {
    (**(code **)(*(int *)*param_1 + 8))(*param_1);
    *param_1 = 0;
  }
  CoUninitialize();
  return;
}

