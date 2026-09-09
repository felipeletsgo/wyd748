// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00520216 | Name: FUN_00520216


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00520216(int *param_1,uint param_2,uint param_3,undefined4 param_4,int param_5)

{
  int iVar1;
  undefined4 *puVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int local_2d8;
  int local_2d4;
  int local_2d0;
  int local_2cc;
  int local_284;
  int local_27c;
  undefined1 local_278 [4];
  undefined4 local_274;
  short local_26c;
  short local_26a;
  undefined4 local_268;
  undefined4 local_264;
  ushort local_260;
  ushort uStack_25e;
  int *local_244;
  int local_240;
  uint local_23c;
  int local_238;
  uint local_234;
  uint local_230;
  uint local_22c;
  int local_228;
  int local_224;
  int local_220;
  int local_21c;
  undefined1 local_218 [384];
  int local_98;
  int local_94;
  int local_90;
  int local_8c;
  int local_88;
  undefined1 local_84 [4];
  undefined4 local_80;
  short local_78;
  short local_76;
  undefined4 local_74;
  undefined4 local_70;
  ushort local_6c;
  ushort uStack_6a;
  undefined1 auStack_68 [24];
  int local_50;
  int local_4c;
  float local_48;
  uint local_44;
  uint local_40;
  char local_3c [48];
  int local_c;
  int local_8;
  
  if ((param_1[0x85] == 0) && ((param_2 != param_1[0x121] || (param_3 != param_1[0x122])))) {
    local_238 = param_1[0x105];
    if ((float)_DAT_005a45c8 < (float)param_1[0x8c]) {
      local_238 = param_1[0x105] + 1;
    }
    local_22c = __ftol();
    local_234 = __ftol();
    local_230 = 1000;
    local_8 = (**(code **)(*DAT_0092e654 + 8))();
    if ((param_1 == *(int **)(DAT_0067cf38 + 0x4c)) && (iVar1 = __ftol(), iVar1 == 4)) {
      local_230 = 500;
    }
    if ((param_1 == *(int **)(DAT_0067cf38 + 0x4c)) && (iVar1 = __ftol(), iVar1 == 5)) {
      local_230 = 500;
    }
    if ((param_1 == *(int **)(DAT_0067cf38 + 0x4c)) && (iVar1 = __ftol(), iVar1 == 6)) {
      local_230 = 100;
    }
    if ((param_1 == *(int **)(DAT_0067cf38 + 0x4c)) && (iVar1 = __ftol(), iVar1 == 7)) {
      local_230 = 100;
    }
    if ((param_1 == *(int **)(DAT_0067cf38 + 0x4c)) && (*(int *)(DAT_0067cf38 + 0x24) == 30000)) {
      local_240 = DAT_0067cf38;
      local_23c = (**(code **)(*DAT_0092e654 + 8))();
      if (local_23c < DAT_013b7220 + 6000U) {
        return;
      }
      if (local_23c < *(int *)(local_240 + 0x27138) + 6000U) {
        return;
      }
      if (local_23c < *(int *)(local_240 + 0x2713c) + 6000U) {
        return;
      }
      if (local_23c < *(int *)(local_240 + 0x27140) + 6000U) {
        return;
      }
      if (local_23c < *(int *)(local_240 + 0x27144) + 6000U) {
        return;
      }
    }
    if ((local_230 < (uint)(local_8 - param_1[0x17b])) || (param_5 != 0)) {
      local_220 = DAT_0067cf38 + 0x94;
      local_40 = param_2;
      local_44 = param_3;
      _memset(local_3c,0,0x30);
      local_21c = DAT_0067cf38;
      iVar1 = FUN_0049c70f(param_1[10],param_1[0xb]);
      local_48 = (float)iVar1;
      local_98 = 0xc;
      if (*(int *)(DAT_0067cf38 + 0x24) == 0x7535) {
        local_98 = 0x10;
      }
      FUN_00555ad8(local_22c,local_234,&param_2,&param_3,local_3c,local_98,local_220,8);
      puVar2 = (undefined4 *)FUN_00430f20((float)param_1[0x121],(float)param_1[0x122]);
      local_c = FUN_0049c70f(*puVar2,puVar2[1]);
      puVar2 = (undefined4 *)FUN_00430f20((float)(int)local_22c,(float)(int)local_234);
      local_228 = FUN_0049c70f(*puVar2,puVar2[1]);
      puVar2 = (undefined4 *)FUN_00430f20((float)(int)param_2,(float)(int)param_3);
      local_50 = FUN_0049c70f(*puVar2,puVar2[1]);
      if (local_50 == local_228 || local_50 - local_228 < 0) {
        local_2cc = -(local_50 - local_228);
      }
      else {
        local_2cc = local_50 - local_228;
      }
      local_94 = local_2cc;
      if (local_50 == local_c || local_50 - local_c < 0) {
        local_2d0 = -(local_50 - local_c);
      }
      else {
        local_2d0 = local_50 - local_c;
      }
      local_90 = local_2d0;
      if (((local_2d0 < 0x1f) || (param_1[0x121] == 0)) || (param_1[0x122] == 0)) {
        if (0x1e < local_2cc) {
          _memset(local_3c,0,0x30);
          FUN_00555ad8(local_22c,local_234,&param_2,&param_3,local_3c,local_98 / 2,local_220,8);
          puVar2 = (undefined4 *)FUN_00430f20((float)(int)param_2,(float)(int)param_3);
          local_50 = FUN_0049c70f(*puVar2,puVar2[1]);
          if (local_50 == local_228 || local_50 - local_228 < 0) {
            local_2d4 = -(local_50 - local_228);
          }
          else {
            local_2d4 = local_50 - local_228;
          }
          local_94 = local_2d4;
          if (0x1e < local_2d4) {
            _memset(local_3c,0,0x30);
            FUN_00555ad8(local_22c,local_234,&param_2,&param_3,local_3c,
                         (int)(local_98 + (local_98 >> 0x1f & 3U)) >> 2,local_220,8);
            puVar2 = (undefined4 *)FUN_00430f20((float)(int)param_2,(float)(int)param_3);
            local_50 = FUN_0049c70f(*puVar2,puVar2[1]);
            if (local_50 == local_228 || local_50 - local_228 < 0) {
              local_2d8 = -(local_50 - local_228);
            }
            else {
              local_2d8 = local_50 - local_228;
            }
            local_94 = local_2d8;
          }
        }
        uVar3 = __ftol();
        if ((uVar3 == param_2) && (uVar3 = __ftol(), uVar3 == param_3)) {
          if ((param_5 != 0) &&
             ((param_1[0x121] != (param_2 & 0xffff) || (param_1[0x122] != (param_3 & 0xffff))))) {
            *(undefined1 *)((int)param_1 + 0x247) = 1;
            local_244 = *(int **)(DAT_0067cf38 + 0x4c);
            if ((param_1 == local_244) &&
               ((*(int *)(DAT_0067cf38 + 0x24) != 0x7534 &&
                (*(int *)(DAT_0067cf38 + 0x24) != 0x7535)))) {
              param_1[0x121] = param_2 & 0xffff;
              param_1[0x122] = param_3 & 0xffff;
              _memset(local_278,0,0x34);
              iVar1 = DAT_0067cf38;
              local_26c = (short)local_22c;
              local_26a = (short)local_234;
              local_264 = 0;
              local_274 = CONCAT22((short)param_1[8],0x367);
              local_268 = DAT_005bb418;
              local_260 = (ushort)param_2;
              uStack_25e = (ushort)param_3;
              (**(code **)(*param_1 + 4))(0x366,local_278);
              if (param_5 != 2) {
                *(int *)(iVar1 + 0x2878c) = (int)local_26c;
                *(int *)(iVar1 + 0x28790) = (int)local_26a;
                *(uint *)(iVar1 + 0x28794) = (uint)local_260;
                *(uint *)(iVar1 + 0x28798) = (uint)uStack_25e;
                FUN_0055f2dd(local_278,0x34);
                DAT_005d03fc = local_274 & 0xffff;
              }
              for (local_27c = 0; local_27c < 0x30; local_27c = local_27c + 1) {
                param_1[local_27c * 2 + 0xa4] = (int)((float)(int)local_22c + _DAT_005a34a0);
                param_1[local_27c * 2 + 0xa5] = (int)((float)(int)local_234 + _DAT_005a34a0);
              }
              iVar1 = (**(code **)(*DAT_0092e654 + 8))();
              param_1[0x17b] = iVar1;
            }
          }
        }
        else if (local_3c[0] != '\0') {
          FUN_00421910(local_218,8,0x30,FUN_00430f10);
          _memset(local_218,0,0x180);
          FUN_005211be(local_22c,local_234,local_3c,local_218,&local_88);
          local_4c = 0;
          local_224 = local_88;
          if (*(int *)(DAT_0067cf38 + 0x24) != 0x7535) {
            for (; 0 < local_224; local_224 = local_224 + -1) {
              local_4c = 0;
              local_284 = *(int *)(*(int *)(local_21c + 0x34) + 0x10);
              while( true ) {
                if ((local_284 == 0) || (*(int *)(local_284 + 0xc) == 0)) goto LAB_00520c35;
                iVar1 = __ftol();
                iVar4 = __ftol();
                iVar5 = __ftol();
                if ((iVar5 == iVar1) &&
                   ((iVar1 = __ftol(), iVar1 == iVar4 && (*(char *)(local_284 + 0x245) == '\0'))))
                break;
                local_284 = *(int *)(local_284 + 0xc);
              }
              local_4c = 1;
LAB_00520c35:
              if (local_4c == 0) break;
            }
          }
          param_2 = __ftol();
          param_3 = __ftol();
          if ((param_1 == *(int **)(DAT_0067cf38 + 0x4c)) &&
             (*(int *)(DAT_0067cf38 + 0x24) == 30000)) {
            FUN_0040bd30(&param_2);
          }
          if (((param_2 != local_22c) || (param_3 != local_234)) &&
             ((param_2 != param_1[0x11f] || ((param_3 != param_1[0x120] || (param_5 != 0)))))) {
            _memset(param_1 + 0x98,0,0x30);
            FUN_0058f220(param_1 + 0x98,local_3c,local_224);
            _memset(local_84,0,0x34);
            local_78 = (short)local_22c;
            local_76 = (short)local_234;
            local_70 = 0;
            if (param_5 == 0) {
              local_80 = CONCAT22((short)param_1[8],0x366);
            }
            else {
              local_80 = CONCAT22((short)param_1[8],0x367);
            }
            if (param_1 == *(int **)(DAT_0067cf38 + 0x4c)) {
              local_74 = DAT_005bb418;
            }
            else {
              local_74 = __ftol();
            }
            local_6c = (ushort)param_2;
            uStack_6a = (ushort)param_3;
            for (local_8c = 0; local_8c < 0x17; local_8c = local_8c + 1) {
              auStack_68[local_8c] = *(undefined1 *)((int)param_1 + local_8c % 0x18 + 0x260);
            }
            if ((param_1[0x121] != (param_2 & 0xffff)) || (param_1[0x122] != (param_3 & 0xffff))) {
              if ((*(char *)((int)param_1 + 0x247) == param_5) && (param_5 == 1)) {
                return;
              }
              *(undefined1 *)((int)param_1 + 0x247) = (undefined1)param_5;
              iVar1 = DAT_0067cf38;
              if ((((param_1 == *(int **)(DAT_0067cf38 + 0x4c)) &&
                   (*(int *)(DAT_0067cf38 + 0x24) != 0x7534)) &&
                  (*(int *)(DAT_0067cf38 + 0x24) != 0x7535)) &&
                 ((1000 < (uint)(local_8 - param_1[0x17b]) && (param_5 != 2)))) {
                *(int *)(DAT_0067cf38 + 0x2878c) = (int)local_78;
                *(int *)(iVar1 + 0x28790) = (int)local_76;
                *(uint *)(iVar1 + 0x28794) = param_2 & 0xffff;
                *(uint *)(iVar1 + 0x28798) = param_3 & 0xffff;
                FUN_0055f2dd(local_84,0x34);
                DAT_005d03fc = local_80 & 0xffff;
                param_1[0x121] = (uint)local_6c;
                param_1[0x122] = (uint)uStack_6a;
                iVar1 = (**(code **)(*DAT_0092e654 + 8))();
                param_1[0x17b] = iVar1;
              }
            }
            (**(code **)(*param_1 + 4))(0x366,local_84);
          }
        }
      }
    }
  }
  return;
}

