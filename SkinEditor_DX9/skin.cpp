#include "skin.h"
#include "../LR2/LR2.h"
#include "../LR2/Scenes.h"
#include <cmath>

int LR2SEInit(game* g) {
	InitBmsList(&g->sSelect);
	InitObjectString(&g->txtStruct);
	InitGameplay(&g->gameplay, &g->config.play);


	DeleteGraph(g->skstruct.GrHandle[GrH_BackBMP]); 
	DeleteGraph(g->skstruct.GrHandle[GrH_Preview]);
	DeleteGraph(g->skstruct.GrHandle[GrH_Stage]);
	DeleteGraph(g->skstruct.GrHandle[GrH_Banner]);
	g->skstruct.GrHandle[GrH_BackBMP] = LoadGraph("LR2files/Config/white.bmp");
	g->skstruct.GrHandle[GrH_Preview] = LoadGraph("LR2files/Config/white.bmp");
	g->skstruct.GrHandle[GrH_Stage] = LoadGraph("LR2files/Config/title.bmp");
	g->skstruct.GrHandle[GrH_Banner] = LoadGraph("LR2files/Config/white.bmp");

	//LoadGraph("LR2files/Config/loading.bmp", 0);


	InitializeCriticalSection(&g->gameplay.criticalSection);
	InitializeCriticalSection(&g->criticalSection);

	return 0;
}

