// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055baa8 | Name: FUN_0055baa8


void __fastcall FUN_0055baa8(undefined4 param_1)

{
  int iVar1;
  size_t sVar2;
  undefined4 local_20c;
  char local_208 [256];
  undefined1 local_108 [17];
  undefined1 local_f7 [239];
  int local_8;
  
  local_8 = FUN_0058f716(s_Music_txt_005c4bf8,&DAT_005c4bf4,param_1);
  if (local_8 != 0) {
    local_20c = 0;
    while ((local_20c < 0xf && (iVar1 = FUN_00590a06(local_108,0x100,local_8), iVar1 != 0))) {
      FUN_0058ee20(local_208,local_f7);
      sVar2 = _strlen(local_208);
      if ((local_208[sVar2 - 1] == '\n') || (local_208[sVar2 - 1] == '\r')) {
        local_208[sVar2 - 1] = '\0';
      }
      if ((local_208[sVar2 - 2] == '\n') || (local_208[sVar2 - 2] == '\r')) {
        local_208[sVar2 - 2] = '\0';
      }
      FUN_0058f078(s_music_login_mp3_005b3888 + local_20c * 0x100,local_208);
      local_20c = local_20c + 1;
    }
    FUN_0058f62c(local_8);
  }
  FUN_0058f078(&DAT_005b3788,s_Mesh_tn_dat_005c4c04);
  FUN_0058f078(&DAT_005b3688,s_Mesh_ed_dat_005c4c10);
  return;
}

