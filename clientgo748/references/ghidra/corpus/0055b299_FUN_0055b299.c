// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055b299 | Name: FUN_0055b299


void FUN_0055b299(void)

{
  int iVar1;
  undefined1 local_210 [512];
  int local_10;
  int local_c;
  int local_8;
  
  local_c = FUN_0058f716(s_UI_EffectString_txt_005c4aac,&DAT_005c4aa8);
  if (local_c != 0) {
    for (local_10 = 1; local_10 < 0x28; local_10 = local_10 + 1) {
      FUN_00590a68(local_c,&DAT_005c4ac0,&DAT_005c0d38 + local_10 * 0x18);
    }
    FUN_0058f62c(local_c);
  }
  local_8 = FUN_0058f716(s_UI_GuildString_txt_005c4ac8,&DAT_005c4ac4);
  if (local_8 != 0) {
    iVar1 = FUN_00590a06(local_210,0x200,local_8);
    if (iVar1 == 0) {
      FUN_0058f62c(local_8);
    }
    else {
      _memset(s_0_1_2_005c0c88,0,7);
      FUN_0058f220(s_0_1_2_005c0c88,local_210,6);
      iVar1 = FUN_00590a06(local_210,0x200,local_8);
      if (iVar1 == 0) {
        FUN_0058f62c(local_8);
      }
      else {
        _memset(s_0_1_2_3_4_5_6_7_8_9_A_B_C_D_E_F_005c0c90,0,0x21);
        FUN_0058f220(s_0_1_2_3_4_5_6_7_8_9_A_B_C_D_E_F_005c0c90,local_210,0x20);
        iVar1 = FUN_00590a06(local_210,0x200,local_8);
        if (iVar1 == 0) {
          FUN_0058f62c(local_8);
        }
        else {
          _memset(s_0_1_2_3_4_5_6_7_8_9_____A_B_C_D_E_005c0cb4,0,0x81);
          FUN_0058f220(s_0_1_2_3_4_5_6_7_8_9_____A_B_C_D_E_005c0cb4,local_210,0x80);
          FUN_0058f62c(local_8);
        }
      }
    }
  }
  return;
}

