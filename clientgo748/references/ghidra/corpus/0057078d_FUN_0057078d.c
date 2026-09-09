// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057078d | Name: FUN_0057078d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0057078d(uint param_1)

{
  int *piVar1;
  float fVar2;
  float fVar3;
  int iVar4;
  undefined4 uVar5;
  int iVar6;
  float *pfVar7;
  undefined4 uVar8;
  float local_5c [16];
  undefined1 local_1c [8];
  int local_14;
  int local_10;
  
  iVar4 = param_1;
  if (*(int *)(param_1 + 0xc) == 0) {
    if (*(int *)(param_1 + 0x10) == 0) {
      param_1 = 0;
      do {
        (**(code **)(**(int **)(iVar4 + 8) + 0xd0))(*(int **)(iVar4 + 8));
        (**(code **)(**(int **)(iVar4 + 8) + 0x130))
                  (*(int **)(iVar4 + 8),(uint)(*(int *)(iVar4 + 0x1c) != 0) * 2 + 0x142);
        if (*(int *)(iVar4 + 0x1c) == 0) {
          local_5c[0xe] = 0.0;
          local_5c[0xd] = 0.0;
          local_5c[0xc] = 0.0;
          local_5c[0xb] = 0.0;
          local_5c[9] = 0.0;
          local_5c[8] = 0.0;
          local_5c[7] = 0.0;
          local_5c[6] = 0.0;
          local_5c[4] = 0.0;
          local_5c[3] = 0.0;
          local_5c[2] = 0.0;
          local_5c[1] = 0.0;
          local_5c[0xf] = 1.0;
          local_5c[10] = 1.0;
          local_5c[5] = 1.0;
          local_5c[0] = 1.0;
          (**(code **)(**(int **)(iVar4 + 8) + 0x94))(*(int **)(iVar4 + 8),2,local_5c);
          (**(code **)(**(int **)(iVar4 + 8) + 0x94))(*(int **)(iVar4 + 8),3,local_5c);
          if (param_1 == 0) {
            (**(code **)(**(int **)(iVar4 + 8) + 0x94))(*(int **)(iVar4 + 8),0x100,local_5c);
          }
        }
        (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),7,0);
        (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),8,3);
        (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x16,1);
        (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x80,0);
        (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x88,1);
        (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x97,0);
        (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x98,0);
        (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0xa4,1);
        (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0xa7,0);
        if (*(int *)(iVar4 + 0x1c) == 0) {
          (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x1c,0);
          (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x1d,0);
          (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x34,0);
          (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x89,0);
          (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x91,1);
        }
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,1,4);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,2,2);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,3,0);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,4,4);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,5,2);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,6,0);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),1,1,1);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),1,4,1);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,0x10,2);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,0x11,2);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,0x12,0);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,0xb,0);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,0xd,3);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,0xe,3);
        (**(code **)(**(int **)(iVar4 + 8) + 0xfc))(*(int **)(iVar4 + 8),0,0x18,0);
        piVar1 = *(int **)(iVar4 + 8);
        if (*(int *)(iVar4 + 0x18) == 0) {
          (**(code **)(*piVar1 + 200))(piVar1,0xf,1);
          (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x19,7);
          uVar8 = 0x7f;
          uVar5 = 0x18;
        }
        else {
          (**(code **)(*piVar1 + 200))(piVar1,0x1b);
          (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x13,5);
          (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),0x14,6);
          uVar8 = 1;
          uVar5 = 0xab;
        }
        (**(code **)(**(int **)(iVar4 + 8) + 200))(*(int **)(iVar4 + 8),uVar5,uVar8);
        if (param_1 == 0) {
          (**(code **)(**(int **)(iVar4 + 8) + 0xf4))(*(int **)(iVar4 + 8),0,0);
          (**(code **)(**(int **)(iVar4 + 8) + 0x14c))(*(int **)(iVar4 + 8),0,0,0);
          iVar6 = iVar4 + 0x10;
        }
        else {
          iVar6 = iVar4 + 0x14;
        }
        (**(code **)(**(int **)(iVar4 + 8) + 0xd4))(*(int **)(iVar4 + 8),iVar6);
        param_1 = param_1 + 1;
      } while (param_1 < 2);
    }
    (**(code **)(**(int **)(iVar4 + 8) + 0xdc))(*(int **)(iVar4 + 8),*(undefined4 *)(iVar4 + 0x10));
    (**(code **)(**(int **)(iVar4 + 8) + 0xd8))(*(int **)(iVar4 + 8),*(undefined4 *)(iVar4 + 0x14));
    if (*(int *)(iVar4 + 0x1c) == 0) {
      (**(code **)(**(int **)(iVar4 + 8) + 0xa4))(*(int **)(iVar4 + 8),local_1c);
      fVar2 = (float)local_14;
      if (local_14 < 0) {
        fVar2 = fVar2 + _DAT_005a67dc;
      }
      fVar2 = _DAT_005a67d8 / fVar2;
      fVar3 = (float)local_10;
      if (local_10 < 0) {
        fVar3 = fVar3 + _DAT_005a67dc;
      }
      pfVar7 = local_5c;
      for (iVar6 = 0x10; iVar6 != 0; iVar6 = iVar6 + -1) {
        *pfVar7 = 0.0;
        pfVar7 = pfVar7 + 1;
      }
      local_5c[0] = fVar2 + fVar2;
      local_5c[0xc] = -fVar2 - _DAT_005a67d8;
      local_5c[5] = (_DAT_005a67d8 / fVar3) * _DAT_005a681c;
      local_5c[0xd] = _DAT_005a67d8 / fVar3 + _DAT_005a67d8;
      local_5c[0xf] = 1.0;
      (**(code **)(**(int **)(iVar4 + 8) + 0x94))(*(int **)(iVar4 + 8),0x100,local_5c);
    }
    *(undefined4 *)(iVar4 + 0xc) = 1;
    uVar5 = 0;
  }
  else {
    uVar5 = 0x80004005;
  }
  return uVar5;
}

