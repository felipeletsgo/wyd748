// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00555911 | Name: FUN_00555911


/* WARNING: Removing unreachable block (ram,0x0055594a) */
/* WARNING: Removing unreachable block (ram,0x0055597d) */
/* WARNING: Heritage AFTER dead removal. Example location: s0xffffffe8 : 0x00555a66 */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

undefined4 FUN_00555911(int param_1,ushort *param_2,ushort *param_3,byte *param_4)

{
  byte bVar1;
  byte bVar2;
  int iVar3;
  undefined4 uVar4;
  int local_20;
  int local_10;
  
  *param_4 = 0;
  if (param_3 == (ushort *)0x0) {
    uVar4 = 0;
  }
  else {
    if (0x3ff < *param_3) {
      *param_3 = *param_3 & 0x3ff;
    }
    if ((param_2 != (ushort *)0x0) && (0x3ff < *param_2)) {
      *param_2 = *param_2 & 0x3ff;
    }
    bVar1 = *(byte *)(param_1 + 0x47);
    bVar2 = *(byte *)(param_1 + 0x2e6);
    if (((param_2 != (ushort *)0x0) && (param_3 != (ushort *)0x0)) && (*param_3 != *param_2)) {
      *param_2 = *param_3;
    }
    iVar3 = *(int *)(&DAT_00e33b14 + (uint)*param_3 * 4);
    for (local_10 = 0; local_10 < 2; local_10 = local_10 + 1) {
      local_20 = 0;
      if ((local_10 == 0) && (iVar3 < (((int)(uint)bVar1 >> 4) + -5) * 100)) {
        local_20 = 1;
      }
      if ((local_10 == 1) && (1000 - iVar3 < (int)((uint)bVar2 << 1))) {
        local_20 = 1;
      }
      *param_4 = *param_4 | (byte)(local_20 << ((byte)local_10 & 0x1f));
    }
    if (param_2 != (ushort *)0x0) {
      *param_2 = *param_2 + 1;
    }
    *param_3 = *param_3 + 1;
    uVar4 = 1;
  }
  return uVar4;
}

