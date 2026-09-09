// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004862b6 | Name: FUN_004862b6


undefined4 __thiscall FUN_004862b6(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int local_18;
  undefined2 local_14 [4];
  int local_c;
  int *local_8;
  
  local_c = param_2;
  local_8 = (int *)(**(code **)(*DAT_013b71e8 + 0x38))(*(undefined4 *)(param_2 + 0xc),param_1);
  if (local_8 != (int *)0x0) {
    iVar2 = FUN_0054cd07(local_8 + 0x58,0x22);
    piVar1 = local_8;
    if (0 < iVar2) {
      _memset(local_14,0,8);
      local_14[0] = (undefined2)piVar1[0x58];
      uVar3 = FUN_0054cd07(local_14,0x22);
      local_18 = piVar1[0xba];
      if (3 < local_18) {
        local_18 = local_18 + -3;
      }
      *(undefined2 *)(piVar1 + 0xc0) = 1;
      uVar4 = __ftol((int)*(char *)(local_c + 0x12));
      uVar4 = __ftol(param_1 + 0x94,uVar4);
      uVar4 = __ftol(uVar4);
      FUN_005554cc(uVar3,local_18,(int)*(short *)(local_c + 0x10),uVar4);
      (**(code **)(*piVar1 + 0x80))(*(short *)(local_c + 0x10) + 3);
    }
  }
  FUN_004431e4(0);
  return 1;
}

