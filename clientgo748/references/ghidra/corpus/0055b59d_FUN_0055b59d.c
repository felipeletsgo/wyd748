// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055b59d | Name: FUN_0055b59d


void FUN_0055b59d(void)

{
  int iVar1;
  int iVar2;
  undefined4 local_21c;
  int local_218;
  undefined4 local_214;
  undefined4 local_210;
  int local_20c;
  char local_208 [4];
  CHAR local_204 [256];
  char local_104 [256];
  
  wsprintfA(local_204,s_D__project_wyd_run_Mesh_meshText_005c4b00,0);
  local_21c = 0;
  local_20c = 0;
  local_218 = 0;
  local_214 = 0;
  local_210 = 0;
  iVar1 = FUN_0058f716(local_204,&DAT_005c4b2c);
  if (iVar1 != 0) {
    local_214 = FUN_0058f716(s_MeshTextureList_txt_005c4b34,&DAT_005c4b30);
    while (iVar2 = FUN_00590a68(iVar1,s__d__s__c_005c4b48,&local_21c,local_104,local_208),
          iVar2 != -1) {
      iVar2 = _strcmp(local_104,s_mesh_ch010101_wyt_005c4b54);
      if ((iVar2 == 0) || (iVar2 = _strcmp(local_104,s_mesh_ch010101_wys_005c4b68), iVar2 == 0)) {
        local_218 = 400;
        local_20c = 400;
      }
      if (local_208[0] == 'N') {
        FUN_0058f6a9(local_214,s__d__s__c_005c4b7c,local_20c,local_104,0x4e);
      }
      else {
        FUN_0058f6a9(local_214,s__d__s__c_005c4b88,local_20c,local_104,0x41);
      }
      local_218 = local_218 + 1;
      local_20c = local_218;
    }
    FUN_0058f62c(local_214);
    FUN_0058f62c(iVar1);
  }
  return;
}

