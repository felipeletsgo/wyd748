// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b9478 | Name: FUN_004b9478


undefined4 __thiscall FUN_004b9478(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_3c [8];
  int local_34;
  int local_30;
  undefined4 local_2c;
  int local_28;
  int local_24;
  undefined4 local_20;
  uint local_1c;
  void *local_18;
  uint local_14;
  undefined1 local_10;
  undefined1 local_c [8];
  
  if (*(int *)(*(int *)(param_1 + 0xe8330) + param_2 * 4) == 0) {
    local_28 = 0;
    for (local_24 = 0; local_24 < 0xff; local_24 = local_24 + 1) {
      if ((*(char *)(param_1 + 0x43328 + local_24 + param_2 * 0x108) == '.') &&
         (iVar1 = _strcmp((char *)(param_1 + 0x43328 + local_24 + param_2 * 0x108),&DAT_005ba958),
         iVar1 == 0)) {
        local_28 = 1;
        break;
      }
    }
    local_18 = (void *)0x0;
    local_30 = FUN_0058fb07(param_1 + 0x43328 + param_2 * 0x108,0x8000,0);
    if (local_30 == -1) {
      uVar2 = 0;
    }
    else {
      local_10 = 0;
      iVar1 = FUN_0058f83f(local_30);
      local_2c = *(undefined4 *)(DAT_005ccf9c + 0x2a514);
      local_34 = 0x12;
      local_20 = 0xff000000;
      if (local_28 == 0) {
        local_14 = iVar1 - 4;
        local_18 = operator_new(iVar1 + 0xe);
        FUN_0058f8c9(local_30,local_c,4);
        FUN_0058f8c9(local_30,local_18,local_14);
        FUN_0058f220((int)local_18 + local_14,param_1 + 0x304,0x12);
        FUN_0058f75f(local_30);
        if (*(char *)(param_1 + 0x43427 + param_2 * 0x108) == 'A') {
          if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) || (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10))
          {
            local_2c = 0x1a;
          }
          else {
            local_2c = 0;
          }
        }
        else if (*(char *)(param_1 + 0x43427 + param_2 * 0x108) == 'a') {
          if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) || (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10))
          {
            local_2c = 0x19;
          }
          else {
            local_2c = 0;
          }
        }
      }
      else {
        local_14 = iVar1 - 1;
        local_34 = 0;
        local_18 = operator_new(local_14);
        FUN_0058f8c9(local_30,local_c,1);
        FUN_0058f8c9(local_30,local_18,local_14);
        FUN_0058f220(local_18,param_1 + 0x317,3);
        if (*(char *)((int)local_18 + 0x54) == '2') {
          FUN_0058f078(local_3c,&DAT_005ba960);
          FUN_0058f220((int)local_18 + 0x54,local_3c,4);
        }
        else {
          FUN_0058f078(local_3c,&DAT_005ba968);
          FUN_0058f220((int)local_18 + 0x54,local_3c,4);
        }
        FUN_0058f75f(local_30);
        if (((*(int *)(DAT_005ccf9c + 0x2a5c0) == 1) && (*(int *)(DAT_005ccf9c + 0x2a5c4) == 1)) &&
           (DAT_005b4924 == 1)) {
          local_20 = 0;
          local_2c = 0;
        }
        else {
          local_20 = 0;
          local_2c = *(undefined4 *)(DAT_005ccf9c + 0x2a514);
          if ((*(char *)(param_1 + 0x43427 + param_2 * 0x108) == 'A') ||
             (*(char *)(param_1 + 0x43427 + param_2 * 0x108) == 'C')) {
            if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) ||
               (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10)) {
              local_2c = 0x1a;
            }
            else {
              local_2c = 0x15;
            }
          }
          else if (*(char *)(param_1 + 0x43427 + param_2 * 0x108) == 'a') {
            if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) ||
               (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10)) {
              local_2c = 0x19;
            }
            else {
              local_2c = 0x15;
            }
          }
          if (*(char *)(param_1 + 0x43427 + param_2 * 0x108) == 'N') {
            if ((*(int *)(DAT_005ccf9c + 0x2a390) == 1) ||
               (*(int *)(DAT_005ccf9c + 0x2a510) == 0x10)) {
              local_2c = 0x18;
            }
            else {
              local_2c = 0x16;
            }
          }
        }
      }
      local_1c = 0;
      if (DAT_005b4920 < 0x32) {
        if ((DAT_005b4920 < 0x32) && (0x27 < DAT_005b4920)) {
          local_1c = (uint)(0xb < param_2);
        }
        else if ((DAT_005b4920 < 0x28) && (0x1d < DAT_005b4920)) {
          if ((param_2 < 400) && (0xb < param_2)) {
            local_1c = 1;
          }
        }
        else if (DAT_005b4920 < 0x1e) {
          local_1c = 0;
        }
      }
      else {
        local_1c = 1;
      }
      if (local_1c == 0) {
        iVar1 = FUN_00560eab(*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),local_18,local_14 + local_34,
                             0xffffffff,0xffffffff,1,0,local_2c,1,1,1,local_20,0,0,
                             *(int *)(param_1 + 0xe8330) + param_2 * 4);
        if (iVar1 < 0) {
          *(undefined4 *)(*(int *)(param_1 + 0xe8330) + param_2 * 4) = 0;
          FUN_0058ea3a(local_18);
          return 0;
        }
      }
      else {
        iVar1 = FUN_00560eab(*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),local_18,local_14 + local_34,
                             0xffffffff,0xffffffff,4,0,local_2c,1,5,5,local_20,0,0,
                             *(int *)(param_1 + 0xe8330) + param_2 * 4);
        if (iVar1 < 0) {
          *(undefined4 *)(*(int *)(param_1 + 0xe8330) + param_2 * 4) = 0;
          FUN_0058ea3a(local_18);
          return 0;
        }
      }
      FUN_0058ea3a(local_18);
      uVar2 = 1;
    }
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

