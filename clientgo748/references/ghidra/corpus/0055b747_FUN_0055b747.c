// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055b747 | Name: FUN_0055b747


undefined4 FUN_0055b747(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 local_30c;
  undefined1 local_308 [4];
  CHAR local_304 [256];
  CHAR local_204 [256];
  CHAR local_104 [256];
  
  local_30c = 0;
  wsprintfA(local_104,s_Mesh__s_005c4b94,param_1,0);
  wsprintfA(local_304,s_D__project_wyd_run_Mesh_meshText_005c4b9c);
  iVar1 = FUN_0058f716(local_304,&DAT_005c4bc8);
  if (iVar1 == 0) {
    uVar2 = 1;
  }
  else {
    do {
      iVar3 = FUN_00590a68(iVar1,s__d__s__c_005c4bcc,&local_30c,local_204,local_308);
      if (iVar3 == -1) {
        return 0xffffff00;
      }
    } while (local_104 != local_204);
    uVar2 = CONCAT31((int3)((uint)local_204 >> 8),1);
  }
  return uVar2;
}

