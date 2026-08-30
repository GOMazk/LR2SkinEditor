#include "skin.h"
#include "../LR2/LR2.h"
#include "../LR2/Scenes.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

static const char* g_previewRenderStage = "not started";
static int g_previewRenderIndex = -1;
static bool g_previewRenderFaulted = false;
static const char* g_sceneInitStage = "not started";
static const char* g_sceneProcStage = "not started";

void LR2SEResetRenderFault() {
	g_previewRenderStage = "not started";
	g_previewRenderIndex = -1;
	g_previewRenderFaulted = false;
}

int LR2SEDrawLoopSafe(game* g, int gHandle, int sizeX, int sizeY, bool staticSpecialPreview) {
	if (g_previewRenderFaulted) return -1;
	__try {
		return LR2SEDrawLoop(g, gHandle, sizeX, sizeY, staticSpecialPreview);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		g_previewRenderFaulted = true;
		FILE* fp = fopen("SkinEditor_load_crash.log", "a");
		if (fp) {
			fprintf(fp, "Preview render exception : code=0x%08lX stage=%s index=%d\n",
				GetExceptionCode(), g_previewRenderStage, g_previewRenderIndex);
			fclose(fp);
		}
		return -1;
	}
}

static void LogSceneException(const char* stage, unsigned long code) {
	FILE* fp = fopen("SkinEditor_load_crash.log", "a");
	if (!fp) return;
	fprintf(fp, "Scene exception : code=0x%08lX stage=%s\n", code, stage);
	fflush(fp);
	fclose(fp);
}

int LR2SESceneInitSafe(game* g, int type, LR2SEPreviewChartMode chartMode) {
	__try {
		return LR2SESceneInit(g, type, chartMode);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		LogSceneException(g_sceneInitStage, GetExceptionCode());
		return -1;
	}
}

int LR2SESceneProcSafe(game* g, int type, LR2SEPreviewChartMode chartMode) {
	__try {
		return LR2SESceneProc(g, type, chartMode);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		LogSceneException(g_sceneProcStage, GetExceptionCode());
		return -1;
	}
}

static void LR2SEInitSelectPreviewState(game* g) {
	SONGSELECT& select = g->sSelect;
	select.bmsListCount = 30;
	select.cur = 0;
	select.cur_song = 0;
	select.listCalculatedBar = 0;
	select.prevCalculatedBar = 0;
	select.listTopbar = 0;
	select.prevTopbar = 0;
	select.nowBar = 0;
	select.oldBar = 0;
	select.scrollDirection = 0;
	select.listSelectedBarFromScreenTop = g->skstruct.BAR_CENTER;
	select.prevSelectedBarFromScreenTop = g->skstruct.BAR_CENTER;
	select.panel = 0;
	select.panel_unk = 0;
	select.isFolder = 0;
	select.isExLevel = 0;
	select.filterDifficulty = 0;
	select.filterKey = 0;
	select.filterSort = 0;
	select.stack_query[0] = "PREVIEW";
	select.stack_folderPath[0] = "LR2files\\Preview";
	select.stack_searchTitle[0] = "ALL SONGS";
	select.stack_isFolder[0] = 0;
	select.stack_rivalID[0] = 0;
	select.ROOT = "LR2files\\Preview";
	select.directory = "LR2files\\Preview";

	static const char* difficulties[5] = { "BEGINNER", "NORMAL", "HYPER", "ANOTHER", "INSANE" };
	for (int i = 0; i < select.bmsListCount; ++i) {
		SONGDATA& song = select.bmsList[i];
		char title[64];
		char subtitle[64];
		snprintf(title, sizeof(title), "PREVIEW SONG %02d", i + 1);
		snprintf(subtitle, sizeof(subtitle), "[%s]", difficulties[i % 5]);
		song.title = title;
		song.subtitle = subtitle;
		song.fulltitle = title;
		song.fulltitle.add(" ");
		song.fulltitle.add(subtitle);
		song.genre = "SKIN EDITOR PREVIEW";
		song.artist = "LR2 SKIN EDITOR";
		song.subartist = "SCENE SIMULATOR";
		song.tag = "PREVIEW";
		song.filepath = "LR2files\\Config\\sample_7.bme";
		song.folder = "LR2files\\Preview";
		song.hash = "00000000000000000000000000000000";
		song.folderType = (i == 5) ? 1 : 0;
		song.difficulty = i % 5;
		song.level = 3 + (i % 10);
		song.exlevel = song.level + 1;
		song.keymode = 7;
		song.minBPM = 128;
		song.maxBPM = 174;
		song.longnote = 1;
		song.bga = 1;
		song.txt = 0;
		song.isBanner = 0;
		song.isStagefile = 0;
		song.isBackBMP = 0;
		song.grHandle = g->skstruct.GrHandle[GrH_Banner];
		for (int difficulty = 0; difficulty < 5; ++difficulty) {
			song.difficultyExist[difficulty] = 1;
			song.difficultyLevel[difficulty] = 2 + difficulty * 2 + (i % 3);
			song.difficultyLevelBarLamp[difficulty] = difficulty + 1;
		}
		song.difficultyCount = 5;
		song.mybest.total_notes = 1000;
		song.mybest.stat_pgreat = 720;
		song.mybest.stat_great = 180;
		song.mybest.stat_good = 45;
		song.mybest.stat_bad = 12;
		song.mybest.stat_poor = 8;
		song.mybest.stat_exscore = 1620 - i * 7;
		song.mybest.stat_score = 145000 - i * 900;
		song.mybest.stat_maxcombo = 640 - i * 3;
		song.mybest.minbp = 12 + i;
		song.mybest.rate = 81;
		song.mybest.rank = 2 + (i % 4);
		song.mybest.clear = 1 + (i % 7);
		song.mybest.clear_db = song.mybest.clear;
		song.mybest.clear_sd = song.mybest.clear;
		song.mybest.clear_ex = song.mybest.clear;
		song.mybest.playcount = 12 + i;
		song.mybest.clearcount = 8 + i / 2;
		song.mybest.failcount = 4 + i / 3;
		song.rivalRecord = song.mybest;
		song.rivalRecord.stat_exscore -= 75;
		song.rivalRecord.rank = 4;
	}

	// Keep the selected row a regular song so song-only conditions and values
	// are deterministic. Neighbouring folder rows still exercise bar variants.
	select.bmsList[0].folderType = 0;
	select.old = select.bmsList[0].mybest;
	g->config.select.key = 1;
	g->config.select.difficulty = 0;
	g->config.select.sort = 0;
	if (g->config.select.levelbarflash_7 <= 0) g->config.select.levelbarflash_7 = 12;
	if (g->config.select.levelbarflash_5 <= 0) g->config.select.levelbarflash_5 = 9;
	if (g->config.select.levelbarflash_9 <= 0) g->config.select.levelbarflash_9 = 42;
	g->config.play.battle = 0;
	g->config.play.bga = 1;
	g->config.play.bgasize = 0;
	g->config.play.scoregraph = 1;
	g->config.play.p1_target = 1;
	g->config.play.target_percent = 80;
	g->gameplay.isAutoplay = 0;
	g->gameplay.song_runtime = 120000.0;
	g->gameplay.loadObject_total = 100;
	g->gameplay.loadObject_loaded = 100;
	g->net.isOnline = 0;
	g->procSelecter = 2;
	g->procPhase = 1;

	ProcS_Select(g);
	for (int difficulty = 0; difficulty < 5; ++difficulty) {
		select.levelIndicatorAnimation[difficulty] = select.bmsList[0].difficultyLevel[difficulty];
		select.levelBarGraph[difficulty] = select.levelIndicatorAnimation[difficulty];
		select.levelsOfSong[difficulty] = select.bmsList[0].difficultyLevel[difficulty];
	}
}

