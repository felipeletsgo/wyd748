// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047dee5 | Name: FUN_0047dee5


undefined4 FUN_0047dee5(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined1 local_4c;
  undefined4 local_4b;
  undefined4 local_c;
  int local_8;
  
  if ((param_1 == (int *)0x0) || (*param_1 == 0)) {
    uVar1 = 0;
  }
  else if ((short)param_1[0x27] == 0) {
    uVar1 = 0;
  }
  else {
    local_8 = 0xffffffff;
    local_8 = FUN_0047e21d(param_1[2] * 0x10000 + param_1[3]);
    param_1[0x26] = 1;
    if (local_8 == -1) {
      iVar2 = FUN_0047e26a();
      param_1[0x24] = iVar2;
      if (param_1[0x24] == -1) {
        iVar2 = FUN_0047e2b3();
        param_1[0x24] = iVar2;
      }
      if (param_1[0x24] != -1) {
        local_4c = DAT_005d04b0;
        puVar3 = &local_4b;
        for (iVar2 = 0xf; iVar2 != 0; iVar2 = iVar2 + -1) {
          *puVar3 = 0;
          puVar3 = puVar3 + 1;
        }
        *(undefined2 *)puVar3 = 0;
        *(undefined1 *)((int)puVar3 + 2) = 0;
        local_c = 0;
        if (((((short)param_1[0x27] == 0x20e) || ((short)param_1[0x27] == 0x211)) ||
            ((short)param_1[0x27] == 0x214)) || ((short)param_1[0x27] == 0x217)) {
          local_c = 1;
        }
        if ((((short)param_1[0x27] == 0x20f) || ((short)param_1[0x27] == 0x212)) ||
           (((short)param_1[0x27] == 0x215 || ((short)param_1[0x27] == 0x218)))) {
          local_c = 2;
        }
        if ((((short)param_1[0x27] == 0x210) || ((short)param_1[0x27] == 0x213)) ||
           (((short)param_1[0x27] == 0x216 || ((short)param_1[0x27] == 0x219)))) {
          local_c = 3;
        }
        FUN_0047e191(&local_4c,param_1[3],local_c,param_1[2]);
        FUN_0058ee20(param_1 + 4,&local_4c);
        *(int *)(DAT_0092e640 + 8 + param_1[0x24] * 0xc) = param_1[2] * 0x10000 + param_1[3];
        FUN_00590e7f(FUN_00433ef1,0,param_1);
      }
    }
    else {
      FUN_0047e12a(*param_1,local_8,(int)(short)param_1[0x27]);
    }
    uVar1 = 1;
  }
  return uVar1;
}

