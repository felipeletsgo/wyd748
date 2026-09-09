// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00442dd0 | Name: FUN_00442dd0


undefined4 FUN_00442dd0(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  char *pcVar4;
  void *pvVar5;
  int local_23c;
  int local_220;
  char local_21c [6];
  char local_216 [250];
  undefined1 local_11c [256];
  char local_1c [8];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f594;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_0058f716(param_2,&DAT_005b7bb4);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else if (param_1 == 0) {
    uVar2 = 0;
  }
  else {
    local_220 = 0;
    while ((local_220 < 100 && (iVar3 = FUN_00590a06(local_21c,0x100,iVar1), iVar3 != 0))) {
      _memset(local_1c,0,7);
      _strncpy(local_1c,local_21c,6);
      FUN_005909d2(local_1c,&DAT_005b7bb8,&local_14);
      pcVar4 = _strstr(local_21c,&DAT_005b7bbc);
      if (pcVar4 != (char *)0x0) {
        *pcVar4 = '\0';
      }
      if (local_216[0] == ' ') {
        FUN_0058f078(local_11c,&DAT_005b7bc0,local_216);
        pvVar5 = operator_new(0xe50);
        local_8 = 0;
        if (pvVar5 == (void *)0x0) {
          local_23c = 0;
        }
        else {
          local_23c = FUN_00407203(local_11c,local_14 | 0xff000000,0,0,0x43960000,0x41800000,0,
                                   0x77777777,1,0);
        }
        local_8 = 0xffffffff;
        if (local_23c != 0) {
          FUN_00408d33(local_23c);
        }
      }
      local_220 = local_220 + 1;
    }
    if (*(int *)(param_1 + 0x20c) != 0) {
      (**(code **)(**(int **)(param_1 + 0x20c) + 0x80))(0);
    }
    FUN_0058f62c(iVar1);
    uVar2 = 1;
  }
  ExceptionList = local_10;
  return uVar2;
}

