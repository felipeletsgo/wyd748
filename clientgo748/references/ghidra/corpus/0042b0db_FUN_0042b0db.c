// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042b0db | Name: FUN_0042b0db


int __thiscall FUN_0042b0db(int param_1,int param_2)

{
  uint uVar1;
  uint local_2c;
  void *local_28;
  uint local_24;
  size_t local_20;
  int local_1c;
  int local_18;
  undefined1 local_14 [4];
  int local_10;
  int local_c;
  uint local_8;
  
  local_28 = (void *)0x0;
  local_18 = 0;
  if ((*(int *)(param_1 + 4) == 0) || (*(int *)(param_1 + 0xc) == 0)) {
    local_10 = -0x7ffbfe10;
  }
  else {
    local_10 = FUN_0042ab3f(**(undefined4 **)(param_1 + 4),&local_c);
    if (-1 < local_10) {
      if (local_c == 0) {
        local_10 = (**(code **)(*(int *)**(undefined4 **)(param_1 + 4) + 0x2c))
                             (**(undefined4 **)(param_1 + 4),*(undefined4 *)(param_1 + 0x20),
                              *(undefined4 *)(param_1 + 0x1c),&local_28,&local_20,&local_18,local_14
                              ,0);
        if (-1 < local_10) {
          if (local_18 == 0) {
            if (*(int *)(param_1 + 0x24) == 0) {
              local_10 = FUN_0042bae7(local_28,local_20,&local_24);
              if (local_10 < 0) {
                return local_10;
              }
            }
            else {
              _memset(local_28,(-(uint)(*(short *)(**(int **)(param_1 + 0xc) + 0xe) != 8) &
                               0xffffff80) + 0x80 & 0xff,local_20);
              local_24 = local_20;
            }
            if (local_24 < local_20) {
              if (param_2 == 0) {
                _memset((void *)((int)local_28 + local_24),
                        (-(*(short *)(**(int **)(param_1 + 0xc) + 0xe) != 8) & 0x80) + 0x80 & 0xff,
                        local_20 - local_24);
                *(undefined4 *)(param_1 + 0x24) = 1;
              }
              else {
                for (local_2c = local_24; local_2c < local_20; local_2c = local_2c + local_24) {
                  local_10 = FUN_0042b9e9();
                  if (local_10 < 0) {
                    return local_10;
                  }
                  local_10 = FUN_0042bae7((int)local_28 + local_2c,local_20 - local_2c,&local_24);
                  if (local_10 < 0) {
                    return local_10;
                  }
                }
              }
            }
            (**(code **)(*(int *)**(undefined4 **)(param_1 + 4) + 0x4c))
                      (**(undefined4 **)(param_1 + 4),local_28,local_20,0,0);
            local_10 = (**(code **)(*(int *)**(undefined4 **)(param_1 + 4) + 0x10))
                                 (**(undefined4 **)(param_1 + 4),&local_8,0);
            if (-1 < local_10) {
              if (local_8 < *(uint *)(param_1 + 0x14)) {
                local_1c = *(int *)(param_1 + 8) - *(int *)(param_1 + 0x14);
              }
              else {
                local_1c = -*(int *)(param_1 + 0x14);
              }
              local_1c = local_1c + local_8;
              *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) + local_1c;
              *(uint *)(param_1 + 0x14) = local_8;
              if ((*(int *)(param_1 + 0x24) != 0) &&
                 (uVar1 = FUN_0042b9d8(), uVar1 <= *(uint *)(param_1 + 0x18))) {
                (**(code **)(*(int *)**(undefined4 **)(param_1 + 4) + 0x48))
                          (**(undefined4 **)(param_1 + 4));
              }
              *(size_t *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) + local_20;
              *(uint *)(param_1 + 0x20) = *(uint *)(param_1 + 0x20) % *(uint *)(param_1 + 8);
              local_10 = 0;
            }
          }
          else {
            local_10 = -0x7fff0001;
          }
        }
      }
      else {
        local_10 = FUN_0042a97c(**(undefined4 **)(param_1 + 4),0);
        if (-1 < local_10) {
          local_10 = 0;
        }
      }
    }
  }
  return local_10;
}

