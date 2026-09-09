// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004561e0 | Name: FUN_004561e0


void __thiscall FUN_004561e0(int param_1,uint param_2)

{
  POINT pt;
  LONG LVar1;
  LONG LVar2;
  undefined4 uVar3;
  undefined1 local_a0 [128];
  RECT local_20;
  int *local_10;
  BOOL local_c;
  int local_8;
  
  local_8 = *(int *)(param_1 + 0x7c);
  if ((((local_8 == 0) || (*(uint *)(local_8 + 0x20) < 1000)) && (local_8 != 0)) &&
     ((*(int *)(local_8 + 0x238) == 1 && (*(int *)(param_1 + 0x27470) + 500U < param_2)))) {
    *(undefined4 *)(param_1 + 0x26ed8) = *(undefined4 *)(local_8 + 0x20);
    local_10 = *(int **)(param_1 + 0x28728);
    FUN_0058f078(local_a0,&DAT_00a35f78,local_8 + 0x168);
    if (DAT_005b892c == 2) {
      (**(code **)(*local_10 + 0x80))(&DAT_005d0440,0);
    }
    else {
      (**(code **)(*local_10 + 0x80))(local_a0,0);
    }
    local_20.left = 0xa04;
    local_20.top = 0x699;
    local_20.right = 0xa13;
    local_20.bottom = 0x6af;
    LVar1 = __ftol();
    LVar2 = __ftol();
    pt.y = LVar2;
    pt.x = LVar1;
    local_c = PtInRect(&local_20,pt);
    (**(code **)(**(int **)(param_1 + 0x287b8) + 0x60))(1);
    (**(code **)(**(int **)(param_1 + 0x287b4) + 0x60))(1);
    (**(code **)(**(int **)(param_1 + 0x287bc) + 0x60))(1);
    (**(code **)(**(int **)(param_1 + 0x287d4) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x287d8) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x287dc) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x287e0) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x287c4) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x287c8) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x287cc) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x287d0) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x287c0) + 0x60))(1);
    (**(code **)(**(int **)(param_1 + 0x287e4) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x287e8) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x287ec) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x287f0) + 0x60))(0);
    *(int *)(param_1 + 0x26e94) = local_8;
    (**(code **)(**(int **)(param_1 + 0x27c74) + 0x60))(1);
    uVar3 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(param_1 + 0x27470) = uVar3;
  }
  return;
}