static void LR2SEInitKeyconfigPreviewState(game* g) {
	// A normal LR2 transition enters KEYCONFIG with the 7-key mode and the
	// first logical button selected.  The editor has no preceding SELECT scene,
	// so establish that state before #IF and destination options are evaluated.
	g->KeyInput.config_keymode = 0;
	g->KeyInput.config_button = 1;
	g->KeyInput.config_button_inMap = 1;
	g->KeyInput.config_key = -1;

	// Keep an existing configuration when the host has loaded one.  A fresh
	// editor session otherwise contains an empty map, which makes all key-name
	// TEXT objects display "-" and is not useful as a scene preview.
	bool hasAssignedKey = false;
	for (int button = 0; button < 40 && !hasAssignedKey; ++button) {
		for (int slot = 0; slot < 16; ++slot) {
			if (g->config.input.buttonMap[button][slot] != 0) {
				hasAssignedKey = true;
				break;
			}
		}
	}
	if (!hasAssignedKey) {
		static const int previewKeys[7] = {
			KEY_INPUT_Z, KEY_INPUT_S, KEY_INPUT_X, KEY_INPUT_D,
			KEY_INPUT_C, KEY_INPUT_F, KEY_INPUT_V
		};
		for (int button = 1; button <= 7; ++button)
			g->config.input.buttonMap[button][0] = previewKeys[button - 1];
	}

	ProcS_Keyconfig(g);
}

static void LR2SEInitSkinSelectPreviewState(game* g) {
	SkinManage& skins = g->skinData;
	if (!skins.Data || skins.Count <= 0) {
		SetObjectString(50, CSTR("SKIN SELECT PREVIEW"), g->txtStruct.objectStr);
		SetObjectString(51, CSTR("LR2 SKIN EDITOR"), g->txtStruct.objectStr);
		return;
	}

	// LR2 normally fills this table during SetFirstSkins. The editor scans the
	// same skin headers but opens documents independently, so reconstruct the
	// first available entry for every scene type here.
	bool foundType[21] = {};
	for (int i = 0; i < skins.Count; ++i) {
		const int type = (int)skins.Data[i].type;
		if (type >= 0 && type < 21 && !foundType[type]) {
			skins.skinID[type] = i;
			foundType[type] = true;
		}
	}

	skins.select = foundType[SKINTYPE_7KEYS] ? SKINTYPE_7KEYS : 0;
	skins.previewID = foundType[skins.select] ? skins.skinID[skins.select] : 0;
	if (skins.previewID < 0 || skins.previewID >= skins.Count) skins.previewID = 0;
	skins.previewCustomID = 0;

	SkinHeader& header = skins.Data[skins.previewID];
	SetObjectString(50, header.title, g->txtStruct.objectStr);
	SetObjectString(51, header.maker, g->txtStruct.objectStr);
	for (int row = 0; row < 5; ++row) {
		CSTR empty("-");
		if (row >= header.custom_count) {
			SetObjectString(100 + row, empty, g->txtStruct.objectStr);
			SetObjectString(110 + row, empty, g->txtStruct.objectStr);
			continue;
		}
		SkinCustom& custom = header.customs[row];
		SetObjectString(100 + row, custom.title, g->txtStruct.objectStr);
		int label = custom.dst_op_selected;
		if (custom.dst_op_start > 0) label -= custom.dst_op_start;
		if (label >= 0 && label < custom.dst_op_count)
			SetObjectString(110 + row, custom.op_label[label], g->txtStruct.objectStr);
		else
			SetObjectString(110 + row, empty, g->txtStruct.objectStr);
	}

	// SRC id 105 is the selected-skin preview in LR2's SKIN SELECT scene.
	// Prefer the header thumbnail without loading the full nested scene.
	if (header.thumbnail.length() > 0) {
		DeleteGraph(g->skstruct.GrHandle[GrH_Preview]);
		g->skstruct.GrHandle[GrH_Preview] = LoadGraph(header.thumbnail);
		if (g->skstruct.GrHandle[GrH_Preview] < 0)
			g->skstruct.GrHandle[GrH_Preview] = LoadGraph("LR2files\\Config\\white.bmp");
	}
}

static void LR2SEInitDecidePreviewState(game* g) {
	// DECIDE inherits the song selected in the preceding SELECT scene.  Build
	// the same deterministic song first so all song strings and option flags
	// are valid even when a DECIDE skin is opened directly in the editor.
	LR2SEInitSelectPreviewState(g);
	SONGDATA& song = g->sSelect.bmsList[g->sSelect.cur_song];
	song.isStagefile = g->skstruct.GrHandle[GrH_Stage] >= 0;
	song.isBackBMP = g->skstruct.GrHandle[GrH_BackBMP] >= 0;
	song.isBanner = g->skstruct.GrHandle[GrH_Banner] >= 0;
	song.grHandle = g->skstruct.GrHandle[GrH_Banner];
	g->procSelecter = 3;
	g->procPhase = 1;
	ProcS_Select(g);
	// ProcS_Select does not change the scene selector, but keep this explicit
	// so future LR2-side changes cannot turn a DECIDE preview back into SELECT.
	g->procSelecter = 3;
}

struct LR2SEPlayModeInfo {
	int keymode;
	bool battle;
	const char* samplePath;
};

static LR2SEPlayModeInfo LR2SEGetPlayModeInfo(int type) {
	switch (type) {
	case SKINTYPE_5KEYS:
		return { 5, false, "LR2files\\Config\\sample_5.bme" };
	case SKINTYPE_9KEYS:
		return { 9, false, "LR2files\\Config\\sample_9.pms" };
	case SKINTYPE_10KEYS:
		return { 10, false, "LR2files\\Config\\sample_10.bme" };
	case SKINTYPE_14KEYS:
		return { 14, false, "LR2files\\Config\\sample_14.bme" };
	case SKINTYPE_5KEYSBATTLE:
		return { 5, true, "LR2files\\Config\\sample_5.bme" };
	case SKINTYPE_7KEYSBATTLE:
		return { 7, true, "LR2files\\Config\\sample_7.bme" };
	case SKINTYPE_9KEYSBATTLE:
		return { 9, true, "LR2files\\Config\\sample_9.pms" };
	case SKINTYPE_7KEYS:
	default:
		return { 7, false, "LR2files\\Config\\sample_7.bme" };
	}
}

static int LR2SEPreviewLaneAt(int type, int ordinal) {
	const LR2SEPlayModeInfo mode = LR2SEGetPlayModeInfo(type);
	int keysPerPlayer = mode.keymode;
	if (keysPerPlayer == 10) keysPerPlayer = 5;
	else if (keysPerPlayer == 14) keysPerPlayer = 7;
	const bool hasScratch = keysPerPlayer != 9;
	const int lanesPerPlayer = keysPerPlayer + (hasScratch ? 1 : 0);
	const int playerCount = mode.battle || mode.keymode == 10 || mode.keymode == 14 ? 2 : 1;
	if (ordinal < 0 || ordinal >= lanesPerPlayer * playerCount) return -1;
	const int player = ordinal / lanesPerPlayer;
	const int lane = ordinal % lanesPerPlayer;
	return player * 10 + lane + (hasScratch ? 0 : 1);
}

