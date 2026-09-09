// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b8333 | Name: FUN_004b8333


undefined4 __thiscall FUN_004b8333(int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_34 [8];
  int local_2c;
  int local_28;
  undefined4 local_24;
  int local_20;
  int local_1c;
  undefined4 local_18;
  void *local_14;
  uint local_10;
  undefined1 local_c [8];
  
  if (param_3 < 0) {
    param_3 = param_2;
  }
  if (*(int *)(*(int *)(param_1 + 0xe8328) + param_2 * 4) == 0) {
    local_20 = 0;
    for (local_1c = 0; local_1c < 0xff; local_1c = local_1c + 1) {
      if ((*(char *)(param_1 + 0x1328 + local_1c + param_2 * 0x108) == '.') &&
         (iVar1 = _strcmp((char *)(param_1 + 0x1328 + local_1c + param_2 * 0x108),&DAT_005ba7d0),
         iVar1 == 0)) {
        local_20 = 1;
        break;
      }
    }
    local_14 = (void *)0x0;
    local_28 = FUN_0058fb07(param_1 + 0x1328 + param_3 * 0x108,0x8000,0);
    if (local_28 == -1) {
      uVar2 = 0;
    }
    else {
      iVar1 = FUN_0058f83f(local_28);
      local_24 = *(undefined4 *)(DAT_005ccf9c + 0x2a514);
      local_2c = 0x12;
      local_18 = 0xff000000;
      if (local_20 == 0) {
        local_10 = iVar1 - 4;
        local_14 = operator_new(iVar1 + 0xe);
        FUN_0058f8c9(local_28,local_c,4);
        FUN_0058f8c9(local_28,local_14,local_10);
        FUN_0058f220((int)local_14 + local_10,param_1 + 0x304,0x12);
        FUN_0058f75f(local_28);
        if (*(char *)(param_1 + 0x1427 + param_2 * 0x108) == 'A') {
          if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) || (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10))
          {
            local_24 = 0x1a;
          }
          else {
            local_24 = 0;
          }
        }
        else if (*(char *)(param_1 + 0x1427 + param_2 * 0x108) == 'a') {
          if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) || (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10))
          {
            local_24 = 0x19;
          }
          else {
            local_24 = 0;
          }
        }
      }
      else {
        local_10 = iVar1 - 1;
        local_2c = 0;
        local_14 = operator_new(local_10);
        FUN_0058f8c9(local_28,local_c,1);
        FUN_0058f8c9(local_28,local_14,local_10);
        FUN_0058f220(local_14,param_1 + 0x317,3);
        if (*(char *)((int)local_14 + 0x54) == '2') {
          FUN_0058f078(local_34,&DAT_005ba7d8);
          FUN_0058f220((int)local_14 + 0x54,local_34,4);
        }
        else {
          FUN_0058f078(local_34,&DAT_005ba7e0);
          FUN_0058f220((int)local_14 + 0x54,local_34,4);
        }
        FUN_0058f75f(local_28);
        if (((*(int *)(DAT_005ccf9c + 0x2a5c0) == 1) && (*(int *)(DAT_005ccf9c + 0x2a5c4) == 1)) &&
           (DAT_005b4924 == 1)) {
          local_18 = 0;
          local_24 = 0;
        }
        else {
          local_18 = 0;
          local_24 = *(undefined4 *)(DAT_005ccf9c + 0x2a514);
          if (((*(char *)(param_1 + 0x1427 + param_2 * 0x108) == 'A') ||
              (*(char *)(param_1 + 0x1427 + param_2 * 0x108) == 'a')) ||
             (*(char *)(param_1 + 0x1427 + param_2 * 0x108) == 'C')) {
            if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) ||
               (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10)) {
              local_24 = 0x1a;
            }
            else {
              local_24 = 0x15;
            }
          }
          if (*(char *)(param_1 + 0x1427 + param_2 * 0x108) == 'N') {
            if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) ||
               (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10)) {
              local_24 = 0x18;
            }
            else {
              local_24 = 0x16;
            }
          }
        }
      }
      iVar1 = FUN_00560eab(*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),local_14,local_10 + local_2c,
                           0xffffffff,0xffffffff,1,0,local_24,1,1,1,local_18,0,0,
                           *(int *)(param_1 + 0xe8328) + param_2 * 4);
      if (iVar1 < 0) {
        if (*(int *)(*(int *)(param_1 + 0xe8328) + param_2 * 4) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0xe8328) + param_2 * 4) = 0;
        }
        FUN_0058ea3a(local_14);
        uVar2 = 0;
      }
      else {
        FUN_0058ea3a(local_14);
        uVar2 = 1;
      }
    }
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

