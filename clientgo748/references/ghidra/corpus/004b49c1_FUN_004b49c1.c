// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b49c1 | Name: FUN_004b49c1


void FUN_004b49c1(undefined4 param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  void *pvVar2;
  undefined *local_8;
  
  local_8 = &DAT_005c9998;
  if (param_3 != 0) {
    local_8 = (undefined *)FUN_0058f716(param_3,&DAT_005ba120);
    if (local_8 != (undefined *)0x0) {
      FUN_00591a3e(local_8,0,2);
      iVar1 = FUN_005918bb(local_8);
      if (0xfffff < iVar1) {
        pvVar2 = _malloc(0x104);
        FUN_0058f62c(local_8,pvVar2);
        FUN_0058ee20(pvVar2,param_3);
        FUN_0058ee30(pvVar2,&DAT_005ba124);
        FUN_00591891(pvVar2);
        FUN_00591863(param_3,pvVar2);
        local_8 = (undefined *)FUN_0058f716(param_3,&DAT_005ba12c);
        FUN_0058ffd8(pvVar2);
      }
    }
  }
  if (local_8 == (undefined *)0x0) {
    local_8 = &DAT_005c9998;
  }
  FUN_004b4ac8(param_1,param_2,local_8);
  FUN_0058f62c(local_8);
  return;
}

