#include "op.h"

//dst OP
const char* dstName(unsigned n, bool skipzero) {
	if (n == 0 && skipzero) return "";
	switch (n) {
		case 0: return "AlwaysTrue";
		case 1: return "ThisBarFolder";
		case 2: return "ThisBarSong";
		case 3: return "ThisBarCourse";
		case 4: return "ThisBarCreateCourse";
		case 5: return "ThisBarPlayable";


		case 10: return "DOUBLEorDOUBLEBATTLE";
		case 11: return "BATTLE";
		case 12: return "BATTLEorDOUBLEBATTLEorBATTLE";
		case 13: return "GBATTLEorBaTTLE";


		case 20: return "NOPANEL";
		case 21: return "PANEL1";
		case 22: return "PANEL2";
		case 23: return "PANEL3";
		case 24: return "PANEL4";
		case 25: return "PANEL5";
		case 26: return "PANEL6";
		case 27: return "PANEL7";
		case 28: return "PANEL8";
		case 29: return "PANEL9";
		case 30: return "BGANOEXTEND";
		case 31: return "BGAEXTEND";
		case 32: return "AUTO_OFF";
		case 33: return "AUTO_ON";
		case 34: return "GHOST_OFF";
		case 35: return "GHOST_A";
		case 36: return "GHOST_B";
		case 37: return "GHOST_C";

		case 38: return "SCOREGRAPH_OFF";
		case 39: return "SCOREGRAPH_ON";
		case 40: return "BGA_OFF";
		case 41: return "BGA_ON";

		case 42: return "1PGROOVEGAUGE";
		case 43: return "1PREDGAUGE";
		case 44: return "2PGROOVEGAUGE";
		case 45: return "2PREDGAUGE";

		case 46: return "DIFFILCULTYFILTER_ON";
		case 47: return "DIFFILCULTYFILTER_OFF";


		case 50: return "OFFLINE";
		case 51: return "ONLINE";
		case 52: return "EXTRA_OFF";
		case 53: return "EXTRA_ON";

		case 54: return "1PAUTOSCRATCH_OFF";
		case 55: return "1PAUTOSCRATCH_ON";
		case 56: return "2PAUTOSCRATCH_OFF";
		case 57: return "2PAUTOSCRATCH_ON";


		case 60: return "NOSAVE";
		case 61: return "SAVE";
		case 62: return "NOCLEAR";
		case 63: return "ASEASYCLEAR";
		case 64: return "ASNORMALCLEAR";
		case 65: return "ASHARDCLEARR";
		case 66: return "ASFULLCOMBO";


		case 70: return "BEGINNER";
		case 71: return "NORMAL";
		case 72: return "HYPER";
		case 73: return "ANOTHER";
		case 74: return "INSANE";
		case 75: return "BEGINNER_OVERLEVEL";
		case 76: return "NORMAL_OVERLEVEL";
		case 77: return "HYPER_OVERLEVEL";
		case 78: return "ANOTHER_OVERLEVEL";
		case 79: return "INSANE_OVERLEVEL";

		case 80: return "LOADING";
		case 81: return "LOADED";

		case 82: return "REPLAY_OFF";
		case 83: return "REPLAY_RECORDING";
		case 84: return "REPLAY_PLAYING";


		case 90: return "RESULT_CLEAR";
		case 91: return "RESULT_MISS";


		case 100: return "CLEARED_NOPLAY";
		case 101: return "CLEARED_FAILED";
		case 102: return "CLEARED_EASY";
		case 103: return "CLEARED_CLEAR";
		case 104: return "CLEARED_HARD";
		case 105: return "CLEARED_FULLCOMBO";


		case 110: return "RANK_AAA";
		case 111: return "RANK_AA";
		case 112: return "RANK_A";
		case 113: return "RANK_B";
		case 114: return "RANK_C";
		case 115: return "RANK_D";
		case 116: return "RANK_E";
		case 117: return "RANK_F";

		case 118: return "CLEAROPTION_GROOVE";
		case 119: return "CLEAROPTION_SURVIVAL";
		case 120: return "CLEAROPTION_DEATH";
		case 121: return "CLEAROPTION_EASY";
		case 122: return "CLEAROPTION_PATTACK";
		case 123: return "CLEAROPTION_GATTACK";
		case 124: return "CLEAROPTION_NOTYET124";
		case 125: return "CLEAROPTION_NOTYET125";

		case 126: return "CLEAROPTION_NORANDOM";
		case 127: return "CLEAROPTION_MIRROR";
		case 128: return "CLEAROPTION_RANDOM";
		case 129: return "CLEAROPTION_SRANDOM";
		case 130: return "CLEAROPTION_SCATTER";
		case 131: return "CLEAROPTION_CONVERGE";
		case 132: return "CLEAROPTION_NOTYET132";
		case 133: return "CLEAROPTION_NOTYET133";

		case 134: return "CLEAROPTION_NOHIDSUD";
		case 135: return "CLEAROPTION_HID";
		case 136: return "CLEAROPTION_SUD";
		case 137: return "CLEAROPTION_HIDSUD";
		case 138: return "CLEAROPTION_NOTYET138";
		case 139: return "CLEAROPTION_NOTYET139";
		case 140: return "CLEAROPTION_NOTYET140";
		case 141: return "CLEAROPTION_NOTYET141";

		case 142: return "CLEAROPTION_AUTOSCRATCH";
		case 143: return "CLEAROPTION_EXTRAMODE";
		case 144: return "CLEAROPTION_DBATTLE";
		case 145: return "CLEAROPTION_SPTODP";


		case 150: return "DIFFICULTY_NO";
		case 151: return "DIFFICULTY_EASY";
		case 152: return "DIFFICULTY_NORMAL";
		case 153: return "DIFFICULTY_HYPER";
		case 154: return "DIFFICULTY_ANOTHER";
		case 155: return "DIFFICULTY_INSANE";


		case 160: return "BMS_7KEYS";
		case 161: return "BMS_5KEYS";
		case 162: return "BMS_14KEYS";
		case 163: return "BMS_10KEYS";
		case 164: return "BMS_9KEYS";
		case 165: return "PLAY_7KEYS";
		case 166: return "PLAY_5KEYS";
		case 167: return "PLAY_14KEYS";
		case 168: return "PLAY_10KEYS";
		case 169: return "PLAY_9KEYS";

		case 170: return "BGAEXIST_NO";
		case 171: return "BGAEXIST_YES";
		case 172: return "LNEXIST_NO";
		case 173: return "LNEXIST_YES";
		case 174: return "TEXTEXIST_NO";
		case 175: return "TEXTEXIST_YES";
		case 176: return "SOFLANEXIST_NO";
		case 177: return "SOFLANEXIST_YES";
		case 178: return "RANDOMEXIST_NO";
		case 179: return "RANDOMEXIST_YES";

		case 180: return "JUDGE_VHARD";
		case 181: return "JUDGE_HARD";
		case 182: return "JUDGE_NORMAL";
		case 183: return "JUDGE_EASY";


		case 185: return "OVERLEVEL_NO";
		case 186: return "OVERLEVEL_YES";


		case 190: return "STAGEFILEEXIST_NO";
		case 191: return "STAGEFILEEXIST_YES";
		case 192: return "BANNEREXIST_NO";
		case 193: return "BANNEREXIST_YES";
		case 194: return "BACKBMPEXIST_NO";
		case 195: return "BACKBMPEXIST_YES";
		case 196: return "REPLAYEXIST_NO";
		case 197: return "REPLAYEXIST_YES";


		case 200: return "1PRANK_AAA";
		case 201: return "1PRANK_AA";
		case 202: return "1PRANK_A";
		case 203: return "1PRANK_B";
		case 204: return "1PRANK_C";
		case 205: return "1PRANK_D";
		case 206: return "1PRANK_E";
		case 207: return "1PRANK_F";


		case 210: return "2PRANK_AAA";
		case 211: return "2PRANK_AA";
		case 212: return "2PRANK_A";
		case 213: return "2PRANK_B";
		case 214: return "2PRANK_C";
		case 215: return "2PRANK_D";
		case 216: return "2PRANK_E";
		case 217: return "2PRANK_F";


		case 220: return "AAAconfirm";
		case 221: return "AAconfirm";
		case 222: return "Aconfirm";
		case 223: return "Bconfirm";
		case 224: return "Cconfirm";
		case 225: return "Dconfirm";
		case 226: return "Econfirm";
		case 227: return "Fconfirm";


		case 230: return "1Pgauge_0-10";
		case 231: return "1Pgauge10-19";
		case 232: return "1Pgauge20-29";
		case 233: return "1Pgauge30-39";
		case 234: return "1Pgauge40-49";
		case 235: return "1Pgauge50-59";
		case 236: return "1Pgauge60-69";
		case 237: return "1Pgauge70-79";
		case 238: return "1Pgauge80-89";
		case 239: return "1Pgauge90-99";
		case 240: return "1Pgauge100";

		case 241: return "1P_PERFECT";
		case 242: return "1P_GREAT";
		case 243: return "1P_GOOD";
		case 244: return "1P_BAD";
		case 245: return "1P_POOR";
		case 246: return "1P_MPOOR";

		case 247: return "1P_POORBGA_NOTTIME";
		case 248: return "1P_POORBGA_TIME";


		case 250: return "2Pgauge0-10";
		case 251: return "2Pgauge10-19";
		case 252: return "2Pgauge20-29";
		case 253: return "2Pgauge30-39";
		case 254: return "2Pgauge40-49";
		case 255: return "2Pgauge50-59";
		case 256: return "2Pgauge60-69";
		case 257: return "2Pgauge70-79";
		case 258: return "2Pgauge80-89";
		case 259: return "2Pgauge90-99";
		case 260: return "2Pgauge100";

		case 261: return "2P_PERFECT";
		case 262: return "2P_GREAT";
		case 263: return "2P_GOOD";
		case 264: return "2P_BAD";
		case 265: return "2P_POOR";
		case 266: return "2P_MPOOR";

		case 267: return "2P_POORBGA_NOTTIME";
		case 268: return "2P_POORBGA_TIME";


		case 270: return "1P_SUD+Setting";
		case 271: return "2P_SUD+Setting";


		case 280: return "CourseStage1";
		case 281: return "CourseStage2";
		case 282: return "CourseStage3";
		case 283: return "CourseStage4";
		case 289: return "CourseStageFINAL";

		case 290: return "Course";
		case 291: return "Nonstop";
		case 292: return "Expert";
		case 293: return "Dan";


		case 300: return "1P_AAA";
		case 301: return "1P_AA";
		case 302: return "1P_A";
		case 303: return "1P_B";
		case 304: return "1P_C";
		case 305: return "1P_D";
		case 306: return "1P_E";
		case 307: return "1P_F";
		case 308: return "1P_0";


		case 310: return "2P_AAA";
		case 311: return "2P_AA";
		case 312: return "2P_A";
		case 313: return "2P_B";
		case 314: return "2P_C";
		case 315: return "2P_D";
		case 316: return "2P_E";
		case 317: return "2P_F";
		case 318: return "2P_0";


		case 320: return "old_AAA";
		case 321: return "old_AA";
		case 322: return "old_A";
		case 323: return "old_B";
		case 324: return "old_C";
		case 325: return "old_D";
		case 326: return "old_E";
		case 327: return "old_F";


		case 330: return "ScoreUpdate";
		case 331: return "MAXCOMBOUpdate";
		case 332: return "BPUpdate";
		case 333: return "TrialUpdate";
		case 334: return "IRrankUpdate";
		case 335: return "ScoreRankUpdate";


		case 340: return "new_AAA";
		case 341: return "new_AA";
		case 342: return "new_A";
		case 343: return "new_B";
		case 344: return "new_C";
		case 345: return "new_D";
		case 346: return "new_E";
		case 347: return "new_F";


		case 350: return "ResultFlip_OFF";
		case 351: return "ResultFlip_ON";

		case 352: return "1PWIN_2PLOSE";
		case 353: return "1PLOSE_2PWIN";
		case 354: return "DRAW";


		case 400: return "7/14KEYS";
		case 401: return "9KEYS";
		case 402: return "5/10KEYS";


		case 500: return "Beginner_Exist_NO";
		case 501: return "normal_Exist_NO";
		case 502: return "hyper_Exist_NO";
		case 503: return "another_Exist_NO";
		case 504: return "insane_Exist_NO";

		case 505: return "beginner_Exist_YES";
		case 506: return "normal_Exist_YES";
		case 507: return "hyper_Exist_YES";
		case 508: return "another_Exist_YES";
		case 509: return "insane_Exist_YES";

		case 510: return "beginner_Exist_ONLYONE";
		case 511: return "normal_Exist_ONLYONE";
		case 512: return "hyper_Exist_ONLYONE";
		case 513: return "another_Exist_ONLYONE";
		case 514: return "insane_Exist_ONLYONE";

		case 515: return "beginner_Exist_SEVERAL";
		case 516: return "normal_Exist_SEVERAL";
		case 517: return "hyper_Exist_SEVERAL";
		case 518: return "another_Exist_SEVERAL";
		case 519: return "nsan_Exist_SEVERAL";

		case 520: return "levelbar_beginner_noplay";
		case 521: return "levelbar_beginner_failed";
		case 522: return "levelbar_beginner_easy";
		case 523: return "levelbar_beginner_clear";
		case 524: return "levelbar_beginner_hardclear";
		case 525: return "levelbar_beginner_fullcombo";


		case 530: return "levelbar_normal_noplay";
		case 531: return "levelbar_normal_failed";
		case 532: return "levelbar_normal_easy";
		case 533: return "levelbar_normal_clear";
		case 534: return "levelbar_normal_hardclear";
		case 535: return "levelbar_normal_fullcombo";


		case 540: return "levelbar_hyper_no_play";
		case 541: return "levelbar_hyper_failed";
		case 542: return "levelbar_hyper_easy";
		case 543: return "levelbar_hyper_clear";
		case 544: return "levelbar_hyper_hardclear";
		case 545: return "levelbar_hyper_fullcombo";


		case 550: return "levelbar_another_no_play";
		case 551: return "levelbar_another_failed";
		case 552: return "levelbar_another_easy";
		case 553: return "levelbar_another_clear";
		case 554: return "levelbar_another_hardclear";
		case 555: return "levelbar_another_fullcombo";


		case 560: return "levelbar_insane_no_play";
		case 561: return "levelbar_insane_failed";
		case 562: return "levelbar_insane_easy";
		case 563: return "levelbar_insane_clear";
		case 564: return "levelbar_insane_hardclear";
		case 565: return "levelbar_insane_fullcombo";


		case 571: return "isCourseSelect_YES";
		case 572: return "isCourseSelect_NO";


		case 580: return "stage_morethan_1";
		case 581: return "stage_morethan_2";
		case 582: return "stage_morethan_3";
		case 583: return "stage_morethan_4";
		case 584: return "stage_morethan_5";
		case 585: return "stage_morethan_6";
		case 586: return "stage_morethan_7";
		case 587: return "stage_morethan_8";
		case 588: return "stage_morethan_9";
		case 589: return "stage_morethan_10";

		case 590: return "CourseSelect_stage1";
		case 591: return "CourseSelect_stage2";
		case 592: return "CourseSelect_stage3";
		case 593: return "CourseSelect_stage4";
		case 594: return "CourseSelect_stage5";
		case 595: return "CourseSelect_stage6";
		case 596: return "CourseSelect_stage7";
		case 597: return "CourseSelect_stage8";
		case 598: return "CourseSelect_stage9";
		case 599: return "CourseSelect_stage10";



		case 700: return "stage1_difficultyNone";
		case 701: return "stage1_difficulty1";
		case 702: return "stage1_difficulty2";
		case 703: return "stage1_difficulty3";
		case 704: return "stage1_difficulty4";
		case 705: return "stage1_difficulty5";


		case 710: return "stage2_difficultyNone";
		case 711: return "stage2_difficulty1";
		case 712: return "stage2_difficulty2";
		case 713: return "stage2_difficulty3";
		case 714: return "stage2_difficulty4";
		case 715: return "stage2_difficulty5";


		case 720: return "stage3_difficultyNone";
		case 721: return "stage3_difficulty1";
		case 722: return "stage3_difficulty2";
		case 723: return "stage3_difficulty3";
		case 724: return "stage3_difficulty4";
		case 725: return "stage3_difficulty5";


		case 730: return "stage4_difficultyNone";
		case 731: return "stage4_difficulty1";
		case 732: return "stage4_difficulty2";
		case 733: return "stage4_difficulty3";
		case 734: return "stage4_difficulty4";
		case 735: return "stage4_difficulty5";


		case 740: return "stage5_difficultyNone";
		case 741: return "stage5_difficulty1";
		case 742: return "stage5_difficulty2";
		case 743: return "stage5_difficulty3";
		case 744: return "stage5_difficulty4";
		case 745: return "stage5_difficulty5";


		case 600: return "IR_NOTTARGET";
		case 601: return "IR_LOAD";
		case 602: return "IR_COMPLETE";
		case 603: return "IR_NOPLAYER";
		case 604: return "IR_CONNECTIONFAIL";
		case 605: return "BANNED_SONG";
		case 606: return "IR_UPDATE_WAIT";
		case 607: return "IR_ACCESSING";
		case 608: return "IR_BUSY";


		case 620: return "SHOW_RANK_NO";
		case 621: return "SHOW_RANK_YES";

		case 622: return "GBATTLE_NO";
		case 623: return "GBATTLE_YES";

		case 624: return "COMPARING_SCORE_NO";
		case 625: return "COMPARING_SCORE_YES";


		case 640: return "NOT_PLAYED";
		case 641: return "FAILED";
		case 642: return "EASY_CLEARED";
		case 643: return "NORMAL_CLEARED";
		case 644: return "HARD_CLEARED";
		case 645: return "FULL_COMBO";

		case 650: return "AAA_8/9";
		case 651: return "AA_7/9";
		case 652: return "A_6/9";
		case 653: return "B_5/9";
		case 654: return "C_4/9";
		case 655: return "D_3/9";
		case 656: return "E_2/9";
		case 657: return "F_1/9";

		case 900: return "CUSTOM900";
		case 901: return "CUSTOM901";
		case 902: return "CUSTOM902";
		case 903: return "CUSTOM903";
		case 904: return "CUSTOM904";
		case 905: return "CUSTOM905";
		case 906: return "CUSTOM906";
		case 907: return "CUSTOM907";
		case 908: return "CUSTOM908";
		case 909: return "CUSTOM909";
		case 910: return "CUSTOM910";
		case 911: return "CUSTOM911";
		case 912: return "CUSTOM912";
		case 913: return "CUSTOM913";
		case 914: return "CUSTOM914";
		case 915: return "CUSTOM915";
		case 916: return "CUSTOM916";
		case 917: return "CUSTOM917";
		case 918: return "CUSTOM918";
		case 919: return "CUSTOM919";
		case 920: return "CUSTOM920";
		case 921: return "CUSTOM921";
		case 922: return "CUSTOM922";
		case 923: return "CUSTOM923";
		case 924: return "CUSTOM924";
		case 925: return "CUSTOM925";
		case 926: return "CUSTOM926";
		case 927: return "CUSTOM927";
		case 928: return "CUSTOM928";
		case 929: return "CUSTOM929";
		case 930: return "CUSTOM930";
		case 931: return "CUSTOM931";
		case 932: return "CUSTOM932";
		case 933: return "CUSTOM933";
		case 934: return "CUSTOM934";
		case 935: return "CUSTOM935";
		case 936: return "CUSTOM936";
		case 937: return "CUSTOM937";
		case 938: return "CUSTOM938";
		case 939: return "CUSTOM939";
		case 940: return "CUSTOM940";
		case 941: return "CUSTOM941";
		case 942: return "CUSTOM942";
		case 943: return "CUSTOM943";
		case 944: return "CUSTOM944";
		case 945: return "CUSTOM945";
		case 946: return "CUSTOM946";
		case 947: return "CUSTOM947";
		case 948: return "CUSTOM948";
		case 949: return "CUSTOM949";
		case 950: return "CUSTOM950";
		case 951: return "CUSTOM951";
		case 952: return "CUSTOM952";
		case 953: return "CUSTOM953";
		case 954: return "CUSTOM954";
		case 955: return "CUSTOM955";
		case 956: return "CUSTOM956";
		case 957: return "CUSTOM957";
		case 958: return "CUSTOM958";
		case 959: return "CUSTOM959";
		case 960: return "CUSTOM960";
		case 961: return "CUSTOM961";
		case 962: return "CUSTOM962";
		case 963: return "CUSTOM963";
		case 964: return "CUSTOM964";
		case 965: return "CUSTOM965";
		case 966: return "CUSTOM966";
		case 967: return "CUSTOM967";
		case 968: return "CUSTOM968";
		case 969: return "CUSTOM969";
		case 970: return "CUSTOM970";
		case 971: return "CUSTOM971";
		case 972: return "CUSTOM972";
		case 973: return "CUSTOM973";
		case 974: return "CUSTOM974";
		case 975: return "CUSTOM975";
		case 976: return "CUSTOM976";
		case 977: return "CUSTOM977";
		case 978: return "CUSTOM978";
		case 979: return "CUSTOM979";
		case 980: return "CUSTOM980";
		case 981: return "CUSTOM981";
		case 982: return "CUSTOM982";
		case 983: return "CUSTOM983";
		case 984: return "CUSTOM984";
		case 985: return "CUSTOM985";
		case 986: return "CUSTOM986";
		case 987: return "CUSTOM987";
		case 988: return "CUSTOM988";
		case 989: return "CUSTOM989";
		case 990: return "CUSTOM990";
		case 991: return "CUSTOM991";
		case 992: return "CUSTOM992";
		case 993: return "CUSTOM993";
		case 994: return "CUSTOM994";
		case 995: return "CUSTOM995";
		case 996: return "CUSTOM996";
		case 997: return "CUSTOM997";
		case 998: return "CUSTOM998";

		case 999: return "Alwaysfalse";


		default: return "";
	}
}

