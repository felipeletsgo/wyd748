// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042a97c | Name: FUN_0042a97c


int __thiscall FUN_0042a97c(int param_1,int *param_2,int param_3)

{
  uint local_18;
  void *local_14;
  uint local_10;
  int local_c;
  uint local_8;
  
  local_14 = (void *)0x0;
  local_10 = 0;
  local_8 = 0;
  if (param_2 == (int *)0x0) {
    local_c = -0x7ffbfe10;
  }
  else {
    local_c = FUN_0042ab3f(param_2,0);
    if ((-1 < local_c) &&
       (local_c = (**(code **)(*param_2 + 0x2c))
                            (param_2,0,*(undefined4 *)(param_1 + 8),&local_14,&local_10,0,0,0),
       -1 < local_c)) {
      FUN_0042b9e9();
      local_c = FUN_0042bae7(local_14,local_10,&local_8);
      if (-1 < local_c) {
        if (local_8 == 0) {
          _memset(local_14,(-(uint)(*(short *)(**(int **)(param_1 + 0xc) + 0xe) != 8) & 0xffffff80)
                           + 0x80 & 0xff,local_10);
        }
        else if (local_8 < local_10) {
          if (param_3 == 0) {
            _memset((void *)((int)local_14 + local_8),
                    (-(uint)(*(short *)(**(int **)(param_1 + 0xc) + 0xe) != 8) & 0xffffff80) + 0x80
                    & 0xff,local_10 - local_8);
          }
          else {
            for (local_18 = local_8; local_18 < local_10; local_18 = local_18 + local_8) {
              local_c = FUN_0042b9e9();
              if (local_c < 0) {
                return local_c;
              }
              local_c = FUN_0042bae7((int)local_14 + local_18,local_10 - local_18,&local_8);
              if (local_c < 0) {
                return local_c;
              }
            }
          }
        }
        (**(code **)(*param_2 + 0x4c))(param_2,local_14,local_10,0,0);
        local_c = 0;
      }
    }
  }
  return local_c;
}