int LR2SEBuildPreviewChart(int type, LR2SEPreviewChartMode chartMode,
	LR2SEPreviewChartNote* notes, int capacity) {
	if (!notes || capacity <= 0) return 0;
	int lanes[20] = {};
	int laneCount = 0;
	for (int ordinal = 0; ordinal < 20; ++ordinal) {
		const int lane = LR2SEPreviewLaneAt(type, ordinal);
		if (lane < 0) break;
		lanes[laneCount++] = lane;
	}
	if (laneCount <= 0) return 0;

	constexpr unsigned int firstNoteMs = 2200;
	const bool full = chartMode == LR2SE_PREVIEW_CHART_FULL;
	const int pulseCount = full ? 160 : 16;
	const unsigned int pulseIntervalMs = full ? 180U : 1800U;
	int count = 0;
	for (int pulse = 0; pulse < pulseCount; ++pulse) {
		const unsigned int timing = firstNoteMs + pulse * pulseIntervalMs;
		const int ordinal = (pulse * 5 + pulse / laneCount) % laneCount;
		LR2SEPreviewNoteKind kind = LR2SE_PREVIEW_NOTE_NORMAL;
		if (full ? pulse % 19 == 18 : pulse == 15)
			kind = LR2SE_PREVIEW_NOTE_MINE;
		else if (full ? pulse % 13 == 12 : pulse == 10)
			kind = LR2SE_PREVIEW_NOTE_LONG;
		if (count >= capacity) return count;
		notes[count++] = {
			lanes[ordinal], timing,
			kind == LR2SE_PREVIEW_NOTE_LONG ? timing + 450U : 0U,
			kind
		};

		// Periodic two-note chords exercise simultaneous lane explosions while
		// preserving ascending time order within every LR2 LaneStruct.
		if (full && pulse % 8 == 4 && laneCount > 1) {
			if (count >= capacity) return count;
			notes[count++] = {
				lanes[(ordinal + laneCount / 2) % laneCount], timing, 0U,
				LR2SE_PREVIEW_NOTE_NORMAL
			};
		}
	}
	return count;
}

static constexpr double LR2SEPreviewBpm = 150.0;
static constexpr unsigned int LR2SEPreviewChartDurationMs = 34000U;
static constexpr unsigned int LR2SEPreviewMeasureIntervalMs =
	(unsigned int)(60000.0 * 4.0 / LR2SEPreviewBpm);
static constexpr float LR2SEStaticNormalSampleFractions[] = {
	0.20f, 0.47f, 0.74f
};
static constexpr float LR2SEStaticLongNoteNearFraction = 0.42f;
static constexpr float LR2SEStaticLongNoteFarFraction = 0.68f;

bool LR2SEShouldDrawStaticNormalSample(int sampleIndex,
	bool longNoteVisible) {
	if (sampleIndex < 0 || sampleIndex >=
		(int)(sizeof(LR2SEStaticNormalSampleFractions) /
			sizeof(LR2SEStaticNormalSampleFractions[0])))
		return false;
	if (!longNoteVisible) return true;
	const float fraction = LR2SEStaticNormalSampleFractions[sampleIndex];
	return fraction < LR2SEStaticLongNoteNearFraction ||
		fraction > LR2SEStaticLongNoteFarFraction;
}

int LR2SEBuildPreviewTimeline(LR2SEPreviewTimelineEvent* events, int capacity) {
	if (!events || capacity <= 0) return 0;
	int count = 0;
	for (unsigned int timing = 0; timing < LR2SEPreviewChartDurationMs;
		timing += LR2SEPreviewMeasureIntervalMs) {
		if (count >= capacity) return count;
		events[count++] = { timing, 2, 0.0, false };
	}

	// ProcGame examines the next event before checking whether the queue has
	// ended. Keep a valid event at the exact loop boundary; LR2SESceneProc
	// restarts the chart before this sentinel can be consumed.
	if (count < capacity) {
		events[count++] = {
			LR2SEPreviewChartDurationMs, 2, 0.0, true
		};
	}
	return count;
}

static bool LR2SEAppendPreviewTimelineEvent(LaneStruct& lane,
	const LR2SEPreviewTimelineEvent& event) {
	if (!lane.notes || lane.size <= 0)
		InitNoteBuffer(&lane, 100);
	if (lane.count >= lane.size)
		ExpandNoteBuffer(&lane, 100);
	if (!lane.notes || lane.count >= lane.size) return false;

	NoteStruct& note = lane.notes[lane.count++];
	note.bmsTiming = event.timingMs;
	note.realTiming = event.timingMs;
	note.val = event.value;
	note.active = -1;
	note.lnHeadFast = false;
	note.bmsTiming_ln = -1.0;
	note.realTiming_ln = -1.0;
	note.op = event.op;
	note.mine = -1;
	note.stage = 0;
	return true;
}

static int LR2SEPopulatePreviewChart(game* g, int type,
	LR2SEPreviewChartMode chartMode) {
	if (!g) return -1;
	LR2SEPreviewChartNote chart[256] = {};
	const int chartCount = LR2SEBuildPreviewChart(type, chartMode, chart,
		(int)(sizeof(chart) / sizeof(chart[0])));
	if (chartCount <= 0) return -1;

	gameplay& preview = g->gameplay;
	if (preview.bpmt_buffersize < 2 || !preview.bpmt_data) {
		BPMtiming* timing = (BPMtiming*)realloc(preview.bpmt_data,
			sizeof(BPMtiming) * 2);
		if (!timing) return -1;
		preview.bpmt_data = timing;
		preview.bpmt_buffersize = 2;
	}
	preview.bpmt_count = 2;
	preview.bpmt_start = 1;
	preview.bpmt_data[0] = { 0.0, LR2SEPreviewBpm, 0.0 };
	preview.bpmt_data[1] = {
		(double)LR2SEPreviewChartDurationMs, LR2SEPreviewBpm,
		(double)LR2SEPreviewChartDurationMs
	};
	preview.BPM_fix = LR2SEPreviewBpm;
	preview.BPM = LR2SEPreviewBpm;
	preview.minBPM = LR2SEPreviewBpm;
	preview.maxBPM = LR2SEPreviewBpm;
	preview.song_runtime = LR2SEPreviewChartDurationMs;
	preview.keymode = LR2SEGetPlayModeInfo(type).keymode;

	for (int laneIndex = 0; laneIndex < 20; ++laneIndex) {
		LaneStruct& lane = preview.bmsobj_note[laneIndex];
		lane.count = 0;
		lane.note_count = 0;
		lane.draw_count = 0;
		lane.noteVal = -1;
		lane.autoplay = 0;
	}

	int totalNotes[2] = {};
	const LR2SEPlayModeInfo playMode = LR2SEGetPlayModeInfo(type);
	for (int index = 0; index < chartCount; ++index) {
		const LR2SEPreviewChartNote& sample = chart[index];
		LaneStruct& lane = preview.bmsobj_note[sample.lane];
		if (!lane.notes || lane.size <= 0)
			InitNoteBuffer(&lane, 100);
		if (lane.count >= lane.size)
			ExpandNoteBuffer(&lane, 100);

		NoteStruct& note = lane.notes[lane.count++];
		note.bmsTiming = sample.timingMs;
		note.realTiming = sample.timingMs;
		note.val = 0.0;
		note.active = -1;
		note.lnHeadFast = false;
		note.bmsTiming_ln = sample.kind == LR2SE_PREVIEW_NOTE_LONG
			? (double)sample.endTimingMs : -1.0;
		note.realTiming_ln = note.bmsTiming_ln;
		note.op = 10 + sample.lane;
		note.mine = sample.kind == LR2SE_PREVIEW_NOTE_MINE ? 1 : -1;
		note.stage = 0;

		if (sample.kind != LR2SE_PREVIEW_NOTE_MINE) {
			const int player = playMode.battle && sample.lane >= 10 ? 1 : 0;
			totalNotes[player]++;
		}
	}

	LR2SEPreviewTimelineEvent timeline[32] = {};
	const int timelineCount = LR2SEBuildPreviewTimeline(timeline,
		(int)(sizeof(timeline) / sizeof(timeline[0])));
	if (timelineCount <= 0 || !timeline[timelineCount - 1].terminal)
		return -1;

	// ProcGame consumes bmsobj. bmsobj_line is only the visual copy used by
	// DrawNotes, so populating it alone leaves Rhythm 140 permanently inactive.
	preview.bmsobj.count = 0;
	preview.bmsobj.note_count = 0;
	preview.bmsobj.draw_count = 0;
	preview.bmsobj_line.count = 0;
	preview.bmsobj_line.note_count = 0;
	preview.bmsobj_line.draw_count = 0;
	for (int index = 0; index < timelineCount; ++index) {
		const LR2SEPreviewTimelineEvent& event = timeline[index];
		if (!LR2SEAppendPreviewTimelineEvent(preview.bmsobj, event))
			return -1;
		if (!event.terminal && g->skstruct.dst_LINE[0].dstCount > 0 &&
			!LR2SEAppendPreviewTimelineEvent(preview.bmsobj_line, event))
			return -1;
	}

	for (int player = 0; player < 2; ++player) {
		preview.player[player].totalnotes = totalNotes[player];
		preview.player[player].total_note = totalNotes[player];
		preview.player[player].recent_judge = 5;
		preview.player[player].judge_draw = 5;
		preview.player[player].combo_draw = 0;
		preview.player[player].flag_active = player == 0 || playMode.battle;
	}
	// LoadBmsResource normally owns this initialization.  The preview builds
	// the same gameplay data in memory, so prepare the judge history before
	// DrawNotes' autoplay path reaches ApplyJudgeNote for the first note.
	preview.p1Score.InitJudgeQueue();
	preview.p1Score.ResetJudgeQueue((std::max)(2, totalNotes[0] * 2));
	return 0;
}

