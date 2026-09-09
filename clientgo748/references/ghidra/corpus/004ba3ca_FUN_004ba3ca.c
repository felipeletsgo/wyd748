// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ba3ca | Name: FUN_004ba3ca


undefined4 __thiscall
FUN_004ba3ca(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6,int param_7,
            int param_8,int param_9)

{
  undefined4 uVar1;
  int iVar2;
  int local_8c;
  int local_88;
  int local_84;
  int local_78;
  int local_74;
  int local_70;
  int local_60 [8];
  int local_40;
  undefined1 local_3c [4];
  int local_38;
  int *local_34;
  undefined1 local_30 [4];
  int local_2c;
  int *local_28;
  int local_24 [6];
  int local_c;
  
  if (DAT_005ba694 == 0x100) {
    param_8 = param_8 / 2;
    param_9 = param_9 / 2;
    param_4 = param_4 / 2;
    param_5 = param_5 / 2;
  }
  if (DAT_005ba694 < param_8) {
    param_8 = DAT_005ba694;
  }
  if (DAT_005ba698 < param_9) {
    param_9 = DAT_005ba698;
  }
  if ((*(int *)(*(int *)(param_1 + 0xe8328) + param_3 * 4) == 0) &&
     (FUN_004b8333(param_3,0xffffffff), *(int *)(*(int *)(param_1 + 0xe8328) + param_3 * 4) == 0)) {
    uVar1 = 0;
  }
  else {
    local_34 = (int *)0x0;
    local_28 = (int *)0x0;
    (**(code **)(**(int **)(*(int *)(param_1 + 0xe8328) + param_3 * 4) + 0x3c))
              (*(undefined4 *)(*(int *)(param_1 + 0xe8328) + param_3 * 4),0,&local_34);
    (**(code **)(*local_34 + 0x20))(local_34,local_24);
    (**(code **)(*local_34 + 0x24))(local_34,local_3c,0,0);
    if (*(int *)(*(int *)(param_1 + 0xe8328) + param_2 * 4) == 0) {
      local_40 = local_24[0];
      iVar2 = FUN_005602a0(*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),DAT_005ba694,DAT_005ba698,1,0,
                           local_24[0],1,*(int *)(param_1 + 0xe8328) + param_2 * 4);
      if (iVar2 < 0) {
        *(undefined4 *)(*(int *)(param_1 + 0xe8328) + param_2 * 4) = 0;
        if (local_34 != (int *)0x0) {
          (**(code **)(*local_34 + 8))(local_34);
        }
        return 0;
      }
      (**(code **)(**(int **)(*(int *)(param_1 + 0xe8328) + param_2 * 4) + 0x3c))
                (*(undefined4 *)(*(int *)(param_1 + 0xe8328) + param_2 * 4),0,&local_28);
    }
    else {
      (**(code **)(**(int **)(*(int *)(param_1 + 0xe8328) + param_2 * 4) + 0x3c))
                (*(undefined4 *)(*(int *)(param_1 + 0xe8328) + param_2 * 4),0,&local_28);
      (**(code **)(*local_28 + 0x20))(local_28,local_60);
      if (local_24[0] != local_60[0]) {
        if (local_28 != (int *)0x0) {
          (**(code **)(*local_28 + 8))(local_28);
          local_28 = (int *)0x0;
        }
        if (*(int *)(*(int *)(param_1 + 0xe8328) + param_2 * 4) != 0) {
          (**(code **)(**(int **)(*(int *)(param_1 + 0xe8328) + param_2 * 4) + 8))
                    (*(undefined4 *)(*(int *)(param_1 + 0xe8328) + param_2 * 4));
          *(undefined4 *)(*(int *)(param_1 + 0xe8328) + param_2 * 4) = 0;
        }
        iVar2 = FUN_005602a0(*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),DAT_005ba694,DAT_005ba698,1,0,
                             local_24[0],1,*(int *)(param_1 + 0xe8328) + param_2 * 4);
        if (iVar2 < 0) {
          *(undefined4 *)(*(int *)(param_1 + 0xe8328) + param_2 * 4) = 0;
          if (local_34 != (int *)0x0) {
            (**(code **)(*local_34 + 8))(local_34);
          }
          return 0;
        }
        (**(code **)(**(int **)(*(int *)(param_1 + 0xe8328) + param_2 * 4) + 0x3c))
                  (*(undefined4 *)(*(int *)(param_1 + 0xe8328) + param_2 * 4),0,&local_28);
      }
    }
    (**(code **)(*local_28 + 0x24))(local_28,local_30,0,0);
    if (((local_24[0] == 0x17) || (local_24[0] == 0x18)) || (local_24[0] == 0x19)) {
      if (DAT_005ba694 <= param_4 + param_8) {
        param_8 = DAT_005ba694 - param_4;
      }
      if (DAT_005ba698 <= param_5 + param_9) {
        param_9 = DAT_005ba698 - param_5;
      }
      if (DAT_005ba694 == 0x100) {
        for (local_70 = 0; local_70 < param_9; local_70 = local_70 + 1) {
          for (local_74 = 0; local_74 < param_8; local_74 = local_74 + 1) {
            *(undefined2 *)
             (local_2c + (param_5 + local_70) * DAT_005ba694 * 2 + param_4 * 2 + local_74 * 2) =
                 *(undefined2 *)
                  (local_38 + (param_7 + local_70) * local_c * 4 + param_6 * 2 + local_74 * 4);
          }
        }
      }
      else {
        for (local_78 = 0; local_78 < param_9; local_78 = local_78 + 1) {
          FUN_0058f220(local_2c + (param_5 + local_78) * DAT_005ba694 * 2 + param_4 * 2,
                       local_38 + (param_7 + local_78) * local_c * 2 + param_6 * 2,param_8 << 1);
        }
      }
    }
    else {
      if (DAT_005ba694 <= param_4 + param_8) {
        param_8 = DAT_005ba694 - param_4;
      }
      if (DAT_005ba698 <= param_5 + param_9) {
        param_9 = DAT_005ba698 - param_5;
      }
      if (DAT_005ba694 == 0x100) {
        for (local_84 = 0; local_84 < param_9; local_84 = local_84 + 1) {
          for (local_88 = 0; local_88 < param_8; local_88 = local_88 + 1) {
            *(undefined4 *)
             (local_2c + (param_5 + local_84) * DAT_005ba694 * 4 + param_4 * 4 + local_88 * 4) =
                 *(undefined4 *)
                  (local_38 + (param_7 + local_84) * local_c * 8 + param_6 * 4 + local_88 * 8);
          }
        }
      }
      else {
        for (local_8c = 0; local_8c < param_9; local_8c = local_8c + 1) {
          FUN_0058f220(local_2c + (param_5 + local_8c) * DAT_005ba694 * 4 + param_4 * 4,
                       local_38 + (param_7 + local_8c) * local_c * 4 + param_6 * 4,param_8 << 2);
        }
      }
    }
    (**(code **)(*local_34 + 0x28))(local_34);
    (**(code **)(*local_28 + 0x28))(local_28);
    if (local_34 != (int *)0x0) {
      (**(code **)(*local_34 + 8))(local_34);
      local_34 = (int *)0x0;
    }
    if (local_28 != (int *)0x0) {
      (**(code **)(*local_28 + 8))(local_28);
    }
    uVar1 = 1;
  }
  return uVar1;
}

