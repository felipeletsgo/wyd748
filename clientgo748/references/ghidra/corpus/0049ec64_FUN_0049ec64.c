// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049ec64 | Name: FUN_0049ec64


undefined4 __thiscall
FUN_0049ec64(undefined4 param_1,int param_2,undefined4 param_3,undefined1 param_4)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int local_11c;
  int local_118;
  char local_114 [256];
  char local_14 [12];
  undefined1 local_8 [4];
  
  iVar1 = FUN_0058f716(param_3,&DAT_005b926c,param_1);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    local_118 = 1;
    local_11c = 0;
    while ((local_11c < 100 && (iVar3 = FUN_00590a06(local_114,0x100,iVar1), iVar3 != 0))) {
      _memset(local_14,0,0xb);
      _strncpy(local_14,local_114,10);
      FUN_005909d2(local_14,s__d__x_005b9270,&local_118,local_8);
      *(undefined1 *)(param_2 + local_118 + -1) = param_4;
      local_11c = local_11c + 1;
    }
    FUN_0058f62c(iVar1);
    uVar2 = 1;
  }
  return uVar2;
}