static bool LR2SEPreviewFileExists(const char* path) {
	if (!path || !*path) return false;
	FILE* file = fopen(path, "rb");
	if (!file) return false;
	fclose(file);
	return true;
}

static int LR2SEPopulateSamplePreviewChart(game* g, int type) {
	if (!g) return -1;
	const LR2SEPlayModeInfo playMode = LR2SEGetPlayModeInfo(type);
	if (!LR2SEPreviewFileExists(playMode.samplePath)) return -1;

	ConfigStruct sampleConfig = g->config;
	sampleConfig.play.battle = playMode.battle ? 1 : 0;
	// The bundled sample charts contain no keysounds. Keep the loader silent and
	// synchronous so Simple mode does not depend on FMOD or a worker thread.
	sampleConfig.sound.disabledsp = 1;
	sampleConfig.system.isablebmsthread = 0;

	SONGDATA& song = g->sSelect.bmsList[g->sSelect.cur_song];
	song.filepath = playMode.samplePath;
	song.keymode = playMode.keymode;
	g->sSelect.metaSelected.filepath = playMode.samplePath;
	g->sSelect.metaSelected.keymode = playMode.keymode;

	InitGameplay(&g->gameplay, &sampleConfig.play);
	g->gameplay.isAutoplay = 1;
	const int scratchSide = g->skstruct.scratchside_1 +
		g->skstruct.scratchside_2 * 2;
	if (ParseBmsFile(&g->gameplay, playMode.samplePath, &g->audio,
		&sampleConfig, &g->sSelect.metaSelected, 1, scratchSide) < 0)
		return -1;
	// LR2's normal synchronous completion returns 0 here; several intentional
	// early-completion paths return 1. Neither value means that parsing failed.
	LoadBmsResource(&g->gameplay, playMode.samplePath, &g->audio,
		&sampleConfig, &g->sSelect.metaSelected, 1, scratchSide, 0);
	g->gameplay.bmsResourceLoaded = 1;
	return 0;
}

static void LR2SEInitPlayPreviewState(game* g, int type) {
	const LR2SEPlayModeInfo mode = LR2SEGetPlayModeInfo(type);
	// A PLAY skin normally inherits this information from SELECT.  Populate it
	// explicitly because the editor can open every PLAY skin in isolation.
	LR2SEInitSelectPreviewState(g);
	SONGDATA& song = g->sSelect.bmsList[g->sSelect.cur_song];
	song.filepath = "SkinEditor://preview-chart";
	song.keymode = mode.keymode;
	g->sSelect.metaSelected.title = song.title;
	g->sSelect.metaSelected.subtitle = song.subtitle;
	g->sSelect.metaSelected.artist = song.artist;
	g->sSelect.metaSelected.subartist = song.subartist;
	g->sSelect.metaSelected.genre = song.genre;
	g->sSelect.metaSelected.filepath = song.filepath;
	g->sSelect.metaSelected.keymode = mode.keymode;
	g->sSelect.metaSelected.minbpm = song.minBPM;
	g->sSelect.metaSelected.maxbpm = song.maxBPM;
	g->sSelect.metaSelected.difficulty = song.difficulty;
	g->sSelect.metaSelected.selLevel = song.level;
	g->sSelect.metaSelected.exlevel = song.exlevel;
	g->sSelect.metaSelected.longnote = song.longnote;
	g->sSelect.metaSelected.bga = song.bga;
	g->config.play.battle = mode.battle ? 1 : 0;
	// Use a practical editor-only lane speed. A zero-initialized config makes
	// notes overlap at the judge line and also displays HI-SPEED as 0.
	g->config.play.hiSpeed[0] = 200;
	g->config.play.hiSpeed[1] = 200;
	// ReactInput clamps HI-SPEED every tick. The editor does not load LR2's
	// player config, so supply the same safe defaults used by ReadConfig;
	// otherwise zero-initialized bounds immediately clamp 200 back to 0.
	g->config.play.hsmin = 10;
	g->config.play.hsmax = 900;
	g->config.play.hsmargin = 10;
	g->config.play.basespeed = 100;
	// Preview interaction belongs to the editor.  Do not let ProcI_Play treat
	// an editor right-click as LR2's gameplay-exit shortcut.
	g->config.play.disableleftclickexit = 1;
	g->gameplay.isAutoplay = 1;
	g->gameplay.song_runtime = 120000.0;
	g->gameplay.loadObject_total = 100;
	g->gameplay.loadObject_loaded = 100;
	g->procSelecter = 4;
	g->procPhase = 1;
}

