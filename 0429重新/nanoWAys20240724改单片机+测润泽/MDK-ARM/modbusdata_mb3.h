#ifndef __MODBUSDATA_MB3_H
#define __MODBUSDATA_MB3_H
#include "Modbus.h"
//#include "stm32f4xx_hal.h"
//#include "FreeRTOS.h"
//#include "cmsis_os.h"
//#include "instrumentdef.h"
//#include "stdint.h"
#include "main.h"
#define MB3_HOLDING_START   100
#define MB3_HOLDING_NREGS   8
#define MB3_INPUT_START     200
#define MB3_INPUT_NREGS     202
#pragma pack(push, 1)

//typedef enum{
//	START_APP = 0x1,
//	START_BOOT = 0x02,
//}MPU_START_MODE;
/**************************¶¨Òå±£³Ö¼Ä´æÆ÷Î»ÒÆÁ¿*************************************/
//typedef enum{							//

//	HOST_CMD = 0,
//	COMM_BAUD = 19,
//	LOCAL_ADDR = 20,	
//	
//}HOLD_BUFF_CFG;

/***************************±£³Ö¼Ä´æÆ÷¶¨Òå£¨¿É¶Á¿ÉĞ´£©*****************************************/
typedef struct {	
		uint16_t						blank;							//¿Õ°×´ıÌî³ä1
		uint16_t						blank2;							//¿Õ°×´ıÌî³ä2
		uint16_t						blank3;							//¿Õ°×´ıÌî³ä3
		uint16_t						blank4;							//¿Õ°×´ıÌî³ä4
		uint16_t						blank5;							//¿Õ°×´ıÌî³ä5
		uint16_t						blank6;							//¿Õ°×´ıÌî³ä6
		uint16_t						blank7;							//¿Õ°×´ıÌî³ä7
		uint16_t						blank8;							//¿Õ°×´ıÌî³ä8
		uint16_t						blank9;							//¿Õ°×´ıÌî³ä9
		uint16_t						blank10;						//¿Õ°×´ıÌî³ä10
		uint16_t						blank11;						//¿Õ°×´ıÌî³ä11
		uint16_t						blank12;						//¿Õ°×´ıÌî³ä12
		uint16_t						blank13;						//¿Õ°×´ıÌî³ä13
		uint16_t						blank14;						//¿Õ°×´ıÌî³ä14
		uint16_t						blank15;						//¿Õ°×´ıÌî³ä15
		uint16_t						blank16;						//¿Õ°×´ıÌî³ä16
		uint16_t						blank17;						//¿Õ°×´ıÌî³ä17
		uint16_t						blank18;						//¿Õ°×´ıÌî³ä18
		uint16_t						blank19;						//¿Õ°×´ıÌî³ä19
		uint16_t						blank20;						//¿Õ°×´ıÌî³ä20
		uint16_t						blank21;						//¿Õ°×´ıÌî³ä21
		uint16_t						blank22;						//¿Õ°×´ıÌî³ä22
		uint16_t						blank23;						//¿Õ°×´ıÌî³ä23
		uint16_t						blank24;						//¿Õ°×´ıÌî³ä24
		uint16_t						blank25;						//¿Õ°×´ıÌî³ä25
		uint16_t						blank26;						//¿Õ°×´ıÌî³ä26
		uint16_t						blank27;						//¿Õ°×´ıÌî³ä27
		uint16_t						blank28;						//¿Õ°×´ıÌî³ä28
		uint16_t						blank29;						//¿Õ°×´ıÌî³ä29
		uint16_t						blank30;						//¿Õ°×´ıÌî³ä30
		uint16_t						blank31;						//¿Õ°×´ıÌî³ä31
		uint16_t						blank32;						//¿Õ°×´ıÌî³ä32
		uint16_t						blank33;						//¿Õ°×´ıÌî³ä33
		uint16_t						blank34;						//¿Õ°×´ıÌî³ä34
		uint16_t						blank35;						//¿Õ°×´ıÌî³ä35
		uint16_t						blank36;						//¿Õ°×´ıÌî³ä36
		uint16_t						blank37;						//¿Õ°×´ıÌî³ä37
		uint16_t						blank38;						//¿Õ°×´ıÌî³ä38
		uint16_t						blank39;						//¿Õ°×´ıÌî³ä39
		uint16_t						blank40;						//¿Õ°×´ıÌî³ä40
		uint16_t						blank41;						//¿Õ°×´ıÌî³ä41
		uint16_t						blank42;						//¿Õ°×´ıÌî³ä42
		uint16_t						blank43;						//¿Õ°×´ıÌî³ä43
		uint16_t						blank44;						//¿Õ°×´ıÌî³ä44
		uint16_t						blank45;						//¿Õ°×´ıÌî³ä45
		uint16_t						blank46;						//¿Õ°×´ıÌî³ä46
		uint16_t						blank47;						//¿Õ°×´ıÌî³ä47
		uint16_t						blank48;						//¿Õ°×´ıÌî³ä48
		uint16_t						blank49;						//¿Õ°×´ıÌî³ä49
		uint16_t						blank50;						//¿Õ°×´ıÌî³ä50
		uint16_t						blank51;						//¿Õ°×´ıÌî³ä51
		uint16_t						blank52;						//¿Õ°×´ıÌî³ä52
		uint16_t						blank53;						//¿Õ°×´ıÌî³ä53
		uint16_t						blank54;						//¿Õ°×´ıÌî³ä54
		uint16_t						blank55;						//¿Õ°×´ıÌî³ä55
		uint16_t						blank56;						//¿Õ°×´ıÌî³ä56
		uint16_t						blank57;						//¿Õ°×´ıÌî³ä57
		uint16_t						blank58;						//¿Õ°×´ıÌî³ä58
		uint16_t						blank59;						//¿Õ°×´ıÌî³ä59
		uint16_t						blank60;						//¿Õ°×´ıÌî³ä60
		uint16_t						blank61;						//¿Õ°×´ıÌî³ä61
		uint16_t						blank62;						//¿Õ°×´ıÌî³ä62
		uint16_t						blank63;						//¿Õ°×´ıÌî³ä63
		uint16_t						blank64;						//¿Õ°×´ıÌî³ä64
		uint16_t						blank65;						//¿Õ°×´ıÌî³ä65
		uint16_t						blank66;						//¿Õ°×´ıÌî³ä66
		uint16_t						blank67;						//¿Õ°×´ıÌî³ä67
		uint16_t						blank68;						//¿Õ°×´ıÌî³ä68
		uint16_t						blank69;						//¿Õ°×´ıÌî³ä69
		uint16_t						blank70;						//¿Õ°×´ıÌî³ä70
		uint16_t						blank71;						//¿Õ°×´ıÌî³ä71
		uint16_t						blank72;						//¿Õ°×´ıÌî³ä72
		uint16_t						blank73;						//¿Õ°×´ıÌî³ä73
		uint16_t						blank74;						//¿Õ°×´ıÌî³ä74
		uint16_t						blank75;						//¿Õ°×´ıÌî³ä75
		uint16_t						blank76;						//¿Õ°×´ıÌî³ä76
		uint16_t						blank77;						//¿Õ°×´ıÌî³ä77
		uint16_t						blank78;						//¿Õ°×´ıÌî³ä78
		uint16_t						blank79;						//¿Õ°×´ıÌî³ä79
		uint16_t						blank80;						//¿Õ°×´ıÌî³ä80
		uint16_t						blank81;						//¿Õ°×´ıÌî³ä81
		uint16_t						blank82;						//¿Õ°×´ıÌî³ä82
		uint16_t						blank83;						//¿Õ°×´ıÌî³ä83
		uint16_t						blank84;						//¿Õ°×´ıÌî³ä84
		uint16_t						blank85;						//¿Õ°×´ıÌî³ä85
		uint16_t						blank86;						//¿Õ°×´ıÌî³ä86
		uint16_t						blank87;						//¿Õ°×´ıÌî³ä87
		uint16_t						blank88;						//¿Õ°×´ıÌî³ä88
		uint16_t						blank89;						//¿Õ°×´ıÌî³ä89
		uint16_t						blank90;						//¿Õ°×´ıÌî³ä90
		uint16_t						blank91;						//¿Õ°×´ıÌî³ä91
		uint16_t						blank92;						//¿Õ°×´ıÌî³ä92
		uint16_t						blank93;						//¿Õ°×´ıÌî³ä93
		uint16_t						blank94;						//¿Õ°×´ıÌî³ä94
		uint16_t						blank95;						//¿Õ°×´ıÌî³ä95
		uint16_t						blank96;						//¿Õ°×´ıÌî³ä96
		uint16_t						blank97;						//¿Õ°×´ıÌî³ä97
		uint16_t						blank98;						//¿Õ°×´ıÌî³ä98
		uint16_t						blank99;						//¿Õ°×´ıÌî³ä99
		/******************* ·´¿ØÃüÁî ********************/
		uint16_t						RecriminateYear;						//·´¿ØĞ£×¼Ê±¼äÄê100
		uint16_t						RecriminateMonth;						//·´¿ØĞ£×¼Ê±¼äÔÂ101
		uint16_t						RecriminateDay;						//·´¿ØĞ£×¼Ê±¼äÈÕ102
		uint16_t						RecriminateHours;						//·´¿ØĞ£×¼Ê±¼äÊ±103
		uint16_t						RecriminateMin;						//·´¿ØĞ£×¼Ê±¼ä·Ö104
		uint16_t						RecriminateSec;						//·´¿ØĞ£×¼Ê±¼äÃë105
		uint16_t						RecriminateCMD;						//·´¿ØÃüÁî106 Æô¶¯²âÁ¿£¨0£©¡¢Ğ£×¼£¨1£©¡¢ÁãµãĞ£×¼£¨2£©¡¢Á¿³ÌĞ£×¼£¨3£©¡¢±êÒººË²é£¨4£©¡¢¿ÉÀ©Õ¹
	
}MODBUS3_HOLDING_REG_t;