const char* buttonName(unsigned n) {
	switch (n) {

	case 1: return "PANEL1";
	case 2: return "PANEL2";
	case 3: return "PANEL3";
	case 4: return "PANEL4";
	case 5: return "PANEL5";
	case 6: return "PANEL6";
	case 7: return "PANEL7";
	case 8: return "PANEL8";
	case 9: return "PANEL9";


	case 10: return "CurrentDifficultyFilter";
	case 11: return "CurrentKeys";
	case 12: return "CurrentSort";

	case 13: return "KeyConfig";
	case 14: return "SkinConfig";

	case 15: return "Play";
	case 16: return "AutoPlay";
	case 17: return "ReadMe";
	case 18: return "TagReset";
	case 19: return "Replay";

	case 20: return "FX0_NAME";
	case 21: return "FX1_NAME";
	case 22: return "FX2_NAME";

	case 23: return "FX0";
	case 24: return "FX1";
	case 25: return "FX2";

	case 26: return "FX0_TARGET";
	case 27: return "FX1_TARGET";
	case 28: return "FX2_TARGET";
	case 29: return "EQ";
	case 30: return "EQ_PRESET";

	case 31: return "VOLUME_CONTROL";
	case 32: return "PITCH";
	case 33: return "PITCH_TYPE";

	case 40: return "GAUGE1P";
	case 41: return "GAUGE2P";

	case 42: return "RANDOM1P";
	case 43: return "RANDOM2P";

	case 44: return "ASSIST1P";
	case 45: return "ASSIST2P";

	case 46: return "shutter";

	case 48: return "reserved48";
	case 49: return "reserved49";

	case 50: return "EFFECT1P";
	case 51: return "EFFECT2P";

	case 52: return "reserved52";
	case 53: return "reserved53";


	case 54: return "DP_FLIP";

	case 55: return "HS_FIX";

	case 56: return "BATTLE";

	case 57: return "HS_1P";
	case 58: return "HS_2P";

	case 70: return "SCOREGRAPH";
	case 71: return "GHOST_POSITION";
	case 72: return "BGA";
	case 73: return "BGA_SIZE";
	case 74: return "JUDGE_TIMING";
	case 75: return "JUDGE_TIMING_AUTO_ADJUST";
	case 76: return "DEFAULT_TARGET_RATE";
	case 77: return "TARGET";

	case 80: return "SCREEN_MODE";
	case 81: return "COLOR_MODE";
	case 82: return "VSYNC";
	case 83: return "SAVE_REPLAY";

	case 90: return "favorite";

	case 91: return "all";
	case 92: return "beginner";
	case 93: return "normal";
	case 94: return "hyper";
	case 95: return "another";
	case 96: return "insane";



	case 101: return "1P_1";
	case 102: return "1P_2";
	case 103: return "1P_3";
	case 104: return "1P_4";
	case 105: return "1P_5";
	case 106: return "1P_6";
	case 107: return "1P_7";
	case 108: return "1P_8";
	case 109: return "1P_9";

	case 110: return "1P_SC_LEFT";
	case 111: return "1P_SC_RIGHT";
	case 112: return "1P_START";
	case 113: return "1P_SELECT";


	case 121: return "2P_1";
	case 122: return "2P_2";
	case 123: return "2P_3";
	case 124: return "2P_4";
	case 125: return "2P_5";
	case 126: return "2P_6";
	case 127: return "2P_7";
	case 128: return "2P_8";
	case 129: return "2P_9";

	case 130: return "2P_SC_LEFT";
	case 131: return "2P_SC_RIGHT";
	case 132: return "2P_START";
	case 133: return "2P_SELECT";

	case 140: return "ChangeKey7";
	case 141: return "ChangeKey9";
	case 142: return "ChangeKey5";
	case 143: return "7to9to5Button";

	case 150: return "KEYCONFIG_SLOT_0";
	case 151: return "KEYCONFIG_SLOT_1";
	case 152: return "KEYCONFIG_SLOT_2";
	case 153: return "KEYCONFIG_SLOT_3";
	case 154: return "KEYCONFIG_SLOT_4";
	case 155: return "KEYCONFIG_SLOT_5";
	case 156: return "KEYCONFIG_SLOT_6";
	case 157: return "KEYCONFIG_SLOT_7";


	case 170: return "SKINSELECT_7KEYS";
	case 171: return "SKINSELECT_5KEYS";
	case 172: return "SKINSELECT_14KEYS";
	case 173: return "SKINSELECT_10KEYS";
	case 174: return "SKINSELECT_9KEYS";
	case 175: return "SKINSELECT_SELECT";
	case 176: return "SKINSELECT_DECIDE";
	case 177: return "SKINSELECT_RESULT";
	case 178: return "SKINSELECT_KEYCONFIG";
	case 179: return "SKINSELECT_SKINSELECT";
	case 180: return "SKINSELECT_SOUNDSET";
	case 181: return "SKINSELECT_THEME";
	case 182: return "SKINSELECT_BATTLE7";
	case 183: return "SKINSELECT_BATTLE5";
	case 184: return "SKINSELECT_BATTLE9";
	case 185: return "SKINSELECT_COURSERESULT";

	case 190: return "SKINSELECT_SKINCHANGEBUTTON";

	case 200: return "Help1";
	case 201: return "Help2";
	case 202: return "Help3";
	case 203: return "Help4";
	case 204: return "Help5";
	case 205: return "Help6";
	case 206: return "Help7";
	case 207: return "Help8";
	case 208: return "Help9";
	case 209: return "Help10";

	case 210: return "IRpageButton";


	case 220: return "SkinCustomizeItem1";
	case 221: return "SkinCustomizeItem2";
	case 222: return "SkinCustomizeItem3";
	case 223: return "SkinCustomizeItem4";
	case 224: return "SkinCustomizeItem5";
	case 225: return "SkinCustomizeItem6";
	case 226: return "SkinCustomizeItem7";
	case 227: return "SkinCustomizeItem8";
	case 228: return "SkinCustomizeItem9";
	case 229: return "SkinCustomizeItem10";

	case 230: return "COURSESELECT_DECIDE";
	case 231: return "COURSESELECT_DECIDE_CANCLE";
	case 232: return "COURSE_MODIFY";
	case 233: return "COURSE_DELETE";

	case 240: return "CourseConnectionstage1-2";
	case 241: return "CourseConnectionstage2-3";
	case 242: return "CourseConnectionstage3-4";
	case 243: return "CourseConnectionstage4-5";
	case 244: return "CourseConnectionstage5-6";
	case 245: return "CourseConnectionstage6-7";
	case 246: return "CourseConnectionstage7-8";
	case 247: return "CourseConnectionstage8-9";
	case 248: return "CourseConnectionstage9-10";
	case 249: return "CourseConnectionstage10-11";

	case 250: return "CourseOption_SOFLAN";
	case 251: return "CourseOption_GAUGE";
	case 252: return "CourseOption_Option";
	case 253: return "CourseOption_IRavailable";

	case 260: return "RandomCourseOption_levelForME";
	case 261: return "RandomCourseOption_levelMax";
	case 262: return "RandomCourseOption_levelMin";
	case 263: return "RandomCourseOption_BPMRange";
	case 264: return "RandomCourseOption_BPMMax";
	case 265: return "RandomCourseOption_BPMMin";
	case 266: return "RandomCourseOption_StageCountChange";

	case 268: return "DefaultCourseConnection";
	case 269: return "DefaultCourseGauge";
	default: return "";
	}
}

