// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578ec6 | Name: FUN_00578ec6


uint __fastcall FUN_00578ec6(undefined4 param_1,int param_2)

{
  int iVar1;
  int iVar2;
  uint in_EAX;
  
  if (((((*(char *)(param_2 + 0x48) != '\0') || (*(char *)(param_2 + 0x108) != '\0')) ||
       (*(int *)(param_2 + 0x24) != 3)) ||
      (((*(int *)(param_2 + 0x20) != 3 || (*(int *)(param_2 + 0x28) != 2)) ||
       ((*(int *)(param_2 + 100) != 3 ||
        ((iVar1 = *(int *)(param_2 + 0xc4), *(int *)(iVar1 + 8) != 2 ||
         (in_EAX = 1, *(int *)(iVar1 + 0x5c) != 1)))))))) ||
     ((*(int *)(iVar1 + 0xb0) != 1 ||
      (((((2 < *(int *)(iVar1 + 0xc) || (*(int *)(iVar1 + 0x60) != 1)) ||
         (*(int *)(iVar1 + 0xb4) != 1)) ||
        ((iVar2 = *(int *)(param_2 + 0x114), *(int *)(iVar1 + 0x24) != iVar2 ||
         (*(int *)(iVar1 + 0x78) != iVar2)))) || (*(int *)(iVar1 + 0xcc) != iVar2)))))) {
    in_EAX = in_EAX & 0xffffff00;
  }
  return in_EAX;
}