static void LR2SEInitResultPreviewState(game* g, bool courseResult) {
	// RESULT normally inherits both the selected song and the finished PLAY
	// state. Build both explicitly so it can be opened as a standalone skin.
	LR2SEInitSelectPreviewState(g);
	SONGDATA& song = g->sSelect.bmsList[g->sSelect.cur_song];
	song.keymode = 7;
	song.level = 12;
	song.difficulty = 3;
	song.mybest.total_notes = 1000;
	song.mybest.stat_pgreat = 680;
	song.mybest.stat_great = 190;
	song.mybest.stat_good = 65;
	song.mybest.stat_bad = 25;
	song.mybest.stat_poor = 40;
	song.mybest.stat_exscore = 1550;
	song.mybest.stat_score = 142000;
	song.mybest.stat_maxcombo = 530;
	song.mybest.minbp = 65;
	song.mybest.rate = 77;
	song.mybest.rank = 4;
	song.mybest.clear = 2;
	song.mybest.playcount = 21;
	song.mybest.clearcount = 15;
	song.mybest.failcount = 6;

	g->sSelect.metaSelected.keymode = song.keymode;
	g->sSelect.metaSelected.selLevel = song.level;
	g->sSelect.metaSelected.difficulty = song.difficulty;
	g->config.play.battle = 0;
	g->config.play.gaugeOption[0] = 0;
	g->config.play.gaugeOption[1] = 0;
	g->gameplay.isCourse = courseResult ? 1 : 0;
	g->gameplay.courseType = courseResult ? 0 : -1;

	for (int p = 0; p < 2; ++p) {
		PLAYERSTATUS& player = g->gameplay.player[p];
		player.totalnotes = courseResult ? 2400 : 1000;
		player.judgecount[5] = courseResult ? 1680 : 720;
		player.judgecount[4] = courseResult ? 480 : 190;
		player.judgecount[3] = courseResult ? 120 : 50;
		player.judgecount[2] = courseResult ? 55 : 18;
		player.judgecount[1] = courseResult ? 45 : 12;
		player.judgecount[0] = courseResult ? 20 : 10;
		player.exscore = player.judgecount[5] * 2 + player.judgecount[4];
		player.score = courseResult ? 189500 : 176400;
		player.max_combo = courseResult ? 1320 : 742;
		player.now_combo = player.max_combo;
		player.note_current = player.totalnotes;
		player.clearType = p == 0 ? 3 : 2;
		player.rate = player.exscore * 100.0 / (player.totalnotes * 2.0);
		player.HP = p == 0 ? 86.0 : 72.0;
		player.HP_print = player.HP;
		player.score_print = player.score;

		for (int i = 0; i < 1000; ++i) {
			const double progress = i / 999.0;
			g->gameplay.statgraph[p].combo[i] = (int)(player.max_combo * progress);
			g->gameplay.statgraph[p].exscore[i] = (int)(player.exscore * progress);
			int hp = (int)(28.0 + progress * (player.HP - 28.0) + sin(progress * 18.0 + p) * 8.0);
			if (hp < 2) hp = 2;
			if (hp > 100) hp = 100;
			g->gameplay.statgraph[p].hp[i] = hp;
		}
		g->gameplay.statgraph[p].cursor = 1000;
	}
	for (int i = 0; i < 1000; ++i) {
		g->gameplay.rategraph[0].val[i] = g->gameplay.statgraph[0].exscore[i] * 7 / 8;
		g->gameplay.rategraph[1].val[i] = g->gameplay.statgraph[0].exscore[i] * 3 / 4;
	}
	g->gameplay.rategraph[0].cursor = 1000;
	g->gameplay.rategraph[1].cursor = 1000;
	g->procSelecter = 5;
	g->procPhase = 1;
}

void LR2SEPreparePreviewState(game* g, int type) {
	if (!g) return;
	InitTimer(&g->timer1);
	InitTimer(&g->timer2);
	SetTimeLapse(0, &g->timer1);
	SetTimeLapse(0, &g->timer2);
	switch (type) {
	case SKINTYPE_SELECT:
		LR2SEInitSelectPreviewState(g);
		break;
	case SKINTYPE_DECIDE:
		LR2SEInitDecidePreviewState(g);
		break;
	case SKINTYPE_5KEYS:
	case SKINTYPE_7KEYS:
	case SKINTYPE_9KEYS:
	case SKINTYPE_10KEYS:
	case SKINTYPE_14KEYS:
	case SKINTYPE_5KEYSBATTLE:
	case SKINTYPE_7KEYSBATTLE:
	case SKINTYPE_9KEYSBATTLE:
		LR2SEInitPlayPreviewState(g, type);
		break;
	case SKINTYPE_RESULT:
		LR2SEInitResultPreviewState(g, false);
		break;
	case SKINTYPE_COURSERESULT:
		LR2SEInitResultPreviewState(g, true);
		break;
	case SKINTYPE_KEYCONFIG:
		LR2SEInitKeyconfigPreviewState(g);
		break;
	case SKINTYPE_SKINSELECT:
		LR2SEInitSkinSelectPreviewState(g);
		break;
	default:
		break;
	}
}

int LR2SEInit(game* g, bool initializeCore) {
	// These buffers belong to game, which is owned by one WORKSPACE. Reloading
	// the same workspace must reuse them, while a newly-created workspace must
	// initialize its own copy. A process-wide flag left the second workspace's
	// CSTR and list storage uninitialized and crashed during its first load.
	if (initializeCore) {
		InitBmsList(&g->sSelect);
		InitObjectString(&g->txtStruct);
		InitGameplay(&g->gameplay, &g->config.play);
		InitializeCriticalSection(&g->gameplay.criticalSection);
		InitializeCriticalSection(&g->criticalSection);
	}


	DeleteGraph(g->skstruct.GrHandle[GrH_BackBMP]); 
	DeleteGraph(g->skstruct.GrHandle[GrH_Preview]);
	DeleteGraph(g->skstruct.GrHandle[GrH_Stage]);
	DeleteGraph(g->skstruct.GrHandle[GrH_Banner]);
	g->skstruct.GrHandle[GrH_BackBMP] = LoadGraph("LR2files/Config/white.bmp");
	g->skstruct.GrHandle[GrH_Preview] = LoadGraph("LR2files/Config/white.bmp");
	g->skstruct.GrHandle[GrH_Stage] = LoadGraph("LR2files/Config/title.bmp");
	g->skstruct.GrHandle[GrH_Banner] = LoadGraph("LR2files/Config/white.bmp");

	//LoadGraph("LR2files/Config/loading.bmp", 0);


	return 0;
}

