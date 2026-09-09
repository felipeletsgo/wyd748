// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b87be | Name: FUN_004b87be


undefined4 __thiscall FUN_004b87be(int param_1,int param_2,undefined4 param_3)

{
  undefined4 uVar1;
  int iVar2;
  DWORD DVar3;
  
  if ((param_2 < 0x200) && (-1 < param_2)) {
    if ((*(int *)(*(int *)(param_1 + 0xe8328) + param_2 * 4) == 0) &&
       (iVar2 = FUN_004b8333(param_2,0xffffffff), iVar2 == 0)) {
      return 0;
    }
    DVar3 = timeGetTime();
    *(DWORD *)(param_1 + 0x1428 + param_2 * 0x108) = DVar3;
    *(undefined4 *)(param_1 + 0x142c + param_2 * 0x108) = param_3;
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 0xe8328) + param_2 * 4);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

