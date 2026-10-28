//#define EF_NONE     			  0	// * EFFECT type 0 means no effect. An ITEM cannot add the value of Score[0].
// Score start
#define EF_LEVEL     			  1	// Level * Score[1] - Score[15]: the bonus score value is added as is.
#define EF_DAMAGE                 2	// Damage bonus
#define EF_AC       			  3	// Defense
#define EF_HP        			  4	// HP bonus; HP, MP, attack speed, move speed, the various MAX values
#define EF_MP       			  5	// MP bonus
#define EF_EXP      			  6	// Experience
#define EF_STR      		      7	// Strength
#define EF_INT      		      8	// Intelligence
#define EF_DEX       		      9	// Dexterity
#define EF_CON      		     10	// Constitution
#define EF_SPECIAL1    		     11	// Weapon mastery
#define EF_SPECIAL2    			 12	// Special 1
#define EF_SPECIAL3    			 13	// Special 2
#define EF_SPECIAL4    			 14	// Special 3
// Score end.
#define EF_SCORE14     			 15	//
#define EF_SCORE15     			 16	//
////////////////      REQUIREMENT        //////////////////////////////////////////
#define EF_POS                   17	// Position: bit mask of the slots the item can be worn in - default NOWHERE (bit 0 is EQUIP 0)
#define EF_CLASS                 18	// Class: bit mask of the (gender) classes that can wear it - default YES
#define EF_R1SIDC                19	// One hand: STR INT DEX CON needed to wear a one-handed weapon or armor - default 0
#define EF_R2SIDC                20	// Two hands: STR INT DEX CON needed to wear a two-handed weapon - default not allowed
////////////////   BONUS  ////////////////////////////////
#define EF_WTYPE         	     21	// Weapon type: weapon class (animation)
//  1:small sword (club)
//  2:medium sword (mace)
//  3:large sword

// 11:small axe
// 12:medium axe
// 13:large axe

// 21:small spear
// 22:medium spear
// 23:large spear

// 31:small staff
// 32:medium staff
// 33:large staff

// 41:small claw

// 51:small shield

// 61:small club
// 62:medium club
// 63:large club

// 101:bow
// 102:dart
// 103:tomahawk
// 104:javelin
////////////////////////////////////////////////////////////////////////////
#define EF_REQ_STR               22	// Required STR
#define EF_REQ_INT               23	// Required INT
#define EF_REQ_DEX               24	// Required DEX
#define EF_REQ_CON               25	// Required CON
/////////////////////////////////////////////////////////////////////////
// * Main requirement values; for an item they are added to the base SIDC.
/////////////////////////////////////////////////////////////////////////


#define EF_ATTSPEED  	       	 26	// Attack speed
#define EF_RANGE                 27	// Range: attack range. The highest value of all items applies.
////#define EF_PRICE				 28	// Price
#define EF_RUNSPEED              29	// Run speed.
#define EF_SPELL    	    	 30	// Spell cast when the item is eaten or worn
#define EF_DURATION              31	// Duration: item parameter 1 - time the effect lasts when eaten; worn items always apply
#define EF_PARM2     			 32	// Item parameter 2
#define EF_GRID                  33	// Size: item grid occupied in Carry
#define EF_GROUND                34	// Ground cells: size occupied on the ground by castle gates and the like
#define EF_CLAN                  35	// Race
#define EF_HWORDCOIN             36	// Gold (high word)
#define EF_LWORDCOIN             37	// Gold (low word)
#define EF_VOLATILE              38	// Consumable: item that disappears when used.  0:regular gear  1:potions  2:money   3:key 4:blessing 1  5:blessing 2  6:blessing 3
//                               6:spell+15   7:level+1
#define EF_KEYID                 39	// Key number
#define EF_PARRY                 40	// Evasion: evasion rate bonus
#define EF_HITRATE               41	// Accuracy: hit rate bonus
#define EF_CRITICAL              42	// Critical
#define EF_SANC                  43	// Refinement (Sanctuary), mostly stored with the bonus points
#define EF_SAVEMANA              44	// Mana saving 1 -> 99%
#define EF_HPADD                 45	// HP increase 1 -> 101%   MAX_MP
#define EF_MPADD                 46	// MP increase 1 -> 101%   MAX_MP
#define EF_REGENHP               47	// HP regeneration
#define EF_REGENMP               48	// MP regeneration
#define EF_RESIST1               49	// Fire resistance
#define EF_RESIST2               50	// Ice resistance
#define EF_RESIST3               51	// Holy resistance
#define EF_RESIST4               52	// Lightning resistance
#define EF_ACADD                 53	// Defense increase
#define EF_RESISTALL             54	// All resistances
#define EF_BONUS                 55	// Bonus
#define EF_HWORDGUILD            56  // Ataque PvP
#define EF_LWORDGUILD            57  // Defesa PvP
#define EF_QUEST                 58	// Quest: must start at 1; usually set together with Volatile.
#define EF_UNIQUE                59	// Unique
#define EF_MAGIC                 60	// Magic attack 1 -> 1% amplification
#define EF_AMOUNT                61	// Amount
#define EF_HWORDINDEX            62	// Number
#define EF_LWORDINDEX            63	// Number
#define EF_INIT1                 64	// Initial
#define EF_INIT2                 65	// Initial
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


#define EF_MOUNTLIFE			 79	// Mount total life
#define EF_MOUNTHP				 80	// Mount current HP
#define EF_MOUNTSANC			 81	// Mount growth
#define EF_MOUNTFEED			 82	// Mount fullness
#define EF_MOUNTKILL			 83	// Number of monsters killed by the mount?

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
#define EF_NOTRADE				 111
