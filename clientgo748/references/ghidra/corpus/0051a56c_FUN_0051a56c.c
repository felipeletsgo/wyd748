// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0051a56c | Name: FUN_0051a56c


void __thiscall FUN_0051a56c(int param_1,int param_2)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  undefined1 local_50 [4];
  undefined2 local_4c;
  undefined2 local_4a;
  undefined4 local_44;
  int local_40;
  undefined2 local_3c;
  undefined2 local_3a;
  undefined2 local_38;
  int local_34;
  uint local_30;
  int local_2c;
  int *local_28;
  undefined4 local_24;
  int local_20;
  int local_1c;
  undefined4 local_18;
  int local_14;
  int local_10;
  int local_c;
  uint local_8;
  
  if ((((*(int *)(param_1 + 0x214) == 0) && (param_2 != 0)) &&
      (*(int *)(DAT_0067cf38 + 0x24) == 30000)) && (DAT_00e37d2c <= *(int *)(param_2 + 0x20))) {
    local_c = DAT_0067cf38;
    local_8 = (**(code **)(*DAT_0092e654 + 8))();
    *(int *)(local_c + 0x26e98) = param_2;
    if (*(int *)(local_c + 0x27474) + 1000U < local_8) {
      local_18 = __ftol();
      local_24 = __ftol();
      if (*(int *)(local_c + 0x28794) != 0) {
        local_18 = *(undefined4 *)(local_c + 0x28794);
        local_24 = *(undefined4 *)(local_c + 0x28798);
      }
      local_10 = __ftol();
      local_14 = __ftol();
      local_20 = FUN_005563a0(local_18,local_24,local_10,local_14);
      local_1c = 1;
      FUN_00556416(local_18,local_24,&local_10,&local_14,local_c + 0x94,8);
      if (((*(short *)(param_2 + 0x160) != 0x6bf) || (999 < *(short *)(param_1 + 0x444))) &&
         ((*(short *)(param_2 + 0x160) != 0x167 &&
          ((*(short *)(param_2 + 0x160) < 0x6c5 || (0x6c8 < *(short *)(param_2 + 0x160))))))) {
        if ((local_1c < local_20) ||
           ((((iVar2 = __ftol(), local_10 != iVar2 || (iVar2 = __ftol(), local_14 != iVar2)) ||
             (iVar2 = __ftol(), *(int *)(param_1 + 0x484) != iVar2)) ||
            (iVar2 = __ftol(), *(int *)(param_1 + 0x488) != iVar2)))) {
          if (((*(int *)(param_1 + 0x740) == 0) || (*(int *)(param_1 + 0x740) == 1)) &&
             (uVar1 = GetKeyState(0x10), (int)(uint)uVar1 >> 8 == 0)) {
            uVar3 = __ftol();
            *(uint *)(local_c + 0x275b0) = uVar3 & 0xffff;
            uVar3 = __ftol();
            *(uint *)(local_c + 0x275b4) = uVar3 & 0xffff;
            FUN_00520216(*(undefined4 *)(local_c + 0x275b0),*(undefined4 *)(local_c + 0x275b4),0,0);
          }
        }
        else {
          local_28 = *(int **)(local_c + 0x2873c);
          local_2c = FUN_0054cd07(param_2 + 0x160,0x21);
          if ((7 < local_2c) || (local_2c < 0)) {
            local_2c = 0;
          }
          (**(code **)(*local_28 + 0x98))
                    (&local_34,*(undefined4 *)(&DAT_005bec0c + local_2c * 8),
                     *(undefined4 *)(&DAT_005bec10 + local_2c * 8));
          if ((((-1 < local_34) && (local_30 < 0x80000000)) ||
              (iVar2 = FUN_0054cd07(param_2 + 0x160,0x26), iVar2 == 2)) &&
             ((*(short *)(param_2 + 0x160) != 0x305 && (*(short *)(param_2 + 0x160) != 0x2ea)))) {
            _memset(local_50,0,0x1c);
            local_4a = *(undefined2 *)(param_1 + 0x20);
            local_4c = 0x270;
            local_3c = *(undefined2 *)(param_2 + 0x20);
            local_44 = 1;
            local_40 = local_30 * 9 + local_34;
            local_3a = __ftol();
            local_38 = __ftol();
            FUN_0055f2dd(local_50,0x1c);
            *(undefined4 *)(local_c + 0x26e98) = 0;
            *(uint *)(local_c + 0x27460) = local_8;
            *(uint *)(local_c + 0x27474) = local_8;
          }
        }
      }
    }
  }
  return;
}

