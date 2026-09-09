// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049e77b | Name: FUN_0049e77b


undefined4 FUN_0049e77b(int param_1,undefined4 param_2,int param_3,int param_4)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  char *pcVar4;
  void *pvVar5;
  int local_254;
  int local_250;
  int local_238;
  int local_228;
  int local_224;
  char local_220 [10];
  char local_216 [246];
  undefined1 local_120 [256];
  char local_20 [12];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0202;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_0058f716(param_2,&DAT_005b9244);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else if (param_1 == 0) {
    uVar2 = 0;
  }
  else {
    local_224 = 300;
    FUN_004091ab();
    local_228 = 0;
    while ((local_228 < 100 && (iVar3 = FUN_00590a06(local_220,0x100,iVar1), iVar3 != 0))) {
      _memset(local_20,0,0xb);
      _strncpy(local_20,local_220,10);
      FUN_005909d2(local_20,s__d__x_005b9248,&local_224,&local_14);
      if (param_3 < local_224) {
        local_14 = 0xff777777;
      }
      if ((0x15e < local_224) && (param_4 < 6)) {
        local_14 = 0xff777777;
      }
      if (DAT_0067cf38 != 0) {
        iVar3 = FUN_004ff388();
        if (iVar3 == 2) {
          local_14 = 0xff777777;
        }
        else if (param_3 < local_224) {
          local_14 = 0xff777777;
        }
      }
      pcVar4 = _strstr(local_220,&DAT_005b9250);
      if (pcVar4 != (char *)0x0) {
        *pcVar4 = '\0';
      }
      if (local_216[0] == ' ') {
        FUN_0058f078(local_120,&DAT_005b9254,local_216);
        if (DAT_005b892c == 2) {
          pvVar5 = operator_new(0xe50);
          local_8 = 0;
          if (pvVar5 == (void *)0x0) {
            local_250 = 0;
          }
          else {
            local_250 = FUN_00407203(local_120,local_14 | 0xff000000,0,0,
                                     *(undefined4 *)(param_1 + 0x54),0x41800000,0,0x77777777,1,0);
          }
          local_238 = local_250;
        }
        else {
          pvVar5 = operator_new(0xe50);
          local_8 = 1;
          if (pvVar5 == (void *)0x0) {
            local_254 = 0;
          }
          else {
            local_254 = FUN_00407203(local_120,local_14 | 0xff000000,0,0,
                                     *(undefined4 *)(param_1 + 0x54),0x41800000,0,0x77777777,1,0);
          }
          local_238 = local_254;
        }
        local_8 = 0xffffffff;
        if (local_238 != 0) {
          FUN_00408d33(local_238);
        }
      }
      local_228 = local_228 + 1;
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

