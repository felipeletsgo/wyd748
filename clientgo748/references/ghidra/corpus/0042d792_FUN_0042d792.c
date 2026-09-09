// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042d792 | Name: FUN_0042d792


undefined4 __thiscall FUN_0042d792(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0xc))(*(undefined4 *)(param_1 + 0x2a3e4));
  if (iVar1 < 0) {
    if (iVar1 == -0x7789f798) {
      uVar2 = 0;
    }
    else {
      if (iVar1 == -0x7789f797) {
        if (*(int *)(param_1 + 0x2a384) != 0) {
          iVar1 = param_1 + 4 + *(int *)(param_1 + 0x2a380) * 0x438c;
          (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x20))
                    (*(undefined4 *)(param_1 + 0x2a3e0),*(undefined4 *)(param_1 + 0x2a380),
                     iVar1 + 0x42c);
          *(undefined4 *)(param_1 + 0x2a3ac) = *(undefined4 *)(iVar1 + 0x438);
        }
        iVar1 = FUN_00427c0c();
        if (iVar1 < 0) {
          return 0;
        }
      }
      uVar2 = 0;
    }
  }
  else {
    if ((param_2 == 1) &&
       (iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x90))
                          (*(undefined4 *)(param_1 + 0x2a3e4),0,0,3,
                           *(undefined4 *)(param_1 + 0x2b034),0x3f800000,0), iVar1 < 0)) {
      return 0;
    }
    iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x88))(*(undefined4 *)(param_1 + 0x2a3e4));
    if (iVar1 < 0) {
      uVar2 = 1;
    }
    else {
      uVar2 = 1;
    }
  }
  return uVar2;
}

