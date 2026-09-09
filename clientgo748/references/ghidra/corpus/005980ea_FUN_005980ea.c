// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005980ea | Name: FUN_005980ea


uint __thiscall FUN_005980ea(undefined4 param_1,int param_2,uint param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  if (param_2 + 1U < 0x101) {
    param_2._2_2_ = *(ushort *)(PTR_DAT_005cc110 + param_2 * 2);
  }
  else {
    if ((PTR_DAT_005cc110[(param_2 >> 8 & 0xffU) * 2 + 1] & 0x80) == 0) {
      local_8 = CONCAT31((int3)((uint)param_1 >> 8),(char)param_2) & 0xffff00ff;
      uVar2 = 1;
    }
    else {
      local_8._0_2_ = CONCAT11((char)param_2,(char)((uint)param_2 >> 8));
      local_8 = CONCAT22((short)((uint)param_1 >> 0x10),(undefined2)local_8) & 0xff00ffff;
      uVar2 = 2;
    }
    iVar1 = FUN_0059ba5d(1,&local_8,uVar2,(int)&param_2 + 2,0,0,1);
    if (iVar1 == 0) {
      return 0;
    }
  }
  return param_2._2_2_ & param_3;
}

