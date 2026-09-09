// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00598cfc | Name: FUN_00598cfc


bool FUN_00598cfc(int *param_1)

{
  int iVar1;
  int iVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  undefined2 uVar5;
  
  if (DAT_005cc44c != 0) {
    iVar2 = param_1[5];
    if ((iVar2 != DAT_005cc4e0) || (iVar2 != DAT_005cc4f0)) {
      if (DAT_013b80b8 == 0) {
        FUN_00598ea8(1,1,iVar2,4,1,0,0,2,0,0,0);
        FUN_00598ea8(0,1,param_1[5],10,5,0,0,2,0,0,0);
      }
      else {
        if (DAT_013b8158 != 0) {
          uVar4 = 0;
          uVar3 = 0;
          uVar5 = DAT_013b815e;
        }
        else {
          uVar4 = DAT_013b815c;
          uVar3 = DAT_013b815e;
          uVar5 = 0;
        }
        FUN_00598ea8(1,DAT_013b8158 == 0,iVar2,DAT_013b815a,uVar3,uVar4,uVar5,DAT_013b8160,
                     DAT_013b8162,DAT_013b8164,DAT_013b8166);
        if (DAT_013b8104 != 0) {
          uVar4 = 0;
          uVar3 = 0;
          iVar2 = param_1[5];
          uVar5 = DAT_013b810a;
        }
        else {
          iVar2 = param_1[5];
          uVar4 = DAT_013b8108;
          uVar3 = DAT_013b810a;
          uVar5 = 0;
        }
        FUN_00598ea8(0,DAT_013b8104 == 0,iVar2,DAT_013b8106,uVar3,uVar4,uVar5,DAT_013b810c,
                     DAT_013b810e,DAT_013b8110,DAT_013b8112);
      }
    }
    iVar2 = param_1[7];
    if (DAT_005cc4e4 < DAT_005cc4f4) {
      if ((DAT_005cc4e4 <= iVar2) && (iVar2 <= DAT_005cc4f4)) {
        if ((DAT_005cc4e4 < iVar2) && (iVar2 < DAT_005cc4f4)) {
          return true;
        }
LAB_00598e74:
        iVar1 = ((param_1[2] * 0x3c + param_1[1]) * 0x3c + *param_1) * 1000;
        if (iVar2 == DAT_005cc4e4) {
          return DAT_005cc4e8 <= iVar1;
        }
        return iVar1 < DAT_005cc4f8;
      }
    }
    else {
      if (iVar2 < DAT_005cc4f4) {
        return true;
      }
      if (DAT_005cc4e4 < iVar2) {
        return true;
      }
      if ((iVar2 <= DAT_005cc4f4) || (DAT_005cc4e4 <= iVar2)) goto LAB_00598e74;
    }
  }
  return false;
}

