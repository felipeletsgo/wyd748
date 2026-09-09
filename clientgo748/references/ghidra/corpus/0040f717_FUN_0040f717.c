// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040f717 | Name: FUN_0040f717


undefined4 __thiscall FUN_0040f717(int *param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (param_1[0x7f] <= local_8) {
      return 0;
    }
    iVar1 = param_1[local_8 + 0x80];
    if (iVar1 == param_2) break;
    local_8 = local_8 + 1;
  }
  uVar2 = (**(code **)(*param_1 + 0xa4))
                    (*(undefined4 *)(iVar1 + 0x1d0),*(undefined4 *)(iVar1 + 0x1d4));
  return uVar2;
}

