// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b3962 | Name: FUN_004b3962


undefined4 FUN_004b3962(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  iVar1 = (int)(param_1 + (param_1 >> 0x1f & 3U)) >> 2;
  iVar2 = (int)(param_2 + (param_2 >> 0x1f & 3U)) >> 2;
  if ((((iVar2 < 0x400) && (iVar1 < 0x400)) && (-1 < iVar2)) && (-1 < iVar1)) {
    if (((&DAT_00934178)[iVar1 + iVar2 * 0x400] & 0x80) == 0) {
      uVar3 = 0;
    }
    else {
      uVar3 = 1;
    }
  }
  else {
    FUN_00431159(s_Wrong_Position__X__d_Y__d__005b9b1c);
    MessageBoxA(*(HWND *)(DAT_013b71e0 + 4),s_Wrong_Character_Information__005b9b44,s_Error_005b9b3c
                ,0x1000);
    PostMessageA(*(HWND *)(DAT_013b71e0 + 4),0x10,0,0);
    uVar3 = 0;
  }
  return uVar3;
}

