// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00455950 | Name: FUN_00455950


undefined4 __thiscall FUN_00455950(int param_1,uint param_2)

{
  undefined4 uVar1;
  undefined1 local_1c [4];
  undefined2 local_18;
  undefined2 local_16;
  short local_10;
  undefined4 local_c;
  uint local_8;
  
  if ((param_2 < 0x60) || (0x69 < param_2)) {
    uVar1 = 0;
  }
  else {
    local_8 = (**(code **)(*DAT_0092e654 + 8))();
    if (((*(int *)(*(int *)(param_1 + 0x28) + 300) == 0) ||
        (*(int *)(*(int *)(*(int *)(param_1 + 0x28) + 300) + 0x40) != 0xd)) || (DAT_013b722c != 0))
    {
      if (*(char *)(*(int *)(param_1 + 0x4c) + 0x79c) == '\0') {
        if (*(char *)(*(int *)(param_1 + 0x4c) + 0x245) == '\x01') {
          uVar1 = 1;
        }
        else if (local_8 < *(int *)(param_1 + 0x2746c) + 500U) {
          uVar1 = 1;
        }
        else if (*(int *)(*(int *)(param_1 + 0x4c) + 0x73c) == -1) {
          if (((*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0xd) ||
              (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0xf)) ||
             ((*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0x19 ||
              (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0x1b)))) {
            uVar1 = 1;
          }
          else {
            _memset(local_1c,0,0x14);
            local_16 = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
            local_18 = 0x36a;
            local_10 = (short)param_2 + -0x51;
            if (param_2 == 0x69) {
              if (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0x18) {
                local_10 = 0x19;
              }
              else {
                if (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0x1a) {
                  return 1;
                }
                local_10 = 0xd;
              }
            }
            else if (param_2 == 0x60) {
              if (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0x1a) {
                local_10 = 0x1b;
              }
              else {
                if (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0x18) {
                  return 1;
                }
                local_10 = 0xf;
              }
            }
            *(int *)(*(int *)(param_1 + 0x4c) + 0x73c) = (int)local_10;
            local_c = 0;
            FUN_0055f2dd(local_1c,0x14);
            *(uint *)(param_1 + 0x2746c) = local_8;
            *(undefined4 *)(param_1 + 0x26eb0) = 1;
            uVar1 = 1;
          }
        }
        else {
          uVar1 = 1;
        }
      }
      else {
        uVar1 = 1;
      }
    }
    else {
      uVar1 = 1;
    }
  }
  return uVar1;
}

