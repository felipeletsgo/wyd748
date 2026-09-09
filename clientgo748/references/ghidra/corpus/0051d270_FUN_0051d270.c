// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0051d270 | Name: FUN_0051d270


void __thiscall FUN_0051d270(int param_1,uint param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined2 local_20 [4];
  int local_18;
  undefined4 local_14;
  undefined2 local_10 [4];
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    _memset(local_20,0,8);
    _memset(local_10,0,8);
    local_20[0] = *(undefined2 *)(param_1 + 0x66c);
    local_10[0] = *(undefined2 *)(param_1 + 0x66e);
    local_14 = FUN_0054cd07(local_20,0x11);
    local_18 = FUN_0054cd07(local_10,0x11);
    local_8 = 0x79;
    if (*(int *)(param_1 + 0x5c) == 3) {
      if ((((param_3 == 0) && (*(int *)(param_1 + 0x160) != 0)) ||
          ((param_3 == 1 && ((local_18 != 0 && (local_18 != 0x80)))))) &&
         ((DAT_005ccf98 != 0 && (iVar1 = FUN_00429a6d(0x79), iVar1 != 0)))) {
        FUN_00429a6d(local_8);
        iVar1 = FUN_0042afd0();
        if ((iVar1 == 0) || (*(short *)(param_1 + 0x7b4) != param_3)) {
          uVar4 = 0;
          uVar3 = 0;
          FUN_00429a6d(local_8);
          FUN_0042ad2b(uVar3,uVar4);
        }
      }
      *(undefined2 *)(param_1 + 0x7b4) = (undefined2)param_3;
    }
    else if (*(int *)(param_1 + 0x24c) == 0x28) {
      if (((((param_3 == 0) && (*(int *)(param_1 + 0x160) != 0)) ||
           ((param_3 == 1 && ((local_18 != 0 && (local_18 != 0x80)))))) && (DAT_005ccf98 != 0)) &&
         (iVar1 = FUN_00429a6d(0x79), iVar1 != 0)) {
        FUN_00429a6d(local_8);
        iVar1 = FUN_0042afd0();
        if ((iVar1 == 0) || (*(short *)(param_1 + 0x7b4) != param_3)) {
          uVar4 = 0;
          uVar3 = 0;
          FUN_00429a6d(local_8);
          FUN_0042ad2b(uVar3,uVar4);
        }
      }
      *(undefined2 *)(param_1 + 0x7b4) = (undefined2)param_3;
    }
    else {
      if (((*(int *)(param_1 + 0x160) == 1) && (param_3 == 0)) ||
         ((*(int *)(param_1 + 0x164) == 1 && (param_3 == 1)))) {
        if (((*(short *)(&DAT_00d44996 + *(short *)(param_1 + 0x66c) * 0x8c) < 0x5b) ||
            (param_3 != 0)) &&
           ((*(short *)(&DAT_00d44996 + *(short *)(param_1 + 0x66e) * 0x8c) < 0x5b || (param_3 != 0)
            ))) {
          uVar2 = param_2 & 0x80000001;
          if ((int)uVar2 < 0) {
            uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
          }
          if (uVar2 == 0) {
            local_8 = 0x79;
          }
          else {
            param_2 = param_2 & 0x80000001;
            if ((int)param_2 < 0) {
              param_2 = (param_2 - 1 | 0xfffffffe) + 1;
            }
            if (param_2 == 1) {
              local_8 = 0x7a;
            }
          }
        }
        else {
          uVar2 = param_2 & 0x80000001;
          if ((int)uVar2 < 0) {
            uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
          }
          if (uVar2 == 0) {
            local_8 = 0x7c;
          }
          else {
            param_2 = param_2 & 0x80000001;
            if ((int)param_2 < 0) {
              param_2 = (param_2 - 1 | 0xfffffffe) + 1;
            }
            if (param_2 == 1) {
              local_8 = 0x7d;
            }
          }
        }
      }
      else if (((*(int *)(param_1 + 0x160) == 2) && (param_3 == 0)) ||
              ((*(int *)(param_1 + 0x164) == 2 && (param_3 == 1)))) {
        uVar2 = param_2 & 0x80000001;
        if ((int)uVar2 < 0) {
          uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
        }
        if (uVar2 == 0) {
          local_8 = 0x7c;
        }
        else {
          param_2 = param_2 & 0x80000001;
          if ((int)param_2 < 0) {
            param_2 = (param_2 - 1 | 0xfffffffe) + 1;
          }
          if (param_2 == 1) {
            local_8 = 0x7d;
          }
        }
      }
      else if (((*(int *)(param_1 + 0x160) == 3) && (param_3 == 0)) ||
              ((*(int *)(param_1 + 0x164) == 3 && (param_3 == 1)))) {
        uVar2 = param_2 & 0x80000001;
        if ((int)uVar2 < 0) {
          uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
        }
        if (uVar2 == 0) {
          local_8 = 0x7e;
        }
        else {
          param_2 = param_2 & 0x80000001;
          if ((int)param_2 < 0) {
            param_2 = (param_2 - 1 | 0xfffffffe) + 1;
          }
          if (param_2 == 1) {
            local_8 = 0x7f;
          }
        }
      }
      else if (((param_3 == 0) &&
               (((((*(int *)(param_1 + 0x160) == 0xb || (*(int *)(param_1 + 0x160) == 0xc)) ||
                  (*(int *)(param_1 + 0x160) == 0x1f)) ||
                 ((*(int *)(param_1 + 0x160) == 0x20 || (*(int *)(param_1 + 0x160) == 0x3d)))) ||
                (*(int *)(param_1 + 0x160) == 0x3e)))) ||
              ((param_3 == 1 &&
               (((*(int *)(param_1 + 0x164) == 0xb || (*(int *)(param_1 + 0x164) == 0xc)) ||
                ((*(int *)(param_1 + 0x164) == 0x1f ||
                 (((*(int *)(param_1 + 0x164) == 0x20 || (*(int *)(param_1 + 0x164) == 0x3d)) ||
                  (*(int *)(param_1 + 0x164) == 0x3e)))))))))) {
        uVar2 = param_2 & 0x80000001;
        if ((int)uVar2 < 0) {
          uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
        }
        if (uVar2 == 0) {
          local_8 = 0x80;
        }
        else {
          param_2 = param_2 & 0x80000001;
          if ((int)param_2 < 0) {
            param_2 = (param_2 - 1 | 0xfffffffe) + 1;
          }
          if (param_2 == 1) {
            local_8 = 0x81;
          }
        }
      }
      else if (((*(int *)(param_1 + 0x160) == 0xd) || (*(int *)(param_1 + 0x160) == 0x21)) ||
              (*(int *)(param_1 + 0x160) == 0x3f)) {
        uVar2 = param_2 & 0x80000001;
        if ((int)uVar2 < 0) {
          uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
        }
        if (uVar2 == 0) {
          local_8 = 0x83;
        }
        else {
          param_2 = param_2 & 0x80000001;
          if ((int)param_2 < 0) {
            param_2 = (param_2 - 1 | 0xfffffffe) + 1;
          }
          if (param_2 == 1) {
            local_8 = 0x84;
          }
        }
      }
      else {
        if (*(int *)(param_1 + 0x160) == 0x65) {
          return;
        }
        if (*(int *)(param_1 + 0x160) == 0x29) {
          uVar2 = param_2 & 0x80000001;
          if ((int)uVar2 < 0) {
            uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
          }
          if (uVar2 == 0) {
            local_8 = 0x87;
          }
          else {
            param_2 = param_2 & 0x80000001;
            if ((int)param_2 < 0) {
              param_2 = (param_2 - 1 | 0xfffffffe) + 1;
            }
            if (param_2 == 1) {
              local_8 = 0x88;
            }
          }
        }
        else if (((*(int *)(param_1 + 0x160) == 0x15) || (*(int *)(param_1 + 0x160) == 0x16)) ||
                (*(int *)(param_1 + 0x160) == 0x17)) {
          uVar2 = param_2 & 0x80000001;
          if ((int)uVar2 < 0) {
            uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
          }
          if (uVar2 == 0) {
            local_8 = 0x89;
          }
          else {
            param_2 = param_2 & 0x80000001;
            if ((int)param_2 < 0) {
              param_2 = (param_2 - 1 | 0xfffffffe) + 1;
            }
            if (param_2 == 1) {
              local_8 = 0x8a;
            }
          }
        }
        else if (*(int *)(param_1 + 0x160) == 0x66) {
          local_8 = 0x8b;
        }
        else if ((*(int *)(param_1 + 0x160) == 0x67) || (*(int *)(param_1 + 0x160) == 0x68)) {
          local_8 = 0x8c;
        }
        else {
          local_8 = 0x79;
        }
      }
      if ((((param_3 == 0) && (*(int *)(param_1 + 0x160) != 0)) ||
          ((param_3 == 1 && ((local_18 != 0 && (local_18 != 0x80)))))) &&
         ((DAT_005ccf98 != 0 && (iVar1 = FUN_00429a6d(local_8), iVar1 != 0)))) {
        FUN_00429a6d(local_8);
        iVar1 = FUN_0042afd0();
        if ((iVar1 == 0) || (*(short *)(param_1 + 0x7b4) != param_3)) {
          uVar4 = 0;
          uVar3 = 0;
          FUN_00429a6d(local_8);
          FUN_0042ad2b(uVar3,uVar4);
        }
      }
      *(undefined2 *)(param_1 + 0x7b4) = (undefined2)param_3;
    }
  }
  return;
}

