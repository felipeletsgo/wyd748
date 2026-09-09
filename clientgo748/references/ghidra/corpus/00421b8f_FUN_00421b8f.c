// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00421b8f | Name: FUN_00421b8f


undefined4 __thiscall FUN_00421b8f(int param_1,char *param_2,uint param_3,int param_4)

{
  int iVar1;
  size_t sVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  ushort *local_1d0;
  int local_1cc;
  undefined1 local_1c4 [8];
  int local_1bc;
  size_t local_1b8;
  int local_1b4;
  int local_1b0;
  char local_1ac [44];
  size_t local_180;
  char local_17c [44];
  tagSIZE local_150;
  int local_148;
  RECT local_144;
  int local_134;
  uint local_130;
  int local_12c;
  int local_128;
  uint local_124;
  ushort *local_120;
  size_t local_11c;
  char *local_118;
  int local_114;
  size_t local_110;
  char local_10c [256];
  int local_c;
  ushort *local_8;
  
  *(undefined4 *)(param_1 + 0x14) = 0;
  local_110 = _strlen(param_2);
  if ((0x77 < (int)local_110) || ((int)local_110 < 0)) {
    return 0;
  }
  if (0x3a < DAT_005b491c) {
    local_134 = DAT_005b491c + -0x28;
    local_130 = param_3 & 0xff000000;
    local_128 = ((param_3 & 0xff0000) >> 0x10) - local_134;
    local_12c = ((param_3 & 0xff00) >> 8) - local_134;
    local_124 = (param_3 & 0xff) - local_134;
    if (local_128 < 0) {
      local_128 = 0;
    }
    if (local_12c < 0) {
      local_12c = 0;
    }
    if ((int)local_124 < 0) {
      local_124 = 0;
    }
    param_3 = local_130 | local_128 << 0x10 | local_12c << 8 | local_124;
  }
  *(uint *)(param_1 + 8) = param_3;
  FUN_0058f078(param_1 + 0x18,&DAT_005b16b8,param_2);
  *(undefined1 *)(param_1 + 0x69) = 0;
  *(undefined1 *)(param_1 + 0x68) = 0;
  local_118 = local_10c;
  FUN_0058ee20(local_118,param_1 + 0x18);
  local_11c = _strlen((char *)(param_1 + 0x18));
  if ((int)local_11c < 0x52) {
    if ((int)local_11c < 0x29) {
      _memset((void *)(param_1 + 0x118),0,0x2b);
      _strncpy((char *)(param_1 + 0x118),local_118,local_11c);
      *(undefined4 *)(param_1 + 0x14) = 1;
    }
    else {
      _memset((void *)(param_1 + 0x118),0,0x2b);
      _memset((void *)(param_1 + 0x143),0,0x2b);
      iVar1 = FUN_00421a90(local_118,0x28);
      if (iVar1 == 0) {
        _strncpy((char *)(param_1 + 0x118),local_118,0x28);
        local_118 = local_118 + 0x28;
        FUN_0058ee20(param_1 + 0x143,local_118);
      }
      else {
        _strncpy((char *)(param_1 + 0x118),local_118,0x29);
        local_118 = local_118 + 0x29;
        FUN_0058ee20(param_1 + 0x143,local_118);
      }
      *(undefined4 *)(param_1 + 0x14) = 2;
    }
  }
  else {
    iVar1 = FUN_00421a90(local_118,0x28);
    if (iVar1 == 0) {
      _strncpy((char *)(param_1 + 0x118),local_118,0x28);
      local_118 = local_118 + 0x28;
    }
    else {
      _strncpy((char *)(param_1 + 0x118),local_118,0x29);
      local_118 = local_118 + 0x29;
    }
    iVar1 = FUN_00421a90(local_118,0x28);
    if (iVar1 == 0) {
      _strncpy((char *)(param_1 + 0x143),local_118,0x28);
      local_118 = local_118 + 0x28;
      FUN_0058ee20(param_1 + 0x16e,local_118);
    }
    else {
      _strncpy((char *)(param_1 + 0x143),local_118,0x29);
      local_118 = local_118 + 0x29;
      sVar2 = _strlen(local_118);
      _strncpy((char *)(param_1 + 0x16e),local_118,sVar2);
    }
    *(undefined4 *)(param_1 + 0x14) = 3;
  }
  if (*(int *)(DAT_005ccf9c + 0x2a85c) != 0) {
    _memset((void *)(param_1 + 0x1a8),0,6);
    local_144.left = 0;
    local_144.top = 0;
    local_144.right = DAT_005b4918;
    local_144.bottom = *(int *)(param_1 + 0x14) * DAT_005b4914;
    FillRect(*(HDC *)(DAT_005ccf9c + 0x2a858),&local_144,(HBRUSH)0x0);
    for (local_148 = 0; local_148 < *(int *)(param_1 + 0x14); local_148 = local_148 + 1) {
      _memset(local_17c,0,0x2c);
      FUN_0058f078(local_17c,&DAT_005b16bc,param_1 + 0x118 + local_148 * 0x2b);
      sVar2 = _strlen(local_17c);
      TextOutA(*(HDC *)(DAT_005ccf9c + 0x2a858),0,local_148 * (DAT_005b4914 + 1),local_17c,sVar2);
      if (param_4 == 1) {
        _memset(local_1ac,0,0x2c);
        local_1b4 = 0;
        local_1b8 = _strlen(local_17c);
        for (local_1b0 = 0; local_1b0 < (int)local_1b8; local_1b0 = local_1b0 + 1) {
          if (local_17c[local_1b0] == '0') {
            local_1ac[local_1b0] = '/';
            local_1b4 = 1;
          }
          else {
            local_1ac[local_1b0] = ' ';
          }
        }
        if (local_1b4 != 0) {
          SetBkMode(*(HDC *)(DAT_005ccf9c + 0x2a858),1);
          sVar2 = _strlen(local_1ac);
          TextOutA(*(HDC *)(DAT_005ccf9c + 0x2a858),0,local_148 * (DAT_005b4914 + 1),local_1ac,sVar2
                  );
          SetBkMode(*(HDC *)(DAT_005ccf9c + 0x2a858),2);
        }
      }
      FUN_0058f078(local_17c,s__005b16c0);
      sVar2 = _strlen(local_17c);
      TextOutA(*(HDC *)(DAT_005ccf9c + 0x2a858),0,(local_148 + 1) * DAT_005b4914,local_17c,sVar2);
      local_180 = _strlen((char *)(param_1 + 0x118 + local_148 * 0x2b));
      GetTextExtentPoint32A
                (*(HDC *)(DAT_005ccf9c + 0x2a858),(LPCSTR)(param_1 + 0x118 + local_148 * 0x2b),
                 local_180,&local_150);
      *(undefined2 *)(param_1 + 0x1a8 + local_148 * 2) = (undefined2)local_150.cx;
    }
  }
  if (*(int *)(param_1 + 0x19c) == 0) {
    if (*(int *)(DAT_005ccf9c + 0x2a39c) == 1) {
      (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x50))
                (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),DAT_005b4918,
                 (int)(DAT_005b4918 + (DAT_005b4918 >> 0x1f & 7U)) >> 3,1,0,0x1a,1,param_1 + 0x19c);
    }
    else {
      if (DAT_005ccf78 == (void *)0x0) {
        iVar1 = FUN_0058fb07(s_UI_minimap_wyt_005b16ec,0x8000,0);
        if (iVar1 == -1) {
          return 0;
        }
        FUN_0058f8c9(iVar1,local_1c4,4);
        iVar3 = FUN_0058f83f(iVar1);
        DAT_005ccf7c = iVar3 + -4;
        DAT_005ccf78 = operator_new(iVar3 + 0xe);
        if (DAT_005ccf78 == (void *)0x0) {
          FUN_0058f75f(iVar1);
          return 0;
        }
        FUN_0058f8c9(iVar1,DAT_005ccf78,DAT_005ccf7c);
        FUN_0058f75f(iVar1);
        FUN_0058ee20(&DAT_005ccf78 + DAT_005ccf7c,s_TRUEVISION_XFILE_005b16fc);
      }
      local_1bc = FUN_00560eab(*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),DAT_005ccf78,
                               DAT_005ccf7c + 0x12,DAT_005b4918,
                               (int)(DAT_005b4918 + (DAT_005b4918 >> 0x1f & 7U)) >> 3,1,0,0x1a,1,1,1
                               ,0xff000000,0,0,param_1 + 0x19c);
      if (local_1bc < 0) {
        *(undefined4 *)(param_1 + 0x19c) = 0;
        return 0;
      }
    }
  }
  if (*(int *)(param_1 + 0x19c) == 0) {
    uVar4 = 0;
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x19c) + 0x40))
              (*(undefined4 *)(param_1 + 0x19c),0,&local_c,0,0);
    local_120 = local_8;
    for (local_114 = 0; local_114 < *(int *)(param_1 + 0x14) * (DAT_005b4914 + 1);
        local_114 = local_114 + 1) {
      local_1d0 = local_120;
      for (local_1cc = 0; local_1cc < DAT_005b4918; local_1cc = local_1cc + 1) {
        uVar5 = (*(uint *)(*(int *)(DAT_005ccf9c + 0x2a88c) +
                          (DAT_005b4918 * local_114 + local_1cc) * 4) & 0xff) >> 4;
        if (uVar5 == 0) {
          *local_1d0 = 0;
        }
        else {
          *local_1d0 = (ushort)(uVar5 << 0xc) | 0xfff;
        }
        local_1d0 = local_1d0 + 1;
      }
      local_120 = (ushort *)((int)local_120 + local_c);
    }
    (**(code **)(**(int **)(param_1 + 0x19c) + 0x44))(*(undefined4 *)(param_1 + 0x19c),0);
    uVar4 = 1;
  }
  return uVar4;
}

