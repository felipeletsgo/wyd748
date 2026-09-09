// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056f302 | Name: FUN_0056f302


void __thiscall FUN_0056f302(int param_1,int param_2,int param_3,undefined4 *param_4)

{
  int iVar1;
  undefined4 *puVar2;
  
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = (undefined4 *)FUN_0056b152(param_4);
  }
  iVar1 = FUN_0056ef89(param_2 + *(int *)(param_1 + 0x1034),param_3 + *(int *)(param_1 + 0x1040),
                       *(int *)(param_1 + 0x1088) != *(int *)(param_1 + 0x1058));
  if (-1 < iVar1) {
    puVar2 = (undefined4 *)
             ((*(int *)(param_1 + 0x1030) - *(int *)(param_1 + 0x1070)) * 0x10 +
             *(int *)(param_1 + 0x106c));
    for (iVar1 = (*(uint *)(param_1 + 0x1058) & 0xfffffff) << 2; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar2 = *param_4;
      param_4 = param_4 + 1;
      puVar2 = puVar2 + 1;
    }
    for (iVar1 = 0; iVar1 != 0; iVar1 = iVar1 + -1) {
      *(undefined1 *)puVar2 = *(undefined1 *)param_4;
      param_4 = (undefined4 *)((int)param_4 + 1);
      puVar2 = (undefined4 *)((int)puVar2 + 1);
    }
    *(undefined4 *)(param_1 + 0x108c) = 1;
  }
  return;
}

