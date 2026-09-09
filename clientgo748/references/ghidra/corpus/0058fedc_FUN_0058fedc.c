// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058fedc | Name: FUN_0058fedc


void FUN_0058fedc(uint param_1)

{
  int iVar1;
  uint dwBytes;
  void *local_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &DAT_005a9f90;
  puStack_10 = &LAB_00596b2c;
  local_14 = ExceptionList;
  if (DAT_013b8608 == 3) {
    ExceptionList = &local_14;
    if (param_1 <= DAT_013b8600) {
      ExceptionList = &local_14;
      FUN_00596aad(9);
      local_8 = 0;
      iVar1 = FUN_00595c0c(param_1);
      local_8 = 0xffffffff;
      FUN_0058ff43();
      if (iVar1 != 0) {
        ExceptionList = local_14;
        return;
      }
    }
  }
  else {
    ExceptionList = &local_14;
    if (DAT_013b8608 == 2) {
      if (param_1 == 0) {
        dwBytes = 0x10;
      }
      else {
        dwBytes = param_1 + 0xf & 0xfffffff0;
      }
      ExceptionList = &local_14;
      if (dwBytes <= DAT_005cbf5c) {
        ExceptionList = &local_14;
        FUN_00596aad(9);
        local_8 = 1;
        iVar1 = FUN_005966af(dwBytes >> 4);
        local_8 = 0xffffffff;
        FUN_0058ffa2();
        if (iVar1 != 0) {
          ExceptionList = local_14;
          return;
        }
      }
      goto LAB_0058ffbb;
    }
  }
  if (param_1 == 0) {
    param_1 = 1;
  }
  dwBytes = param_1 + 0xf & 0xfffffff0;
LAB_0058ffbb:
  HeapAlloc(DAT_013b8604,0,dwBytes);
  ExceptionList = local_14;
  return;
}

