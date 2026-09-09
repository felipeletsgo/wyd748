// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e5f4b | Name: FUN_004e5f4b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004e5f4b(undefined4 *param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  float fVar3;
  void *pvVar4;
  int local_3c;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_005a1122;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a4ff8;
  local_8 = 0;
  if (param_1[0x2b] != 0) {
    if ((undefined4 *)param_1[0x2b] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x2b])(1);
    }
    param_1[0x2b] = 0;
  }
  uVar1 = param_1[0x1d];
  uVar2 = param_1[0x1f];
  fVar3 = (float)param_1[0x1e] - _DAT_005a3660;
  if (DAT_0092e660 == 0) {
    pvVar4 = operator_new(0xb0);
    local_8._0_1_ = 1;
    if (pvVar4 == (void *)0x0) {
      local_3c = 0;
    }
    else {
      local_3c = FUN_004e3ba0(uVar1,fVar3,uVar2,1,0,0);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    if (local_3c != 0) {
      FUN_0054ac09(local_3c);
    }
  }
  local_8 = 0xffffffff;
  FUN_004c8d6c();
  ExceptionList = local_10;
  return;
}

