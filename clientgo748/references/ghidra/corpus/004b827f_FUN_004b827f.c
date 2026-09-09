// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b827f | Name: FUN_004b827f


bool __fastcall FUN_004b827f(int param_1)

{
  size_t sVar1;
  bool bVar2;
  undefined **ppuVar3;
  undefined1 local_88 [128];
  int local_8;
  
  local_8 = 0;
  FUN_0058f078(local_88,&DAT_005ba78c,s_UI_UITextureList_txt_005ba774,param_1);
  ppuVar3 = &PTR_DAT_005ba790;
  sVar1 = _strlen(s_UI_UITextureList_txt_005ba794);
  FUN_0058f078(local_88 + (sVar1 - 3),ppuVar3);
  local_8 = FUN_0058f716(local_88,&DAT_005ba7ac);
  bVar2 = local_8 == 0;
  if (bVar2) {
    FUN_00431159(s_Cannot_Init_UI_Texture_List_005ba7b0);
  }
  else {
    FUN_00590d68(param_1 + 0x1328,0x21000,1,local_8);
    FUN_0058f62c(local_8);
  }
  return !bVar2;
}

