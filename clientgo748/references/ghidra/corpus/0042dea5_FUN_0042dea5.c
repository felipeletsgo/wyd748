// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042dea5 | Name: FUN_0042dea5


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0042dea5(int *param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 *puVar3;
  int local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (param_2 == 0) {
    (**(code **)(*param_1 + 0x30))(7,1);
    (**(code **)(*param_1 + 0x30))(0x1c,0);
    (**(code **)(*param_1 + 0x30))(0x89,0);
    (**(code **)(*param_1 + 0x30))(0x1d,1);
    (**(code **)(*param_1 + 0x30))(0x16,3);
    (**(code **)(*param_1 + 0x30))(9,2);
    (**(code **)(*param_1 + 0x30))(0x8b,&DAT_00aaaaaa);
    (**(code **)(*param_1 + 0x30))(0x13,5);
    (**(code **)(*param_1 + 0x30))(0x14,7);
    (**(code **)(*param_1 + 0x30))(0x8d,1);
    (**(code **)(*param_1 + 0x30))(0x1a,1);
    (**(code **)(*param_1 + 0x30))(0x1b,0);
    (**(code **)(*param_1 + 0x34))(0,1,2);
    (**(code **)(*param_1 + 0x34))(0,2,2);
    (**(code **)(*param_1 + 0x34))(0,3,1);
    (**(code **)(*param_1 + 0x34))(0,4,2);
    (**(code **)(*param_1 + 0x34))(0,5,2);
    (**(code **)(*param_1 + 0x34))(0,6,1);
    (**(code **)(*param_1 + 0x34))(0,0x11,2);
    (**(code **)(*param_1 + 0x34))(0,0x10,2);
    (**(code **)(*param_1 + 0x34))(0,0x12,1);
    (**(code **)(*param_1 + 0x34))(0,0xb,0);
    (**(code **)(*(int *)param_1[0xa8f9] + 0x130))(param_1[0xa8f9],0x142);
  }
  if ((param_2 == 1) || (param_2 == 3)) {
    if (param_2 == 1) {
      param_1[0xa9e2] = param_1[0xa9b9];
      param_1[0xa9e3] = param_1[0xa9ba];
      param_1[0xa9e4] = param_1[0xa9bb];
      param_1[0xa9e5] = param_1[0xa9bc];
      param_1[0xa9e6] = param_1[0xa9b9];
      param_1[0xa9e7] = param_1[0xa9ba];
      param_1[0xa9e8] = param_1[0xa9bb];
      param_1[0xa9e9] = param_1[0xa9bc];
      param_1[0xa9fc] = param_1[0xa9bd];
      param_1[0xa9fd] = param_1[0xa9be];
      param_1[0xa9fe] = param_1[0xa9bf];
      param_1[0xa9ff] = param_1[0xa9c0];
      param_1[0xaa00] = param_1[0xa9bd];
      param_1[0xaa01] = param_1[0xa9be];
      param_1[0xaa02] = param_1[0xa9bf];
      param_1[0xaa03] = param_1[0xa9c0];
    }
    else {
      param_1[0xa9e4] = 0x3f800000;
      param_1[0xa9e3] = 0x3f800000;
      param_1[0xa9e2] = 0x3f800000;
      param_1[0xa9e8] = 0x3f800000;
      param_1[0xa9e7] = 0x3f800000;
      param_1[0xa9e6] = 0x3f800000;
      param_1[0xa9fe] = 0x3f19999a;
      param_1[0xa9fd] = 0x3f19999a;
      param_1[0xa9fc] = 0x3f19999a;
      param_1[0xaa02] = 0x3f19999a;
      param_1[0xaa01] = 0x3f19999a;
      param_1[0xaa00] = 0x3f19999a;
    }
    piVar1 = (int *)FUN_00430f50(0x3f800000,0xbf800000,0xbf800000);
    param_1[0xa9f1] = *piVar1;
    param_1[0xa9f2] = piVar1[1];
    param_1[0xa9f3] = piVar1[2];
    piVar1 = (int *)FUN_00430f50(0,0,0x3f800000);
    param_1[0xaa0b] = *piVar1;
    param_1[0xaa0c] = piVar1[1];
    param_1[0xaa0d] = piVar1[2];
    for (local_18 = 0; local_18 < 2; local_18 = local_18 + 1) {
      iVar2 = (**(code **)(*(int *)param_1[0xa8f9] + 0xb0))
                        (param_1[0xa8f9],local_18,param_1 + local_18 * 0x1a + 0xa9e1);
      if (iVar2 < 0) {
        return 0;
      }
      iVar2 = (**(code **)(*(int *)param_1[0xa8f9] + 0xb8))(param_1[0xa8f9],local_18,1);
      if (iVar2 < 0) {
        return 0;
      }
    }
    FUN_00430f10();
    if ((DAT_005ccfa0 == 0) || (DAT_005ccfa0 == 4)) {
      puVar3 = (undefined4 *)FUN_00430f80(-(float)param_1[0xa9f1],0x3f000000,param_1[0xa9f3],0);
      local_14 = *puVar3;
      local_10 = puVar3[1];
      local_c = puVar3[2];
      local_8 = puVar3[3];
    }
    else {
      puVar3 = (undefined4 *)
               FUN_00430f80(*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x58) -
                            (float)param_1[0xa9f1],0x3f000000,
                            (*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x60) +
                            (float)param_1[0xa9f3]) - _DAT_005a40ec,0);
      local_14 = *puVar3;
      local_10 = puVar3[1];
      local_c = puVar3[2];
      local_8 = puVar3[3];
    }
    (**(code **)(*(int *)param_1[0xa8f9] + 0x13c))(param_1[0xa8f9],1,&local_14,1);
    if (DAT_005ccf9c[0xa944] == 0x20) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xaa);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xff);
    }
    (**(code **)(*param_1 + 0x30))(0x1b,1);
    (**(code **)(*param_1 + 0x30))(0xf,1);
    (**(code **)(*param_1 + 0x30))(0x19,7);
    (**(code **)(*param_1 + 0x30))(0x8b,0xffffffff);
    (**(code **)(*param_1 + 0x30))(7,1);
    (**(code **)(*param_1 + 0x30))(0x17,4);
    (**(code **)(*param_1 + 0x30))(0x2f,0);
    (**(code **)(*param_1 + 0x30))(0xe,1);
    (**(code **)(*param_1 + 0x30))(0x89,1);
    (**(code **)(*param_1 + 0x30))(0x1d,0);
    (**(code **)(*param_1 + 0x30))(0x16,3);
    (**(code **)(*param_1 + 0x30))(9,2);
    (**(code **)(*param_1 + 0x30))(0x8d,1);
    (**(code **)(*param_1 + 0x30))(0x8f,1);
    (**(code **)(*param_1 + 0x30))(0x1a,1);
    (**(code **)(*param_1 + 0x30))(0x13,2);
    (**(code **)(*param_1 + 0x30))(0x14,6);
    (**(code **)(*param_1 + 0x30))(0x1c,param_1[0xac0e]);
    (**(code **)(*param_1 + 0x30))(0x30,0);
    (**(code **)(*param_1 + 0x30))(0x22,param_1[0xac0d]);
    (**(code **)(*param_1 + 0x30))(0x8c,3);
    (**(code **)(*param_1 + 0x30))(0x24,param_1[0xac11]);
    (**(code **)(*param_1 + 0x30))(0x25,param_1[0xac12]);
    (**(code **)(*param_1 + 0x30))(0x91,1);
    (**(code **)(*param_1 + 0x30))(0x92,2);
    (**(code **)(*param_1 + 0x34))(0,1,4);
    (**(code **)(*param_1 + 0x34))(0,2,2);
    (**(code **)(*param_1 + 0x34))(0,3,0);
    (**(code **)(*param_1 + 0x34))(0,4,2);
    (**(code **)(*param_1 + 0x34))(0,5,2);
    (**(code **)(*param_1 + 0x34))(0,6,1);
    (**(code **)(*param_1 + 0x34))(0,0x11,2);
    (**(code **)(*param_1 + 0x34))(0,0x10,2);
    (**(code **)(*param_1 + 0x34))(0,0x12,2);
    (**(code **)(*param_1 + 0x34))(0,0xb,0);
    (**(code **)(*param_1 + 0x34))(1,1,1);
    (**(code **)(*param_1 + 0x34))(1,2,2);
    (**(code **)(*param_1 + 0x34))(1,3,1);
    (**(code **)(*param_1 + 0x34))(1,4,2);
    (**(code **)(*param_1 + 0x34))(1,5,2);
    (**(code **)(*param_1 + 0x34))(1,6,1);
    (**(code **)(*param_1 + 0x34))(1,0x11,2);
    (**(code **)(*param_1 + 0x34))(1,0x10,2);
    (**(code **)(*param_1 + 0x34))(1,0x12,0);
    (**(code **)(*param_1 + 0x34))(1,0xb,1);
    (**(code **)(*param_1 + 0x34))(2,1,1);
  }
  if (param_2 == 2) {
    (**(code **)(*param_1 + 0x30))(7,0);
    (**(code **)(*param_1 + 0x30))(0x1b,1);
    (**(code **)(*param_1 + 0x30))(0x13,5);
    (**(code **)(*param_1 + 0x30))(0x14,6);
    (**(code **)(*param_1 + 0x30))(0xf,1);
    (**(code **)(*param_1 + 0x30))(0x18,8);
    (**(code **)(*param_1 + 0x30))(0x19,7);
    (**(code **)(*param_1 + 0x30))(8,3);
    (**(code **)(*param_1 + 0x30))(0x16,3);
    (**(code **)(*param_1 + 0x30))(0x34,0);
    (**(code **)(*param_1 + 0x30))(0x88,1);
    (**(code **)(*param_1 + 0x30))(0x28,0);
    (**(code **)(*param_1 + 0x30))(0x98,0);
    (**(code **)(*param_1 + 0x30))(0x97,0);
    (**(code **)(*param_1 + 0x30))(0xa7,0);
    (**(code **)(*param_1 + 0x30))(0x1c,0);
    (**(code **)(*param_1 + 0x34))(0,1,4);
    (**(code **)(*param_1 + 0x34))(0,2,2);
    (**(code **)(*param_1 + 0x34))(0,3,0);
    (**(code **)(*param_1 + 0x34))(0,4,4);
    (**(code **)(*param_1 + 0x34))(0,5,2);
    (**(code **)(*param_1 + 0x34))(0,6,0);
    (**(code **)(*param_1 + 0x34))(0,0x11,1);
    (**(code **)(*param_1 + 0x34))(0,0x10,1);
    (**(code **)(*param_1 + 0x34))(0,0x12,0);
    (**(code **)(*param_1 + 0x34))(0,0xb,0);
    (**(code **)(*param_1 + 0x34))(0,0x18,0);
    (**(code **)(*param_1 + 0x34))(1,1,1);
    (**(code **)(*param_1 + 0x34))(1,4,1);
  }
  return 1;
}

