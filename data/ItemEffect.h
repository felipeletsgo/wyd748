//#define EF_NONE     			  0	// * EFFECT Type�� 0 �̸� ����Ʈ�� ����.  ITEM���� Score[0]�� ���� ���Ҽ� ����.
// Score ����
#define EF_LEVEL     			  1	// ���� * Score[1] - Score[15] Bonus Score���� �״�� ���� ������.
#define EF_DAMAGE                 2 // ����
#define EF_AC       			  3	// ���
#define EF_HP        			  4	// ����  HP,MP,���ݼӵ�,�̵��ӵ�,���� MAX���
#define EF_MP       			  5	// ����
#define EF_EXP      			  6	// ����
#define EF_STR      		      7	// ��
#define EF_INT      		      8	// ����
#define EF_DEX       		      9	// ��ø
#define EF_CON      		     10	// ü��
#define EF_SPECIAL1    		     11	// ����
#define EF_SPECIAL2    			 12	// Ư1
#define EF_SPECIAL3    			 13	// Ư2
#define EF_SPECIAL4    			 14	// Ư3
// Score ��.
#define EF_SCORE14     			 15	//
#define EF_SCORE15     			 16	//
////////////////      REQUIREMENT        //////////////////////////////////////////
#define EF_POS                   17  // ��ġ ������ �ִ� ��� bit��         - ����Ʈ NOWHERE (0��Ʈ �� EQUIP 0)
#define EF_CLASS                 18  // ���� ������ �ִ� (����)Ŭ���� bit�� - ����Ʈ YES
#define EF_R1SIDC                19  // �Ѽ� 1�ڵ� �Ǵ� ������ ������ �ִ� STR INT DEC CON         - ����Ʈ 0
#define EF_R2SIDC                20  // �μ� 2�ڵ�          �� ������ �ִ� STR INT DEC CON         - ����Ʈ �Ұ�
////////////////   BONUS  ////////////////////////////////
#define EF_WTYPE         	     21	 // ���� ����з� (�ִϸ��̼�)
//  1:�Ұ�(Ŭ��)
//  2:�߰�(���̽�)
//  3:���

// 11:�ҵ���
// 12:�ߵ���
// 13:�뵵��

// 21:��â
// 22:��â
// 23:��â

// 31:��������
// 32:��������
// 33:��������

// 41:��Ŭ��

// 51:�ҹ���

// 61:��Ŭ��
// 62:��Ŭ��
// 63:��Ŭ��

// 101:Ȱ
// 102:��Ʈ
// 103:�丶ȣũ
// 104:����
////////////////////////////////////////////////////////////////////////////
#define EF_REQ_STR               22  // ����
#define EF_REQ_INT               23  // ����
#define EF_REQ_DEX               24  // ����
#define EF_REQ_CON               25  // ����
/////////////////////////////////////////////////////////////////////////
// * �� �䱸ġ�̳�, �������� �⺻ SIDC�� �߰��Ǵ� ���̴�.
/////////////////////////////////////////////////////////////////////////


#define EF_ATTSPEED  	       	 26	 // ���� ���ݼӵ�
#define EF_RANGE                 27  // ���� �����Ÿ�. ��ü�� �ִ밪�� �����.
////#define EF_PRICE				 28	 // ����
#define EF_RUNSPEED              29  // ���� �޸���ӵ�.
#define EF_SPELL    	    	 30	 // ���� Item�� �԰ų� �Ծ����� �ɸ��� ����
#define EF_DURATION              31  // ���� Item �Ķ��Ÿ1 - ������ ȿ���� �߻��ϴ� �ð�, �Դ¾������� �׻�
#define EF_PARM2     			 32	 // �Ķ� Item �Ķ��Ÿ2
#define EF_GRID                  33  // ũ�� Carry�� �����ϴ� �����۱׸���
#define EF_GROUND                34  // ��ĭ ������� ���� �����ϴ� ũ��
#define EF_CLAN                  35  // ����
#define EF_HWORDCOIN             36  // �뵷
#define EF_LWORDCOIN             37  // �ҵ�
#define EF_VOLATILE              38  // �Ҹ� ������ �ٷ� ������� ������.  0:�Ϲ��屸  1:�������  2:������   3:���� 4:��1  5:��2  6:��3
//                               6:����+15   7:����+1
#define EF_KEYID                 39  // Ű��
#define EF_PARRY                 40  // ȸ�� ȸ���� ������
#define EF_HITRATE               41  // ���� ���߷� ������
#define EF_CRITICAL              42  // ũ�� ũ��Ƽ��
#define EF_SANC                  43  // ���� Sanctuary �ַ� ������ ����Ʈ�� ����Ǹ�
#define EF_SAVEMANA              44  // ���� 1 -> 99%
#define EF_HPADD                 45  // �ǻ� 1 -> 101%   MAX_MP
#define EF_MPADD                 46  // ���� 1 -> 101%   MAX_MP
#define EF_REGENHP               47  // ��ȸ
#define EF_REGENMP               48  // ��ȸ
#define EF_RESIST1               49  // ����
#define EF_RESIST2               50  // ����
#define EF_RESIST3               51  // ����
#define EF_RESIST4               52  // ����
#define EF_ACADD                 53  // ����
#define EF_RESISTALL             54  // ����
#define EF_BONUS                 55  // ����
#define EF_HWORDGUILD            56  // Ataque PvP
#define EF_LWORDGUILD            57  // Defesa PvP
#define EF_QUEST                 58  // �ӹ� �ݵ�� 1���� ����, ��κ� Volatile�� �԰� ���õ� �� ��.
#define EF_UNIQUE                59  // ����
#define EF_MAGIC                 60  // ���� 1 -> 1% ����
#define EF_AMOUNT                61  // ����
#define EF_HWORDINDEX            62  // ��ȣ
#define EF_LWORDINDEX            63  // ��ȣ
#define EF_INIT1                 64  // �ʱ�
#define EF_INIT2                 65  // �ʱ�
#define EF_INIT3                 66
#define EF_DAMAGEADD             67
#define EF_MAGICADD              68

#define EF_HPADD2                69
#define EF_MPADD2                70
#define EF_CRITICAL2             71
#define EF_ACADD2                72
#define EF_DAMAGE2               73
#define EF_SPECIALALL            74

#define	EF_CURKILL				 75
#define EF_LTOTKILL				 76
#define EF_HTOTKILL				 77
#define EF_INCUBATE				 78


#define EF_MOUNTLIFE			 79  // �� �� ����
#define EF_MOUNTHP				 80  // �� �� ��
#define EF_MOUNTSANC			 81  // �� ���嵵
#define EF_MOUNTFEED			 82  // �� ��θ� ����
#define EF_MOUNTKILL			 83  // ���� ���� ���� ����?

#define EF_INCUDELAY             84
#define EF_SUBGUILD				 85

#define EF_ITEMLEVEL			 87
#define EF_DONATE				 91

#define EF_GRADE0				 100 // A
#define EF_GRADE1				 101 // B
#define EF_GRADE2				 102 // C
#define EF_GRADE3				 103 // D
#define EF_GRADE4				 104 // E
#define EF_GRADE5				 105 // E Arch

#define EF_WDAY					 106
#define EF_HOUR					 107
#define EF_MIN					 108
#define EF_YEAR					 109
#define EF_WMONTH				 110

#define EF_MOBTYPE				 112
#define EF_ITEMTYPE				 113

#define EF_NOSANC				 126
#define EF_NOTRADE				 127