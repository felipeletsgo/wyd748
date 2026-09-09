// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b8d26 | Name: FUN_004b8d26


undefined4 __thiscall FUN_004b8d26(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_38 [8];
  int local_30;
  int local_2c;
  undefined4 local_28;
  int local_24;
  int local_20;
  undefined4 local_1c;
  uint local_18;
  void *local_14;
  uint local_10;
  undefined1 local_c [8];
  
  if (*(int *)(*(int *)(param_1 + 0xe832c) + param_2 * 4) == 0) {
    local_24 = 0;
    for (local_20 = 0; local_20 < 0xff; local_20 = local_20 + 1) {
      if ((*(char *)(param_1 + 0x22328 + local_20 + param_2 * 0x108) == '.') &&
         (iVar1 = _strcmp((char *)(param_1 + 0x22328 + local_20 + param_2 * 0x108),&DAT_005ba8d8),
         iVar1 == 0)) {
        local_24 = 1;
        break;
      }
    }
    local_30 = 0x12;
    local_1c = 0xff000000;
    local_14 = (void *)0x0;
    local_2c = FUN_0058fb07(param_1 + 0x22328 + param_2 * 0x108,0x8000,0);
    if (local_2c == -1) {
      uVar2 = 0;
    }
    else {
      iVar1 = FUN_0058f83f(local_2c);
      local_28 = *(undefined4 *)(DAT_005ccf9c + 0x2a514);
      if (local_24 == 0) {
        local_10 = iVar1 - 4;
        local_14 = operator_new(local_10 + local_30);
        FUN_0058f8c9(local_2c,local_c,4);
        FUN_0058f8c9(local_2c,local_14,local_10);
        FUN_0058f220((int)local_14 + local_10,param_1 + 0x304,local_30);
        FUN_0058f75f(local_2c);
        if (*(char *)(param_1 + 0x22427 + param_2 * 0x108) == 'A') {
          if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) || (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10))
          {
            local_28 = 0x1a;
          }
          else {
            local_28 = 0;
          }
        }
        else if (*(char *)(param_1 + 0x22427 + param_2 * 0x108) == 'a') {
          if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) || (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10))
          {
            local_28 = 0x19;
          }
          else {
            local_28 = 0;
          }
        }
      }
      else {
        local_10 = iVar1 - 1;
        local_30 = 0;
        local_14 = operator_new(local_10);
        FUN_0058f8c9(local_2c,local_c,1);
        FUN_0058f8c9(local_2c,local_14,local_10);
        FUN_0058f220(local_14,param_1 + 0x317,3);
        if (*(char *)((int)local_14 + 0x54) == '2') {
          FUN_0058f078(local_38,&DAT_005ba8e0);
          FUN_0058f220((int)local_14 + 0x54,local_38,4);
        }
        else {
          FUN_0058f078(local_38,&DAT_005ba8e8);
          FUN_0058f220((int)local_14 + 0x54,local_38,4);
        }
        FUN_0058f75f(local_2c);
        if (((*(int *)(DAT_005ccf9c + 0x2a5c0) == 1) && (*(int *)(DAT_005ccf9c + 0x2a5c4) == 1)) &&
           (DAT_005b4924 == 1)) {
          local_1c = 0;
          local_28 = 0;
        }
        else {
          local_1c = 0;
          local_28 = *(undefined4 *)(DAT_005ccf9c + 0x2a514);
          if ((*(char *)(param_1 + 0x22427 + param_2 * 0x108) == 'A') ||
             (*(char *)(param_1 + 0x22427 + param_2 * 0x108) == 'C')) {
            if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) ||
               (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10)) {
              local_28 = 0x1a;
            }
            else {
              local_28 = 0x15;
            }
          }
          else if (*(char *)(param_1 + 0x22427 + param_2 * 0x108) == 'a') {
            if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) ||
               (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10)) {
              local_28 = 0x19;
            }
            else {
              local_28 = 0x15;
            }
          }
          if (*(char *)(param_1 + 0x22427 + param_2 * 0x108) == 'N') {
            if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) ||
               (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10)) {
              local_28 = 0x18;
            }
            else {
              local_28 = 0x16;
            }
          }
        }
      }
      local_18 = (uint)(9 < DAT_005b4920);
      if (local_18 == 0) {
        iVar1 = FUN_00560eab(*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),local_14,local_10 + local_30,
                             0xffffffff,0xffffffff,1,0,local_28,1,1,1,local_1c,0,0,
                             *(int *)(param_1 + 0xe832c) + param_2 * 4);
        if (iVar1 < 0) {
          *(undefined4 *)(*(int *)(param_1 + 0xe832c) + param_2 * 4) = 0;
          FUN_0058ea3a(local_14);
          return 0;
        }
      }
      else {
        iVar1 = FUN_00560eab(*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),local_14,local_10 + local_30,
                             0xffffffff,0xffffffff,4,0,local_28,1,5,5,local_1c,0,0,
                             *(int *)(param_1 + 0xe832c) + param_2 * 4);
        if (iVar1 < 0) {
          *(undefined4 *)(*(int *)(param_1 + 0xe832c) + param_2 * 4) = 0;
          FUN_0058ea3a(local_14);
          return 0;
        }
      }
      FUN_0058ea3a(local_14);
      uVar2 = 1;
    }
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

