// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7be5 | Name: FUN_004b7be5


undefined4 __fastcall FUN_004b7be5(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_004b827f(param_1);
  if (iVar1 == 0) {
    FUN_00431159(s_Error_in_Init_UI_Texture_005ba6b4);
    uVar2 = 0;
  }
  else {
    iVar1 = FUN_004b8928();
    if (iVar1 == 0) {
      FUN_00431159(s_Error_in_Init_UI_Texture_Set_005ba6d0);
      uVar2 = 0;
    }
    else {
      iVar1 = FUN_004b8c72();
      if (iVar1 == 0) {
        FUN_00431159(s_Error_in_Init_Effect_Texture_005ba6f0);
        uVar2 = 0;
      }
      else {
        iVar1 = FUN_004b93c4();
        if (iVar1 == 0) {
          FUN_00431159(s_Error_in_Init_Model_Texture_005ba710);
          uVar2 = 0;
        }
        else {
          iVar1 = FUN_004b9ce5();
          if (iVar1 == 0) {
            FUN_00431159(s_Error_in_Init_Environment_Textur_005ba730);
            uVar2 = 0;
          }
          else {
            iVar1 = FUN_004ba353();
            if (iVar1 == 0) {
              FUN_00431159(s_Error_in_Init_Dynamic_Texture_005ba754);
            }
            uVar2 = 1;
          }
        }
      }
    }
  }
  return uVar2;
}

