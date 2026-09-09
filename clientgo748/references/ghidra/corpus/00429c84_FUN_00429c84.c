// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00429c84 | Name: FUN_00429c84


int __thiscall FUN_00429c84(int *param_1,short param_2,int param_3,ushort param_4)

{
  int iVar1;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_30;
  int *local_1c;
  undefined2 local_18;
  short local_16;
  undefined2 uStack_14;
  undefined2 uStack_12;
  int local_10;
  ushort local_c;
  ushort uStack_a;
  
  local_1c = (int *)0x0;
  if (*param_1 == 0) {
    iVar1 = -0x7ffbfe10;
  }
  else {
    _memset(&local_40,0,0x24);
    local_40 = 0x24;
    local_3c = 1;
    local_38 = 0;
    local_30 = 0;
    iVar1 = (**(code **)(*(int *)*param_1 + 0xc))(*param_1,&local_40,&local_1c,0);
    if (-1 < iVar1) {
      _memset(&local_18,0,0x12);
      local_18 = 1;
      local_16 = param_2;
      uStack_14 = (undefined2)param_3;
      uStack_12 = (undefined2)((uint)param_3 >> 0x10);
      uStack_a = param_4;
      local_c = (short)((int)(uint)param_4 >> 3) * param_2;
      local_10 = param_3 * (uint)local_c;
      iVar1 = (**(code **)(*local_1c + 0x38))(local_1c,&local_18);
      if (-1 < iVar1) {
        if (local_1c != (int *)0x0) {
          (**(code **)(*local_1c + 8))(local_1c);
        }
        iVar1 = 0;
      }
    }
  }
  return iVar1;
}

