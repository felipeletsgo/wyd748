// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004fff0b | Name: FUN_004fff0b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004fff0b(int param_1)

{
  int iVar1;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_EDX;
  undefined4 extraout_EDX_00;
  bool bVar2;
  char local_1b4 [64];
  size_t local_174;
  uint local_170;
  uint local_16c;
  int local_168;
  uint local_164;
  int local_160;
  float local_15c;
  uint local_158;
  uint local_154;
  uint local_150;
  uint local_14c;
  uint local_148;
  uint local_144;
  undefined1 local_140 [128];
  int local_c0;
  size_t local_bc;
  int local_b8;
  undefined1 local_b4 [32];
  undefined1 local_94 [32];
  undefined1 local_74 [32];
  undefined1 local_54 [32];
  undefined4 local_34;
  int local_30;
  int *local_2c;
  int *local_28;
  int *local_24;
  int *local_20;
  int *local_1c;
  undefined4 local_18;
  int *local_14;
  int local_10;
  int local_c;
  char *local_8;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    if (*(int *)(param_1 + 0x21c) == 0) {
      if (*(int *)(param_1 + 0x434) != 0) {
        (**(code **)(**(int **)(param_1 + 0x434) + 0x8c))((int)*(short *)(param_1 + 0x44c));
        (**(code **)(**(int **)(param_1 + 0x434) + 0x88))((int)*(short *)(param_1 + 0x450));
        (**(code **)(**(int **)(param_1 + 0x43c) + 0x8c))((int)*(short *)(param_1 + 0x44c));
        (**(code **)(**(int **)(param_1 + 0x43c) + 0x88))((int)*(short *)(param_1 + 0x450));
      }
      FUN_004ff37b((int)*(short *)(param_1 + 0x450));
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x434) + 0x8c))(*(undefined4 *)(param_1 + 0x21c));
      (**(code **)(**(int **)(param_1 + 0x434) + 0x88))(*(undefined4 *)(param_1 + 0x218));
      (**(code **)(**(int **)(param_1 + 0x43c) + 0x8c))(*(undefined4 *)(param_1 + 0x21c));
      (**(code **)(**(int **)(param_1 + 0x43c) + 0x88))(*(undefined4 *)(param_1 + 0x218));
      FUN_004ff37b(*(undefined4 *)(param_1 + 0x218));
    }
    FUN_004ff365();
    local_c = DAT_0067cf38;
    iVar1 = FUN_00494dcf();
    if ((iVar1 == 30000) && (local_10 = local_c, *(int *)(local_c + 0x4c) == param_1)) {
      local_30 = DAT_013b71e8 + 0x6ec;
      local_1c = *(int **)(local_c + 0x27acc);
      local_28 = *(int **)(local_c + 0x27ad0);
      local_2c = *(int **)(local_c + 0x27ab0);
      local_14 = *(int **)(local_c + 0x27ab4);
      local_24 = *(int **)(local_c + 0x27ab8);
      local_20 = *(int **)(local_c + 0x27abc);
      local_34 = *(undefined4 *)(local_c + 0x27ac0);
      local_18 = *(undefined4 *)(local_c + 0x27ac4);
      FUN_0058f220(DAT_013b71e8 + 0x72c,param_1 + 0x444,0x1c);
      if (local_2c != (int *)0x0) {
        if (*(short *)(param_1 + 0x44c) < *(short *)(param_1 + 0x450)) {
          *(undefined2 *)(param_1 + 0x450) = *(undefined2 *)(param_1 + 0x44c);
        }
        FUN_013c0cc6();
        FUN_0058f078(local_54,&DAT_005bb738,extraout_ECX);
        (**(code **)(*local_2c + 0x80))(local_54,0);
      }
      if (local_14 != (int *)0x0) {
        FUN_013c0ce4();
        FUN_0058f078(local_74,&DAT_005bb73c,extraout_ECX_00);
        (**(code **)(*local_14 + 0x80))(local_74,0);
      }
      if (local_24 != (int *)0x0) {
        FUN_013c0d02();
        FUN_0058f078(local_94,&DAT_005bb740,extraout_ECX_01);
        (**(code **)(*local_24 + 0x80))(local_94,0);
      }
      if (local_20 != (int *)0x0) {
        FUN_013c0d20();
        FUN_0058f078(local_b4,&DAT_005bb744,extraout_ECX_02);
        (**(code **)(*local_20 + 0x80))(local_b4,0);
      }
      if (local_1c != (int *)0x0) {
        FUN_013c0d3e();
        (**(code **)(*local_1c + 0x8c))(extraout_ECX_03);
        FUN_013c0d5c();
        (**(code **)(*local_1c + 0x88))(extraout_EDX);
      }
      if (local_28 != (int *)0x0) {
        FUN_013c0d7a();
        (**(code **)(*local_28 + 0x8c))(extraout_ECX_04);
        FUN_013c0d98();
        (**(code **)(*local_28 + 0x88))(extraout_EDX_00);
        (**(code **)(*local_28 + 0x60))(1);
      }
    }
    local_8 = _strchr((char *)(param_1 + 0x168),0x5e);
    if (local_8 != (char *)0x0) {
      local_b8 = (int)local_8 - (param_1 + 0x168);
      iVar1 = FUN_00421b0f(param_1 + 0x168,local_b8);
      if (iVar1 == 0) {
        local_8 = (char *)0x0;
      }
    }
    if (local_8 == (char *)0x0) {
      if (*(int *)(param_1 + 0x418) != 0) {
        (**(code **)(**(int **)(param_1 + 0x418) + 0x80))(param_1 + 0x168,1);
      }
      if (*(int *)(param_1 + 0x440) != 0) {
        (**(code **)(**(int **)(param_1 + 0x440) + 0x80))(param_1 + 0x168,1);
      }
      if (*(int *)(param_1 + 0x428) != 0) {
        (**(code **)(**(int **)(param_1 + 0x428) + 0x80))(param_1 + 0x178,1);
        (**(code **)(**(int **)(param_1 + 0x428) + 0x84))(0xffcccccc);
      }
      iVar1 = FUN_00494dcf();
      if (((iVar1 == 30000) && (local_c0 = local_c, *(uint *)(param_1 + 0x20) < 1000)) &&
         ((0 < *(short *)(param_1 + 0x192) || (0 < *(short *)(param_1 + 0x194))))) {
        iVar1 = __ftol();
        if (((iVar1 >> 7 < 0x11) || (iVar1 = __ftol(), 0x13 < iVar1 >> 7)) ||
           (iVar1 = __ftol(), iVar1 >> 7 < 0x1e)) {
          bVar2 = false;
        }
        else {
          bVar2 = true;
        }
        if (bVar2) {
          bVar2 = *(int *)(local_c0 + 0x4c) == param_1;
        }
        else {
          bVar2 = true;
        }
        if ((bVar2) &&
           (FUN_0058f078(local_140,s__d__d_005bb748,(int)*(short *)(param_1 + 0x192),
                         (int)*(short *)(param_1 + 0x194)), *(int *)(param_1 + 0x41c) != 0)) {
          (**(code **)(**(int **)(param_1 + 0x41c) + 0x80))(local_140,0);
        }
      }
      if (*(byte *)(param_1 + 0x196) < 10) {
        *(undefined1 *)(*(int *)(param_1 + 0x418) + 0xcdc) = 1;
        (**(code **)(**(int **)(param_1 + 0x418) + 0x84))(0xff000000);
      }
      else {
        if (*(byte *)(param_1 + 0x196) < 0x97) {
          if (*(byte *)(param_1 + 0x196) < 10) {
            *(undefined1 *)(param_1 + 0x196) = 5;
          }
        }
        else {
          *(undefined1 *)(param_1 + 0x196) = 0x96;
        }
        local_168 = (int)(*(byte *)(param_1 + 0x196) - 5) / 0x14;
        local_160 = (uint)*(byte *)(param_1 + 0x196) - (local_168 * 0x14 + 5);
        local_15c = (float)local_160 / _DAT_005a4314;
        local_154 = (*(uint *)(local_168 * 4 + 0x5bb464) & 0xff0000) >> 0x10;
        local_170 = (*(uint *)(local_168 * 4 + 0x5bb468) & 0xff0000) >> 0x10;
        iVar1 = __ftol();
        local_14c = iVar1 << 0x10;
        local_164 = (*(uint *)(local_168 * 4 + 0x5bb464) & 0xff00) >> 8;
        local_144 = (*(uint *)(local_168 * 4 + 0x5bb468) & 0xff00) >> 8;
        iVar1 = __ftol();
        local_158 = iVar1 << 8;
        local_150 = *(uint *)(local_168 * 4 + 0x5bb464) & 0xff;
        local_16c = *(uint *)(local_168 * 4 + 0x5bb468) & 0xff;
        local_148 = __ftol();
        (**(code **)(**(int **)(param_1 + 0x418) + 0x84))
                  (local_14c | 0xff000000 | local_158 | local_148);
        if (*(uint *)(param_1 + 0x20) < 1000) {
          *(undefined1 *)(*(int *)(param_1 + 0x418) + 0xcdc) = 0;
        }
      }
      local_bc = _strlen((char *)(param_1 + 0x168));
      (**(code **)(**(int **)(param_1 + 0x418) + 0x74))
                ((float)(int)local_bc * _DAT_005a3794 + _DAT_005a4204,0x41800000);
      if (*(char *)(*(int *)(param_1 + 0x418) + 0xcdc) == '\0') {
        *(undefined1 *)(param_1 + 0x249) = 0;
      }
    }
    else {
      _memset(local_1b4,0,0x40);
      FUN_0058f220(local_1b4,param_1 + 0x168,(int)local_8 - (param_1 + 0x168));
      *(undefined1 *)(param_1 + 0x249) = 1;
      (**(code **)(**(int **)(param_1 + 0x418) + 0x80))(local_1b4,0);
      local_174 = _strlen(local_1b4);
      (**(code **)(**(int **)(param_1 + 0x418) + 0x74))
                ((float)(int)local_174 * _DAT_005a3794 + _DAT_005a4204,0x41800000);
    }
    if (*(int *)(param_1 + 0x60) != 0) {
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = *(undefined4 *)(param_1 + 0x38);
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x28) = *(undefined4 *)(param_1 + 0x38);
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = *(undefined4 *)(param_1 + 0x38);
    }
  }
  return;
}

