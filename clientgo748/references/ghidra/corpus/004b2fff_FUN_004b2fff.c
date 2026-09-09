// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b2fff | Name: FUN_004b2fff


void __fastcall FUN_004b2fff(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int local_10;
  int local_8;
  
  if (((*(int *)(param_1 + 0x1e424) != 0) && (DAT_0067cf38 != 0)) &&
     (*(int *)(DAT_0067cf38 + 0x28) != 0)) {
    (**(code **)(*DAT_005ccf9c + 0x4c))();
    (**(code **)(*DAT_005ccf9c + 0x38))(0,0);
    (**(code **)(*DAT_005ccf9c + 0x38))(1,0);
    (**(code **)(*DAT_005ccf9c + 0x38))(2,0);
    iVar1 = *(int *)(DAT_0067cf38 + 0x28);
    iVar3 = iVar1 + 0x3c;
    (**(code **)(*DAT_005ccf9c + 0x30))(7,0);
    for (local_8 = 0; local_8 < 0x1e; local_8 = local_8 + 1) {
      if (*(int *)(iVar3 + local_8 * 8) != 0) {
        local_10 = *(int *)(iVar3 + local_8 * 8);
        while (local_10 != 0) {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
          FUN_0042e8a1(local_10);
          iVar2 = *(int *)(local_10 + 0x150);
          *(undefined4 *)(local_10 + 0x150) = 0;
          local_10 = iVar2;
        }
        *(undefined4 *)(iVar3 + local_8 * 8) = 0;
        *(undefined4 *)(iVar1 + 0x40 + local_8 * 8) = 0;
      }
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(7,1);
  }
  return;
}

