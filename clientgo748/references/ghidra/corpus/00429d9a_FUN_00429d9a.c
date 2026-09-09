// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00429d9a | Name: FUN_00429d9a


int __thiscall FUN_00429d9a(int *param_1,undefined4 *param_2)

{
  undefined4 local_30;
  undefined4 local_2c;
  int local_c;
  int *local_8;
  
  local_8 = (int *)0x0;
  if (param_2 == (undefined4 *)0x0) {
    local_c = -0x7ff8ffa9;
  }
  else if (*param_1 == 0) {
    local_c = -0x7ffbfe10;
  }
  else {
    *param_2 = 0;
    _memset(&local_30,0,0x24);
    local_30 = 0x24;
    local_2c = 0x11;
    local_c = (**(code **)(*(int *)*param_1 + 0xc))(*param_1,&local_30,&local_8,0);
    if (-1 < local_c) {
      local_c = (**(code **)*local_8)(local_8,&DAT_005a9eac,param_2);
      if (local_c < 0) {
        if (local_8 != (int *)0x0) {
          (**(code **)(*local_8 + 8))(local_8);
        }
      }
      else {
        if (local_8 != (int *)0x0) {
          (**(code **)(*local_8 + 8))(local_8);
        }
        local_c = 0;
      }
    }
  }
  return local_c;
}