const char* sliderName(unsigned n) {
	switch (n) {
	case 1: return "SongSelectPosition";

	case 2: return "HISPEED1P";
	case 3: return "HISPEED2P";

	case 4: return "SHUTTER1P";
	case 5: return "SHUTTER2P";

	case 6: return "SongTime";

	case 7: return "SkinCustomScroll";

	case 8: return "IR_SCROLL_BAR";

	case 10: return "EQ0";
	case 11: return "EQ1";
	case 12: return "EQ2";
	case 13: return "EQ3";
	case 14: return "EQ4";
	case 15: return "EQ5";
	case 16: return "EQ6";

	case 17: return "MASTER_VOLUME";
	case 18: return "KEY_VOLUME";
	case 19: return "BGM_VOLUME";

	case 20: return "MASTER_FX_P1";
	case 21: return "MASTER_FX_P2";
	case 22: return "KEY_FX_P1";
	case 23: return "KEY_FX_P2";
	case 24: return "BGM_FX_P1";
	case 25: return "BGM_FX_P2";

	case 26: return "PITC";
	default: return "";
	}
}

const char* numberName(unsigned n) {
	switch (n){
		case 10: return "HS_1P";
		case 11: return "HS_2P";

		case 12: return "JUDGE_TIMING";
		case 13: return "DEFAULT_TARGET_RATE";

		case 14: return "SUD_1P";
		case 15: return "SUD_2P";


		case 20: return "fps";
		case 21: return "year";
		case 22: return "month";
		case 23: return "day";
		case 24: return "hour";
		case 25: return "minute";
		case 26: return "second";


		case 30: return "TOTAL_PLAY_COUNT";
		case 31: return "TOTAL_CLEAR_COUNT";
		case 32: return "TOTAL_FAIL_COUNT";

		case 33: return "TOTAL_PERFECT";
		case 34: return "TOTAL_GREAT";
		case 35: return "TOTAL_GOOD";
		case 36: return "TOTAL_BAD";
		case 37: return "TOTAL_POOR";

		case 38: return "RUNNING_COMBO";
		case 39: return "RUNNING_MAXCOMBO";

		case 40: return "TRIAL_LEVEL";
		case 41: return "TRIAL_LEVEL_Old";


		case 45: return "beginnerSongLevel";
		case 46: return "normalSongLevel";
		case 47: return "hyperSongLevel";
		case 48: return "anotherSongLevel";
		case 49: return "insaneSongLevel";

		case 50: return "EQ0";
		case 51: return "EQ1";
		case 52: return "EQ2";
		case 53: return "EQ3";
		case 54: return "EQ4";
		case 55: return "EQ5";
		case 56: return "EQ6";

		case 57: return "MASTER_VOLUME";
		case 58: return "KEY_VOLUME";
		case 59: return "BGM_VOLUME";

		case 60: return "FX0_P1";
		case 61: return "FX0_P2";
		case 62: return "FX1_P1";
		case 63: return "FX1_P2";
		case 64: return "FX2_P1";
		case 65: return "FX2_P2";

		case 66: return "PITCH";

		case 70: return "score";
		case 71: return "exscore";
		case 72: return "exscoreMAX";
		case 73: return "rate";
		case 74: return "totalnotes";
		case 75: return "maxcombo";
		case 76: return "min_bp";
		case 77: return "playcount";
		case 78: return "clear";
		case 79: return "fail";

		case 80: return "perfect";
		case 81: return "great";
		case 82: return "good";
		case 83: return "bad";
		case 84: return "poor";
		case 85: return "perfect_percent";
		case 86: return "great_percent";
		case 87: return "good_percent";
		case 88: return "bad_percent";
		case 89: return "poor_percent";

		case 90: return "bpm_max";
		case 91: return "bpm_min";

		case 92: return "IR_rank";
		case 93: return "IR_totalplayer";
		case 94: return "IR_clearrate";

		case 95: return "IR_diffRival";


		case 100: return "score";
		case 101: return "exscore";
		case 102: return "rate";
		case 103: return "rate_point";
		case 104: return "nowcombo";
		case 105: return "maxcombo";
		case 106: return "totalnotes";
		case 107: return "groovegauge";
		case 108: return "diffexscore2p";
		case 110: return "perfect";
		case 111: return "great";
		case 112: return "good";
		case 113: return "bad";
		case 114: return "poor";
		case 115: return "total_rate";
		case 116: return "total_rate_point";


		case 120: return "opponent_score";
		case 121: return "opponent_exscore";
		case 122: return "opponent_rate";
		case 123: return "opponent_rate_point";
		case 124: return "opponent_nowcombo";
		case 125: return "opponent_maxcombo";
		case 126: return "opponent_totalnotes";
		case 127: return "opponent_groovegauge";
		case 128: return "opponent_diffexscore2p";
		case 130: return "opponent_perfect";
		case 131: return "opponent_great";
		case 132: return "opponent_good";
		case 133: return "opponent_bad";
		case 134: return "opponent_poor";
		case 135: return "opponent_total_rate";
		case 136: return "opponent_total_rate_point";


		case 150: return "Current_Highscore";
		case 151: return "TargetScore";
		case 152: return "Diff_Highscore_1P";
		case 153: return "Diff_Target_1P";
		case 154: return "DiffNextRank";
		case 155: return "HighScore_rate";
		case 156: return "HighScore_ratepoint";
		case 157: return "Target_rate";
		case 158: return "Target_ratepoint";


		case 160: return "bpm";
		case 161: return "minute";
		case 162: return "second";
		case 163: return "left_minute";
		case 164: return "left_second";
		case 165: return "loadPercentage";


		case 170: return "OldEXSCORE";
		case 171: return "NewEXSCORE";
		case 172: return "DiffEXSCORE";

		case 173: return "OldMAXCOMBO";
		case 174: return "NewMAXCOMBO";
		case 175: return "DiffMAXCOMBO";

		case 176: return "OldBP";
		case 177: return "NewBP";
		case 178: return "diffBP";

		case 179: return "IR_rank";
		case 180: return "IR_totalplayer";
		case 181: return "IR_clearrate";
		case 182: return "old_IRrank";

		case 183: return "old_rate";
		case 184: return "old_ratepoint";


		case 200: return "IR_TOTALPLAYER";
		case 201: return "IR_TOTALPLAYCOUNT!LastHitOffset_P1";

		case 210: return "FAILED_PLAYER!LastHitFS_P1";
		case 211: return "FAILED_RATE!LastHitFS_P2";
		case 212: return "EASY_PLAYER!Fast_P1";
		case 213: return "EASY_RATE!LastHitOffset_P2";
		case 214: return "CLEAR_PLAYER!Slow_P1";
		case 215: return "CLEAR_RATE";
		case 216: return "HARD_PLAYER!CB";
		case 217: return "HARD_RATE!NoteTotal";
		case 218: return "FULLCOMBO_PLAYER!NoteCurrent";
		case 219: return "FULLCOMBO_RATE";

		case 220: return "remainIRautoUpdateTime";

		case 250: return "CourseLevelStage1";
		case 251: return "CourseLevelStage2";
		case 252: return "CourseLevelStage3";
		case 253: return "CourseLevelStage4";
		case 254: return "CourseLevelStage5";


		case 270: return "rival_score";
		case 271: return "rival_exscore";
		case 272: return "rival_exscoreMax";
		case 273: return "rival_rate";
		case 274: return "rival_totalnotes";
		case 275: return "rival_maxcombo";
		case 276: return "rival_min_bp";
		case 277: return "rival_playcount";
		case 278: return "rival_clear";
		case 279: return "rival_fail";

		case 280: return "rival_perfect";
		case 281: return "rival_great";
		case 282: return "rival_good";
		case 283: return "rival_bad";
		case 284: return "rival_poor";
		case 285: return "rival_perfect_percent";
		case 286: return "rival_great_percent";
		case 287: return "rival_good_percent";
		case 288: return "rival_bad_percent";
		case 289: return "rival_poor_percent";

		case 290: return "rival_bpm_max";
		case 291: return "rival_bpm_min";

		case 292: return "rival_IR_rank";
		case 293: return "rival_IR_totalplayer";
		case 294: return "rival_IR_clearrat";
		default: return "";
	}
}

