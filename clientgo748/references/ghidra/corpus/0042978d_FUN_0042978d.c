// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042978d | Name: FUN_0042978d


int __thiscall
FUN_0042978d(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)

{
  int iVar1;
  undefined4 local_21c;
  char local_218 [17];
  undefined1 local_207 [239];
  undefined1 local_118 [256];
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  undefined4 local_8;
  
  local_8 = 0;
  if (*param_1 != 0) {
    (**(code **)(*(int *)*param_1 + 8))(*param_1);
    *param_1 = 0;
  }
  local_c = DirectSoundCreate8(0,param_1,0);
  if (local_c < 0) {
    FUN_00431129(0);
    FUN_00431159(s_Sound_Error_Type__0x_x_005b484c,local_c);
    DAT_005ccf98 = 0;
  }
  else {
    local_c = (**(code **)(*(int *)*param_1 + 0x18))(*param_1,param_2,param_3);
    if (local_c < 0) {
      FUN_00431129(1);
      DAT_005ccf98 = 0;
    }
    else {
      FUN_00429c84(param_4,param_5,param_6);
      local_10 = FUN_0058f716(s_sound_soundlist_txt_005b486c,&DAT_005b4868);
      if (local_10 != 0) {
        local_14 = -1;
        do {
          iVar1 = FUN_00590a68(local_10,&DAT_005b4880,&local_14);
          if (iVar1 == -1) goto LAB_00429906;
          iVar1 = FUN_00590a68(local_10,s__s__d_005b4884,param_1 + local_14 * 0x42 + 3,
                               param_1 + local_14 * 0x42 + 0x43);
        } while (iVar1 != -1);
        FUN_00431159(s_Cannot_Init_Sound_Index____d_005b488c,local_14);
LAB_00429906:
        FUN_0058f62c(local_10);
      }
      local_10 = 0;
      local_10 = FUN_0058f716(s_Sound_txt_005b48b0,&DAT_005b48ac);
      if (local_10 != 0) {
        local_18 = FUN_00590a06(local_218,0xff,local_10);
        if ((local_18 != 0) && (local_21c = _strlen(local_218), 0 < (int)local_21c)) {
          if (local_218[local_21c - 1] == '\n') {
            local_218[local_21c - 1] = '\0';
          }
          FUN_005909d2(local_218,&DAT_005b48bc,local_118);
          _memset(param_1 + 0xc63,0,0x100);
          FUN_0058ee20(param_1 + 0xc63,local_207);
        }
        FUN_0058f62c(local_10);
      }
      FUN_00429d9a(param_1 + 0x8403);
      _memset(param_1 + 0x8404,0,0x40);
      param_1[0x8404] = 0x40;
      (**(code **)(*(int *)param_1[0x8403] + 0xc))(param_1[0x8403],param_1 + 0x8404);
      local_c = 0;
    }
  }
  return local_c;
}

