// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004afbc6 | Name: FUN_004afbc6


undefined4 __fastcall FUN_004afbc6(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  int local_10c;
  undefined1 local_108 [256];
  int local_8;
  
  iVar1 = (**(code **)(*param_1 + 4))();
  if (iVar1 == 0) {
    FUN_00431159(s_Initialize_Bone_Animation_Failed_005b9890);
    uVar2 = 0;
  }
  else {
    local_8 = 0;
    local_8 = FUN_0058f716(s_mesh_MeshList_txt_005b98b8,&DAT_005b98b4);
    if (local_8 == 0) {
      uVar2 = 0;
    }
    else {
      local_10c = 0;
      while (iVar1 = FUN_00590a68(local_8,s__d__s_005b98cc,&local_10c,local_108), iVar1 != -1) {
        FUN_0058f078(param_1 + local_10c * 0x16 + 2,&DAT_005b98d4,local_108);
      }
      FUN_0058f62c(local_8);
      uVar2 = FUN_004afca2();
    }
  }
  return uVar2;
}