const char* textName(unsigned n) {
	switch (n) {
		case 1: return "TargetName";
		case 2: return "MyName";

		case 10: return "THISSONG_TITLE";
		case 11: return "THISSONG_SUBTITLE";
		case 12: return "THISSONG_FULLTITLE";
		case 13: return "THISSONG_GENRE";
		case 14: return "THISSONG_ARTIST";
		case 15: return "THISSONG_SUBARTIST";
		case 16: return "THISSONG_SEARCHTAG";
		case 17: return "THISSONG_PLAYLEVEL";
		case 18: return "THISSONG_DIFFICULTYCOUNT";


		case 20: return "TAGEDIT_TITLE/IRMESSAGE";
		case 21: return "TAGEDIT_SUBTITLE";
		case 22: return "TAGEDIT_FULLTITLE";
		case 23: return "TAGEDIT_GENRE";
		case 24: return "TAGEDIT_ARTIST";
		case 25: return "TAGEDIT_SUBARTIST";
		case 26: return "TAGEDIT_SEARCHTAG";
		case 27: return "TAGEDIT_PLAYLEVEL";
		case 28: return "TAGEDIT_DIFFICULTYCOUNT";
		case 29: return "TAGEDIT_EXLEVEL";
		case 30: return "SEARCH_COMMAND_JUKEBOXNAME";


		case 40: return "Slot0";
		case 41: return "Slot1";
		case 42: return "Slot2";
		case 43: return "Slot3";
		case 44: return "Slot4";
		case 45: return "Slot5";
		case 46: return "Slot6";
		case 47: return "Slot7";


		case 50: return "SKIN_NAME";
		case 51: return "SKIN_ARTIST";


		case 60: return "PLAYMODE_KEY";
		case 61: return "PLAYMODE_SORT";
		case 62: return "PLAYMODE_DIFFICULTY";


		case 63: return "RANDOM1P";
		case 64: return "RANDOM2P";
		case 65: return "GAUGE1P";
		case 66: return "GAUGEP";
		case 67: return "ASSIST1P";
		case 68: return "ASSIST2P";
		case 69: return "BATTLE";
		case 70: return "FLIP";
		case 71: return "SCOREGRAPH";
		case 72: return "GHOST";
		case 73: return "SHUTTER";
		case 74: return "SCROLLTYPE";

		case 75: return "BGA_SIZE";
		case 76: return "BGA_DRAW";

		case 77: return "ScreenColors";
		case 78: return "vsync";
		case 79: return "ScreenMode";

		case 80: return "AUTOJUDGE";
		case 81: return "REPLAYOPTION";

		case 82: return "TRIAL1";
		case 83: return "TRIAL2";

		case 84: return "EFFECT1P";
		case 85: return "EFFECT2P";



		case 100: return "SkinCustomize_Category1";
		case 101: return "SkinCustomize_Category2";
		case 102: return "SkinCustomize_Category3";
		case 103: return "SkinCustomize_Category4";
		case 104: return "SkinCustomize_Category5";
		case 105: return "SkinCustomize_Category6";
		case 106: return "SkinCustomize_Category7";
		case 107: return "SkinCustomize_Category8";
		case 108: return "SkinCustomize_Category9";
		case 109: return "SkinCustomize_Category10";

		case 110: return "SkinCustomize_Item1";
		case 111: return "SkinCustomize_Item2";
		case 112: return "SkinCustomize_Item3";
		case 113: return "SkinCustomize_Item4";
		case 114: return "SkinCustomize_Item5";
		case 115: return "SkinCustomize_Item6";
		case 116: return "SkinCustomize_Item7";
		case 117: return "SkinCustomize_Item8";
		case 118: return "SkinCustomize_Item9";
		case 119: return "SkinCustomize_Item10";

		case 120: return "Ranking_PlayerName1";
		case 121: return "Ranking_PlayerName2";
		case 122: return "Ranking_PlayerName3";
		case 123: return "Ranking_PlayerName4";
		case 124: return "Ranking_PlayerName5";
		case 125: return "Ranking_PlayerName6";
		case 126: return "Ranking_PlayerName7";
		case 127: return "Ranking_PlayerName8";
		case 128: return "Ranking_PlayerName9";
		case 129: return "Ranking_PlayerName10";

		case 130: return "ClearText_Temp1";
		case 131: return "ClearText_Temp2";


		case 150: return "CourseTitle_1stage";
		case 151: return "CourseTitle_2stage";
		case 152: return "CourseTitle_3stage";
		case 153: return "CourseTitle_4stage";
		case 154: return "CourseTitle_5stage";
		case 155: return "CourseTitle_6stage";
		case 156: return "CourseTitle_7stage";
		case 157: return "CourseTitle_8stage";
		case 158: return "CourseTitle_9stage";
		case 159: return "CourseTitle_10stage";

		case 160: return "CourseSubTitle_1stage";
		case 161: return "CourseSubTitle_2stage";
		case 162: return "CourseSubTitle_3stage";
		case 163: return "CourseSubTitle_4stage";
		case 164: return "CourseSubTitle_5stage";
		case 165: return "CourseSubTitle_6stage";
		case 166: return "CourseSubTitle_7stage";
		case 167: return "CourseSubTitle_8stage";
		case 168: return "CourseSubTitle_9stage";
		case 169: return "CourseSubTitle_10stage";

		case 170: return "CourseEditAndName";

		case 171: return "CourseConnectionstage1-2";
		case 172: return "CourseConnectionstage2-3";
		case 173: return "CourseConnectionstage3-4";
		case 174: return "CourseConnectionstage4-5";
		case 175: return "CourseConnectionstage5-6";
		case 176: return "CourseConnectionstage6-7";
		case 177: return "CourseConnectionstage7-8";
		case 178: return "CourseConnectionstage8-9";
		case 179: return "CourseConnectionstage9-10";


		case 180: return "CourseOption_SOFLAN";
		case 181: return "CourseOption_GAUGETYPE";
		case 182: return "CourseOption_OPTIONBAN";
		case 183: return "CourseOption_IR";


		case 190: return "RandomCourseOptionLevelForMe";
		case 191: return "RandomCourseOptionLevelMax";
		case 192: return "RandomCourseOptionLevelMin";
		case 193: return "RandomCourseOptionBPMrange";
		case 194: return "RandomCourseOptionBPMmax";
		case 195: return "RandomCourseOptionBPMmin";
		case 196: return "RandomCourseOptionStages";


		case 198: return "AllCourseOptionDefaultConnection";
		case 199: return "AllCourseOptionDefaultGauge";
		default: return "";
	}
}