int LR2SEDrawLoop(game* g, int gHandle, int sizeX, int sizeY) {
	SetDrawScreen(DX_SCREEN_BACK);

	for (int i = 0; i < g->skstruct.image.srcSize; i++) {

		if (g->skstruct.image.dst[i].dstCount && GetOptionFlag_dst(g, g->skstruct.image.dst[i].op1)
			&& GetOptionFlag_dst(g, g->skstruct.image.dst[i].op2) && GetOptionFlag_dst(g, g->skstruct.image.dst[i].op3)) {

			if ((g->skstruct.adjust.dark_type == 1 && g->skstruct.image.dst[i].timer)
				|| (g->skstruct.adjust.dark_type != 1 && (g->skstruct.adjust.dark_type != 2
					|| g->skstruct.image.dst[i].timer == 2
					|| g->skstruct.image.dst[i].timer == 3
					|| g->skstruct.image.dst[i].timer == 48
					|| g->skstruct.image.dst[i].timer == 49
					|| (50 <= g->skstruct.image.dst[i].timer && g->skstruct.image.dst[i].timer < 70)
					|| g->skstruct.image.dst[i].timer == 140))
				) {
				int objx = 0, objy = 0;
				if ((g->skstruct.adjust.note_1p_x || g->skstruct.adjust.note_1p_y || g->skstruct.adjust.note_2p_x || g->skstruct.adjust.note_2p_y) && g->procSelecter == 4) {
					int t = g->skstruct.image.dst[i].timer;
					//refactored
					if ((50 <= t && t < 60) || (70 <= t && t < 80) || t == 48) {
						objx = g->skstruct.adjust.note_1p_x;
						objy = g->skstruct.adjust.note_1p_y;
					}
					else if ((60 <= t && t < 70) || (80 <= t && t < 90) || t == 49) {
						objx = g->skstruct.adjust.note_2p_x;
						objy = g->skstruct.adjust.note_2p_y;
					}
					else if ((100 <= t && t < 110) || (120 <= t && t < 130)) {
						objx = g->skstruct.adjust.note_1p_x;
						objy = g->skstruct.adjust.note_1p_y;
						if (-100.0 < g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].h && g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].h < 100.0
							&& -100.0 < g->skstruct.image.dst[i].draw[0].h && g->skstruct.image.dst[i].draw[0].h < 100.0) {

							objx = 0;
							objy = 0;
						}
					}
					else if ((110 <= t && t < 120) || (130 <= t && t < 140)) {
						objx = g->skstruct.adjust.note_2p_x;
						objy = g->skstruct.adjust.note_2p_y;
						if (-100.0 < g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].h && g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].h < 100.0
							&& -100.0 < g->skstruct.image.dst[i].draw[0].h && g->skstruct.image.dst[i].draw[0].h < 100.0) {

							objx = 0;
							objy = 0;
						}
					}
					else {
						objx = 0;
						objy = 0;
						if (g->skstruct.dst_JUDGELINE[0].dstCount > 0) {
							if (abs(g->skstruct.dst_JUDGELINE[0].draw->w - g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].w) <= 10.0
								&& abs(g->skstruct.dst_JUDGELINE[0].draw->x - g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].x) <= 5.0
								&& (g->skstruct.dst_JUDGELINE[0].draw->y >= g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].y || g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].h < 0.0)) {

								objx = g->skstruct.adjust.note_1p_x;
								objy = g->skstruct.adjust.note_1p_y;
							}

							else if (g->skstruct.dst_JUDGELINE[1].dstCount > 0) {
								if (abs(g->skstruct.dst_JUDGELINE[1].draw->w - g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].w) <= 10.0
									&& abs(g->skstruct.dst_JUDGELINE[1].draw->x - g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].x) <= 5.0
									&& (g->skstruct.dst_JUDGELINE[1].draw->y >= g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].y || g->skstruct.image.dst[i].draw[g->skstruct.image.dst[i].dstCount - 1].h < 0.0)) {

									objx = g->skstruct.adjust.note_2p_x;
									objy = g->skstruct.adjust.note_2p_y;
								}
							}
						}
					}
				}

				if (g->skstruct.image.dst[i].op4 == 1) {
					AddDrawingBuffer_Scratch(&g->skstruct.drBuf, &g->skstruct.image.src[i], &g->skstruct.image.dst[i], &g->timer1, g->skstruct.scratchAngle_1);
				}
				else if (g->skstruct.image.dst[i].op4 == 2) {
					AddDrawingBuffer_Scratch(&g->skstruct.drBuf, &g->skstruct.image.src[i], &g->skstruct.image.dst[i], &g->timer1, g->skstruct.scratchAngle_2);
				}
				else {
					AddDrawingBuffer_Object(&g->skstruct.drBuf, &g->skstruct.image.src[i], &g->skstruct.image.dst[i], &g->timer1, objx, objy);
				}
			}
		}

	}

	if (g->gameplay.courseType == 1) {
		if (g->procSelecter == 4) {
			if (GetTimeLapse(41, &g->timer1) >= 0.0 && g->config.play.bga > 0) {
				int intTemp = 0;
				while (g->gameplay.bgaMixer[intTemp] <= 0) {
					intTemp++;
					if (intTemp >= g->sSelect.bmsList[g->sSelect.cur_song].courseStageCount - 1) break;
				}

				if (GetRand(g->gameplay.bgaMixer[intTemp + 1] + g->gameplay.bgaMixer[intTemp]) > g->gameplay.bgaMixer[intTemp]) {
					g->gameplay.missLayer = g->gameplay.courseMissLayer[intTemp + 1];
					g->gameplay.bgaLayer1 = g->gameplay.courseBgaLayer1[intTemp + 1];
					g->gameplay.bgaLayer2 = g->gameplay.courseBgaLayer2[intTemp + 1];
				}
				else {
					g->gameplay.missLayer = g->gameplay.courseMissLayer[intTemp];
					g->gameplay.bgaLayer1 = g->gameplay.courseBgaLayer1[intTemp];
					g->gameplay.bgaLayer2 = g->gameplay.courseBgaLayer2[intTemp];
				}
			}
		}

	}

	if (g->procSelecter == 4 || g->is_starter) {
		if (GetTimeLapse(41, &g->timer1) >= 0.0 && g->config.play.bga > 0) {
			for (int i = 0; i < g->skstruct.otherObject[4].srcSize; i++) {
				if (GetOptionFlag_dst(g, g->skstruct.otherObject[4].dst[i].op1) && GetOptionFlag_dst(g, g->skstruct.otherObject[4].dst[i].op2)
					&& GetOptionFlag_dst(g, g->skstruct.otherObject[4].dst[i].op3) && g->skstruct.adjust.dark_type != 2) {

					if (g->config.play.poorbga > GetTimeWrap() - g->gameplay.lastMissTime && g->gameplay.missLayer >= 0 && g->config.play.bga != 3) {
						AddDrawingBuffer_BGA(&g->skstruct.drBuf, &g->skstruct.otherObject[4].src[i], &g->skstruct.otherObject[4].dst[i], &g->timer1, g->gameplay.bgaHandle[g->gameplay.missLayer], 0);
					}
					else {
						if (g->gameplay.bgaLayer1 >= 0)
							AddDrawingBuffer_BGA(&g->skstruct.drBuf, &g->skstruct.otherObject[4].src[i], &g->skstruct.otherObject[4].dst[i], &g->timer1, g->gameplay.bgaHandle[g->gameplay.bgaLayer1], 1);
						if (g->gameplay.bgaLayer2 >= 0)
							AddDrawingBuffer_BGA(&g->skstruct.drBuf, &g->skstruct.otherObject[4].src[i], &g->skstruct.otherObject[4].dst[i], &g->timer1, g->gameplay.bgaHandle[g->gameplay.bgaLayer2], 0);
					}
				}
			}
		}
	}

	for (int i = 0; i < g->skstruct.otherObject[1].srcSize; i++) {
		if (GetOptionFlag_dst(g, g->skstruct.otherObject[1].dst[i].op1) && GetOptionFlag_dst(g, g->skstruct.otherObject[1].dst[i].op2)
			&& GetOptionFlag_dst(g, g->skstruct.otherObject[1].dst[i].op3) && g->skstruct.adjust.dark_type != 2) {

			AddDrawingBuffer_Image(&g->skstruct.drBuf, &g->skstruct.otherObject[1].src[i], &g->skstruct.otherObject[1].dst[i], &g->timer1);

		}
	}
	for (int i = 0; i < g->skstruct.otherObject[2].srcSize; i++) {
		if (GetOptionFlag_dst(g, g->skstruct.otherObject[2].dst[i].op1) && GetOptionFlag_dst(g, g->skstruct.otherObject[2].dst[i].op2)
			&& GetOptionFlag_dst(g, g->skstruct.otherObject[2].dst[i].op3) && g->skstruct.adjust.dark_type != 2) {

			AddDrawingBuffer_Slider(&g->skstruct.drBuf, &g->skstruct.otherObject[2].src[i], &g->skstruct.otherObject[2].dst[i], &g->timer1);

		}
	}
	for (int i = 0; i < g->skstruct.otherObject[6].srcSize; i++) {
		if (GetOptionFlag_dst(g, g->skstruct.otherObject[6].dst[i].op1) && GetOptionFlag_dst(g, g->skstruct.otherObject[6].dst[i].op2)
			&& GetOptionFlag_dst(g, g->skstruct.otherObject[6].dst[i].op3) && g->skstruct.adjust.dark_type != 2) {

			AddDrawingBuffer_Numbers(&g->skstruct.drBuf, &g->skstruct.otherObject[6].src[i], &g->skstruct.otherObject[6].dst[i], &g->timer1, SetObjectValue_Num(g, g->skstruct.otherObject[6].src[i].op1), 0, 0);

		}
	}
	if (g->txtStruct.readme.show != true) {
		for (int i = 0; i < g->skstruct.otherObject[3].srcSize; i++) {
			if (GetOptionFlag_dst(g, g->skstruct.otherObject[3].dst[i].op1) && GetOptionFlag_dst(g, g->skstruct.otherObject[3].dst[i].op2)
				&& GetOptionFlag_dst(g, g->skstruct.otherObject[3].dst[i].op3) && g->skstruct.adjust.dark_type != 2) {

				AddDrawingBuffer_OnMouse(&g->skstruct.drBuf, &g->skstruct.otherObject[3].src[i], &g->skstruct.otherObject[3].dst[i], &g->timer1, &g->KeyInput, g->sSelect.panel);

			}
		}
	}
	if (g->skstruct.adjust.dark_type != 2) {
		//Proc_Text(g, sql3, 0); //TEXT INPUT
		if (g->skstruct.adjust.dark_type != 2) {
			SetObjectValue_Bargraph(g);
		}
	}
	if (GetTimeWrap() < g->KeyInput.mouse_recentMoveTime + 10000)
		AddDrawingBuffer_Object(&g->skstruct.drBuf, &g->skstruct.src_MOUSECURSOR, &g->skstruct.dst_MOUSECURSOR, &g->timer1, g->KeyInput.mouse_oldX, g->KeyInput.mouse_oldY);
	else if (GetTimeWrap() < g->KeyInput.mouse_recentMoveTime + 10500)
		AddDrawingBuffer_ObjectAlpha(&g->skstruct.drBuf, &g->skstruct.src_MOUSECURSOR, &g->skstruct.dst_MOUSECURSOR, &g->timer1, g->KeyInput.mouse_oldX, g->KeyInput.mouse_oldY,
			(g->KeyInput.mouse_recentMoveTime - GetTimeWrap() + 10500) * 255 / 500);

	if (g->procSelecter == 2) {
		int h = g->txtStruct.readme.h;
		for (int i = 0; i < g->txtStruct.readme.lines; i++) {
			int y = g->skstruct.src_README[0].op1 * i + h;
			if (y < 480 && (g->skstruct.src_README[0].op1 * (i + 1) + h > 0)) {
				AddDrawingBuffer_TextXY(&g->skstruct.drBuf, &g->skstruct.src_README[0], &g->skstruct.dst_README[0], &g->timer1, i + 1000, g->txtStruct.readme.w, y);
				AddDrawingBuffer_TextXY(&g->skstruct.drBuf, &g->skstruct.src_README[1], &g->skstruct.dst_README[1], &g->timer1, i + 1000, g->txtStruct.readme.w, g->skstruct.src_README[1].op1 * i + h);
				h = g->txtStruct.readme.h;
			}
		}
	}
	for (int i = 0; i < g->skstruct.drBuf.count; i++) {
		int quake_x = 0, quake_y = 0;
		if (((g->procPhase == 1) && (g->procSelecter == 4)) && (0 < g->config.play.m_earthquake)) {
			quake_x = (double)g->gameplay.earthquake_x;
			quake_y = (double)g->gameplay.earthquake_y;
		}
		LRDraw(&g->skstruct.drBuf, &g->txtStruct, &g->sSelect, &g->skstruct, i, quake_x, quake_y);
		if (g->config.system.thread == 0 && g->gameplay.flag_gameinput != 0) {
			ProcGame(g);
		}
	}
	InitDrawingBuffer(&g->skstruct.drBuf);
	GetTimeWrap();

	GetTimeWrap();
	if (g->procSelecter == 4) {
		if (g->KeyInput.inputID[KEY_INPUT_1] == 2) {
			printfDx("スキン位置の変更(カーソルキーで調節)\nx:%d\ny:%d\n", g->skstruct.adjust.shift_x, g->skstruct.adjust.shift_y);
		}
		else if (g->KeyInput.inputID[KEY_INPUT_2] == 2) {
			printfDx("スキン拡大率の変更(カーソルキーで調節)\nx:%d\ny:%d\n", g->skstruct.adjust.rate_x, g->skstruct.adjust.rate_y);
		}
		else if (g->KeyInput.inputID[KEY_INPUT_3] == 2) {
			printfDx("ジャッジ表示位置の変更(カーソルキーで調節)\nx:%d\ny:%d\n", g->skstruct.adjust.judge_x, g->skstruct.adjust.judge_y);
		}
		else if (g->KeyInput.inputID[KEY_INPUT_4] == 2) {
			printfDx("ノートサイズの変更(カーソルキーで調節)\nx:%d\ny:%d\n", g->skstruct.adjust.size_x, g->skstruct.adjust.size_y);
		}
		else if (g->KeyInput.inputID[KEY_INPUT_5] == 2) {
			if (g->skstruct.adjust.dark_type == 1) {
				printfDx("スキン描画制限(カーソルキーで調節)\nDARK 1\n");
			}
			else if (g->skstruct.adjust.dark_type == 2) {
				printfDx("スキン描画制限(カーソルキーで調節)\nDARK 2\n");
			}
			else {
				printfDx("スキン描画制限(カーソルキーで調節)\nOFF\n");
			}
		}
		else if (g->KeyInput.inputID[KEY_INPUT_6] == '\x02') {
			printfDx("ノート位置(1P)の変更(カーソルキーで調節)\nx:%d\ny:%d\n", g->skstruct.adjust.note_1p_x, g->skstruct.adjust.note_1p_y);
		}
		else if (g->KeyInput.inputID[KEY_INPUT_7] == '\x02') {
			printfDx("ノート位置(2P)の変更(カーソルキーで調節)\nx:%d\ny:%d\n", g->skstruct.adjust.note_2p_x, g->skstruct.adjust.note_2p_y);
		}
	}
	if (g->KeyInput.inputID[KEY_INPUT_F1] == '\x02' && g->sSelect.flag_maniacPanel == '\0' && g->sSelect.unk4f74 == '\0' && g->is_starter == '\0') {
		printfDx((g->sSelect.bmsList[g->sSelect.cur_song].folderType == 8) ?
			"F2 マニアックオプション F3 コースのソート変更\nF4 ウインドウモード切り替え F5 IRに接続\nF6  スクリーンショット F7 FPS表示\nF8 フォルダのリロード\n"
			: "F2 マニアックオプション F3 レベルの変更\nF4 ウインドウモード切り替え F5 IRに接続\nF6 スク リーンショット F7 FPS表示\nF8 フォルダのリロード\n");
	}
	if (g->flag_showFPS) {
		printfDx("FPS %d\n", (int)g->timer1.FPS);
	}
	g->sSelect.flag_maniacPanel = 0;
	g->sSelect.unk4f74 = '\0';
	if (g->procSelecter == 2) {
		if ((g->KeyInput.inputID[KEY_INPUT_F5] == 1 || g->sSelect.is_buttonIRpage != 0) && g->sSelect.bmsList[g->sSelect.cur_song].keymode > 4 && g->config.network.lr2ir == 1) {
			if (g->config.system.screenmode == 0) {
				g->config.system.screenmode = 1;
				SetObjectStrings_SongSelect(g);
				for (int i = 0; i < 200; i++) {
					g->skstruct.caption[i].fillzero();
				}
				for (int i = 0; i < 10; i++) {
					g->skstruct.ImageFonts[i].filepath[0] = 0;
				}
				SetGraphMode(640, 480, (g->config.system.highcolor == 0 ? 32 : 16), 60);
				SetWaitVSyncFlag(g->config.system.vsync);
				ChangeWindowMode(g->config.system.screenmode);
				SetWaitVSyncFlag(g->config.system.vsync);
				SetDrawScreen(-2);
				LoadScene(&g->skstruct, g->config.skin.skinFilePath[5], g->skinData.Data[g->skinData.skinID[5]].informationP5, 0);
				SetMouseDispFlag(0);
				g->is_clicked_screenModeChange = 0;
				if (g->config.system.screenmode == 0) {
					ChangeWindowMode(1);
					ErrorLogAdd("ウインドウを閉じます\n");
					CloseWindow(GetMainWindowHandle());
					ErrorLogAdd("成功\n");
				}
				if (g->config.network.lr2ir == 1) {
					//same as below
					ErrorLogAdd("IRを出します\n");
					OpenWebRanking(g->sSelect.bmsList[g->sSelect.cur_song].hash);
				}
			}
			else {
				ErrorLogAdd("IRを出します\n");
				OpenWebRanking(g->sSelect.bmsList[g->sSelect.cur_song].hash);
			}

			if (g->config.system.screenmode == 0) {
				ErrorLogAdd("アイコン化が終わるまで待ちます\n");
				while (ProcessMessage() == 0) {
					if (IsIconic(GetMainWindowHandle()) == 0) break;
					Sleep(16);
				}
				SetObjectStrings_SongSelect(g);
				for (int i = 0; i < 200; i++) {
					g->skstruct.caption[i].fillzero();
				}
				for (int i = 0; i < 10; i++) {
					g->skstruct.ImageFonts[i].filepath[0] = 0;
				}
				SetGraphMode(640, 480, (g->config.system.highcolor == 0 ? 32 : 16), 60);
				SetWaitVSyncFlag(g->config.system.vsync);
				ChangeWindowMode(g->config.system.screenmode);
				SetWaitVSyncFlag(g->config.system.vsync);
				SetDrawScreen(-2);
				LoadScene(&g->skstruct, g->config.skin.skinFilePath[5], g->skinData.Data[g->skinData.skinID[5]].informationP5, 0);
				SetMouseDispFlag(0);
				g->is_clicked_screenModeChange = 0;
			}
			g->KeyInput.inputID[KEY_INPUT_F1] = 0; //why F1?
			g->sSelect.is_buttonIRpage = 0;
			InitInputStructure2(&g->KeyInput);
			Sleep(1000);
			if (g->sSelect.flag_maniacPanel || g->sSelect.unk4f74) ClsDrawScreen();
		}
		else if (g->KeyInput.inputID[KEY_INPUT_F2] == 2) {
			g->sSelect.flag_maniacPanel = 1;
			Print_ManiacOptions(g);
			if (g->sSelect.flag_maniacPanel || g->sSelect.unk4f74) ClsDrawScreen();
		}
		else if (g->KeyInput.inputID[KEY_INPUT_F3] == 2) {
			if (g->sSelect.bmsList[g->sSelect.cur_song].folderType == 8) {
				printfDx("カーソルキー↑↓ コースの表示順変更\n");
			}
			else {
				if (g->config.select.disabledifficultyfilter == 0) printfDx("カーソルキー↑↓ 難度カテゴリの変更\nカーソルキー←→ レベルの変更\n");
				else printfDx("カーソルキー←→ レベルの変更\n");
			}
			if (g->sSelect.flag_maniacPanel || g->sSelect.unk4f74) ClsDrawScreen();
		}
	}
	GetTimeWrap();

	GetTimeWrap();
	if (g->isSkipDrawTick == 0) {
		if (g->gameplay.flag_gameinput != 0 && g->config.system.thread == 0 && g->config.system.vsync == 1 && g->is_recordmode == 0) {
			//GetVSyncTime() always return 0 in dxlib3.02, and not exists in 3.12a.
			while ((GetTimeWrap() - g->timer1.vSyncTick >= 0 - 3) == 0) {
				if (GetTimeWrap() - g->timer1.gameTick >= 0 - 4) break;
				ProcGame(g);
				WaitTimer(1);
			}
		}
		g->timer1.vSyncTick = GetTimeWrap();

		if (g->is_recordmode) {
			if (g->rec.recMode == 3) {
				if (g->gameplay.bgaLayer1 >= 0)
					DrawBGA(g->gameplay.bgaHandle[g->gameplay.bgaLayer1]);
				if (g->gameplay.bgaLayer2 >= 0)
					DrawBGA(g->gameplay.bgaHandle[g->gameplay.bgaLayer2]);
			}
			if (g->audio.replay2avi) {
				g->audio.aviTimer = GetTimeWrap();
			}
			g->rec.CpyScreenToAVI();
			if (g->timer1.flagMovieTimer) {
				double time1, time2;
				time1 = GetTimeWrap();//
				MovieTimer(&g->timer1);
				g->audio.aviTimer = GetTimeWrap();
				if (g->audio.replay2avi)
					g->audio.aviTimer = GetTimeWrap();
				time2 = GetTimeWrap();
				if (g->gameplay.flag_gameinput) {

					while (time2 - 1.0 < time1) {
						ProcGame(g);
						SetManualTimer(&g->timer1, time1);
						g->audio.aviTimer = time1;
						time1 += 1.0;
					}
					SetManualTimer(&g->timer1, time2);
					if (g->audio.replay2avi) {
						g->audio.aviTimer = time2;
					}
				}
			}
			if (g->gameplay.courseType == 1) {
				for (int i = 0; i < 10; i++) {
					if (g->gameplay.courseBgaLayer1[i] > 0) {
						SeekMovieToGraph(g->gameplay.bgaHandle[g->gameplay.courseBgaLayer1[i]], GetTimeWrap() - g->gameplay.courseLayer1ChangeTime[i]);
						PlayMovieToGraph(g->gameplay.bgaHandle[g->gameplay.courseBgaLayer1[i]], 1, 0);
					}
					if (g->gameplay.courseBgaLayer2[i] > 0) {
						SeekMovieToGraph(g->gameplay.bgaHandle[g->gameplay.courseBgaLayer2[i]], GetTimeWrap() - g->gameplay.courseLayer2ChangeTime[i]);
						PlayMovieToGraph(g->gameplay.bgaHandle[g->gameplay.courseBgaLayer2[i]], 1, 0);
					}
				}
			}
			else {
				if (1 <= g->gameplay.bgaLayer1 && g->gameplay.bgaLayer1 < 6479) {
					SeekMovieToGraph(g->gameplay.bgaHandle[g->gameplay.bgaLayer1], GetTimeWrap() - g->gameplay.layer1ChangeTime);
					PlayMovieToGraph(g->gameplay.bgaHandle[g->gameplay.bgaLayer1], 1, 0);
				}
				if (1 <= g->gameplay.bgaLayer2 && g->gameplay.bgaLayer2 < 6479) {
					SeekMovieToGraph(g->gameplay.bgaHandle[g->gameplay.bgaLayer2], GetTimeWrap() - g->gameplay.layer2ChangeTime);
					PlayMovieToGraph(g->gameplay.bgaHandle[g->gameplay.bgaLayer2], 1, 0);
				}
			}
		}

		//capture here
		SetDrawScreen(DX_SCREEN_BACK);
		GetDrawScreenSoftImage(0, 0, sizeX, sizeY, gHandle); //for SDL3
		//GetDrawScreenGraph(0, 0, sizeX, sizeY, gHandle); //for DX9? << not works
		SetDrawScreen(DX_SCREEN_BACK);
		

		ScreenFlip();
		GetTimeWrap();

		GetTimeWrap();
		clsDx();
		CalcFPS(&g->timer1);
		g->timer1.tickTime = GetTimeWrap() - g->timer1.gameTick;
		g->timer1.gameTick = GetTimeWrap();
		g->timer2.tickTime = g->timer1.tickTime;
		g->timer2.gameTick = g->timer1.gameTick;
	}
	else g->isSkipDrawTick = 0;

	ClsDrawScreen();

	return 0;
}

