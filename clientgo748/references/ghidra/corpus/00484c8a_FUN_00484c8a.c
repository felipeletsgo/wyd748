// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00484c8a | Name: FUN_00484c8a


undefined4 __thiscall FUN_00484c8a(int param_1,int param_2)

{
  int iVar1;
  int *piVar2;
  
  if ((uint)*(ushort *)(param_2 + 6) == DAT_013b71e8[0x317]) {
    *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
    piVar2 = DAT_013b71e8;
    iVar1 = DAT_013b71e8[0x3bd];
    FUN_0058f220(DAT_013b71e8 + (char)iVar1 * 7 + 0x15,*(int *)(param_1 + 0x4c) + 0x444,0x1c);
    FUN_0058f220(piVar2 + (char)iVar1 * 0x20 + 0x31,DAT_013b71e8 + 0x1d2,0x80);
    (**(code **)(*DAT_013b71e8 + 0x54))(5);
  }
  return 1;
}