const char* timerName(unsigned n, bool skipzero) {
	if (n == 0 && skipzero) return "";
	switch (n) {
		case 0: return "MainTimer";
		case 1: return "StartInputDone";
		case 2: return "FadeOut";
		case 3: return "Close";

		case 4: return "InputTextDone";


		case 10: return "SongListMove";
		case 11: return "SongChange";
		case 12: return "SELECTUP";
		case 13: return "SELECTDONW";
		case 14: return "SELECTSTOP";

		case 15: return "README_Start";
		case 16: return "README_End";

		case 17: return "CourseSelect_Start";
		case 18: return "CourseSelect_End";


		case 21: return "Panel1_Open";
		case 22: return "Panel2_Open";
		case 23: return "Panel3_Open";
		case 24: return "Panel4_Open";
		case 25: return "Panel5_Open";
		case 26: return "Panel6_Open";
		case 27: return "Panel7_Open";
		case 28: return "Panel8_Open";
		case 29: return "Panel9_Open";


		case 31: return "Panel1_Close";
		case 32: return "Panel2_Close";
		case 33: return "Panel3_Close";
		case 34: return "Panel4_Close";
		case 35: return "Panel5_Close";
		case 36: return "Panel6_Close";
		case 37: return "Panel7_Close";
		case 38: return "Panel8_Close";
		case 39: return "Panel9_Close";

		case 40: return "READY";
		case 41: return "PLAYSTART";

		case 42: return "GAUGE_UP_1P";
		case 43: return "GAUGE_UP_2P";

		case 44: return "GAUGE_FULL_1P";
		case 45: return "GAUGE_FULL_2P";

		case 46: return "JUDGETIMER_1P";
		case 47: return "JUDGETIMER_2P";

		case 48: return "FULLCOMBO_1P";
		case 49: return "FULLCOMBO_2P";

		case 50: return "1P_MINE_SC";
		case 51: return "1P_MINE_1";
		case 52: return "1P_MINE_2";
		case 53: return "1P_MINE_3";
		case 54: return "1P_MINE_4";
		case 55: return "1P_MINE_5";
		case 56: return "1P_MINE_6";
		case 57: return "1P_MINE_7";
		case 58: return "1P_MINE_8";
		case 59: return "1P_MINE_9";

		case 60: return "2P_MINE_SC";
		case 61: return "2P_MINE_1";
		case 62: return "2P_MINE_2";
		case 63: return "2P_MINE_3";
		case 64: return "2P_MINE_4";
		case 65: return "2P_MINE_5";
		case 66: return "2P_MINE_6";
		case 67: return "2P_MINE_7";
		case 68: return "2P_MINE_8";
		case 69: return "2P_MINE_9";

		case 70: return "1PLN_EFFECT_SC";
		case 71: return "1PLN_EFFECT_1";
		case 72: return "1PLN_EFFECT_2";
		case 73: return "1PLN_EFFECT_3";
		case 74: return "1PLN_EFFECT_4";
		case 75: return "1PLN_EFFECT_5";
		case 76: return "1PLN_EFFECT_6";
		case 77: return "1PLN_EFFECT_7";
		case 78: return "1PLN_EFFECT_8";
		case 79: return "1PLN_EFFECT_9";

		case 80: return "2PLN_EFFECT_SC";
		case 81: return "2PLN_EFFECT_1";
		case 82: return "2PLN_EFFECT_2";
		case 83: return "2PLN_EFFECT_3";
		case 84: return "2PLN_EFFECT_4";
		case 85: return "2PLN_EFFECT_5";
		case 86: return "2PLN_EFFECT_6";
		case 87: return "2PLN_EFFECT_7";
		case 88: return "2PLN_EFFECT_8";
		case 89: return "2PLN_EFFECT_9";


		case 100: return "1PKeyON_SC";
		case 101: return "1PKeyON_1";
		case 102: return "1PKeyON_2";
		case 103: return "1PKeyON_3";
		case 104: return "1PKeyON_4";
		case 105: return "1PKeyON_5";
		case 106: return "1PKeyON_6";
		case 107: return "1PKeyON_7";
		case 108: return "1PKeyON_8";
		case 109: return "1PKeyON_9";

		case 110: return "2PKeyON_SC";
		case 111: return "2PKeyON_1";
		case 112: return "2PKeyON_2";
		case 113: return "2PKeyON_3";
		case 114: return "2PKeyON_4";
		case 115: return "2PKeyON_5";
		case 116: return "2PKeyON_6";
		case 117: return "2PKeyON_7";
		case 118: return "2PKeyON_8";
		case 119: return "2PKeyON_9";

		case 120: return "1PKeyOFF_SC";
		case 121: return "1PKeyOFF_1";
		case 122: return "1PKeyOFF_2";
		case 123: return "1PKeyOFF_3";
		case 124: return "1PKeyOFF_4";
		case 125: return "1PKeyOFF_5";
		case 126: return "1PKeyOFF_6";
		case 127: return "1PKeyOFF_7";
		case 128: return "1PKeyOFF_8";
		case 129: return "1PKeyOFF_9";

		case 130: return "2PKeyOFF_SC";
		case 131: return "2PKeyOFF_1";
		case 132: return "2PKeyOFF_2";
		case 133: return "2PKeyOFF_3";
		case 134: return "2PKeyOFF_4";
		case 135: return "2PKeyOFF_5";
		case 136: return "2PKeyOFF_6";
		case 137: return "2PKeyOFF_7";
		case 138: return "2PKeyOFF_8";
		case 139: return "2PKeyOFF_9";

		case 140: return "Rhythm";


		case 143: return "LastNote1P";
		case 144: return "LastNote2P";


		case 150: return "ResultGraph_Start";
		case 151: return "ResultGraph_End";
		case 152: return "HighScoreUpdate";


		case 170: return "DBLoad_Start";
		case 171: return "DBLoad_End";

		case 172: return "IR_Connect_Start";
		case 173: return "IR_Connect_Success";
		case 174: return "IR_Connect_Fail";


		case 180: return "CourseEditorStart";
		case 181: return "CourseEditorEn";
		default: return "";
	}
}

