// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b93c4 | Name: FUN_004b93c4


bool __fastcall FUN_004b93c4(int param_1)

{
  size_t sVar1;
  bool bVar2;
  undefined **ppuVar3;
  undefined1 local_88 [128];
  int local_8;
  
  local_8 = 0;
  FUN_0058f078(local_88,&DAT_005ba90c,s_Mesh_MeshTextureList_txt_005ba8f0,param_1);
  ppuVar3 = &PTR_DAT_005ba910;
  sVar1 = _strlen(s_Mesh_MeshTextureList_txt_005ba914);
  FUN_0058f078(local_88 + (sVar1 - 3),ppuVar3);
  local_8 = FUN_0058f716(local_88,&DAT_005ba930);
  bVar2 = local_8 == 0;
  if (bVar2) {
    FUN_00431159(s_Cannot_Init_Model_Texture_List_005ba934);
  }
  else {
    FUN_00590d68(param_1 + 0x43328,0x84000,1,local_8);
    FUN_0058f62c(local_8);
  }
  return !bVar2;
}

