// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053e4ae | Name: FUN_0053e4ae


undefined4 __fastcall FUN_0053e4ae(int param_1)

{
  undefined4 uVar1;
  
  if ((((((*(int *)(param_1 + 0x44) == 1) && (*(int *)(param_1 + 0x18) != -1)) &&
        (*(int *)(param_1 + 0x18) != DAT_005bdef0)) &&
       ((*(int *)(param_1 + 0x18) != DAT_005bdef4 && (*(int *)(param_1 + 0x18) != DAT_005bdef8))))
      && ((*(int *)(param_1 + 0x18) != DAT_005bdefc &&
          ((*(int *)(param_1 + 0x18) != DAT_005bdf00 && (*(int *)(param_1 + 0x18) != DAT_005bdf04)))
          ))) &&
     ((*(int *)(param_1 + 0x18) != DAT_005bdf08 &&
      ((*(int *)(param_1 + 0x18) != DAT_005bdf0c && (*(int *)(param_1 + 0x18) != DAT_005bdf10))))))
  {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