int LR2SEDrawLoop(game* g, int gHandle, int sizeX, int sizeY, bool staticSpecialPreview) {
	SetDrawScreen(DX_SCREEN_BACK);
	// RESULT charts are not generic positioned images. Their width, reveal
	// timing and vertical samples are calculated from the finished score data.
	// Run LR2's real chart renderer even before a scene restart so opening a result
	// skin immediately shows the same graph structure as the live scene.
    if (g->procSelecter == 5) {
        g_previewRenderStage = "RESULT";
        if (staticSpecialPreview) {
            // Keep LR2's real result graph, but isolate the renderer's 150/151
            // timer transitions so a passive editor preview cannot advance
            // TEXT objects into their exit animation.
            Timer resultPreviewTimer = g->timer1;
            Proc_Result(g, &g->skstruct, &resultPreviewTimer);
        }
        else {
            Proc_Result(g, &g->skstruct, &g->timer1);
        }
	}

	g_previewRenderStage = "IMAGE";
	for (int i = 0; i < g->skstruct.image.srcSize; i++) {
		g_previewRenderIndex = i;

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

	g_previewRenderStage = "BUTTON";
	for (int i = 0; i < g->skstruct.otherObject[1].srcSize; i++) {
		g_previewRenderIndex = i;
		if (GetOptionFlag_dst(g, g->skstruct.otherObject[1].dst[i].op1) && GetOptionFlag_dst(g, g->skstruct.otherObject[1].dst[i].op2)
			&& GetOptionFlag_dst(g, g->skstruct.otherObject[1].dst[i].op3) && g->skstruct.adjust.dark_type != 2) {

			AddDrawingBuffer_Image(&g->skstruct.drBuf, &g->skstruct.otherObject[1].src[i], &g->skstruct.otherObject[1].dst[i], &g->timer1);

		}
	}
	g_previewRenderStage = "SLIDER";
	for (int i = 0; i < g->skstruct.otherObject[2].srcSize; i++) {
		g_previewRenderIndex = i;
		if (GetOptionFlag_dst(g, g->skstruct.otherObject[2].dst[i].op1) && GetOptionFlag_dst(g, g->skstruct.otherObject[2].dst[i].op2)
			&& GetOptionFlag_dst(g, g->skstruct.otherObject[2].dst[i].op3) && g->skstruct.adjust.dark_type != 2) {

			AddDrawingBuffer_Slider(&g->skstruct.drBuf, &g->skstruct.otherObject[2].src[i], &g->skstruct.otherObject[2].dst[i], &g->timer1);

		}
	}
	g_previewRenderStage = "NUMBER";
	for (int i = 0; i < g->skstruct.otherObject[6].srcSize; i++) {
		g_previewRenderIndex = i;
		if (GetOptionFlag_dst(g, g->skstruct.otherObject[6].dst[i].op1) && GetOptionFlag_dst(g, g->skstruct.otherObject[6].dst[i].op2)
			&& GetOptionFlag_dst(g, g->skstruct.otherObject[6].dst[i].op3) && g->skstruct.adjust.dark_type != 2) {

			AddDrawingBuffer_Numbers(&g->skstruct.drBuf, &g->skstruct.otherObject[6].src[i], &g->skstruct.otherObject[6].dst[i], &g->timer1, SetObjectValue_Num(g, g->skstruct.otherObject[6].src[i].op1), 0, 0);

		}
	}
	if (g->txtStruct.readme.show != true) {
		g_previewRenderStage = "ONMOUSE";
		for (int i = 0; i < g->skstruct.otherObject[3].srcSize; i++) {
			g_previewRenderIndex = i;
			if (GetOptionFlag_dst(g, g->skstruct.otherObject[3].dst[i].op1) && GetOptionFlag_dst(g, g->skstruct.otherObject[3].dst[i].op2)
				&& GetOptionFlag_dst(g, g->skstruct.otherObject[3].dst[i].op3) && g->skstruct.adjust.dark_type != 2) {

				if (staticSpecialPreview)
					AddDrawingBuffer_Image(&g->skstruct.drBuf, &g->skstruct.otherObject[3].src[i],
						&g->skstruct.otherObject[3].dst[i], &g->timer1);
				else
					AddDrawingBuffer_OnMouse(&g->skstruct.drBuf, &g->skstruct.otherObject[3].src[i],
						&g->skstruct.otherObject[3].dst[i], &g->timer1, &g->KeyInput, g->sSelect.panel);

			}
		}
	}
	if (g->skstruct.adjust.dark_type != 2) {
		// Keep the preview's common-object pass in sync with LR2's main draw
		// loop.  TEXT objects were parsed correctly but never entered into the
		// drawing buffer here, so they were absent from the editor preview.
		g_previewRenderStage = "TEXT";
		g_previewRenderIndex = -1;
		// The current implementation does not dereference its legacy database
		// parameter; the editor does not own LR2's global sqlite connection.
		Proc_Text(g, nullptr, 0);
		if (g->skstruct.adjust.dark_type != 2) {
			g_previewRenderStage = "BARGRAPH";
			SetObjectValue_Bargraph(g);
		}
	}
	else if (staticSpecialPreview) {
		// BGA images normally come from the selected chart. In layout mode use a
		// neutral image, but keep LR2's draw order (behind controls and text).
		const int layoutSampleHandle = g->skstruct.GrHandle[GrH_Banner];
		if (layoutSampleHandle >= 0) {
			for (int i = 0; i < g->skstruct.otherObject[4].srcSize; ++i) {
				DSTstruct& bgaDst = g->skstruct.otherObject[4].dst[i];
				if (bgaDst.dstCount > 0 && bgaDst.dataSize > 0 && bgaDst.draw &&
					GetOptionFlag_dst(g, bgaDst.op1) && GetOptionFlag_dst(g, bgaDst.op2) &&
					GetOptionFlag_dst(g, bgaDst.op3))
					AddDrawingBuffer_EventLoading(&g->skstruct.drBuf, layoutSampleHandle,
						&bgaDst, &g->timer1, 0, 0);
			}
		}
	}
	g_previewRenderStage = "MASK";
	for (int i = 0; i < g->skstruct.otherObject[7].srcSize; ++i) {
		g_previewRenderIndex = i;
		DSTstruct& dst = g->skstruct.otherObject[7].dst[i];
		if (dst.dstCount && GetOptionFlag_dst(g, dst.op1) &&
			GetOptionFlag_dst(g, dst.op2) && GetOptionFlag_dst(g, dst.op3))
			AddDrawingBuffer_Image2(&g->skstruct.drBuf,
				&g->skstruct.otherObject[7].src[i], &dst, &g->timer1);
	}

	// Scene processors draw these objects from live song/result data.  Before
	// Start is pressed there is no live scene, but an editor still needs a
	// useful representation of every positioned special object.  Draw one
	// deterministic sample for each valid declaration in that state.
	if (staticSpecialPreview && g->skstruct.adjust.dark_type != 2) {
		auto visible = [&](DSTstruct& dst) {
			return dst.dstCount > 0 && dst.dataSize > 0 && dst.draw != nullptr
				&& GetOptionFlag_dst(g, dst.op1) && GetOptionFlag_dst(g, dst.op2)
				&& GetOptionFlag_dst(g, dst.op3);
		};
		auto image = [&](SRCstruct& src, DSTstruct& dst) {
			if (src.graphcount > 0 && visible(dst))
				AddDrawingBuffer_Object(&g->skstruct.drBuf, &src, &dst, &g->timer1, 0, 0);
		};
		auto resultChart = [&](SRCstruct& src, DSTstruct& dst, int phase) {
			if (src.graphcount <= 0 || !visible(dst)) return;
			const int chartWidth = src.op1 > 0 ? src.op1 : (int)dst.draw[0].w;
			int step = (int)fabs(dst.draw[0].w);
			if (step < 1) step = 1;
			if (chartWidth <= 0) {
				AddDrawingBuffer_Object(&g->skstruct.drBuf, &src, &dst, &g->timer1, 0, 0);
				return;
			}
			// Deterministic editor data: a smooth rise with a small wave makes both
			// the declared chart extent and vertical scale visible without a score.
			for (int x = 0; x < chartWidth; x += step) {
				const double progress = chartWidth > 1 ? x / (double)(chartWidth - 1) : 0.0;
				double value = 18.0 + progress * 72.0 + sin(progress * 6.28318530718 + phase) * 8.0;
				if (value < 0.0) value = 0.0;
				if (value > 100.0) value = 100.0;
				const int y = (int)(src.op2 * value / -100.0);
				AddDrawingBuffer_Object(&g->skstruct.drBuf, &src, &dst, &g->timer1, x, y);
			}
		};
		auto resultGaugeChart = [&](SRCstruct* src, DSTstruct* dst, int phase) {
			if (src[0].graphcount <= 0 || !visible(dst[0])) return;
			const int chartWidth = src[0].op1 > 0
				? src[0].op1 : (int)dst[0].draw[0].w;
			int step = (int)fabs(dst[0].draw[0].w);
			if (step < 1) step = 1;
			if (chartWidth <= 0) return;
			for (int x = 0; x < chartWidth; x += step) {
				const double progress = chartWidth > 1
					? x / (double)(chartWidth - 1) : 0.0;
				double value = 18.0 + progress * 72.0 +
					sin(progress * 6.28318530718 + phase) * 8.0;
				if (value < 0.0) value = 0.0;
				if (value > 100.0) value = 100.0;
				// LR2 uses slot 0 below the normal-gauge clear border and
				// slot 1 at/above 80%.  Mirror that rule in the editor so a
				// missing second declaration is immediately visible.
				const int sourceIndex = value >= 80.0 &&
					src[1].graphcount > 0 && visible(dst[1]) ? 1 : 0;
				const int y = (int)(src[sourceIndex].op2 * value / -100.0);
				AddDrawingBuffer_Object(&g->skstruct.drBuf, &src[sourceIndex],
					&dst[sourceIndex], &g->timer1, x, y);
			}
		};

		g_previewRenderStage = "SPECIAL_STATIC";
		for (int i = 0; i < 2; ++i) {
			g_previewRenderIndex = i;
			image(g->skstruct.src_LINE[i], g->skstruct.dst_LINE[i]);
			image(g->skstruct.src_JUDGELINE[i], g->skstruct.dst_JUDGELINE[i]);
			if (g->skstruct.src_GROOVEGAUGE[i].graphcount > 0 && visible(g->skstruct.dst_GROOVEGAUGE[i]))
				AddDrawingBuffer_Gauge(&g->skstruct.drBuf, &g->skstruct.src_GROOVEGAUGE[i],
					&g->skstruct.dst_GROOVEGAUGE[i], &g->timer1, 50, 0);
		}
		if (g->procSelecter != 5) {
			resultGaugeChart(g->skstruct.src_GAUGECHART_1P,
				g->skstruct.dst_GAUGECHART_1P, 0);
			resultGaugeChart(g->skstruct.src_GAUGECHART_2P,
				g->skstruct.dst_GAUGECHART_2P, 2);
			for (int i = 0; i < 3; ++i)
				resultChart(g->skstruct.src_SCORECHART[i], g->skstruct.dst_SCORECHART[i], i + 4);
		}

		// Judge/combo slots describe alternate states at the same location.
		// Pick the first valid slot so the preview does not stack all states.
		auto judgeCombo = [&](SRCstruct* judgeSrc, DSTstruct* judgeDst,
			SRCstruct* comboSrc, DSTstruct* comboDst) {
			for (int i = 0; i < 6; ++i) {
				if (judgeSrc[i].graphcount > 0 && visible(judgeDst[i])) {
					AddDrawingBuffer_Object(&g->skstruct.drBuf, &judgeSrc[i], &judgeDst[i], &g->timer1, 0, 0);
					break;
				}
			}
			for (int i = 0; i < 6; ++i) {
				if (comboSrc[i].graphcount > 0 && visible(comboDst[i])) {
					AddDrawingBuffer_Numbers(&g->skstruct.drBuf, &comboSrc[i], &comboDst[i], &g->timer1, 123, 0, 0);
					break;
				}
			}
		};
		judgeCombo(g->skstruct.src_NOWJUDGE_1P, g->skstruct.dst_NOWJUDGE_1P,
			g->skstruct.src_NOWCOMBO_1P, g->skstruct.dst_NOWCOMBO_1P);
		judgeCombo(g->skstruct.src_NOWJUDGE_2P, g->skstruct.dst_NOWJUDGE_2P,
			g->skstruct.src_NOWCOMBO_2P, g->skstruct.dst_NOWCOMBO_2P);

		// A note lane has one shared DST and several source variants. Lay out a
		// small deterministic chart so lane direction, spacing, mines and long
		// notes can be checked without loading a BMS file.
		for (int i = 0; i < 20; ++i) {
			g_previewRenderIndex = i;
			SRCstruct* laneSample = &g->skstruct.src_NOTE[i];
			if (laneSample->graphcount <= 0) laneSample = &g->skstruct.src_AUTO_NOTE[i];
			if (laneSample->graphcount <= 0) laneSample = &g->skstruct.src_LN_START[i];
			if (laneSample->graphcount <= 0) laneSample = &g->skstruct.src_AUTO_LN_START[i];
			if (laneSample->graphcount <= 0 || !visible(g->skstruct.dst_NOTE[i])) continue;
			// DST_NOTE is anchored at the judgement end of the lane.  Spread the
			// editor samples over the distance from that anchor to the entrance of
			// the lane; a fixed fraction of the whole screen left HD notes bunched
			// together near the bottom.
			const float laneAnchor = g->skstruct.horizontal
				? (float)fabs(g->skstruct.dst_NOTE[i].draw->x)
				: (float)fabs(g->skstruct.dst_NOTE[i].draw->y);
			const float screenAxis = (float)(g->skstruct.horizontal ? sizeX : sizeY);
			const float laneTravel = laneAnchor > screenAxis * 0.35f
				? laneAnchor : screenAxis * 0.75f;
			SRCstruct* lnStart = g->skstruct.src_LN_START[i].graphcount > 0
				? &g->skstruct.src_LN_START[i] : &g->skstruct.src_AUTO_LN_START[i];
			SRCstruct* lnEnd = g->skstruct.src_LN_END[i].graphcount > 0
				? &g->skstruct.src_LN_END[i] : &g->skstruct.src_AUTO_LN_END[i];
			SRCstruct* lnBody = g->skstruct.src_LN_BODY[i].graphcount > 0
				? &g->skstruct.src_LN_BODY[i] : &g->skstruct.src_AUTO_LN_BODY[i];
			const bool longNoteVisible = !g->skstruct.horizontal &&
				lnStart->graphcount > 0 && lnEnd->graphcount > 0 &&
				lnBody->graphcount > 0;
			for (int sample = 0; sample <
				(int)(sizeof(LR2SEStaticNormalSampleFractions) /
					sizeof(LR2SEStaticNormalSampleFractions[0])); ++sample) {
				if (!LR2SEShouldDrawStaticNormalSample(sample, longNoteVisible))
					continue;
				const float shift = -laneTravel *
					LR2SEStaticNormalSampleFractions[sample];
				AddDrawingBuffer_PlayArea(&g->skstruct.drBuf, laneSample,
					&g->skstruct.dst_NOTE[i], &g->timer1,
					g->skstruct.horizontal ? shift : 0.0f,
					g->skstruct.horizontal ? 0.0f : shift, 255, 0, 0, 1);
			}

			SRCstruct* mine = g->skstruct.src_MINE[i].graphcount > 0
				? &g->skstruct.src_MINE[i] : &g->skstruct.src_AUTO_MINE[i];
			if (mine->graphcount > 0) {
				const float shift = -laneTravel * 0.92f;
				AddDrawingBuffer_PlayArea(&g->skstruct.drBuf, mine,
					&g->skstruct.dst_NOTE[i], &g->timer1,
					g->skstruct.horizontal ? shift : 0.0f,
					g->skstruct.horizontal ? 0.0f : shift, 255, 0, 0, 1);
			}

			if (longNoteVisible)
				AddDrawingBuffer_LN(&g->skstruct.drBuf, lnStart, lnEnd, lnBody,
					&g->skstruct.dst_NOTE[i], &g->timer1, 0.0f,
					-laneTravel * LR2SEStaticLongNoteNearFraction,
					-laneTravel * LR2SEStaticLongNoteFarFraction, 210, 0, 0);
		}

		g_previewRenderStage = "SELECT_STATIC";
		image(g->skstruct.src_THUMBNAIL, g->skstruct.dst_THUMBNAIL);
		// SELECT bar DSTs describe row anchors.  As in LR2, interpolate adjacent
		// anchors and offset each title/decoration from the resulting row origin.
		// Drawing BAR_TITLE directly at its own DST makes every song overlap.
		if (g->sSelect.bmsListCount > 0) {
			for (int i = 1; i < 30; ++i) {
				// LR2 treats these as positional anchors and does not reject the
				// entire row through the common DST option filter.
				if (g->skstruct.dst_BAR_BODY_OFF[i - 1].dstCount <= 0 ||
					g->skstruct.dst_BAR_BODY_OFF[i].dstCount <= 0) continue;

				DSTdraw rowNow = SetDSTdrawByTime(
					i == g->sSelect.listSelectedBarFromScreenTop
						? g->skstruct.dst_BAR_BODY_ON[i] : g->skstruct.dst_BAR_BODY_OFF[i],
					GetTimeLapse(0, &g->timer1));
				DSTdraw rowPrev = SetDSTdrawByTime(
					i - 1 == g->sSelect.listSelectedBarFromScreenTop
						? g->skstruct.dst_BAR_BODY_ON[i - 1] : g->skstruct.dst_BAR_BODY_OFF[i - 1],
					GetTimeLapse(0, &g->timer1));
				if (rowNow.time == -1 || rowPrev.time == -1) continue;

				DSTdraw row = g->sSelect.listCalculatedBar % 1000 == 0
					? rowNow
					: DSTDbyTime(&rowNow, &rowPrev, 0, 1000.0,
						g->sSelect.listCalculatedBar % 1000);
				int songIndex = g->sSelect.listCalculatedBar / 1000
					- g->skstruct.BAR_CENTER + i;
				while (songIndex < 0) songIndex += g->sSelect.bmsListCount;
				songIndex %= g->sSelect.bmsListCount;
				SONGDATA& song = g->sSelect.bmsList[songIndex];
				const int bodyType = song.folderType >= 0 && song.folderType < 10
					? song.folderType : 0;
				if (g->skstruct.src_BAR_BODY[bodyType].graphcount > 0)
					AddDrawingBuffer_Lunaris(&g->skstruct.drBuf,
						&g->skstruct.src_BAR_BODY[bodyType], &row, &g->timer1);

				if (i == g->sSelect.listSelectedBarFromScreenTop &&
					g->skstruct.src_BAR_FLASH.graphcount > 0)
					AddDrawingBuffer_Object(&g->skstruct.drBuf, &g->skstruct.src_BAR_FLASH,
						&g->skstruct.dst_BAR_FLASH, &g->timer1, rowNow.x, rowNow.y);
				if (g->skstruct.src_BAR_TITLE[0].graphcount > 0 &&
					visible(g->skstruct.dst_BAR_TITLE[0]))
					AddDrawingBuffer_TextXY(&g->skstruct.drBuf,
						&g->skstruct.src_BAR_TITLE[0], &g->skstruct.dst_BAR_TITLE[0],
						&g->timer1, songIndex + 10000, row.x, row.y);
				if (song.folderType == 0 && song.difficulty >= 0 && song.difficulty < 5 &&
					g->skstruct.src_BAR_LEVEL[song.difficulty].graphcount > 0 &&
					visible(g->skstruct.dst_BAR_LEVEL[song.difficulty]))
					AddDrawingBuffer_Numbers(&g->skstruct.drBuf,
						&g->skstruct.src_BAR_LEVEL[song.difficulty],
						&g->skstruct.dst_BAR_LEVEL[song.difficulty], &g->timer1,
						song.level, row.x, row.y);
				const int lamp = song.mybest.clear >= 0 && song.mybest.clear < 10
					? song.mybest.clear : 0;
				if (g->skstruct.src_BAR_LAMP[lamp].graphcount > 0 &&
					visible(g->skstruct.dst_BAR_LAMP[lamp]))
					AddDrawingBuffer_Object(&g->skstruct.drBuf,
						&g->skstruct.src_BAR_LAMP[lamp], &g->skstruct.dst_BAR_LAMP[lamp],
						&g->timer1, row.x, row.y);
			}
		}

		// Event-mode cursors have one source and separate ON/OFF destinations.
		// Prefer ON, matching the selected-state preview used for other variants.
		for (int i = 0; i < 10; ++i) {
			DSTstruct* cursorDst = visible(g->skstruct.dst_EVENT_MODE_CURSOR_ON[i])
				? &g->skstruct.dst_EVENT_MODE_CURSOR_ON[i]
				: &g->skstruct.dst_EVENT_MODE_CURSOR_OFF[i];
			image(g->skstruct.src_EVENT_MODE_CURSOR, *cursorDst);
		}

		// These declarations contain only a DST. A neutral image makes their
		// geometry visible without requiring live song/event data.
		const int layoutSampleHandle = g->skstruct.GrHandle[GrH_Banner];
		if (layoutSampleHandle >= 0) {
			if (visible(g->skstruct.dst_BAR_STAGEFILE))
				AddDrawingBuffer_EventLoading(&g->skstruct.drBuf, layoutSampleHandle,
					&g->skstruct.dst_BAR_STAGEFILE, &g->timer1, 0, 0);
			for (int i = 0; i < 5; ++i) {
				if (visible(g->skstruct.dst_EVENT_LOADINGBG[i]))
					AddDrawingBuffer_EventLoading(&g->skstruct.drBuf, layoutSampleHandle,
						&g->skstruct.dst_EVENT_LOADINGBG[i], &g->timer1, 0, 0);
			}
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

int LR2SESceneInit(game *g, int type, LR2SEPreviewChartMode chartMode) {
	
	//g->skstruct.GrHandle[GrH_Stage] = LoadGraph("LR2files\\Config\\title.bmp", 0);

	g_sceneInitStage = "LR2SESceneInit/InitGameplay";
	ReleaseBGA(g);
	InitGameplay(&g->gameplay, &g->config.play);
	g_sceneInitStage = "LR2SESceneInit/PreparePreviewState";
	LR2SEPreparePreviewState(g, type);
	

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
		g_sceneInitStage = "LR2SESceneInit/PreviewChart";
		const LR2SEPlayModeInfo playMode = LR2SEGetPlayModeInfo(type);
		// Simple restores LR2's bundled sample_*.bme/pms preview. Full keeps the
		// denser deterministic chart. A missing sample file falls back to the small
		// built-in chart instead of passing an invalid path to LR2's parser.
		g->gameplay.isAutoplay = 1;
		g->config.play.battle = playMode.battle ? 1 : 0;
		g->config.play.hiSpeed[0] = 200;
		g->config.play.hiSpeed[1] = 200;
		g->config.play.basespeed = 100;
		g->gameplay.bmsResourceLoaded = 1;
		// The editor advances ProcGame synchronously from LR2SESceneProc. Keep
		// this flag clear so LR2SEDrawLoop cannot also run ProcGame once per
		// drawing-buffer entry through LR2's legacy non-threaded path.
		g->gameplay.flag_gameinput = 0;
		g->procSelecter = 4;
		// The editor supplies the selected Preview chart through LR2's normal
		// PLAY renderer. Start the same chart/judge clocks as gameplay.
		SetTimeLapse(40, &g->timer1);
		SetTimeLapse(41, &g->timer1);
		SetTimeLapse(140, &g->timer1);
		SetTimeLapse(142, &g->timer1);
		g->gameplay.timetick = 0;
		const bool sampleLoaded = chartMode == LR2SE_PREVIEW_CHART_SIMPLE &&
			LR2SEPopulateSamplePreviewChart(g, type) == 0;
		if (!sampleLoaded && LR2SEPopulatePreviewChart(g, type, chartMode) != 0)
			return -1;
		// Sample parsing reinitializes gameplay, so restore the editor-owned play
		// flags and timers after either chart source has been populated.
		g->gameplay.isAutoplay = 1;
		g->gameplay.flag_gameinput = 0;
		g->procSelecter = 4;
		SetTimeLapse(40, &g->timer1);
		SetTimeLapse(41, &g->timer1);
		SetTimeLapse(140, &g->timer1);
		SetTimeLapse(142, &g->timer1);
		g->gameplay.timetick = 0;
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

	g->procPhase = 1;
	g_sceneInitStage = "LR2SESceneInit/complete";

	return 0;
}

int LR2SESceneProc(game* g, int type, LR2SEPreviewChartMode chartMode) {

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
		if (g->procSelecter != 3) return -1;
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
		double chartDuration = chartMode == LR2SE_PREVIEW_CHART_FULL
			? (double)LR2SEPreviewChartDurationMs : g->gameplay.song_runtime;
		if (chartDuration < 1000.0) chartDuration = LR2SEPreviewChartDurationMs;
		if (GetTimeLapse(41, &g->timer1) >= chartDuration) {
			g_sceneProcStage = "LR2SESceneProc/RestartPreviewChart";
			if (LR2SESceneInit(g, type, chartMode) != 0) return -1;
		}
		// Advance LR2's event/input path exactly once before drawing. This updates
		// Rhythm 140 from BPM and resets it on each op-2 measure event. The
		// selected Preview chart must then travel
		// through the same DrawNotes -> ApplyJudgeNote -> 50/100/120 timer
		// pipeline as LR2 so key beams, note explosions and judge/combo objects
		// react exactly as they do during real autoplay.
		g_sceneProcStage = "LR2SESceneProc/ProcGame";
		if (ProcGame(g) < 0) return -1;
		g_sceneProcStage = "LR2SESceneProc/ProcI_Play";
		ProcI_Play(g);
	}
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

	g_sceneProcStage = "LR2SESceneProc/complete";
	return 0;
}
