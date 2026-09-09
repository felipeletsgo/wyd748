// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00427d04 | Name: FUN_00427d04


undefined4 __fastcall FUN_00427d04(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  
  iVar4 = param_1 + 4 + *(int *)(param_1 + 0x2a380) * 0x438c;
  iVar1 = iVar4 + 0x440 + *(int *)(iVar4 + 0x4388) * 0xca8;
  puVar2 = (undefined4 *)(iVar1 + 0xe4 + *(int *)(iVar1 + 0xc9c) * 0x14);
  if ((*(int *)(param_1 + 0x2a384) == 0) && (*(int *)(iVar1 + 0xdc) == 0)) {
    uVar3 = FUN_00427f1c();
  }
  else {
    *(undefined4 *)(param_1 + 0x2a38c) = 0;
    *(uint *)(param_1 + 0x2a384) = (uint)(*(int *)(param_1 + 0x2a384) == 0);
    *(undefined4 *)(iVar1 + 0xca0) = *(undefined4 *)(param_1 + 0x2a384);
    FUN_00428172();
    *(undefined4 *)(param_1 + 0x2a3c0) = *(undefined4 *)(iVar1 + 0xca0);
    *(undefined4 *)(param_1 + 0x2a3b4) = *(undefined4 *)(iVar1 + 0xca4);
    *(undefined4 *)(param_1 + 0x2a3c8) = puVar2[4];
    *(undefined4 *)(param_1 + 0x2a3bc) = *(undefined4 *)(param_1 + 0x2a3d8);
    if (*(int *)(param_1 + 0x2a384) == 0) {
      *(undefined4 *)(param_1 + 0x2a3a4) = *puVar2;
      *(undefined4 *)(param_1 + 0x2a3a8) = puVar2[1];
      *(undefined4 *)(param_1 + 0x2a3ac) = puVar2[2];
    }
    else {
      *(int *)(param_1 + 0x2a3a4) = *(int *)(param_1 + 0x2a4fc) - *(int *)(param_1 + 0x2a4f4);
      *(int *)(param_1 + 0x2a3a8) = *(int *)(param_1 + 0x2a500) - *(int *)(param_1 + 0x2a4f8);
      *(undefined4 *)(param_1 + 0x2a3ac) = *(undefined4 *)(iVar4 + 0x438);
    }
    iVar4 = FUN_00427c0c();
    if (iVar4 < 0) {
      if (*(int *)(param_1 + 0x2a384) == 0) {
        uVar3 = 0x80004005;
      }
      else {
        uVar3 = FUN_00427f1c();
      }
    }
    else {
      if (*(int *)(param_1 + 0x2a384) != 0) {
        SetWindowPos(*(HWND *)(param_1 + 0x2a3d8),(HWND)0xfffffffe,*(int *)(param_1 + 0x2a4e4),
                     *(int *)(param_1 + 0x2a4e8),
                     *(int *)(param_1 + 0x2a4ec) - *(int *)(param_1 + 0x2a4e4),
                     *(int *)(param_1 + 0x2a4f0) - *(int *)(param_1 + 0x2a4e8),0x40);
      }
      *(undefined4 *)(param_1 + 0x2a38c) = 1;
      uVar3 = 0;
    }
  }
  return uVar3;
}