int LR2SESceneInit(game *g, int type) {
	
	//g->skstruct.GrHandle[GrH_Stage] = LoadGraph("LR2files\\Config\\title.bmp", 0);

	InitGameplay(&g->gameplay, &g->config.play);
	
	

	switch (type) {
	case SKINTYPE_SELECT:
		break;
	case SKINTYPE_DECIDE:
		break;
	case SKINTYPE_7KEYS:
	case SKINTYPE_5KEYS:
	case SKINTYPE_14KEYS:
	case SKINTYPE_10KEYS:
	case SKINTYPE_9KEYS:
	case SKINTYPE_7KEYSBATTLE:
	case SKINTYPE_5KEYSBATTLE:
	case SKINTYPE_9KEYSBATTLE:
	{
		InitKeysound(g);

		//PlayPreviewSample(g);
		int scratchSide = 0;
		ConfigStruct tCfg = g->config;
		tCfg.play.battle = (g->skinData.select >= 12);
		g->gameplay.isAutoplay = 1;
		scratchSide = 0;
		if (g->skinData.select == 3 && g->skinData.Data[g->skinData.skinID[3]].type == SKINTYPE_14KEYS)
			scratchSide = g->skstruct.scratchside_1 + g->skstruct.scratchside_2 * 2;
		if (g->skinData.select == 1 && g->skinData.Data[g->skinData.skinID[1]].type == SKINTYPE_7KEYS)
			scratchSide = g->skstruct.scratchside_1 + g->skstruct.scratchside_2 * 2;
		if (g->skinData.select == 13 && g->skinData.Data[g->skinData.skinID[13]].type == SKINTYPE_5KEYSBATTLE)
			scratchSide = g->skstruct.scratchside_1 + g->skstruct.scratchside_2 * 2;
		ProcS_Select(g);
		ReleaseBGA(g);
		switch (g->skinData.select) {
		default: //case 0:?
			ReadKeyConfig(g, "LR2files\\Config\\keyconfig.xml");
			g->sSelect.metaSelected.keymode = 7;
			InitGameplay(&g->gameplay, &tCfg.play);
			ParseBmsFile(&g->gameplay, "LR2files\\Config\\sample_7.bme", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide);
			LoadBmsResource(&g->gameplay, "LR2files\\Config\\sample_7.bme", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide, 0);
			break;

		case 1:
			if (g->skinData.Data[g->skinData.skinID[1]].type == SKINTYPE_7KEYS)
				ReadKeyConfig(g, "LR2files\\Config\\keyconfig.xml");
			else
				ReadKeyConfig(g, "LR2files\\Config\\keyconfig_5.xml");
			g->sSelect.metaSelected.keymode = 5;
			InitGameplay(&g->gameplay, &tCfg.play);
			ParseBmsFile(&g->gameplay, "LR2files\\Config\\sample_5.bme", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide);
			LoadBmsResource(&g->gameplay, "LR2files\\Config\\sample_5.bme", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide, 0);
			break;

		case 2:
			ReadKeyConfig(g, "LR2files\\Config\\keyconfig.xml");
			g->sSelect.metaSelected.keymode = 14;
			InitGameplay(&g->gameplay, &tCfg.play);
			ParseBmsFile(&g->gameplay, "LR2files\\Config\\sample_14.bme", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide);
			LoadBmsResource(&g->gameplay, "LR2files\\Config\\sample_14.bme", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide, 0);
			break;

		case 3:
			if (g->skinData.Data[g->skinData.skinID[3]].type == SKINTYPE_14KEYS)
				ReadKeyConfig(g, "LR2files\\Config\\keyconfig.xml");
			else
				ReadKeyConfig(g, "LR2files\\Config\\keyconfig_5.xml");
			g->sSelect.metaSelected.keymode = 10;
			InitGameplay(&g->gameplay, &tCfg.play);
			ParseBmsFile(&g->gameplay, "LR2files\\Config\\sample_10.bme", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide);
			LoadBmsResource(&g->gameplay, "LR2files\\Config\\sample_10.bme", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide, 0);
			break;

		case 4:
			ReadKeyConfig(g, "LR2files\\Config\\keyconfig_p.xml");
			g->sSelect.metaSelected.keymode = 9;
			InitGameplay(&g->gameplay, &tCfg.play);
			ParseBmsFile(&g->gameplay, "LR2files\\Config\\sample_9.pms", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide);
			LoadBmsResource(&g->gameplay, "LR2files\\Config\\sample_9.pms", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide, 0);
			break;

		case 13:
			if (g->skinData.Data[g->skinData.skinID[13]].type == SKINTYPE_5KEYSBATTLE)
				ReadKeyConfig(g, "LR2files\\Config\\keyconfig.xml");
			else
				ReadKeyConfig(g, "LR2files\\Config\\keyconfig_5.xml");
			g->sSelect.metaSelected.keymode = 5;
			InitGameplay(&g->gameplay, &tCfg.play);
			ParseBmsFile(&g->gameplay, "LR2files\\Config\\sample_5.bme", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide);
			LoadBmsResource(&g->gameplay, "LR2files\\Config\\sample_5.bme", &g->audio, &tCfg, &g->sSelect.metaSelected, 1, scratchSide, 0);
			break;
		}

		g->gameplay.bgaLayer1 = 1;
		if (g->skinData.select == 7) {
			g->gameplay.player[0].clearType = (GetRand(1) == 0) ? 5 : 1;
			g->gameplay.player[1].clearType = g->gameplay.player[0].clearType;
			for (int p = 0; p < 2; p++) {
				g->gameplay.player[p].rate = 100.0;
				g->gameplay.player[p].totalnotes = 1000;
				g->gameplay.player[p].max_combo = 1000;
				g->gameplay.player[p].judgecount[5] = 1000;
				g->gameplay.player[p].exscore = 2000;
				g->gameplay.player[p].score = 200000;

				for (int i = 0; i < 1000; i++) {
					g->gameplay.statgraph[p].combo[i] = g->gameplay.player[p].totalnotes * i / 1000;
					g->gameplay.statgraph[p].exscore[i] = g->gameplay.player[p].totalnotes * i * 16 / 9000;
					g->gameplay.statgraph[p].hp[i] = (100 * i) / 1000;
					if (p == 0) {
						g->gameplay.rategraph[0].val[i] = g->gameplay.statgraph[0].exscore[0] * 7 / 8;
						g->gameplay.rategraph[1].val[i] = g->gameplay.statgraph[0].exscore[0] * 6 / 8;
					}
				}

				g->gameplay.statgraph[p].cursor = 1000;
				g->gameplay.rategraph[0].cursor = 1000;
				g->gameplay.rategraph[1].cursor = 1000;
			}
		}
	}
		break;

	case SKINTYPE_RESULT:
		//LoadScene(&g->skstruct, );
		//ProcS_Result(g);
		break;
	case SKINTYPE_KEYCONFIG:
		g->KeyInput.config_keymode = 0;
		g->KeyInput.config_button = 1;
		g->KeyInput.config_button_inMap = 1;
		g->KeyInput.config_key = -1;
		ProcS_Keyconfig(g);
		break;

	case SKINTYPE_SKINSELECT:
		break;

	case SKINTYPE_COURSERESULT:
		g->gameplay.player[0].judgecount[0] = 1;
		g->gameplay.player[0].judgecount[1] = 2;
		g->gameplay.player[0].judgecount[2] = 3;
		g->gameplay.player[0].judgecount[3] = 4;
		g->gameplay.player[0].judgecount[4] = 5;
		g->gameplay.player[0].judgecount[5] = 6;
		g->gameplay.player[0].exscore = 17;
		g->gameplay.player[0].totalnotes = 20;


		g->gameplay.player[1].judgecount[0] = 1;
		g->gameplay.player[1].judgecount[1] = 10;
		g->gameplay.player[1].judgecount[2] = 100;
		g->gameplay.player[1].judgecount[3] = 1000;
		g->gameplay.player[1].judgecount[4] = 10000;
		g->gameplay.player[1].judgecount[5] = 100000;
		g->gameplay.player[1].exscore = 12345;
		g->gameplay.player[1].totalnotes = 11110;
		break;

	case SKINTYPE_SOUNDSET:
		break;
	case SKINTYPE_THEME:
		break;

	case SKINTYPE_OPENING:
		break;
	case SKINTYPE_MODESELECT:
		break;
	case SKINTYPE_MODEDECIDE:
		break;

	case SKINTYPE_COURSESELECT:
		break;
	case SKINTYPE_COURSEEDIT:
		break;
	}

	InitTimer(&g->timer1);
	SetTimeLapse(0, &g->timer1);
	g->procPhase = 1;

	return 0;
}

