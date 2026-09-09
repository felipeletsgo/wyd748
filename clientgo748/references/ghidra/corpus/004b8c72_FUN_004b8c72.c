// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b8c72 | Name: FUN_004b8c72


bool __fastcall FUN_004b8c72(int param_1)

{
  size_t sVar1;
  bool bVar2;
  undefined **ppuVar3;
  undefined1 local_88 [128];
  int local_8;
  
  local_8 = 0;
  FUN_0058f078(local_88,&DAT_005ba888,s_Effect_EffectTextureList_txt_005ba868,param_1);
  ppuVar3 = &PTR_DAT_005ba88c;
  sVar1 = _strlen(s_Effect_EffectTextureList_txt_005ba890);
  FUN_0058f078(local_88 + (sVar1 - 3),ppuVar3);
  local_8 = FUN_0058f716(local_88,&DAT_005ba8b0);
  bVar2 = local_8 == 0;
  if (bVar2) {
    FUN_00431159(s_Cannot_Init_Effect_Texture_List_005ba8b4);
  }
  else {
    FUN_00590d68(param_1 + 0x22328,0x21000,1,local_8);
    FUN_0058f62c(local_8);
  }
  return !bVar2;
}