/*************************ÊäÈë¼Ä´æÆ÷¶¨Òå£¨´ıÀ©Õ¹£©£¨Ö»¶Á 04£©****************************************/
typedef struct {
		uint16_t						blank;							//¿Õ°×´ıÌî³ä1
		uint16_t						blank2;							//¿Õ°×´ıÌî³ä2
		uint16_t						blank3;							//¿Õ°×´ıÌî³ä3
		uint16_t						blank4;							//¿Õ°×´ıÌî³ä4
		uint16_t						blank5;							//¿Õ°×´ıÌî³ä5
		uint16_t						blank6;							//¿Õ°×´ıÌî³ä6
		uint16_t						blank7;							//¿Õ°×´ıÌî³ä7
		uint16_t						blank8;							//¿Õ°×´ıÌî³ä8
		uint16_t						blank9;							//¿Õ°×´ıÌî³ä9
		uint16_t						blank10;						//¿Õ°×´ıÌî³ä10
		uint16_t						blank11;						//¿Õ°×´ıÌî³ä11
		uint16_t						blank12;						//¿Õ°×´ıÌî³ä12
		uint16_t						blank13;						//¿Õ°×´ıÌî³ä13
		uint16_t						blank14;						//¿Õ°×´ıÌî³ä14
		uint16_t						blank15;						//¿Õ°×´ıÌî³ä15
		uint16_t						blank16;						//¿Õ°×´ıÌî³ä16
		uint16_t						blank17;						//¿Õ°×´ıÌî³ä17
		uint16_t						blank18;						//¿Õ°×´ıÌî³ä18
		uint16_t						blank19;						//¿Õ°×´ıÌî³ä19
		uint16_t						blank20;						//¿Õ°×´ıÌî³ä20
		uint16_t						blank21;						//¿Õ°×´ıÌî³ä21
		uint16_t						blank22;						//¿Õ°×´ıÌî³ä22
		uint16_t						blank23;						//¿Õ°×´ıÌî³ä23
		uint16_t						blank24;						//¿Õ°×´ıÌî³ä24
		uint16_t						blank25;						//¿Õ°×´ıÌî³ä25
		uint16_t						blank26;						//¿Õ°×´ıÌî³ä26
		uint16_t						blank27;						//¿Õ°×´ıÌî³ä27
		uint16_t						blank28;						//¿Õ°×´ıÌî³ä28
		uint16_t						blank29;						//¿Õ°×´ıÌî³ä29
		uint16_t						blank30;						//¿Õ°×´ıÌî³ä30
		uint16_t						blank31;						//¿Õ°×´ıÌî³ä31
		uint16_t						blank32;						//¿Õ°×´ıÌî³ä32
		uint16_t						blank33;						//¿Õ°×´ıÌî³ä33
		uint16_t						blank34;						//¿Õ°×´ıÌî³ä34
		uint16_t						blank35;						//¿Õ°×´ıÌî³ä35
		uint16_t						blank36;						//¿Õ°×´ıÌî³ä36
		uint16_t						blank37;						//¿Õ°×´ıÌî³ä37
		uint16_t						blank38;						//¿Õ°×´ıÌî³ä38
		uint16_t						blank39;						//¿Õ°×´ıÌî³ä39
		uint16_t						blank40;						//¿Õ°×´ıÌî³ä40
		uint16_t						blank41;						//¿Õ°×´ıÌî³ä41
		uint16_t						blank42;						//¿Õ°×´ıÌî³ä42
		uint16_t						blank43;						//¿Õ°×´ıÌî³ä43
		uint16_t						blank44;						//¿Õ°×´ıÌî³ä44
		uint16_t						blank45;						//¿Õ°×´ıÌî³ä45
		uint16_t						blank46;						//¿Õ°×´ıÌî³ä46
		uint16_t						blank47;						//¿Õ°×´ıÌî³ä47
		uint16_t						blank48;						//¿Õ°×´ıÌî³ä48
		uint16_t						blank49;						//¿Õ°×´ıÌî³ä49
		uint16_t						blank50;						//¿Õ°×´ıÌî³ä50
		uint16_t						blank51;						//¿Õ°×´ıÌî³ä51
		uint16_t						blank52;						//¿Õ°×´ıÌî³ä52
		uint16_t						blank53;						//¿Õ°×´ıÌî³ä53
		uint16_t						blank54;						//¿Õ°×´ıÌî³ä54
		uint16_t						blank55;						//¿Õ°×´ıÌî³ä55
		uint16_t						blank56;						//¿Õ°×´ıÌî³ä56
		uint16_t						blank57;						//¿Õ°×´ıÌî³ä57
		uint16_t						blank58;						//¿Õ°×´ıÌî³ä58
		uint16_t						blank59;						//¿Õ°×´ıÌî³ä59
		uint16_t						blank60;						//¿Õ°×´ıÌî³ä60
		uint16_t						blank61;						//¿Õ°×´ıÌî³ä61
		uint16_t						blank62;						//¿Õ°×´ıÌî³ä62
		uint16_t						blank63;						//¿Õ°×´ıÌî³ä63
		uint16_t						blank64;						//¿Õ°×´ıÌî³ä64
		uint16_t						blank65;						//¿Õ°×´ıÌî³ä65
		uint16_t						blank66;						//¿Õ°×´ıÌî³ä66
		uint16_t						blank67;						//¿Õ°×´ıÌî³ä67
		uint16_t						blank68;						//¿Õ°×´ıÌî³ä68
		uint16_t						blank69;						//¿Õ°×´ıÌî³ä69
		uint16_t						blank70;						//¿Õ°×´ıÌî³ä70
		uint16_t						blank71;						//¿Õ°×´ıÌî³ä71
		uint16_t						blank72;						//¿Õ°×´ıÌî³ä72
		uint16_t						blank73;						//¿Õ°×´ıÌî³ä73
		uint16_t						blank74;						//¿Õ°×´ıÌî³ä74
		uint16_t						blank75;						//¿Õ°×´ıÌî³ä75
		uint16_t						blank76;						//¿Õ°×´ıÌî³ä76
		uint16_t						blank77;						//¿Õ°×´ıÌî³ä77
		uint16_t						blank78;						//¿Õ°×´ıÌî³ä78
		uint16_t						blank79;						//¿Õ°×´ıÌî³ä79
		uint16_t						blank80;						//¿Õ°×´ıÌî³ä80
		uint16_t						blank81;						//¿Õ°×´ıÌî³ä81
		uint16_t						blank82;						//¿Õ°×´ıÌî³ä82
		uint16_t						blank83;						//¿Õ°×´ıÌî³ä83
		uint16_t						blank84;						//¿Õ°×´ıÌî³ä84
		uint16_t						blank85;						//¿Õ°×´ıÌî³ä85
		uint16_t						blank86;						//¿Õ°×´ıÌî³ä86
		uint16_t						blank87;						//¿Õ°×´ıÌî³ä87
		uint16_t						blank88;						//¿Õ°×´ıÌî³ä88
		uint16_t						blank89;						//¿Õ°×´ıÌî³ä89
		uint16_t						blank90;						//¿Õ°×´ıÌî³ä90
		uint16_t						blank91;						//¿Õ°×´ıÌî³ä91
		uint16_t						blank92;						//¿Õ°×´ıÌî³ä92
		uint16_t						blank93;						//¿Õ°×´ıÌî³ä93
		uint16_t						blank94;						//¿Õ°×´ıÌî³ä94
		uint16_t						blank95;						//¿Õ°×´ıÌî³ä95
		uint16_t						blank96;						//¿Õ°×´ıÌî³ä96
		uint16_t						blank97;						//¿Õ°×´ıÌî³ä97
		uint16_t						blank98;						//¿Õ°×´ıÌî³ä98
		uint16_t						blank99;						//¿Õ°×´ıÌî³ä99
		uint16_t						blank100;						//¿Õ°×´ıÌî³ä100
		uint16_t						blank101;						//¿Õ°×´ıÌî³ä101
		uint16_t						blank102;						//¿Õ°×´ıÌî³ä102
		uint16_t						blank103;						//¿Õ°×´ıÌî³ä103
		uint16_t						blank104;						//¿Õ°×´ıÌî³ä104
		uint16_t						blank105;						//¿Õ°×´ıÌî³ä105
		uint16_t						blank106;						//¿Õ°×´ıÌî³ä106
		uint16_t						blank107;						//¿Õ°×´ıÌî³ä107
		uint16_t						blank108;						//¿Õ°×´ıÌî³ä108
		uint16_t						blank109;						//¿Õ°×´ıÌî³ä109
		uint16_t						blank110;						//¿Õ°×´ıÌî³ä110
		uint16_t						blank111;						//¿Õ°×´ıÌî³ä111
		uint16_t						blank112;						//¿Õ°×´ıÌî³ä112
		uint16_t						blank113;						//¿Õ°×´ıÌî³ä113
		uint16_t						blank114;						//¿Õ°×´ıÌî³ä114
		uint16_t						blank115;						//¿Õ°×´ıÌî³ä115
		uint16_t						blank116;						//¿Õ°×´ıÌî³ä116
		uint16_t						blank117;						//¿Õ°×´ıÌî³ä117
		uint16_t						blank118;						//¿Õ°×´ıÌî³ä118
		uint16_t						blank119;						//¿Õ°×´ıÌî³ä119
		uint16_t						blank120;						//¿Õ°×´ıÌî³ä120
		uint16_t						blank121;						//¿Õ°×´ıÌî³ä121
		uint16_t						blank122;						//¿Õ°×´ıÌî³ä122
		uint16_t						blank123;						//¿Õ°×´ıÌî³ä123
		uint16_t						blank124;						//¿Õ°×´ıÌî³ä124
		uint16_t						blank125;						//¿Õ°×´ıÌî³ä125
		uint16_t						blank126;						//¿Õ°×´ıÌî³ä126
		uint16_t						blank127;						//¿Õ°×´ıÌî³ä127
		uint16_t						blank128;						//¿Õ°×´ıÌî³ä128
		uint16_t						blank129;						//¿Õ°×´ıÌî³ä129
		uint16_t						blank130;						//¿Õ°×´ıÌî³ä130
		uint16_t						blank131;						//¿Õ°×´ıÌî³ä131
		uint16_t						blank132;						//¿Õ°×´ıÌî³ä132
		uint16_t						blank133;						//¿Õ°×´ıÌî³ä133
		uint16_t						blank134;						//¿Õ°×´ıÌî³ä134
		uint16_t						blank135;						//¿Õ°×´ıÌî³ä135
		uint16_t						blank136;						//¿Õ°×´ıÌî³ä136
		uint16_t						blank137;						//¿Õ°×´ıÌî³ä137
		uint16_t						blank138;						//¿Õ°×´ıÌî³ä138
		uint16_t						blank139;						//¿Õ°×´ıÌî³ä139
		uint16_t						blank140;						//¿Õ°×´ıÌî³ä140
		uint16_t						blank141;						//¿Õ°×´ıÌî³ä141
		uint16_t						blank142;						//¿Õ°×´ıÌî³ä142
		uint16_t						blank143;						//¿Õ°×´ıÌî³ä143
		uint16_t						blank144;						//¿Õ°×´ıÌî³ä144
		uint16_t						blank145;						//¿Õ°×´ıÌî³ä145
		uint16_t						blank146;						//¿Õ°×´ıÌî³ä146
		uint16_t						blank147;						//¿Õ°×´ıÌî³ä147
		uint16_t						blank148;						//¿Õ°×´ıÌî³ä148
		uint16_t						blank149;						//¿Õ°×´ıÌî³ä149
		uint16_t						blank150;						//¿Õ°×´ıÌî³ä150
		uint16_t						blank151;						//¿Õ°×´ıÌî³ä151
		uint16_t						blank152;						//¿Õ°×´ıÌî³ä152
		uint16_t						blank153;						//¿Õ°×´ıÌî³ä153
		uint16_t						blank154;						//¿Õ°×´ıÌî³ä154
		uint16_t						blank155;						//¿Õ°×´ıÌî³ä155
		uint16_t						blank156;						//¿Õ°×´ıÌî³ä156
		uint16_t						blank157;						//¿Õ°×´ıÌî³ä157
		uint16_t						blank158;						//¿Õ°×´ıÌî³ä158
		uint16_t						blank159;						//¿Õ°×´ıÌî³ä159
		uint16_t						blank160;						//¿Õ°×´ıÌî³ä160
		uint16_t						blank161;						//¿Õ°×´ıÌî³ä161
		uint16_t						blank162;						//¿Õ°×´ıÌî³ä162
		uint16_t						blank163;						//¿Õ°×´ıÌî³ä163
		uint16_t						blank164;						//¿Õ°×´ıÌî³ä164
		uint16_t						blank165;						//¿Õ°×´ıÌî³ä165
		uint16_t						blank166;						//¿Õ°×´ıÌî³ä166
		uint16_t						blank167;						//¿Õ°×´ıÌî³ä167
		uint16_t						blank168;						//¿Õ°×´ıÌî³ä168
		uint16_t						blank169;						//¿Õ°×´ıÌî³ä169
		uint16_t						blank170;						//¿Õ°×´ıÌî³ä170
		uint16_t						blank171;						//¿Õ°×´ıÌî³ä171
		uint16_t						blank172;						//¿Õ°×´ıÌî³ä172
		uint16_t						blank173;						//¿Õ°×´ıÌî³ä173
		uint16_t						blank174;						//¿Õ°×´ıÌî³ä174
		uint16_t						blank175;						//¿Õ°×´ıÌî³ä175
		uint16_t						blank176;						//¿Õ°×´ıÌî³ä176
		uint16_t						blank177;						//¿Õ°×´ıÌî³ä177
		uint16_t						blank178;						//¿Õ°×´ıÌî³ä178
		uint16_t						blank179;						//¿Õ°×´ıÌî³ä179
		uint16_t						blank180;						//¿Õ°×´ıÌî³ä180
		uint16_t						blank181;						//¿Õ°×´ıÌî³ä181
		uint16_t						blank182;						//¿Õ°×´ıÌî³ä182
		uint16_t						blank183;						//¿Õ°×´ıÌî³ä183
		uint16_t						blank184;						//¿Õ°×´ıÌî³ä184
		uint16_t						blank185;						//¿Õ°×´ıÌî³ä185
		uint16_t						blank186;						//¿Õ°×´ıÌî³ä186
		uint16_t						blank187;						//¿Õ°×´ıÌî³ä187
		uint16_t						blank188;						//¿Õ°×´ıÌî³ä188
		uint16_t						blank189;						//¿Õ°×´ıÌî³ä189
		uint16_t						blank190;						//¿Õ°×´ıÌî³ä190
		uint16_t						blank191;						//¿Õ°×´ıÌî³ä191
		uint16_t						blank192;						//¿Õ°×´ıÌî³ä192
		uint16_t						blank193;						//¿Õ°×´ıÌî³ä193
		uint16_t						blank194;						//¿Õ°×´ıÌî³ä194
		uint16_t						blank195;						//¿Õ°×´ıÌî³ä195
		uint16_t						blank196;						//¿Õ°×´ıÌî³ä196
		uint16_t						blank197;						//¿Õ°×´ıÌî³ä197
		uint16_t						blank198;						//¿Õ°×´ıÌî³ä198
		uint16_t						blank199;						//¿Õ°×´ıÌî³ä199
		
/********************   ÖÜÆÚĞÔ¼à²âÊı¾İ£¨²ÉÑùÊı¾İ£© *************************/
		uint16_t						DeviceType;					//Éè±¸ÀàĞÍ200
		uint16_t						SamplingYear;						//²ÉÑùÊ±¼äÄê201
		uint16_t						SamplingMonth;						//²ÉÑùÊ±¼äÔÂ202
		uint16_t						SamplingDay;						//²ÉÑùÊ±¼äÈÕ203
		uint16_t						SamplingHours;						//²ÉÑùÊ±¼äÊ±204
		uint16_t						SamplingMin;						//²ÉÑùÊ±¼ä·Ö205
		uint16_t						SamplingSec;						//²ÉÑùÊ±¼äÃë206
		uint16_t						CompletedYear;						//¼ì²âÍê³ÉÊ±¼äÄê207
		uint16_t						CompletedMonth;						//¼ì²âÍê³ÉÊ±¼äÔÂ208
		uint16_t						CompletedDay;						//¼ì²âÍê³ÉÊ±¼äÈÕ209
		uint16_t						CompletedHours;						//¼ì²âÍê³ÉÊ±¼äÊ±210
		uint16_t						CompletedMin;						//¼ì²âÍê³ÉÊ±¼ä·Ö211
		uint16_t						CompletedSec;						//¼ì²âÍê³ÉÊ±¼äÃë212
		float								TestData;								//213,214 ¼ì²âÊµÊ±Öµmg/L
		float								TestAborban;							//215,216 ¼ì²âÎü¹â¶Èmg/L	
		uint16_t						TestSampleFlag;					//¼ì²âÑùÆ·±ê¼Ç217	×Ô¶¯²âÁ¿Ë®Ñù(0)¡¢×Ô¶¯²âÁãÑù(1)¡¢×Ô¶¯²âÁ¿³ÌĞ£×¼Ñù(2)¡¢×Ô¶¯²â±êÒººË²éÑù(3)¡¢ÊÖ¶¯²âÁ¿Ë®Ñù(4)¡¢ÊÖ¶¯²âÁãÑù(5)¡¢ÊÖ¶¯²âÁ¿³ÌĞ£×¼Ñù(6)¡¢ÊÖ¶¯²â±êÒººË²éÑù(7)¡¢¿ÉÀ©Õ¹	
		uint16_t						TestResultFlag;						//218  ¼ì²â½á¹û±ê¼Ç	Õı³££¨0£©¡¢Ë®ÖÊ×Ô¶¯·ÖÎöÒÇ×öÑù¹ÊÕÏ£¨1£©
		uint16_t						ExpandMultiple_factors; 		//30219-30249-------------¶àÒò×ÓÍØÕ¹±¸ÓÃ
		uint16_t						blank220;								//¿Õ°×´ıÌî³ä220
		uint16_t						blank221;								//¿Õ°×´ıÌî³ä221
		uint16_t						blank222;								//¿Õ°×´ıÌî³ä222
		uint16_t						blank223;								//¿Õ°×´ıÌî³ä223
		uint16_t						blank224;								//¿Õ°×´ıÌî³ä224
		uint16_t						blank225;								//¿Õ°×´ıÌî³ä225
		uint16_t						blank226;								//¿Õ°×´ıÌî³ä226
		uint16_t						blank227;								//¿Õ°×´ıÌî³ä227
		uint16_t						blank228;								//¿Õ°×´ıÌî³ä228
		uint16_t						blank229;								//¿Õ°×´ıÌî³ä229
		uint16_t						blank230;								//¿Õ°×´ıÌî³ä230
		uint16_t						blank231;								//¿Õ°×´ıÌî³ä231
		uint16_t						blank232;								//¿Õ°×´ıÌî³ä232
		uint16_t						blank233;								//¿Õ°×´ıÌî³ä233
		uint16_t						blank234;								//¿Õ°×´ıÌî³ä234
		uint16_t						blank235;								//¿Õ°×´ıÌî³ä235
		uint16_t						blank236;								//¿Õ°×´ıÌî³ä236
		uint16_t						blank237;								//¿Õ°×´ıÌî³ä237
		uint16_t						blank238;								//¿Õ°×´ıÌî³ä238
		uint16_t						blank239;								//¿Õ°×´ıÌî³ä239
		uint16_t						blank240;								//¿Õ°×´ıÌî³ä240
		uint16_t						blank241;								//¿Õ°×´ıÌî³ä241
		uint16_t						blank242;								//¿Õ°×´ıÌî³ä242
		uint16_t						blank243;								//¿Õ°×´ıÌî³ä243
		uint16_t						blank244;								//¿Õ°×´ıÌî³ä244
		uint16_t						blank245;								//¿Õ°×´ıÌî³ä245
		uint16_t						blank246;								//¿Õ°×´ıÌî³ä246
		uint16_t						blank247;								//¿Õ°×´ıÌî³ä247
		uint16_t						blank248;								//¿Õ°×´ıÌî³ä248
		uint16_t						blank249;								//¿Õ°×´ıÌî³ä249---------------------¶àÒò×ÓÍØÕ¹±¸ÓÃ
/***********************   ÔËĞĞ×´Ì¬ ****************************/		
		uint16_t 						MainState;         			//30250´ı»ú£¨0£©¡¢×Ô¶¯²âÁ¿£¨1£©¡¢×Ô¶¯Ğ£×¼£¨2£©¡¢×Ô¶¯±êÒººË²é£¨3£©¡¢Î¬»¤£¨4£©¡¢¹ÊÕÏ£¨5£©¡¢¿ÉÀ©Õ¹
		uint16_t						SubState;								//30251×Ó×´Ì¬
		uint16_t						blank252;						//¿Õ°×´ıÌî³ä252
		uint16_t						blank253;						//¿Õ°×´ıÌî³ä253
		uint16_t						blank254;						//¿Õ°×´ıÌî³ä254
		uint16_t						blank255;						//¿Õ°×´ıÌî³ä255
		uint16_t						blank256;						//¿Õ°×´ıÌî³ä256
		uint16_t						blank257;						//¿Õ°×´ıÌî³ä257
		uint16_t						blank258;						//¿Õ°×´ıÌî³ä258
		uint16_t						blank259;						//¿Õ°×´ıÌî³ä259
		uint16_t						blank260;						//¿Õ°×´ıÌî³ä260
		uint16_t						blank261;						//¿Õ°×´ıÌî³ä261
		uint16_t						blank262;						//¿Õ°×´ıÌî³ä262
		uint16_t						blank263;						//¿Õ°×´ıÌî³ä263
		uint16_t						blank264;						//¿Õ°×´ıÌî³ä264
		uint16_t						blank265;						//¿Õ°×´ıÌî³ä265
		uint16_t						blank266;						//¿Õ°×´ıÌî³ä266
		uint16_t						blank267;						//¿Õ°×´ıÌî³ä267
		uint16_t						blank268;						//¿Õ°×´ıÌî³ä268
		uint16_t						blank269;						//¿Õ°×´ıÌî³ä269
		uint16_t						blank270;						//¿Õ°×´ıÌî³ä270
		uint16_t						blank271;						//¿Õ°×´ıÌî³ä271
		uint16_t						blank272;						//¿Õ°×´ıÌî³ä272
		uint16_t						blank273;						//¿Õ°×´ıÌî³ä273
		uint16_t						blank274;						//¿Õ°×´ıÌî³ä274
		uint16_t						blank275;						//¿Õ°×´ıÌî³ä275
		uint16_t						blank276;						//¿Õ°×´ıÌî³ä276
		uint16_t						blank277;						//¿Õ°×´ıÌî³ä277
		uint16_t						blank278;						//¿Õ°×´ıÌî³ä278
		uint16_t						blank279;						//¿Õ°×´ıÌî³ä279
		uint16_t						blank280;						//¿Õ°×´ıÌî³ä280
		uint16_t						blank281;						//¿Õ°×´ıÌî³ä281
		uint16_t						blank282;						//¿Õ°×´ıÌî³ä282
		uint16_t						blank283;						//¿Õ°×´ıÌî³ä283
		uint16_t						blank284;						//¿Õ°×´ıÌî³ä284
		uint16_t						blank285;						//¿Õ°×´ıÌî³ä285
		uint16_t						blank286;						//¿Õ°×´ıÌî³ä286
		uint16_t						blank287;						//¿Õ°×´ıÌî³ä287
		uint16_t						blank288;						//¿Õ°×´ıÌî³ä288
		uint16_t						blank289;						//¿Õ°×´ıÌî³ä289
		uint16_t						blank290;						//¿Õ°×´ıÌî³ä290
		uint16_t						blank291;						//¿Õ°×´ıÌî³ä291
		uint16_t						blank292;						//¿Õ°×´ıÌî³ä292
		uint16_t						blank293;						//¿Õ°×´ıÌî³ä293
		uint16_t						blank294;						//¿Õ°×´ıÌî³ä294
		uint16_t						blank295;						//¿Õ°×´ıÌî³ä295
		uint16_t						blank296;						//¿Õ°×´ıÌî³ä296
		uint16_t						blank297;						//¿Õ°×´ıÌî³ä297
		uint16_t						blank298;						//¿Õ°×´ıÌî³ä298
		uint16_t						blank299;						//¿Õ°×´ıÌî³ä299
/***********************   ¹¤×÷²ÎÊı  ****************************/			
		float								MeasurementCycle;						//30300-30301 ²âÁ¿ÖÜÆÚmin
		float								RangeUplimit;								//30302-30303  Á¿³ÌÉÏÏŞ
		float								RangeLowlimit;							//30304-30305 Á¿³ÌÏÂÏŞ
		float								k;													//30306-30307 Ğ±ÂÊ 
		float								b;													//30308-30309  ½Ø¾à 
		float								DetectionLimit;							//30310-30311   ¼ì³öÏŞ 
		uint16_t						ZerocalibrationYear;				//ÁãµãĞ£×¼¿ªÊ¼Ê±¼äÄê30312
		uint16_t						ZerocalibrationMonth;				//ÁãµãĞ£×¼¿ªÊ¼Ê±¼äÔÂ30313
		uint16_t						ZerocalibrationDay;					//ÁãµãĞ£×¼¿ªÊ¼Ê±¼äÈÕ30314
		uint16_t						ZerocalibrationHours;				//ÁãµãĞ£×¼¿ªÊ¼Ê±¼äÊ±30315
		uint16_t						ZerocalibrationMin;					//ÁãµãĞ£×¼¿ªÊ¼Ê±¼ä·Ö30316
		uint16_t						ZerocalibrationSec;					//ÁãµãĞ£×¼¿ªÊ¼Ê±¼äÃë30317
		float								ZerocalibConcentration;				//30318-30319  ÁãµãĞ£×¼ÉèÖÃÅ¨¶È
		float								ZerocalibValue;								//30320-30321  ÁãµãĞ£×¼²âÁ¿Å¨¶È
		float								ZerocalibAborban;							//30322-30323  ÁãµãĞ£×¼²âÁ¿ĞÅºÅÖµ
		float								ZerocalibAllowDeviation;			//30324-30325  ÁãµãĞ£×¼ÔÊĞíÆ«²îµ¥Î»£º%
		float								ZerocalibActualDeviation;			//30326-30327  ÁãµãĞ£×¼Êµ¼ÊÆ«²îµ¥Î»£º% 
		float								ZerocalibB;										//30328-30329  µ±Ç°¹¤×÷Á¿³Ì¶ÔÓ¦µÄĞ£×¼ÇúÏß½Ø¾à 
		uint16_t						RangecalibrationYear;					//Á¿³ÌĞ£×¼¿ªÊ¼Ê±¼äÄê30330
		uint16_t						RangecalibrationMonth;				//Á¿³ÌĞ£×¼¿ªÊ¼Ê±¼äÔÂ30331
		uint16_t						RangecalibrationDay;					//Á¿³ÌĞ£×¼¿ªÊ¼Ê±¼äÈÕ30332
		uint16_t						RangecalibrationHours;				//Á¿³ÌĞ£×¼¿ªÊ¼Ê±¼äÊ±30333
		uint16_t						RangecalibrationMin;					//Á¿³ÌĞ£×¼¿ªÊ¼Ê±¼ä·Ö30334
		uint16_t						RangecalibrationSec;					//Á¿³ÌĞ£×¼¿ªÊ¼Ê±¼äÃë30335
		float								RangecalibConcentration;				//30336-30337  Á¿³ÌĞ£×¼ÉèÖÃÅ¨¶È
		float								RangecalibValue;								//30338-30339  Á¿³ÌĞ£×¼²âÁ¿Å¨¶È
		float								RangecalibAborban;							//30340-30341  Á¿³ÌĞ£×¼²âÁ¿ĞÅºÅÖµ
		float								RangecalibAllowDeviation;				//30342-30343  Á¿³ÌĞ£×¼ÔÊĞíÆ«²îµ¥Î»£º%
		float								RangecalibActualDeviation;			//30344-30345  Á¿³ÌĞ£×¼Êµ¼ÊÆ«²îµ¥Î»£º% 
		float								RangecalibB;										//30346-30347  µ±Ç°¹¤×÷Á¿³Ì¶ÔÓ¦µÄĞ£×¼ÇúÏß½Ø¾à 
		uint16_t						MidcalibrationYear;						//ÖĞ¼äĞ£×¼¿ªÊ¼Ê±¼äÄê30348
		uint16_t						MidcalibrationMonth;					//ÖĞ¼äĞ£×¼¿ªÊ¼Ê±¼äÔÂ30349
		uint16_t						MidcalibrationDay;						//ÖĞ¼äĞ£×¼¿ªÊ¼Ê±¼äÈÕ30350
		uint16_t						MidcalibrationHours;					//ÖĞ¼äĞ£×¼¿ªÊ¼Ê±¼äÊ±30351
		uint16_t						MidcalibrationMin;						//ÖĞ¼äĞ£×¼¿ªÊ¼Ê±¼ä·Ö30352
		uint16_t						MidcalibrationSec;						//ÖĞ¼äĞ£×¼¿ªÊ¼Ê±¼äÃë30353
		float								MidcalibConcentration;				//30354-30355  ÖĞ¼äĞ£×¼ÉèÖÃÅ¨¶È
		float								MidcalibValue;								//30356-30357  ÖĞ¼äĞ£×¼²âÁ¿Å¨¶È
		float								MidcalibAborban;							//30358-30359  ÖĞ¼äĞ£×¼²âÁ¿ĞÅºÅÖµ
		float								MidcalibAllowDeviation;				//30360-30361  ÖĞ¼äĞ£×¼ÔÊĞíÆ«²îµ¥Î»£º%
		float								MidcalibActualDeviation;			//30362-30363  ÖĞ¼äĞ£×¼Êµ¼ÊÆ«²îµ¥Î»£º% 
		uint16_t						StandardcheckYear;						//±êÒººË²é¿ªÊ¼Ê±¼äÄê30364
		uint16_t						StandardcheckMonth;						//±êÒººË²é¿ªÊ¼Ê±¼äÔÂ30365
		uint16_t						StandardcheckDay;							//±êÒººË²é¿ªÊ¼Ê±¼äÈÕ30366
		uint16_t						StandardcheckHours;						//±êÒººË²é¿ªÊ¼Ê±¼äÊ±30367
		uint16_t						StandardcheckMin;							//±êÒººË²é¿ªÊ¼Ê±¼ä·Ö30368
		uint16_t						StandardcheckSec;							//±êÒººË²é¿ªÊ¼Ê±¼äÃë30369
		float								StandardcheckConcentra;				//30370-30371  ±êÒººË²éÉèÖÃÅ¨¶È
		float								StandardcheckValue;						//30372-30373  ±êÒººË²é²âÁ¿Å¨¶È
		float								StandardcheckAborban;					//30374-30375  ±êÒººË²é²âÁ¿ĞÅºÅÖµ
		float								StandardcheckAllowDev;				//30376-30377  ±êÒººË²éÔÊĞíÆ«²îµ¥Î»£º%
		float								StandardcheckActualDev;				//30378-30379  ±êÒººË²éÊµ¼ÊÆ«²îµ¥Î»£º%
		float								DissolutionTemper;						//30380-30381  Ïû½âÎÂ¶ÈÉèÖÃÖµ µ¥Î»£ºmin
		float								DissolutionTime;							//30382-30383  Ïû½âÊ±¼äÉèÖÃÖµ
		float								ChromogenicTemper;						//30384-30385  ÏÔÉ«ÎÂ¶ÈÉèÖÃÖµ µ¥Î»£ºmin
		float								ChromogenicTime;							//30386-30387  ÏÔÉ«Ê±¼äÉèÖÃÖµ		
		float								TOCombustionTemper;						//30388-30389  È¼ÉÕÎÂ¶ÈÉèÖÃÖµ µ¥Î»£º¡æ TOCÉè±¸×¨ÓÃ
		float								TOCondenserTemper;						//30390-30391  ÀäÄıÆ÷ÎÂ¶È 		 µ¥Î»£º¡æ TOCÉè±¸×¨ÓÃ
		float								TOCarriergasPressure;					//30392-30393  ÔØÆøÑ¹Á¦  	   µ¥Î»£ºkPa TOCÉè±¸×¨ÓÃ
		float								TOCarriergasRate;							//30394-30395  ÔØÆøÁ÷Á¿			 µ¥Î»£ºkPa TOCÉè±¸×¨ÓÃ		
		
}MODBUS3_INPUT_REG_t;

#pragma pack(pop)
/* Íâ²¿ÉùÃ÷»º³åÇøÓëÖ¸Õë£¨¹©»Øµ÷º¯ÊıºÍÒµÎñÂß¼­Ê¹ÓÃ£© */
extern USHORT *usRegHoldingBuf3;
extern USHORT *usRegInputBuf3;
extern UCHAR  *usCoilBuf3;
extern UCHAR  *usDiscreteInputBuf3;
extern MODBUS3_HOLDING_REG_t *pMb3HoldData;
extern MODBUS3_INPUT_REG_t   *pMb3InputData;

extern volatile uint8_t g_mb_inst;


#endif
