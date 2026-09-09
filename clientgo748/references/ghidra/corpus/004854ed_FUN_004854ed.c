// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004854ed | Name: FUN_004854ed


undefined4 __thiscall FUN_004854ed(int param_1,int param_2)

{
  int iVar1;
  int *piVar2;
  undefined1 local_94 [128];
  int *local_14;
  undefined4 *local_10;
  int local_c;
  int local_8;
  
  local_c = param_2;
  local_8 = *(int *)(param_1 + 0x27b38);
  if (((local_8 != 0) && (iVar1 = FUN_0040c0f0(), iVar1 == 1)) &&
     (*(uint *)(local_c + 0xc) == (uint)*(ushort *)(param_1 + 0x275ae))) {
    local_14 = *(int **)(param_1 + 0x27b3c + *(int *)(local_c + 0x10) * 4);
    piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(*(int *)(local_c + 0x10) + 800)
    ;
    *(undefined1 *)((int)piVar2 + 0xcdd) = 1;
    FUN_0058f078(local_94,&DAT_005b8884,0);
    (**(code **)(*piVar2 + 0x80))(local_94,0);
    local_10 = (undefined4 *)(**(code **)(*local_14 + 0xa8))(0,0);
    if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
       (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == local_10)) {
      *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
    }
    if ((local_10 != (undefined4 *)0x0) && (local_10 != (undefined4 *)0x0)) {
      (**(code **)*local_10)(1);
    }
  }
  return 1;
}