int LR2SESceneProc(game* g, int type) {

	if (g->skstruct.startinput_start < GetTimeLapse(0, &g->timer1) && GetTimeLapse(1, &g->timer1) == -1.0) {
		InitInputStructure(&g->KeyInput);
		SetTimeLapse(1, &g->timer1);
		SetTimeLapse(11, &g->timer1);
	}

	switch (type) {
	case SKINTYPE_SELECT:
		//ProcI_Select(g, sql3);
		break;
	case SKINTYPE_DECIDE:
		ProcI_Decide(g);
		break;
	case SKINTYPE_7KEYS:
	case SKINTYPE_5KEYS:
	case SKINTYPE_14KEYS:
	case SKINTYPE_10KEYS:
	case SKINTYPE_9KEYS:
	case SKINTYPE_7KEYSBATTLE:
	case SKINTYPE_5KEYSBATTLE:
	case SKINTYPE_9KEYSBATTLE:
		LR2SE_I_Play(g);
		break;

	case SKINTYPE_RESULT:
		ProcI_Result(g);
		break;
	case SKINTYPE_KEYCONFIG:
		ProcI_Keyconfig(g);
		break;	
	case SKINTYPE_SKINSELECT:
		ProcI_SkinSelect(g);
		break;

	case SKINTYPE_COURSERESULT:
		ProcI_Result(g);
		break;

	case SKINTYPE_SOUNDSET:
		break;
	case SKINTYPE_THEME:
		break;

	case SKINTYPE_OPENING:
		break;
	case SKINTYPE_MODESELECT:
		//ProcI_PO4Menu(g, sql3);
		break;
	case SKINTYPE_MODEDECIDE:
		//ProcI_PO4Decide(g);
		break;

	case SKINTYPE_COURSESELECT:
		//ProcI_PO4Select(g, sql3);
		break;
	case SKINTYPE_COURSEEDIT:
		break;
	}

	return 0;
}