const char* bargraphName(unsigned n) {
	switch (n){
	case 1: return "songProgress";
	case 2: return "songLoaded";

	case 3: return "level_bar";

	case 5: return "samefolder_beginner_level";
	case 6: return "samefolder_normal_level";
	case 7: return "samefolder_hyper_level";
	case 8: return "samefolder_anotherl_level";
	case 9: return "samefolder_insane_level";

	case 10: return "1P_EXSCORE_now";
	case 11: return "1P_EXSCORE_predict";
	case 12: return "1P_HIGHSCORE_now";
	case 13: return "1P_HIGHSCORE_predict";
	case 14: return "2P_target_EXSCORE_now";
	case 15: return "2P_target_EXSCORE_predict";

	case 20: return "1P_PG_totalnotes";
	case 21: return "1P_GR_totalnotes";
	case 22: return "1P_GD_totalnotes";
	case 23: return "1P_BD_totalnotes";
	case 24: return "1P_PR_totalnotes";
	case 25: return "1P_MAXCOMBO_totalnotes";
	case 26: return "1P_SCORE_200000";
	case 27: return "1P_EXSCORE_totalnotes__2";


	case 30: return "2P_target_PG_totalnotes";
	case 31: return "2P_target_GR_totalnotes";
	case 32: return "2P_target_GD_totalnotes";
	case 33: return "2P_target_BD_totalnotes";
	case 34: return "2P_target_PR_totalnotes";
	case 35: return "2P_target_MAXCOMBO_totalnotes";
	case 36: return "2P_target_SCORE_200000";
	case 37: return "2P_target_exscore_totalnotes__2";

	case 40: return "HIGHSCORE_PG_totalnotes";
	case 41: return "HIGHSCORE_GR_totalnotes";
	case 42: return "HIGHSCORE_GD_totalnotes";
	case 43: return "HIGHSCORE_BD_totalnotes";
	case 44: return "HIGHSCORE_PR_totalnotes";
	case 45: return "HIGHSCORE_MAXCOMBO_totalnotes";
	case 46: return "HIGHSCORE_SCORE_200000";
	case 47: return "HIGHSCORE_EXSCORE_totalnotes__";

	default: return "";
	}
}

const char* barindexName(unsigned n) {
	switch (n) {
	case 0: return "Song";
	case 1: return "Folder";
	case 2: return "CustomFolder";
	case 3: return "NewFolder";
	case 4: return "RivalFolder";
	case 5: return "SongInRivalmode";
	case 6: return "CourseFolder";
	case 7: return "CreateFolder";
	case 8: return "Course";
	case 9: return "RandomCourse";
	default: return "";
	}
}