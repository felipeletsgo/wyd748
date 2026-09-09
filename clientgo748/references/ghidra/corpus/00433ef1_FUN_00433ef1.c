// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00433ef1 | Name: FUN_00433ef1


void FUN_00433ef1(int *param_1)

{
  DWORD DVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  undefined4 *puVar5;
  undefined4 local_6cc;
  int local_6c8;
  int *local_6c4;
  undefined1 local_6c0 [632];
  undefined1 local_448;
  undefined4 local_447;
  int local_408;
  undefined1 local_404;
  undefined4 local_403;
  
  local_6c4 = param_1;
  local_6c8 = DAT_0067cf38;
  if ((((DAT_0067cf38 == 0) || (param_1 == (int *)0x0)) || (param_1 == (int *)0xfffffff0)) ||
     (*param_1 == 0)) {
    *(undefined4 *)(DAT_0092e640 + 8 + param_1[0x24] * 0xc) = 0xffffffff;
    FUN_00590fb3();
  }
  else {
    DVar1 = timeGetTime();
    *(DWORD *)(local_6c8 + 0x28e70) = DVar1;
    _memset(local_6c0,0,0x278);
    local_448 = DAT_005d0418;
    puVar5 = &local_447;
    for (iVar3 = 0xf; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar5 = 0;
      puVar5 = puVar5 + 1;
    }
    *(undefined2 *)puVar5 = 0;
    *(undefined1 *)((int)puVar5 + 2) = 0;
    FUN_0058ee20(&local_448,&DAT_00a3fdf8);
    FUN_0058ee30(&local_448,local_6c4 + 4);
    if (*(int *)(local_6c8 + 0x28e6c) == 0) {
      uVar2 = InternetOpenA(&DAT_005b7898,0,0,0,0);
      *(undefined4 *)(local_6c8 + 0x28e6c) = uVar2;
      if (*(int *)(local_6c8 + 0x28e6c) == 0) {
        FUN_00590fb3();
        return;
      }
    }
    iVar3 = InternetOpenUrlA(*(undefined4 *)(local_6c8 + 0x28e6c),&local_448,0,0,0x4000000,0);
    local_404 = DAT_005d041c;
    puVar5 = &local_403;
    for (iVar4 = 0xff; iVar4 != 0; iVar4 = iVar4 + -1) {
      *puVar5 = 0;
      puVar5 = puVar5 + 1;
    }
    *(undefined2 *)puVar5 = 0;
    *(undefined1 *)((int)puVar5 + 2) = 0;
    local_6cc = 0;
    if (iVar3 == 0) {
      InternetCloseHandle(0);
      FUN_00590fb3();
    }
    else {
      _memset(&local_404,0,0x400);
      InternetReadFile(iVar3,&local_404,0x278,&local_6cc);
      FUN_0058f220(local_6c0,&local_404,local_6cc);
      InternetCloseHandle(iVar3);
      local_408 = 0;
      local_408 = FUN_0047e3fd(local_6c0);
      if ((local_408 == 1) && (iVar3 = FUN_004baa4d(local_6c4[0x24],local_6c0), iVar3 == 1)) {
        *(int *)(local_6c8 + 0x28e68) = *(int *)(local_6c8 + 0x28e68) + 1;
        *(int *)(DAT_0092e640 + 8 + local_6c4[0x24] * 0xc) = local_6c4[2] * 0x10000 + local_6c4[3];
        *(int *)(*local_6c4 + 0x1c4) = local_6c4[0x24];
        if ((short)local_6c4[0x27] == 0x1fd) {
          *(undefined4 *)(*local_6c4 + 0x1c8) = 1;
        }
        else if (((short)local_6c4[0x27] < 0x20e) || (0x213 < (short)local_6c4[0x27])) {
          *(undefined4 *)(*local_6c4 + 0x1c8) = 3;
        }
        else {
          *(undefined4 *)(*local_6c4 + 0x1c8) = 2;
        }
      }
      FUN_00590fb3();
    }
  }
  return;
}

