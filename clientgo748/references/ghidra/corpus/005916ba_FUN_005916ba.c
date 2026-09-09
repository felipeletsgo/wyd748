// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005916ba | Name: FUN_005916ba


LPVOID FUN_005916ba(int param_1,int param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint _Size;
  LPVOID local_24;
  void *local_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &DAT_005aa028;
  puStack_10 = &LAB_00596b2c;
  local_14 = ExceptionList;
  uVar2 = param_1 * param_2;
  uVar3 = uVar2;
  ExceptionList = &local_14;
  if (uVar2 < 0xffffffe1) {
    if (uVar2 == 0) {
      uVar3 = 1;
    }
    uVar3 = uVar3 + 0xf & 0xfffffff0;
    ExceptionList = &local_14;
  }
  do {
    local_24 = (LPVOID)0x0;
    if (uVar3 < 0xffffffe1) {
      if (DAT_013b8608 == 3) {
        if (uVar2 <= DAT_013b8600) {
          FUN_00596aad(9);
          local_8 = 0;
          local_24 = (LPVOID)FUN_00595c0c(uVar2);
          local_8 = 0xffffffff;
          FUN_00591753();
          _Size = uVar2;
          if (local_24 == (void *)0x0) goto LAB_005917a7;
LAB_00591796:
          _memset(local_24,0,_Size);
        }
LAB_005917a2:
        if (local_24 != (LPVOID)0x0) {
          ExceptionList = local_14;
          return local_24;
        }
      }
      else {
        if ((DAT_013b8608 != 2) || (DAT_005cbf5c < uVar3)) goto LAB_005917a2;
        FUN_00596aad(9);
        local_8 = 1;
        local_24 = (LPVOID)FUN_005966af(uVar3 >> 4);
        local_8 = 0xffffffff;
        FUN_005917dc();
        _Size = uVar3;
        if (local_24 != (void *)0x0) goto LAB_00591796;
      }
LAB_005917a7:
      local_24 = HeapAlloc(DAT_013b8604,8,uVar3);
    }
    if (local_24 != (LPVOID)0x0) {
      ExceptionList = local_14;
      return local_24;
    }
    if (DAT_013b804c == 0) {
      ExceptionList = local_14;
      return (LPVOID)0x0;
    }
    iVar1 = FUN_00595683(uVar3);
    if (iVar1 == 0) {
      ExceptionList = local_14;
      return (LPVOID)0x0;
    }
  } while( true );
}

