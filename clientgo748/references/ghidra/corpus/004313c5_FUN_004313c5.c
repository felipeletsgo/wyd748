// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004313c5 | Name: FUN_004313c5


HRESULT __thiscall FUN_004313c5(int param_1,char *param_2)

{
  size_t sVar1;
  int local_214;
  short local_210 [259];
  undefined2 local_a;
  HRESULT local_8;
  
  if (param_2 == (char *)0x0) {
    local_8 = -0x7fffbffd;
  }
  else {
    sVar1 = _strlen(param_2);
    for (local_214 = 0; local_214 < (int)sVar1; local_214 = local_214 + 1) {
      local_210[local_214] = (short)param_2[local_214];
    }
    local_210[local_214] = 0;
    local_a = 0;
    local_8 = CoCreateInstance((IID *)&DAT_005a9f7c,(LPUNKNOWN)0x0,1,(IID *)&DAT_005a9f0c,
                               (LPVOID *)(param_1 + 0x110));
    if (((((-1 < local_8) && (-1 < local_8)) &&
         (local_8 = (**(code **)(**(int **)(param_1 + 0x110) + 0x34))
                              (*(undefined4 *)(param_1 + 0x110),local_210,0), -1 < local_8)) &&
        (((local_8 = (**(code **)**(undefined4 **)(param_1 + 0x110))
                               (*(undefined4 *)(param_1 + 0x110),&DAT_005a9f6c,param_1 + 0x114),
          -1 < local_8 &&
          (local_8 = (**(code **)**(undefined4 **)(param_1 + 0x110))
                               (*(undefined4 *)(param_1 + 0x110),&DAT_005a9f5c,param_1 + 0x118),
          -1 < local_8)) &&
         ((local_8 = (**(code **)**(undefined4 **)(param_1 + 0x110))
                               (*(undefined4 *)(param_1 + 0x110),&DAT_005a9f3c,param_1 + 0x11c),
          -1 < local_8 &&
          ((local_8 = (**(code **)**(undefined4 **)(param_1 + 0x110))
                                (*(undefined4 *)(param_1 + 0x110),&DAT_005a9f2c,param_1 + 0x120),
           -1 < local_8 &&
           (local_8 = (**(code **)(**(int **)(param_1 + 0x118) + 0x34))
                                (*(undefined4 *)(param_1 + 0x118),*(undefined4 *)(DAT_013b71e0 + 4),
                                 0x40d,0), -1 < local_8)))))))) &&
       ((local_8 = (**(code **)(**(int **)(param_1 + 0x11c) + 0x74))
                             (*(undefined4 *)(param_1 + 0x11c),*(undefined4 *)(DAT_013b71e0 + 4)),
        -1 < local_8 &&
        ((((local_8 = (**(code **)(**(int **)(param_1 + 0x11c) + 0x24))
                                (*(undefined4 *)(param_1 + 0x11c),0x46000000), -1 < local_8 &&
           (local_8 = (**(code **)(**(int **)(param_1 + 0x11c) + 0x4c))
                                (*(undefined4 *)(param_1 + 0x11c),0), -1 < local_8)) &&
          (local_8 = FUN_0043172e(), -1 < local_8)) &&
         (local_8 = (**(code **)(**(int **)(param_1 + 0x114) + 0x1c))
                              (*(undefined4 *)(param_1 + 0x114)), -1 < local_8)))))) {
      if (*(int *)(param_1 + 0x11c) != 0) {
        (**(code **)(**(int **)(param_1 + 0x11c) + 0x4c))
                  (*(undefined4 *)(param_1 + 0x11c),0xffffffff);
      }
      *(undefined4 *)(param_1 + 0x10c) = 2;
    }
  }
  return local_8;
}

