// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054bc4f | Name: FUN_0054bc4f


undefined4 FUN_0054bc4f(int param_1,undefined4 param_2)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  byte bVar4;
  int iVar5;
  undefined4 local_14;
  undefined2 local_c;
  
  iVar5 = FUN_00559ef5(param_2);
  bVar1 = *(byte *)(param_1 + 0x3c);
  bVar2 = *(byte *)(param_1 + 0x3d);
  bVar3 = *(byte *)(param_1 + 0x3e);
  bVar4 = *(byte *)(param_1 + 0x3f);
  local_c = *(short *)(param_1 + 0x24) * 2;
  if ((iVar5 == 1) && ((*(uint *)(param_1 + 0x2dc) & 0x40000000) != 0)) {
    local_c = 0x357;
  }
  else if (iVar5 == 1) {
    iVar5 = *(short *)(param_1 + 0x24) + -199;
    if (0 < iVar5) {
      local_c = local_c + (short)iVar5;
    }
    local_14 = FUN_0054cd07(param_1 + 0x5c,0x56);
    if (0x38 < local_14) {
      local_14 = 0x38;
    }
    local_c = local_c + (short)local_14 * 2;
    if (0x162 < *(short *)(param_1 + 0x24)) {
      local_c = local_c - (*(short *)(param_1 + 0x24) * 2 + -0x2c4);
    }
  }
  *(ushort *)(param_1 + 0x2e2) =
       local_c - ((ushort)bVar1 + (ushort)bVar2 + (ushort)bVar3 + (ushort)bVar4);
  return 1;
}