int LR2SE_I_Play(game* g) {
	int timeLimit;
	double gameTime;

	DrawHPgauge(g);
	SoundGetCurrentTime(&g->audio, &g->gameplay.muon);
	NONE_004b6770();

	if (GetTimeLapse(41, &g->timer2) > 0.0 && g->config.play.m_lunaris == 0) {
		DrawNotes(g, &g->skstruct, &g->timer2, &g->config.play);
		DrawJudgeCombo(g, &g->skstruct, &g->timer2, &g->config.play);
	}
	else if (GetTimeLapse(41, &g->timer2) > 0.0 && g->config.play.m_lunaris) {
		DrawLunaris(g);
		DrawJudgeCombo(g, &g->skstruct, &g->timer2, &g->config.play);
	}

	if (g->skstruct.dst_JUDGELINE[0].dstCount > 0) {
		AddDrawingBuffer_PlayArea(&g->skstruct.drBuf, &g->skstruct.src_JUDGELINE[0], &g->skstruct.dst_JUDGELINE[0], &g->timer2,
			(float)g->skstruct.adjust.note_1p_x + g->gameplay.nabeatsu_x, (float)g->skstruct.adjust.note_1p_y + g->gameplay.nabeatsu_y, -1,
			(float)g->skstruct.adjust.size_x, (float)g->skstruct.adjust.size_y, 0);
	}
	if (g->skstruct.dst_JUDGELINE[1].dstCount > 0) {
		AddDrawingBuffer_PlayArea(&g->skstruct.drBuf, &g->skstruct.src_JUDGELINE[1], &g->skstruct.dst_JUDGELINE[1], &g->timer2,
			(float)g->skstruct.adjust.note_2p_x + g->gameplay.nabeatsu_x, (float)g->skstruct.adjust.note_2p_y + g->gameplay.nabeatsu_y, -1,
			(float)g->skstruct.adjust.size_x, (float)g->skstruct.adjust.size_y, 0);
	}

	if (((g->KeyInput.inputID[KEY_INPUT_ESCAPE] == 2 || (g->KeyInput.mouse_buttonR == 2 && g->config.play.disableleftclickexit == 0))
		|| (g->KeyInput.p1_buttonInput[13] == 2 && g->KeyInput.p1_buttonInput[12] == 2)
		|| (g->KeyInput.p2_buttonInput[13] == 2 && g->KeyInput.p2_buttonInput[12] == 2)
		|| (g->gameplay.player[0].totalnotes <= g->gameplay.player[0].note_current && (g->KeyInput.p1_buttonInput[13] == 2 || g->KeyInput.p1_buttonInput[12] == 2 || g->KeyInput.p2_buttonInput[13] == 2 || g->KeyInput.p2_buttonInput[12] == 2)))
		&& g->procPhase == 1) {
		SetTimeLapse(2, &g->timer2);
		g->procPhase = 2;
		g->gameplay.flag_closingPhase = 1;
		return 1;
	}

	if (g->procPhase == 2) {
		timeLimit = g->skstruct.fadeout;
		gameTime = GetTimeLapse(2, &g->timer2);
		if (timeLimit < gameTime || timeLimit == 0) g->procSelecter = 5;
	}
	else if (g->procPhase == 3) {
		timeLimit = g->skstruct.close;
		gameTime = GetTimeLapse(3, &g->timer2);
		if (timeLimit < gameTime || timeLimit == 0) g->procSelecter = 5;
	}
	return 1;
}