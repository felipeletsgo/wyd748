// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040e2e6 | Name: FUN_0040e2e6


void __fastcall FUN_0040e2e6(undefined4 *param_1)

{
  int iVar1;
  int local_1c;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059ee7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a4024;
  local_8 = 0;
  FUN_0040e4ee();
  iVar1 = *(int *)(DAT_0067cf38 + 0x28);
  if (iVar1 != 0) {
    if (param_1[0x7e] == 1) {
      for (local_18 = 0; local_18 < (int)param_1[0x7a]; local_18 = local_18 + 1) {
        for (local_1c = 0; local_1c < (int)param_1[0x7b]; local_1c = local_1c + 1) {
          if ((*(int *)(param_1[0x7c] + (local_1c + local_18 * param_1[0x7b]) * 4) == 0) &&
             (-1 < *(int *)(param_1[0x7d] + 0x148 + (local_1c + local_18 * param_1[0x7b]) * 0x16c)))
          {
            FUN_0040c1b0(iVar1 + 0x3c,param_1[0x7d] + (local_1c + local_18 * param_1[0x7b]) * 0x16c,
                         *(undefined4 *)
                          (param_1[0x7d] + 0x148 + (local_1c + local_18 * param_1[0x7b]) * 0x16c));
          }
        }
      }
    }
    if (-1 < (int)param_1[0x155]) {
      FUN_0040c1b0(iVar1 + 0x3c,param_1 + 0x103,param_1[0x155]);
    }
  }
  if (param_1[0x7c] != 0) {
    FUN_0058ea3a(param_1[0x7c]);
    param_1[0x7c] = 0;
  }
  if (param_1[0x7d] != 0) {
    FUN_0058ea3a(param_1[0x7d]);
    param_1[0x7d] = 0;
  }
  local_8 = 0xffffffff;
  FUN_0040151a();
  ExceptionList = local_10;
  return;
}

