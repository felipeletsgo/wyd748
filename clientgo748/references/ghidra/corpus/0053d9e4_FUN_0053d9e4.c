// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053d9e4 | Name: FUN_0053d9e4


undefined4 __fastcall FUN_0053d9e4(int *param_1)

{
  int iVar1;
  undefined1 local_6c [104];
  
  iVar1 = (**(code **)(*param_1 + 0x3c))();
  if (iVar1 == 1) {
    if (param_1[10] == 1) {
      (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0xb4))
                (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),param_1[0xc],local_6c);
      iVar1 = _memcmp(local_6c,param_1 + 0xd,0x68);
      if (iVar1 != 0) {
        (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0xb0))
                  (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),param_1[0xc],param_1 + 0xd);
      }
      (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0xb8))
                (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),param_1[0xc],param_1[10]);
    }
    else {
      (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0xb8))
                (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),param_1[0xc],0);
    }
  }
  return 1;
}

