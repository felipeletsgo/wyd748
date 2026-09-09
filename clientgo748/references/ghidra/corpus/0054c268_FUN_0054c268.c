// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054c268 | Name: FUN_0054c268


void FUN_0054c268(int param_1,uint param_2,undefined1 *param_3,int param_4)

{
  uint uVar1;
  uint uVar2;
  int local_1c;
  undefined1 local_c [4];
  uint local_8;
  
  _memset(local_c,0,4);
  uVar2 = (int)(param_2 & 0xffff) >> 0xc;
  uVar1 = param_2 & 0xfff;
  if (DAT_005be33c != param_1) {
    local_1c = FUN_00557593(param_1,0);
    if (local_1c == 0) {
      local_1c = FUN_00557593(param_1,1);
    }
    if (local_1c == 0) {
      return;
    }
  }
  if (param_3 != (undefined1 *)0x0) {
    if (((param_1 < 0) || (9 < param_1)) || (0xf < uVar2)) {
      *param_3 = 0;
    }
    else if ((uVar1 < 0x1001) && ((&DAT_00a41d78)[uVar1 * 0x30 + uVar2 * 0x30000] != '\0')) {
      FUN_0058f220(param_3,&DAT_00a41d78 + uVar1 * 0x30 + uVar2 * 0x30000,0xc);
      FUN_0054fc81(param_3);
      if ((0 < param_4) && (param_4 < 4)) {
        if ((&DAT_00a41d78)[param_4 * 0xc + uVar1 * 0x30 + uVar2 * 0x30000] == '\0') {
          FUN_0058ee30(param_3,&DAT_005c3fc0);
          FUN_0058f220(local_c,&DAT_005c0c86 + param_4 * 2,2);
          FUN_0058ee30(param_3,local_c);
        }
        else {
          FUN_0058ee30(param_3,&DAT_005c3fbc);
          FUN_0058ee30(param_3,&DAT_00a41d78 + param_4 * 0xc + uVar1 * 0x30 + uVar2 * 0x30000);
        }
      }
    }
    else {
      local_8 = ((int)(param_2 & 0xffff) >> 0xc) * 2;
      if (local_8 < 0x15) {
        FUN_0058f220(local_c,s_0_1_2_3_4_5_6_7_8_9_A_B_C_D_E_F_005c0c90 + local_8,2);
        FUN_0058f078(param_3,&DAT_005c3fc4,local_c);
        local_8 = ((int)(param_2 & 0xffff) >> 6 & 0x3fU) * 2;
        if (local_8 < 0x81) {
          FUN_0058f220(local_c,s_0_1_2_3_4_5_6_7_8_9_____A_B_C_D_E_005c0cb4 + local_8,2);
          FUN_0058ee30(param_3,local_c);
          local_8 = (param_2 & 0x3f) * 2;
          if (local_8 < 0x81) {
            FUN_0058f220(local_c,s_0_1_2_3_4_5_6_7_8_9_____A_B_C_D_E_005c0cb4 + local_8,2);
            FUN_0058ee30(param_3,local_c);
            if ((0 < param_4) && (param_4 < 4)) {
              FUN_0058f220(local_c,&DAT_005c0c86 + param_4 * 2,2);
              FUN_0058ee30(param_3,local_c);
            }
          }
          else {
            FUN_0058f078(param_3,&DAT_00e37d3c);
          }
        }
        else {
          FUN_0058f078(param_3,&DAT_00e37d38);
        }
      }
      else {
        FUN_0058f078(param_3,&DAT_00e37d34);
      }
    }
  }
  return;
}

