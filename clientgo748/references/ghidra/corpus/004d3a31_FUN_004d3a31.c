// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d3a31 | Name: FUN_004d3a31


undefined4 __fastcall FUN_004d3a31(int *param_1)

{
  int iVar1;
  int iVar2;
  
  if ((((param_1[6] != -1) && (param_1[6] != DAT_005bdef0)) && (param_1[6] != DAT_005bdef4)) &&
     ((param_1[6] != DAT_005bdef8 && (param_1[6] != DAT_005bdefc)))) {
    if (param_1[0x21] != 0x1fa) {
      return 0;
    }
    if (*(int *)(DAT_0067cf38 + 0x24) != 0x7531) {
      return 0;
    }
  }
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  iVar2 = FUN_004b14fd(param_1[0x21],1,180000);
  if (iVar2 != 0) {
    param_1[0x1c] = *(int *)(iVar2 + 0x340);
  }
  (**(code **)(*param_1 + 0x38))();
  param_1[0x2a] =
       (int)((float)((uint)(iVar1 - param_1[0x2d]) % (uint)param_1[0x2e]) / (float)param_1[0x2e]);
  if ((param_1[0x2c] != 0) && ((uint)param_1[0x2c] < (uint)(iVar1 - param_1[0x2d]))) {
    param_1[10] = 0;
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  return 1;
}

