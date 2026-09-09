// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040ede0 | Name: FUN_0040ede0


undefined4 __thiscall
FUN_0040ede0(int *param_1,int param_2,undefined4 param_3,undefined4 param_4,int param_5)

{
  short sVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 local_648;
  undefined1 local_644 [516];
  short local_440;
  short local_43c;
  short local_438;
  undefined1 local_434 [4];
  undefined2 local_430;
  undefined2 local_42e;
  undefined1 local_428;
  undefined1 local_427;
  undefined1 local_426;
  undefined1 local_425;
  undefined2 local_424;
  undefined2 local_420;
  int local_41c;
  int local_418;
  short local_414;
  int local_410;
  int local_40c;
  short local_408;
  undefined1 local_404 [1024];
  
  local_410 = DAT_013b71e8 + 0x6ec;
  local_414 = (**(code **)(*param_1 + 0xc0))(param_1[0x79],param_1[0x100]);
  local_408 = (**(code **)(*param_1 + 0xc4))(param_1[0x79]);
  if (local_414 == 0) {
    local_418 = FUN_0054cd07(*(undefined4 *)(param_2 + 0x670),0x11);
    if (param_1[0x11] == 0x153) {
      if (local_418 == 0x80) {
        uVar2 = FUN_00553016(*(undefined4 *)(param_2 + 0x670),local_410 + 0x40,(int)local_408,
                             (int)*(short *)(local_410 + 0x5c),local_410 + 0x5c,
                             (int)*(short *)(DAT_013b71e8 + 0xc4 +
                                            *(char *)(DAT_013b71e8 + 0xef4) * 0x80));
        return uVar2;
      }
      if ((local_418 == 0x40) || (local_418 == 0xc0)) {
        if (*(short *)(local_410 + 0x8c) < 1) {
          local_41c = FUN_00553016(*(undefined4 *)(param_2 + 0x670),local_410 + 0x40,6,
                                   (int)*(short *)(local_410 + 0x5c),local_410 + 0x5c,
                                   (int)*(short *)(DAT_013b71e8 + 0xc4 +
                                                  *(char *)(DAT_013b71e8 + 0xef4) * 0x80));
          if ((local_41c == 1) && (param_5 == 0)) {
            local_420 = (**(code **)(*param_1 + 0xc0))
                                  (*(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x1e4),
                                   *(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x400));
            local_438 = (**(code **)(*param_1 + 0xc4))
                                  (*(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x1e4));
            if (local_438 == -1) {
              local_438 = (short)*(undefined4 *)(param_2 + 0x1d0) +
                          (short)*(undefined4 *)(param_2 + 0x1d4) * 9;
            }
            _memset(local_434,0,0x14);
            local_42e = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
            local_430 = 0x376;
            local_428 = 0;
            local_427 = 6;
            local_426 = (undefined1)local_420;
            local_425 = (undefined1)local_438;
            local_424 = DAT_005d03e8;
            FUN_0055f2dd(local_434,0x14);
          }
          return 0;
        }
        local_43c = (**(code **)(*param_1 + 0xc0))
                              (*(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x1e4),
                               *(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x400));
        local_440 = (**(code **)(*param_1 + 0xc4))
                              (*(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x1e4));
        if ((local_43c == 0) && (local_440 == 6)) {
          return 0;
        }
      }
    }
    uVar2 = FUN_00553016(*(undefined4 *)(param_2 + 0x670),local_410 + 0x40,(int)local_408,
                         (int)*(short *)(local_410 + 0x5c),local_410 + 0x5c,
                         (int)*(short *)(DAT_013b71e8 + 0xc4 +
                                        *(char *)(DAT_013b71e8 + 0xef4) * 0x80));
    return uVar2;
  }
  if (local_414 != 1) {
    local_40c = *(int *)(param_2 + 0x1d0) + *(int *)(param_2 + 0x1d4) * 9;
    FUN_0058f220(local_404,DAT_013b71e8 + 0x2ec,0x400);
    if (*(int *)(*(int *)(param_2 + 0x1e4) + 0x400) == 2) {
      _memset(local_404 + local_40c * 8,0,8);
    }
    uVar2 = FUN_0040eafc(*(undefined4 *)(param_2 + 0x670),local_404,param_3,param_4);
    return uVar2;
  }
  if ((4999 < **(short **)(param_2 + 0x670)) && (**(short **)(param_2 + 0x670) < 0x13ef)) {
    if (param_1[0x100] == 5) {
      return 1;
    }
    return 0;
  }
  FUN_0058f220(local_644,local_410 + 0xdc,0x200);
  sVar1 = (**(code **)(*param_1 + 0xc0))
                    (*(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x1e4),
                     *(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x400));
  if (sVar1 == 1) {
    if ((*(int *)(param_2 + 0x1d0) < 0) || (0x20 < *(int *)(param_2 + 0x1d0))) {
      return 0;
    }
    if ((*(int *)(param_2 + 0x1d4) < 0) || (0x20 < *(int *)(param_2 + 0x1d4))) {
      return 0;
    }
    iVar3 = *(int *)(param_2 + 0x1d0) + *(int *)(param_2 + 0x1d4) * 9;
    if ((iVar3 < 0) || (0x40 < iVar3)) {
      return 0;
    }
    _memset(local_644 + iVar3 * 8,0,8);
  }
  local_648 = 0;
  uVar2 = FUN_00553464(*(undefined4 *)(param_2 + 0x670),local_644,param_3,param_4,&local_648);
  return uVar2;
}

