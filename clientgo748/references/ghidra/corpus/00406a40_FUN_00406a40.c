// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00406a40 | Name: FUN_00406a40


undefined4 __thiscall
FUN_00406a40(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  undefined4 uVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 0x3c) == 0) {
    uVar1 = 0;
  }
  else {
    iVar2 = FUN_00401000(param_4,param_5,*(undefined4 *)(param_1 + 0x4c),
                         *(undefined4 *)(param_1 + 0x50),*(undefined4 *)(param_1 + 0x54),
                         *(undefined4 *)(param_1 + 0x58));
    if (param_2 == 0x201) {
      if (iVar2 == 1) {
        *(undefined4 *)(param_1 + 0x30) = 1;
        uVar1 = 1;
      }
      else {
        uVar1 = 0;
      }
    }
    else {
      uVar1 = FUN_0040bf90(param_2,param_3,param_4,param_5);
    }
  }
  return uVar1;
}

