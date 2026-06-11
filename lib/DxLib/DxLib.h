// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ヘッ?フ?イル
// 
// 				Ver 3.25 
// 
// -------------------------------------------------------------------------------

#ifndef DX_LIB_H
#define DX_LIB_H

#include "DxCompileConfig.h"

// ＤＸライブラリのバ?ジョン
#define DXLIB_VERSION 0x3250
#define DXLIB_VERSION_STR_T _T( "3.25 " )
#define DXLIB_VERSION_STR_W    L"3.25 "

// 定?---------------------------------------------------------------------------

// π
#define DX_PI										(3.1415926535897932384626433832795 )
#define DX_PI_F										(3.1415926535897932384626433832795f)
#define DX_TWO_PI									(3.1415926535897932384626433832795  * 2.0 )
#define DX_TWO_PI_F									(3.1415926535897932384626433832795f * 2.0f)

#define DX_CHAR										char

#define DX_DEFINE_START

#define MAX_IMAGE_NUM								(0x40000)			// 同時に持てるグラフィックハンドルの最大数( ハンドルエラ??ェックの?スクに使用しているので 0x40000 以下の 2 のべき乗にして下さい )
#define MAX_IMAGE_DIVNUM							(64)				// 画像分割の最大数
#define MAX_SHADOWMAP_NUM							(8192)				// シャドウ?ップデ??の最大数
#define MAX_SOFTIMAGE_NUM							(8192)				// 同時に持てる?フトイメ?ジハンドルの最大数( ハンドルエラ??ェックの?スクに使用しているので 0x40000 以下の 2 のべき乗にして下さい )

#define MAX_SOUND_NUM								(32768)				// 同時に持てるサウンドハンドルの最大数
#define MAX_SOFTSOUND_NUM							(8192)				// 同時に持てる?フトウエアサウンドハンドルの最大数
#define MAX_MUSIC_NUM								(256)				// 同時に持てる?ュ?ジックハンドルの最大数
#define MAX_MOVIE_NUM								(100)				// 同時に持てる??ビ?ハンドルの最大数
#define MAX_MASK_NUM								(32768)				// 同時に持てる?スクハンドルの最大数
#define MAX_FONT_NUM								(256)				// 同時に持てるフォントハンドルの最大数
#define MAX_INPUT_NUM								(256)				// 同時に持てる文字列入力ハンドルの最大数
#define MAX_SOCKET_NUM								(8192)				// 同時に持てる通信ハンドルの最大数
#define MAX_LIGHT_NUM								(4096)				// 同時に持てるライトハンドルの最大数
#define MAX_SHADER_NUM								(4096)				// 同時に持てるシェ???ハンドルの最大数
#define MAX_CONSTANT_BUFFER_NUM						(32768)				// 同時に持てる定数バッフ?ハンドルの最大数
#define MAX_MODEL_BASE_NUM							(32768)				// 同時に持てる３Ｄモデル基?デ??ハンドルの最大数
#define MAX_MODEL_NUM								(32768)				// 同時に持てる３Ｄモデルデ??ハンドルの最大数
#define MAX_VERTEX_BUFFER_NUM						(16384)				// 同時に持てる頂?バッフ?ハンドルの最大数
#define MAX_INDEX_BUFFER_NUM						(16384)				// 同時に持てるインデックスバッフ?の最大数
#define MAX_FILE_NUM								(32768)				// 同時に持てるフ?イルハンドルの最大数
#define MAX_LIVE2D_CUBISM4_MODEL_NUM				(32768)				// 同時に持てる Live2D Cubism 4 Model ハンドルの最大数

// ハンドル?イプ定?
#define DX_HANDLETYPE_NONE							(0)					// ハンドル?イプ０は未使用
#define DX_HANDLETYPE_GRAPH							(1)					// グラフィックハンドル
#define DX_HANDLETYPE_SOFTIMAGE						(2)					// ?フトウエアで扱うイメ?ジハンドル
#define DX_HANDLETYPE_SOUND							(3)					// サウンドハンドル
#define DX_HANDLETYPE_SOFTSOUND						(4)					// ?フトサウンドハンドル
#define DX_HANDLETYPE_MUSIC							(5)					// ?ュ?ジックハンドル
#define DX_HANDLETYPE_MOVIE							(6)					// ??ビ?ハンドル
#define DX_HANDLETYPE_GMASK							(7)					// ?スクハンドル
#define DX_HANDLETYPE_FONT							(8)					// フォントハンドル
#define DX_HANDLETYPE_KEYINPUT						(9)					// 文字列入力ハンドル
#define DX_HANDLETYPE_NETWORK						(10)				// ネットワ?クハンドル
#define DX_HANDLETYPE_LIGHT							(11)				// ライト
#define DX_HANDLETYPE_SHADER						(12)				// シェ???ハンドル
#define DX_HANDLETYPE_MODEL_BASE					(13)				// ３Ｄモデル基?デ??
#define DX_HANDLETYPE_MODEL							(14)				// ３Ｄモデル
#define DX_HANDLETYPE_VERTEX_BUFFER					(15)				// 頂?バッフ?ハンドル
#define DX_HANDLETYPE_INDEX_BUFFER					(16)				// インデックスバッフ?ハンドル
#define DX_HANDLETYPE_FILE							(17)				// フ?イルハンドル
#define DX_HANDLETYPE_SHADOWMAP						(18)				// シャドウ?ップハンドル
#define DX_HANDLETYPE_SHADER_CONSTANT_BUFFER		(19)				// シェ???用定数バッフ?ハンドル
#define DX_HANDLETYPE_LIVE2D_CUBISM4_MODEL			(20)				// Live2D Cubism 4 モデルハンドル

#define MAX_JOYPAD_NUM								(16)				// ジョイパッドの最大数

#define DEFAULT_SCREEN_SIZE_X						(640)				// デフォルトの画面の幅
#define DEFAULT_SCREEN_SIZE_Y						(480)				// デフォルトの画面の高さ
#define DEFAULT_COLOR_BITDEPTH						(16)				// デフォルトの色ビット?度
#define DEFAULT_ZBUFFER_BITDEPTH					(16)				// デフォルトのＺバッフ?ビット?度

#define DEFAULT_FOV									(60.0F * 3.1415926535897932384626433832795F / 180.0F)	// デフォルトの視野角
#define DEFAULT_TAN_FOV_HALF						(0.57735026918962576450914878050196F) // tan( FOV * 0.5 )
#define DEFAULT_NEAR								(0.0F)				// NEARクリップ面
#define DEFAULT_FAR									(20000.0F)			// FARクリップ面

#define DX_DEFAULT_FONT_HANDLE						(-2)				// デフォルトフォントを示す値

#define DEFAULT_FONT_SIZE							(16)				// フォントのデフォルトのサイズ
#define DEFAULT_FONT_THICKNESS						(6)					// フォントのデフォルトの太さ
#define DEFAULT_FONT_TYPE							( DX_FONTTYPE_NORMAL )	// フォントのデフォルトの?態
#define DEFAULT_FONT_EDGESIZE						(1)					// フォントのデフォルトの太さ

#define MAX_USERIMAGEREAD_FUNCNUM					(10)				// ユ?ザ?が登?できるグラフィックロ?ド関数の最大数

// ＷＩＮＤＯＷＳのバ?ジョン?クロ
#define DX_WINDOWSVERSION_31						(0x000)
#define DX_WINDOWSVERSION_95						(0x001)
#define DX_WINDOWSVERSION_98						(0x002)
#define DX_WINDOWSVERSION_ME						(0x003)
#define DX_WINDOWSVERSION_NT31						(0x104)
#define DX_WINDOWSVERSION_NT40						(0x105)
#define DX_WINDOWSVERSION_2000						(0x106)
#define DX_WINDOWSVERSION_XP						(0x107)
#define DX_WINDOWSVERSION_VISTA						(0x108)
#define DX_WINDOWSVERSION_7							(0x109)
#define DX_WINDOWSVERSION_8							(0x10A)
#define DX_WINDOWSVERSION_8_1						(0x10B)
#define DX_WINDOWSVERSION_10						(0x10C)
#define DX_WINDOWSVERSION_11						(0x10D)
#define DX_WINDOWSVERSION_NT_TYPE					(0x100)

// DirectXのバ?ジョン
#define DX_DIRECTXVERSION_NON						(0)
#define DX_DIRECTXVERSION_1							(0x10000)
#define DX_DIRECTXVERSION_2							(0x20000)
#define DX_DIRECTXVERSION_3							(0x30000)
#define DX_DIRECTXVERSION_4							(0x40000)
#define DX_DIRECTXVERSION_5							(0x50000)
#define DX_DIRECTXVERSION_6							(0x60000)
#define DX_DIRECTXVERSION_6_1						(0x60100)
#define DX_DIRECTXVERSION_7							(0x70000)
#define DX_DIRECTXVERSION_8							(0x80000)
#define DX_DIRECTXVERSION_8_1						(0x80100)

// Direct3Dのバ?ジョン
#define DX_DIRECT3D_NONE							(0)
#define DX_DIRECT3D_9								(1)
#define DX_DIRECT3D_9EX								(2)
#define DX_DIRECT3D_11								(3)

// Direct3D11 の Feature Level
#define DX_DIRECT3D_11_FEATURE_LEVEL_9_1			(0x9100)
#define DX_DIRECT3D_11_FEATURE_LEVEL_9_2			(0x9200)
#define DX_DIRECT3D_11_FEATURE_LEVEL_9_3			(0x9300)
#define DX_DIRECT3D_11_FEATURE_LEVEL_10_0			(0xa000)
#define DX_DIRECT3D_11_FEATURE_LEVEL_10_1			(0xa100)
#define DX_DIRECT3D_11_FEATURE_LEVEL_11_0			(0xb000)
#define DX_DIRECT3D_11_FEATURE_LEVEL_11_1			(0xb100)

// 文字セット
#define DX_CHARSET_DEFAULT							(0)				// デフォルト文字セット
#define DX_CHARSET_SHFTJIS							(1)				// シフトJIS
#define DX_CHARSET_HANGEUL							(2)				// ハングル文字セット
#define DX_CHARSET_BIG5								(3)				// 繁体文字セット
#define DX_CHARSET_GB2312							(4)				// 簡体文字セット
#define DX_CHARSET_WINDOWS_1252						(5)				// 欧文(ラテン文字の文字コ?ド)
#define DX_CHARSET_ISO_IEC_8859_15					(6)				// 欧文(ラテン文字の文字コ?ド)
#define DX_CHARSET_UTF8								(7)				// UTF-8
#define DX_CHARSET_NUM								(8)				// 文字セットの数

// 文字コ?ド?式
#define DX_CHARCODEFORMAT_SHIFTJIS					(932)			// シフトJISコ?ド
#define DX_CHARCODEFORMAT_GB2312					(936)			// 簡体字文字コ?ド
#define DX_CHARCODEFORMAT_UHC						(949)			// ハングル文字コ?ド
#define DX_CHARCODEFORMAT_BIG5						(950)			// 繁体文字コ?ド
#define DX_CHARCODEFORMAT_UTF16LE					(1200)			// UTF-16 リトルエンディアン
#define DX_CHARCODEFORMAT_UTF16BE					(1201)			// UTF-16 ビッグエンディアン
#define DX_CHARCODEFORMAT_WINDOWS_1252				(1252)			// 欧文(ラテン文字の文字コ?ド)
#define DX_CHARCODEFORMAT_ISO_IEC_8859_15			(32764)			// 欧文(ラテン文字の文字コ?ド)
#define DX_CHARCODEFORMAT_UTF8						(65001)			// UTF-8
#define DX_CHARCODEFORMAT_ASCII						(32765)			// アスキ?文字コ?ド
#define DX_CHARCODEFORMAT_UTF32LE					(32766)			// UTF-32 リトルエンディアン
#define DX_CHARCODEFORMAT_UTF32BE					(32767)			// UTF-32 ビッグエンディアン

// ＭＩＤＩの演奏モ?ド定?
#define DX_MIDIMODE_MCI								(0)				// ＭＣＩによる演奏
#define DX_MIDIMODE_DM								(1)				// ＤｉｒｅｃｔＭｕｓｉｃによる演奏
#define DX_MIDIMODE_DIRECT_MUSIC_REVERB				(1)				// ＤｉｒｅｃｔＭｕｓｉｃ(リバ?ブあり)による演奏
#define DX_MIDIMODE_DIRECT_MUSIC_NORMAL				(2)				// ＤｉｒｅｃｔＭｕｓｉｃ(リバ?ブなし)による演奏
#define DX_MIDIMODE_NUM								(3)				// ＭＩＤＩの演奏モ?ドの数

// ?画モ?ド定?
#define DX_DRAWMODE_NEAREST							(0)				// ネアレストネイバ??で?画
#define DX_DRAWMODE_BILINEAR						(1)				// バイリニア?で?画する
#define DX_DRAWMODE_ANISOTROPIC						(2)				// 異方性フィル?リング?で?画する
#define DX_DRAWMODE_OTHER							(3)				// それ以外
#define DX_DRAWMODE_NUM								(4)				// ?画モ?ドの数

// フォントの?イプ
#define DX_FONTTYPE_NORMAL							(0x00)			// ノ??ルフォント
#define DX_FONTTYPE_EDGE							(0x01)			// エッジつきフォント
#define DX_FONTTYPE_ANTIALIASING					(0x02)			// アン?エイリアスフォント( 標???アン?エイリアス )
#define DX_FONTTYPE_ANTIALIASING_4X4				(0x12)			// アン?エイリアスフォント( 4x4サンプリング )
#define DX_FONTTYPE_ANTIALIASING_8X8				(0x22)			// アン?エイリアスフォント( 8x8サンプリング )
#define DX_FONTTYPE_ANTIALIASING_16X16				(0x32)			// アン?エイリアスフォント( 16x16サンプリング )
#define DX_FONTTYPE_ANTIALIASING_EDGE				(0x03)			// アン?エイリアス＆エッジ付きフォント( 標???アン?エイリアス )
#define DX_FONTTYPE_ANTIALIASING_EDGE_4X4			(0x13)			// アン?エイリアス＆エッジ付きフォント( 4x4サンプリング )
#define DX_FONTTYPE_ANTIALIASING_EDGE_8X8			(0x23)			// アン?エイリアス＆エッジ付きフォント( 8x8サンプリング )
#define DX_FONTTYPE_ANTIALIASING_EDGE_16X16			(0x33)			// アン?エイリアス＆エッジ付きフォント( 16x16サンプリング )

// フォント画像の階調ビット数
#define DX_FONTIMAGE_BIT_1							(0)
#define DX_FONTIMAGE_BIT_4							(1)
#define DX_FONTIMAGE_BIT_8							(2)

// ?画ブレンドモ?ド定?
#define DX_BLENDMODE_NOBLEND						(0)				// ノ?ブレンド
#define DX_BLENDMODE_ALPHA							(1)				// αブレンド
#define DX_BLENDMODE_ADD							(2)				// 加算ブレンド
#define DX_BLENDMODE_SUB							(3)				// 減算ブレンド
#define DX_BLENDMODE_MUL							(4)				// 乗算ブレンド
   // (内部処理用)
#define DX_BLENDMODE_SUB2							(5)				// 内部処理用減算ブレンド２
//#define DX_BLENDMODE_BLINEALPHA					(7)				// 境界線ぼかし
#define DX_BLENDMODE_XOR							(6)				// XORブレンド( ?フトウエアレン?リングモ?ドでのみ有効 )
#define DX_BLENDMODE_DESTCOLOR						(8)				// カラ?は更新されない
#define DX_BLENDMODE_INVDESTCOLOR					(9)				// ?画先の色の反?値を?ける
#define DX_BLENDMODE_INVSRC							(10)			// ?画元の色を反?する
#define DX_BLENDMODE_MULA							(11)			// アルフ??ャンネル考慮付き乗算ブレンド
#define DX_BLENDMODE_ALPHA_X4						(12)			// αブレンドの?画元の輝度を最大４?にできるモ?ド
#define DX_BLENDMODE_ADD_X4							(13)			// 加算ブレンドの?画元の輝度を最大４?にできるモ?ド
#define DX_BLENDMODE_SRCCOLOR						(14)			// ?画元のカラ?でそのまま?画される
#define DX_BLENDMODE_HALF_ADD						(15)			// 半加算ブレンド
#define DX_BLENDMODE_SUB1							(16)			// 内部処理用減算ブレンド１
#define DX_BLENDMODE_PMA_ALPHA						(17)			// 乗算済みαブレンドモ?ドのαブレンド
#define DX_BLENDMODE_PMA_ADD						(18)			// 乗算済みαブレンドモ?ドの加算ブレンド
#define DX_BLENDMODE_PMA_SUB						(19)			// 乗算済みαブレンドモ?ドの減算ブレンド
#define DX_BLENDMODE_PMA_INVSRC						(20)			// 乗算済みαブレンドモ?ドの?画元の色を反?する
#define DX_BLENDMODE_PMA_ALPHA_X4					(21)			// 乗算済みαブレンドモ?ドのαブレンドの?画元の輝度を最大４?にできるモ?ド
#define DX_BLENDMODE_PMA_ADD_X4						(22)			// 乗算済みαブレンドモ?ドの加算ブレンドの?画元の輝度を最大４?にできるモ?ド
#define DX_BLENDMODE_LIVE2D_ZERO					(23)			// Live2D のブレンドモ?ド Zero 用
#define DX_BLENDMODE_LIVE2D_NORMAL					(24)			// Live2D のブレンドモ?ド Normal 用
#define DX_BLENDMODE_LIVE2D_ADD						(25)			// Live2D のブレンドモ?ド Add 用
#define DX_BLENDMODE_LIVE2D_MULT					(26)			// Live2D のブレンドモ?ド Mult 用
#define DX_BLENDMODE_LIVE2D_MASK					(27)			// Live2D のブレンドモ?ド Mask 用
#define DX_BLENDMODE_SPINE_NORMAL					(28)			// Spine のブレンドモ?ド Normal 用
#define DX_BLENDMODE_SPINE_ADDITIVE					(29)			// Spine のブレンドモ?ド Additive 用
#define DX_BLENDMODE_SPINE_MULTIPLY					(30)			// Spine のブレンドモ?ド Multiply 用
#define DX_BLENDMODE_SPINE_SCREEN					(31)			// Spine のブレンドモ?ド Screen 用
#define DX_BLENDMODE_CUSTOM							(32)			// カス??ブレンドモ?ド
#define DX_BLENDMODE_DST_RGB_SRC_A					(33)			// ?画元の A のみを書き込む( ?画先の RGB は変更されない )
#define DX_BLENDMODE_INVDESTCOLOR_A					(34)			// ?画先の A の反?値を?ける( ?画先の RGB は変更されない )
#define DX_BLENDMODE_MUL_A							(35)			// A のみの乗算ブレンド( ?画先の RGB は変更されない )
#define DX_BLENDMODE_PMA_INVDESTCOLOR_A				(36)			// 乗算済みα用の DX_BLENDMODE_INVDESTCOLOR_A
#define DX_BLENDMODE_PMA_MUL_A						(37)			// 乗算済みα用の DX_BLENDMODE_MUL_A
#define DX_BLENDMODE_NUM							(38)			// ブレンドモ?ドの数

// カス??ブレンドモ?ド用のブレンド要素?イプ
// Rs = ?画元のR   Rg = ?画元のG   Rb = ?画元のB   Ra = ?画元のA
// Rd = ?画先のR   Rd = ?画先のG   Rd = ?画先のB   Rd = ?画先のA
#define DX_BLEND_ZERO								(0)				// R = 0         G = 0         B = 0         A = 0
#define DX_BLEND_ONE								(1)				// R = 255       G = 255       B = 255       A = 255
#define DX_BLEND_SRC_COLOR							(2)				// R = Rs        G = Gs        B = Gs        A = As
#define DX_BLEND_INV_SRC_COLOR						(3)				// R = 255 - Rs  G = 255 - Gs  B = 255 - Gs  A = 255 - As
#define DX_BLEND_SRC_ALPHA							(4)				// R = As        G = As        B = As        A = As
#define DX_BLEND_INV_SRC_ALPHA						(5)				// R = 255 - As  G = 255 - As  B = 255 - As  A = 255 - As
#define DX_BLEND_DEST_COLOR							(6)				// R = Rd        G = Gd        B = Bd        A = Ad
#define DX_BLEND_INV_DEST_COLOR						(7)				// R = 255 - Rd  G = 255 - Gd  B = 255 - Bd  A = 255 - Ad
#define DX_BLEND_DEST_ALPHA							(8)				// R = Ad        G = Ad        B = Ad        A = Ad
#define DX_BLEND_INV_DEST_ALPHA						(9)				// R = 255 - Ad  G = 255 - Ad  B = 255 - Ad  A = 255 - Ad
#define DX_BLEND_SRC_ALPHA_SAT						(10)			// R = f         G = f         B = f         A = 255        f = min( As, 255 - Ad )
#define DX_BLEND_NUM								(11)			// ブレンド要素?イプの数

// カス??ブレンドモ?ド用のブレンド処理?イプ
#define DX_BLENDOP_ADD								(0)				// ブレンド??ス1とブレンド??ス2を加算
#define DX_BLENDOP_SUBTRACT							(1)				// ブレンド??ス1からブレンド??ス2を減算
#define DX_BLENDOP_REV_SUBTRACT						(2)				// ブレンド??ス2からブレンド??ス2を減算
#define DX_BLENDOP_MIX								(3)				// ブレンド??ス1とブレンド??ス2の最小値
#define DX_BLENDOP_MAX								(4)				// ブレンド??ス1とブレンド??ス2の最大値
#define DX_BLENDOP_NUM								(5)				// ブレンド処理?イプの数

// DrawGraphF 等の浮動小数?値で座標を指定する関数における座標?イプ
#define DX_DRAWFLOATCOORDTYPE_DIRECT3D9				(0)				// Direct3D9?イプ( -0.5f の補正を行わないとテクス?ャのピクセルが?麗に?ップされない?イプ )
#define DX_DRAWFLOATCOORDTYPE_DIRECT3D10			(1)				// Direct3D10?イプ( -0.5f の補正を行わななくてもテクス?ャのピクセルが?麗に?ップされる?イプ )

// 画像合成?イプ
#define DX_BLENDGRAPHTYPE_NORMAL					(0)				// 通常合成
#define DX_BLENDGRAPHTYPE_WIPE						(1)				// ワイプ処理
#define DX_BLENDGRAPHTYPE_ALPHA						(2)				// ブレンド画像のα値と元画像のα値を?け合わせる
#define DX_BLENDGRAPHTYPE_NUM						(3)

// 画像合成座標?イプ
#define DX_BLENDGRAPH_POSMODE_DRAWGRAPH				(0)				// ?画する画像基?で合成画像の座標を決定
#define DX_BLENDGRAPH_POSMODE_SCREEN				(1)				// スクリ?ン座標基?で合成画像の座標を決定
#define DX_BLENDGRAPH_POSMODE_NUM					(2)

// グラフィックフィル???イプ
#define DX_GRAPH_FILTER_MONO						(0)				// モノト?ンフィル?
#define DX_GRAPH_FILTER_GAUSS						(1)				// ガウスフィル?
#define DX_GRAPH_FILTER_DOWN_SCALE					(2)				// 縮小フィル?
#define DX_GRAPH_FILTER_BRIGHT_CLIP					(3)				// 明るさクリップフィル?
#define DX_GRAPH_FILTER_BRIGHT_SCALE				(4)				// 指定の明るさ領域を拡大するフィル?
#define DX_GRAPH_FILTER_HSB							(5)				// 色相・彩度・明度フィル?
#define DX_GRAPH_FILTER_INVERT						(6)				// 階調の反?フィル?
#define DX_GRAPH_FILTER_LEVEL						(7)				// レベル補正フィル?
#define DX_GRAPH_FILTER_TWO_COLOR					(8)				// ２階調化フィル?
#define DX_GRAPH_FILTER_GRADIENT_MAP				(9)				// グラデ?ション?ップフィル?
#define DX_GRAPH_FILTER_REPLACEMENT					(10)			// 色の置換
#define DX_GRAPH_FILTER_BILATERAL_BLUR				(11)			// バイラテラルブラ?
#define DX_GRAPH_FILTER_PREMUL_ALPHA				(12)			// 通常のアルフ??ャンネル付き画像を乗算済みアルフ?画像に変換するフィル?
#define DX_GRAPH_FILTER_INTERP_ALPHA				(13)			// 乗算済みα画像を通常のアルフ??ャンネル付き画像に変換するフィル?
#define DX_GRAPH_FILTER_YUV_TO_RGB					(14)			// YUVカラ?をRGBカラ?に変換するフィル?
#define DX_GRAPH_FILTER_Y2UV1_TO_RGB				(15)			// YUVカラ?をRGBカラ?に変換するフィル?( UV成分が Y成分の半分・又は４分の１( 横・縦片方若しくは両方 )の解像度しかない場合用 )
#define DX_GRAPH_FILTER_YUV_TO_RGB_RRA				(16)			// YUVカラ?をRGBカラ?に変換するフィル?( 且つ右側半分のRの値をアルフ?値として扱う )
#define DX_GRAPH_FILTER_Y2UV1_TO_RGB_RRA			(17)			// YUVカラ?をRGBカラ?に変換するフィル?( UV成分が Y成分の半分・又は４分の１( 横・縦片方若しくは両方 )の解像度しかない場合用 )( 且つ右側半分のRの値をアルフ?値として扱う )
#define DX_GRAPH_FILTER_BICUBIC_SCALE				(18)			// バイキュ?ビックを使用した拡大・縮小フィル?
#define DX_GRAPH_FILTER_LANCZOS3_SCALE				(19)			// Lanczos-3を使用した拡大・縮小フィル?
#define DX_GRAPH_FILTER_SSAO						(20)			// Screen-Space Ambient Occlusionフィル?
#define DX_GRAPH_FILTER_FLOAT_COLOR_SCALE			(21)			// float?の色のスケ?リング
#define DX_GRAPH_FILTER_PMA_BRIGHT_CLIP				(22)			// 明るさクリップフィル?(乗算済みアルフ?画像用)
#define DX_GRAPH_FILTER_PMA_BRIGHT_SCALE			(23)			// 指定の明るさ領域を拡大するフィル?(乗算済みアルフ?画像用)
#define DX_GRAPH_FILTER_PMA_HSB						(24)			// 色相・彩度・明度フィル?(乗算済みアルフ?画像用)
#define DX_GRAPH_FILTER_PMA_INVERT					(25)			// 階調の反?フィル?(乗算済みアルフ?画像用)
#define DX_GRAPH_FILTER_PMA_LEVEL					(26)			// レベル補正フィル?(乗算済みアルフ?画像用)
#define DX_GRAPH_FILTER_PMA_TWO_COLOR				(27)			// ２階調化フィル?(乗算済みアルフ?画像用)
#define DX_GRAPH_FILTER_PMA_GRADIENT_MAP			(28)			// グラデ?ション?ップフィル?(乗算済みアルフ?画像用)
#define DX_GRAPH_FILTER_PMA_REPLACEMENT				(29)			// 色の置換(乗算済みアルフ?画像用)
#define DX_GRAPH_FILTER_PMA_BILATERAL_BLUR			(30)			// バイラテラルブラ?(乗算済みアルフ?画像用)
#define DX_GRAPH_FILTER_NUM							(31)

// グラフィックブレンド?イプ
#define DX_GRAPH_BLEND_NORMAL						(0)				// 通常
#define DX_GRAPH_BLEND_RGBA_SELECT_MIX				(1)				// RGBAの要素を選択して合成
#define DX_GRAPH_BLEND_MULTIPLE						(2)				// 乗算
#define DX_GRAPH_BLEND_DIFFERENCE					(3)				// 減算
#define DX_GRAPH_BLEND_ADD							(4)				// 加算
#define DX_GRAPH_BLEND_SCREEN						(5)				// スクリ?ン
#define DX_GRAPH_BLEND_OVERLAY						(6)				// オ?バ?レイ
#define DX_GRAPH_BLEND_DODGE						(7)				// 覆い焼き
#define DX_GRAPH_BLEND_BURN							(8)				// 焼き込み
#define DX_GRAPH_BLEND_DARKEN						(9)				// 比較(暗)
#define DX_GRAPH_BLEND_LIGHTEN						(10)			// 比較(明)
#define DX_GRAPH_BLEND_SOFTLIGHT					(11)			// ?フトライト
#define DX_GRAPH_BLEND_HARDLIGHT					(12)			// ハ?ドライト
#define DX_GRAPH_BLEND_EXCLUSION					(13)			// 除外
#define DX_GRAPH_BLEND_NORMAL_ALPHACH				(14)			// α?ャンネル付き画像の通常合成
#define DX_GRAPH_BLEND_ADD_ALPHACH					(15)			// α?ャンネル付き画像の加算合成
#define DX_GRAPH_BLEND_MULTIPLE_A_ONLY				(16)			// アルフ??ャンネルのみの乗算
#define DX_GRAPH_BLEND_PMA_NORMAL					(17)			// 通常( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_RGBA_SELECT_MIX			(18)			// RGBAの要素を選択して合成( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_MULTIPLE					(19)			// 乗算( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_DIFFERENCE				(20)			// 減算( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_ADD						(21)			// 加算( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_SCREEN					(22)			// スクリ?ン( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_OVERLAY					(23)			// オ?バ?レイ( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_DODGE					(24)			// 覆い焼き( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_BURN						(25)			// 焼き込み( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_DARKEN					(26)			// 比較(暗)( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_LIGHTEN					(27)			// 比較(明)( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_SOFTLIGHT				(28)			// ?フトライト( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_HARDLIGHT				(29)			// ハ?ドライト( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_EXCLUSION				(30)			// 除外( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_NORMAL_ALPHACH			(31)			// α?ャンネル付き画像の通常合成( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_ADD_ALPHACH				(32)			// α?ャンネル付き画像の加算合成( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_PMA_MULTIPLE_A_ONLY			(33)			// アルフ??ャンネルのみの乗算( 乗算済みα画像用 )
#define DX_GRAPH_BLEND_MASK							(34)			// ?スク( SrcGrHandle に BlendGrHandle を通常?画した上で、SrcGrHandle の A を優先 )
#define DX_GRAPH_BLEND_PMA_MASK						(35)			// ?スク( DX_GRAPH_BLEND_MASK の乗算済みα画像用 )
#define DX_GRAPH_BLEND_NUM							(36)

// DX_GRAPH_BLEND_RGBA_SELECT_MIX 用の色選択用定?
#define DX_RGBA_SELECT_SRC_R						(0)				// 元画像の赤成分
#define DX_RGBA_SELECT_SRC_G						(1)				// 元画像の緑成分
#define DX_RGBA_SELECT_SRC_B						(2)				// 元画像の青成分
#define DX_RGBA_SELECT_SRC_A						(3)				// 元画像のα成分
#define DX_RGBA_SELECT_BLEND_R						(4)				// ブレンド画像の赤成分
#define DX_RGBA_SELECT_BLEND_G						(5)				// ブレンド画像の緑成分
#define DX_RGBA_SELECT_BLEND_B						(6)				// ブレンド画像の青成分
#define DX_RGBA_SELECT_BLEND_A						(7)				// ブレンド画像のα成分
#define DX_RGBA_SELECT_SRC_INV_R					(8)				// 元画像の赤成分を反?したもの
#define DX_RGBA_SELECT_SRC_INV_G					(9)				// 元画像の緑成分を反?したもの
#define DX_RGBA_SELECT_SRC_INV_B					(10)			// 元画像の青成分を反?したもの
#define DX_RGBA_SELECT_SRC_INV_A					(11)			// 元画像のα成分を反?したもの
#define DX_RGBA_SELECT_BLEND_INV_R					(12)			// ブレンド画像の赤成分を反?したもの
#define DX_RGBA_SELECT_BLEND_INV_G					(13)			// ブレンド画像の緑成分を反?したもの
#define DX_RGBA_SELECT_BLEND_INV_B					(14)			// ブレンド画像の青成分を反?したもの
#define DX_RGBA_SELECT_BLEND_INV_A					(15)			// ブレンド画像のα成分を反?したもの

// フィルモ?ド
#define DX_FILL_WIREFRAME							(2)				// ワイヤ?フレ??
#define DX_FILL_SOLID								(3)				// ?リゴン

// ?リゴンカリングモ?ド
#define DX_CULLING_NONE								(0)				// カリングなし
#define DX_CULLING_LEFT								(1)				// 背面を左回りでカリング
#define DX_CULLING_RIGHT							(2)				// 背面を右回りでカリング
#define DX_CULLING_NUM								(3)				// カリングモ?ドの数

// クリッピング方向
#define DX_CAMERACLIP_LEFT							(0x01)			// 画面左方向にクリップ
#define DX_CAMERACLIP_RIGHT							(0x02)			// 画面右方向にクリップ
#define DX_CAMERACLIP_BOTTOM						(0x04)			// 画面下方向にクリップ
#define DX_CAMERACLIP_TOP							(0x08)			// 画面上方向にクリップ
#define DX_CAMERACLIP_BACK							(0x10)			// 画面後方向にクリップ
#define DX_CAMERACLIP_FRONT							(0x20)			// 画面前方向にクリップ

// MV1モデルの?画モ?ド
#define DX_MV1_DRAWMODE_NORMAL						(0)				// 通常?画
#define DX_MV1_DRAWMODE_NORMAL_DEPTH				(1)				// ?線と?度を?画

// MV1モデルの頂??イプ
#define DX_MV1_VERTEX_TYPE_1FRAME					(0)				// １フレ??の影響を受ける頂?
#define DX_MV1_VERTEX_TYPE_4FRAME					(1)				// １?４フレ??の影響を受ける頂?
#define DX_MV1_VERTEX_TYPE_8FRAME					(2)				// ５?８フレ??の影響を受ける頂?
#define DX_MV1_VERTEX_TYPE_FREE_FRAME				(3)				// ９フレ??以上の影響を受ける頂?
#define DX_MV1_VERTEX_TYPE_NMAP_1FRAME				(4)				// ?線?ップ用の情報が含まれる１フレ??の影響を受ける頂?
#define DX_MV1_VERTEX_TYPE_NMAP_4FRAME				(5)				// ?線?ップ用の情報が含まれる１?４フレ??の影響を受ける頂?
#define DX_MV1_VERTEX_TYPE_NMAP_8FRAME				(6)				// ?線?ップ用の情報が含まれる５?８フレ??の影響を受ける頂?
#define DX_MV1_VERTEX_TYPE_NMAP_FREE_FRAME			(7)				// ?線?ップ用の情報が含まれる９フレ??以上の影響を受ける頂?
#define DX_MV1_VERTEX_TYPE_NUM						(8)				// 頂??イプの数

// メッシュの種類
#define DX_MV1_MESHCATEGORY_NORMAL					(0)				// 普通のメッシュ
#define DX_MV1_MESHCATEGORY_OUTLINE					(1)				// 輪郭線?画用メッシュ
#define DX_MV1_MESHCATEGORY_OUTLINE_ORIG_SHADER		(2)				// 輪郭線?画用メッシュ( オリジナルシェ???での?画用 )
#define DX_MV1_MESHCATEGORY_NUM						(3)				// メッシュの種類の数

// シェイプ率の適用?イプ
#define DX_MV1_SHAPERATE_ADD						(0)				// 元の値に加算
#define DX_MV1_SHAPERATE_OVERWRITE					(1)				// 元の値に上書き

// MV1フ?イルの保存?イプ
#define MV1_SAVETYPE_MESH							(0x0001)		// メッシュ情報のみ保存
#define MV1_SAVETYPE_ANIM							(0x0002)		// アニメ?ション情報のみ保存
#define MV1_SAVETYPE_NORMAL							( MV1_SAVETYPE_MESH | MV1_SAVETYPE_ANIM )	// 通常保存

// アニメ?ションキ?デ???イプ
#define MV1_ANIMKEY_DATATYPE_ROTATE					(0)				// 回?
#define MV1_ANIMKEY_DATATYPE_ROTATE_X				(1)				// 回?Ｘ
#define MV1_ANIMKEY_DATATYPE_ROTATE_Y				(2)				// 回?Ｙ
#define MV1_ANIMKEY_DATATYPE_ROTATE_Z				(3)				// 回?Ｚ
#define MV1_ANIMKEY_DATATYPE_SCALE					(5)				// 拡大
#define MV1_ANIMKEY_DATATYPE_SCALE_X				(6)				// スケ?ルＸ
#define MV1_ANIMKEY_DATATYPE_SCALE_Y				(7)				// スケ?ルＹ
#define MV1_ANIMKEY_DATATYPE_SCALE_Z				(8)				// スケ?ルＺ
#define MV1_ANIMKEY_DATATYPE_TRANSLATE				(10)			// 平行移動
#define MV1_ANIMKEY_DATATYPE_TRANSLATE_X			(11)			// 平行移動Ｘ
#define MV1_ANIMKEY_DATATYPE_TRANSLATE_Y			(12)			// 平行移動Ｙ
#define MV1_ANIMKEY_DATATYPE_TRANSLATE_Z			(13)			// 平行移動Ｚ
#define MV1_ANIMKEY_DATATYPE_MATRIX4X4C				(15)			// ４?４行列の４列目( 0,0,0,1 )固定版
#define MV1_ANIMKEY_DATATYPE_MATRIX3X3				(17)			// ３?３行列
#define MV1_ANIMKEY_DATATYPE_SHAPE					(18)			// シェイプ
#define MV1_ANIMKEY_DATATYPE_OTHRE					(20)			// その他

// ?イ??イプ
#define MV1_ANIMKEY_TIME_TYPE_ONE					(0)				// 時間情報は全体で一つ
#define MV1_ANIMKEY_TIME_TYPE_KEY					(1)				// 時間情報は各キ?に一つ

// アニメ?ションキ??イプ
#define MV1_ANIMKEY_TYPE_QUATERNION_X				(0)				// クォ??ニオン( Xフ?イル?イプ )
#define MV1_ANIMKEY_TYPE_VECTOR						(1)				// ベク??
#define MV1_ANIMKEY_TYPE_MATRIX4X4C					(2)				// ４?４行列の４列目( 0,0,0,1 )固定版
#define MV1_ANIMKEY_TYPE_MATRIX3X3					(3)				// ３?３行列
#define MV1_ANIMKEY_TYPE_FLAT						(4)				// フラット
#define MV1_ANIMKEY_TYPE_LINEAR						(5)				// 線?補間
#define MV1_ANIMKEY_TYPE_BLEND						(6)				// 混合
#define MV1_ANIMKEY_TYPE_QUATERNION_VMD				(7)				// クォ??ニオン( VMD?イプ )

// ?画先画面指定用定?
#define DX_SCREEN_FRONT								(0xfffffffc)
#define DX_SCREEN_BACK								(0xfffffffe) 
#define DX_SCREEN_WORK 								(0xfffffffd)
#define DX_SCREEN_TEMPFRONT							(0xfffffff0)
#define DX_SCREEN_OTHER								(0xfffffffa)

#define DX_NONE_GRAPH								(0xfffffffb)	// グラフィックなしハンドル

// グラフィック減色時の画像劣化緩和処理モ?ド
#define DX_SHAVEDMODE_NONE							(0)				// 画像劣化緩和処理を行わない
#define DX_SHAVEDMODE_DITHER						(1)				// ディザリング
#define DX_SHAVEDMODE_DIFFUS						(2)				// 誤差拡散

// 画像の保存?イプ
#define DX_IMAGESAVETYPE_BMP						(0)				// bitmap
#define DX_IMAGESAVETYPE_JPEG						(1)				// jpeg
#define DX_IMAGESAVETYPE_PNG						(2)				// Png
#define DX_IMAGESAVETYPE_DDS						(3)				// Direct Draw Surface

// サウンド再生?態指定用定?
#define DX_PLAYTYPE_LOOPBIT							(0x0002)		// ル?プ再生ビット
#define DX_PLAYTYPE_BACKBIT							(0x0001)		// バックグラウンド再生ビット

#define DX_PLAYTYPE_NORMAL							(0)												// ノ??ル再生
#define DX_PLAYTYPE_BACK				  			( DX_PLAYTYPE_BACKBIT )							// バックグラウンド再生
#define DX_PLAYTYPE_LOOP							( DX_PLAYTYPE_LOOPBIT | DX_PLAYTYPE_BACKBIT )	// ル?プ再生

// 動画再生?イプ定?
#define DX_MOVIEPLAYTYPE_BCANCEL					(0)				// ??ンキャンセルあり
#define DX_MOVIEPLAYTYPE_NORMAL						(1)				// ??ンキャンセルなし

// サウンドの?イプ
#define DX_SOUNDTYPE_NORMAL							(0)				// ノ??ルサウンド?式
#define DX_SOUNDTYPE_STREAMSTYLE					(1)				// ストリ??風サウンド?式

// サウンドデ???イプの?クロ
#define DX_SOUNDDATATYPE_MEMNOPRESS					(0)				// 圧縮された全デ??は再生が始まる前にサウンドメモリにすべて解?され、格?される
#define DX_SOUNDDATATYPE_MEMNOPRESS_PLUS			(1)				// 圧縮された全デ??はシステ?メモリに格?され、再生しながら?次解?され、最終的にすべてサウンドメモリに格?される(その後システ?メモリに存在する圧縮デ??は破棄される)
#define DX_SOUNDDATATYPE_MEMPRESS					(2)				// 圧縮された全デ??はシステ?メモリに格?され、再生する部分だけ?次解?しながらサウンドメモリに格?する(鳴らし終わると解?したデ??は破棄されるので何度も解?処理が行われる)
#define DX_SOUNDDATATYPE_FILE						(3)				// 圧縮されたデ??の再生する部分だけフ?イルから?次読み込み解?され、サウンドメモリに格?される(鳴らし終わると解?したデ??は破棄されるので何度も解?処理が行われる)

// サウンドの取得する再生時間?イプ
#define DX_SOUNDCURRENTTIME_TYPE_LOW_LEVEL			(0)				// 低レベルAPIを使用してより正確な再生時間を取得する
#define DX_SOUNDCURRENTTIME_TYPE_SOFT				(1)				// APIは使用せず、?フトウェア処理レベルでの再生時間を取得する

// 読み込み処理の?イプ
#define DX_READSOUNDFUNCTION_PCM					(1 << 0)		// PCM の読み込み処理
#define DX_READSOUNDFUNCTION_OGG					(1 << 1)		// Ogg Vorbis の読み込み処理
#define DX_READSOUNDFUNCTION_OPUS					(1 << 2)		// Opus の読み込み処理
#define DX_READSOUNDFUNCTION_DEFAULT_NUM			(3)				// 環境非依存の読み込み処理?イプの数

// ３Ｄサウンドリバ?ブエフェクトのプリセット
#define DX_REVERB_PRESET_DEFAULT					(0)				// デフォルト
#define DX_REVERB_PRESET_GENERIC					(1)				// 一般的な空間
#define DX_REVERB_PRESET_PADDEDCELL					(2)				// 精?病患者室(？)
#define DX_REVERB_PRESET_ROOM						(3)				// 部屋
#define DX_REVERB_PRESET_BATHROOM					(4)				// バスル??
#define DX_REVERB_PRESET_LIVINGROOM					(5)				// リビングル??
#define DX_REVERB_PRESET_STONEROOM					(6)				// 石の部屋
#define DX_REVERB_PRESET_AUDITORIUM					(7)				// 講堂
#define DX_REVERB_PRESET_CONCERTHALL				(8)				// コンサ?トホ?ル
#define DX_REVERB_PRESET_CAVE						(9)				// 洞穴
#define DX_REVERB_PRESET_ARENA						(10)			// 舞台
#define DX_REVERB_PRESET_HANGAR						(11)			// 格?庫
#define DX_REVERB_PRESET_CARPETEDHALLWAY			(12)			// カ?ペットが?かれた玄関
#define DX_REVERB_PRESET_HALLWAY					(13)			// 玄関
#define DX_REVERB_PRESET_STONECORRIDOR				(14)			// 石の廊下
#define DX_REVERB_PRESET_ALLEY						(15)			// 裏通り
#define DX_REVERB_PRESET_FOREST						(16)			// 森
#define DX_REVERB_PRESET_CITY						(17)			// 都市
#define DX_REVERB_PRESET_MOUNTAINS					(18)			// 山
#define DX_REVERB_PRESET_QUARRY						(19)			// 採石場
#define DX_REVERB_PRESET_PLAIN						(20)			// 平原
#define DX_REVERB_PRESET_PARKINGLOT					(21)			// 駐車場
#define DX_REVERB_PRESET_SEWERPIPE					(22)			// 下水管
#define DX_REVERB_PRESET_UNDERWATER					(23)			// 水面下
#define DX_REVERB_PRESET_SMALLROOM					(24)			// 小部屋
#define DX_REVERB_PRESET_MEDIUMROOM					(25)			// 中部屋
#define DX_REVERB_PRESET_LARGEROOM					(26)			// 大部屋
#define DX_REVERB_PRESET_MEDIUMHALL					(27)			// 中ホ?ル
#define DX_REVERB_PRESET_LARGEHALL					(28)			// 大ホ?ル
#define DX_REVERB_PRESET_PLATE						(29)			// 板

#define DX_REVERB_PRESET_NUM						(30)			// プリセットの数

// ?スク透過色モ?ド
#define DX_MASKTRANS_WHITE							(0)				// ?スク画像の白い部分を透過色とする
#define DX_MASKTRANS_BLACK							(1)				// ?スク画像の黒い部分を透過色とする
#define DX_MASKTRANS_NONE							(2) 			// 透過色なし

// ?スク画像?ャンネル
#define DX_MASKGRAPH_CH_A							(0)				// アルフ?
#define DX_MASKGRAPH_CH_R							(1)				// 赤
#define DX_MASKGRAPH_CH_G							(2)				// 緑
#define DX_MASKGRAPH_CH_B							(3)				// 青

// Ｚバッフ?書き込みモ?ド
#define DX_ZWRITE_MASK								(0)				// 書き込めないように?スクする
#define DX_ZWRITE_CLEAR								(1)				// 書き込めるように?スクをクリアする

// 比較モ?ド
#define DX_CMP_NEVER								(1)				// FALSE
#define DX_CMP_LESS									(2)				// Src <  Dest		DrawAlpha <  TestParam
#define DX_CMP_EQUAL								(3)				// Src == Dest		DrawAlpha == TestParam
#define DX_CMP_LESSEQUAL							(4)				// Src <= Dest		DrawAlpha <= TestParam
#define DX_CMP_GREATER								(5)				// Src >  Dest		DrawAlpha >  TestParam
#define DX_CMP_NOTEQUAL								(6)				// Src != Dest		DrawAlpha != TestParam
#define DX_CMP_GREATEREQUAL							(7)				// Src >= Dest		DrawAlpha >= TestParam
#define DX_CMP_ALWAYS								(8)				// TRUE
#define DX_ZCMP_DEFAULT								( DX_CMP_LESSEQUAL )
#define DX_ZCMP_REVERSE								( DX_CMP_GREATEREQUAL )

// シェ?ディングモ?ド
#define DX_SHADEMODE_FLAT							(1)				// D_D3DSHADE_FLAT
#define DX_SHADEMODE_GOURAUD						(2)				// D_D3DSHADE_GOURAUD

// フォグモ?ド
#define DX_FOGMODE_NONE								(0)				// D_D3DFOG_NONE
#define DX_FOGMODE_EXP								(1)				// D_D3DFOG_EXP
#define DX_FOGMODE_EXP2								(2)				// D_D3DFOG_EXP2
#define DX_FOGMODE_LINEAR							(3)				// D_D3DFOG_LINEAR

// ?テリアル?イプ
#define DX_MATERIAL_TYPE_NORMAL									(0)	// 標??テリアル
#define DX_MATERIAL_TYPE_TOON									(1)	// トゥ?ンレン?リング用?テリアル
#define DX_MATERIAL_TYPE_TOON_2									(2)	// トゥ?ンレン?リング用?テリアル_?イプ２( MMD互換 )
#define DX_MATERIAL_TYPE_MAT_SPEC_LUMINANCE_UNORM				(3)	// ?テリアルのスペキュラ色の輝度の指定の範囲の値を 0.0f ? 1.0f の値に正規化して書き込む?テリアル
#define DX_MATERIAL_TYPE_MAT_SPEC_LUMINANCE_CLIP_UNORM			(4)	// DX_MATERIAL_TYPE_MAT_SPEC_LUMINANCE_UNORM に『指定の値未満の場合は書き込む値を 0.0f にする』という処理を加えた?テリアル
#define DX_MATERIAL_TYPE_MAT_SPEC_LUMINANCE_CMP_GREATEREQUAL	(5)	// ?テリアルのスペキュラ色の輝度が指定の?値以上 の場合は 1.0f を、未満の場合は 0.0f を書き込む?テリアル
#define DX_MATERIAL_TYPE_MAT_SPEC_POWER_UNORM					(6)	// ?テリアルのスペキュラハイライトの鮮明度の指定の範囲の値を 0.0f ? 1.0f の値に正規化して書き込む?テリアル
#define DX_MATERIAL_TYPE_MAT_SPEC_POWER_CLIP_UNORM				(7)	// DX_MATERIAL_TYPE_MAT_SPEC_POWER_UNORM に『指定の値未満の場合は書き込む値を 0.0f にする』という処理を加えた?テリアル
#define DX_MATERIAL_TYPE_MAT_SPEC_POWER_CMP_GREATEREQUAL		(8)	// ?テリアルのスペキュラハイライトの鮮明度が指定の?値以上 の場合は 1.0f を、未満の場合は 0.0f を書き込む?テリアル
#define DX_MATERIAL_TYPE_NUM									(9)

// ?テリアルブレンド?イプ
#define DX_MATERIAL_BLENDTYPE_TRANSLUCENT			(0)				// アルフ?合成
#define DX_MATERIAL_BLENDTYPE_ADDITIVE				(1)				// 加算
#define DX_MATERIAL_BLENDTYPE_MODULATE				(2)				// 乗算
#define DX_MATERIAL_BLENDTYPE_NONE					(3)				// 無効

// テクス?ャアドレス?イプ
#define DX_TEXADDRESS_WRAP							(1)				// D_D3DTADDRESS_WRAP
#define DX_TEXADDRESS_MIRROR						(2)				// D_D3DTADDRESS_MIRROR
#define DX_TEXADDRESS_CLAMP							(3)				// D_D3DTADDRESS_CLAMP
#define DX_TEXADDRESS_BORDER						(4)				// D_D3DTADDRESS_BORDER
#define DX_TEXADDRESS_NUM							(5)				// テクス?ャアドレス?イプの数

// シェ????イプ
#define DX_SHADERTYPE_VERTEX						(0)				// 頂?シェ???
#define DX_SHADERTYPE_PIXEL							(1)				// ピクセルシェ???
#define DX_SHADERTYPE_GEOMETRY						(2)				// ジオメトリシェ???
#define DX_SHADERTYPE_COMPUTE						(3)				// コンピュ?トシェ???
#define DX_SHADERTYPE_DOMAIN						(4)				// ドメインシェ???
#define DX_SHADERTYPE_HULL							(5)				// ハルシェ???

// 頂?デ???イプ
#define DX_VERTEX_TYPE_NORMAL_3D					(0)				// VERTEX3D?造体?式
#define DX_VERTEX_TYPE_SHADER_3D					(1)				// VERTEX3DSHADER?造体?式
#define DX_VERTEX_TYPE_NUM							(2)

// インデックスデ???イプ
#define DX_INDEX_TYPE_16BIT							(0)				// 16bitインデックス
#define DX_INDEX_TYPE_32BIT							(1)				// 32bitインデックス

// モデルフ?イル読み込み時の物理演算モ?ド
#define DX_LOADMODEL_PHYSICS_DISABLE				(1)				// 物理演算を使用しない
#define DX_LOADMODEL_PHYSICS_LOADCALC				(0)				// 読み込み時に計算
#define DX_LOADMODEL_PHYSICS_REALTIME				(2)				// 実行時計算

// モデルフ?イル読み込み時の物理演算無効名前ワ?ドのモ?ド
#define DX_LOADMODEL_PHYSICS_DISABLENAMEWORD_ALWAYS	(0)						// 全てのフ?イルに対して無効名ワ?ドを適用する
#define DX_LOADMODEL_PHYSICS_DISABLENAMEWORD_DISABLEPHYSICSFILEONLY	(1)		// vmdフ?イル名に NP を含めた「物理演算無効」のフ?イルに対してのみ無効名ワ?ドを適用する( この場合、無効名ワ?ドが適用されない剛体については物理演算が行われる )
#define DX_LOADMODEL_PHYSICS_DISABLENAMEWORD_NUM	(2)

// PMD, PMX フ?イル読み込み時のアニメ?ションの FPS モ?ド( 主に IK 部分の精度に影響します )
#define DX_LOADMODEL_PMD_PMX_ANIMATION_FPSMODE_30	(0)				// アニメ?ションを 30FPS で読み込む( IK部分の精度:低  デ??サイズ:小 )( デフォルト )
#define DX_LOADMODEL_PMD_PMX_ANIMATION_FPSMODE_60	(1)				// アニメ?ションを 60FPS で読み込む( IK部分の精度:高  デ??サイズ:大 )

// モデルの半透明要素がある箇所に関する?画モ?ド
#define DX_SEMITRANSDRAWMODE_ALWAYS					(0)				// 半透明かどうか関係なく?画する
#define DX_SEMITRANSDRAWMODE_SEMITRANS_ONLY			(1)				// 半透明のもののみ?画する
#define DX_SEMITRANSDRAWMODE_NOT_SEMITRANS_ONLY		(2)				// 半透明ではないもののみ?画する

// キュ?ブ?ップの面番号
#define DX_CUBEMAP_FACE_POSITIVE_X					(0)
#define DX_CUBEMAP_FACE_NEGATIVE_X					(1)
#define DX_CUBEMAP_FACE_POSITIVE_Y					(2)
#define DX_CUBEMAP_FACE_NEGATIVE_Y					(3)
#define DX_CUBEMAP_FACE_POSITIVE_Z					(4)
#define DX_CUBEMAP_FACE_NEGATIVE_Z					(5)

// ?リゴン?画?イプ
#define DX_PRIMTYPE_POINTLIST						(1)				// D_D3DPT_POINTLIST
#define DX_PRIMTYPE_LINELIST						(2)				// D_D3DPT_LINELIST
#define DX_PRIMTYPE_LINESTRIP						(3)				// D_D3DPT_LINESTRIP
#define DX_PRIMTYPE_TRIANGLELIST					(4)				// D_D3DPT_TRIANGLELIST
#define DX_PRIMTYPE_TRIANGLESTRIP					(5)				// D_D3DPT_TRIANGLESTRIP
#define DX_PRIMTYPE_TRIANGLEFAN						(6)				// D_D3DPT_TRIANGLEFAN
#define DX_PRIMTYPE_MIN								(1)
#define DX_PRIMTYPE_MAX								(6)

// ライト?イプ
#define DX_LIGHTTYPE_D3DLIGHT_POINT					(1)				// D_D3DLIGHT_POINT
#define DX_LIGHTTYPE_D3DLIGHT_SPOT					(2)				// D_D3DLIGHT_SPOT
#define DX_LIGHTTYPE_D3DLIGHT_DIRECTIONAL			(3)				// D_D3DLIGHT_DIRECTIONAL
#define DX_LIGHTTYPE_POINT							(1)				// D_D3DLIGHT_POINT
#define DX_LIGHTTYPE_SPOT							(2)				// D_D3DLIGHT_SPOT
#define DX_LIGHTTYPE_DIRECTIONAL					(3)				// D_D3DLIGHT_DIRECTIONAL

// Live2D のシェ???
#define DX_LIVE2D_SHADER_SETUP_MASK_VERTEX								(0)				// ?スク画像生成用頂?シェ???
#define DX_LIVE2D_SHADER_SETUP_MASK_PIXEL								(1)				// ?スク画像生成用ピクセルシェ???
#define DX_LIVE2D_SHADER_NORMAL_VERTEX									(2)				// 標?の頂?シェ???
#define DX_LIVE2D_SHADER_NORMAL_VERTEX_MASKED							(3)				// 標?の頂?シェ??? + ?スク画像
#define DX_LIVE2D_SHADER_NORMAL_PIXEL									(4)				// 標?のピクセルシェ???
#define DX_LIVE2D_SHADER_NORMAL_PIXEL_MASKED							(5)				// 標?のピクセルシェ??? + ?スク画像
#define DX_LIVE2D_SHADER_NORMAL_PIXEL_MASKED_INVERTED					(6)				// 標?のピクセルシェ??? + ?スク画像( ?スク画像の値を反?して使用 )
#define DX_LIVE2D_SHADER_NORMAL_PIXEL_PREMULALPHA						(7)				// 標?のピクセルシェ??? + テクス?ャ画像が乗算済みアルフ?
#define DX_LIVE2D_SHADER_NORMAL_PIXEL_MASKED_PREMULALPHA				(8)				// 標?のピクセルシェ??? + テクス?ャ画像が乗算済みアルフ? + ?スク画像
#define DX_LIVE2D_SHADER_NORMAL_PIXEL_MASKED_INVERTEX_PREMULALPHA		(9)				// 標?のピクセルシェ??? + テクス?ャ画像が乗算済みアルフ? + ?スク画像( ?スク画像の値を反?して使用 )
#define DX_LIVE2D_SHADER_NUM											(10)			// シェ???の数

// グラフィックイメ?ジフォ??ットの定?
#define DX_GRAPHICSIMAGE_FORMAT_3D_PAL4						(0)		// １６色パレットカラ?標?
#define DX_GRAPHICSIMAGE_FORMAT_3D_PAL8						(1)		// ２５６色パレットカラ?標?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_PAL4				(2)		// α?ャンネルつき１６色パレットカラ?標?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_PAL8				(3)		// α?ャンネルつき２５６色パレットカラ?標?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ALPHATEST_PAL4			(4)		// αテストつき１６色パレットカラ?標?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ALPHATEST_PAL8			(5)		// αテストつき２５６色パレットカラ?標?
#define DX_GRAPHICSIMAGE_FORMAT_3D_RGB16					(6)		// １６ビットカラ?標?
#define DX_GRAPHICSIMAGE_FORMAT_3D_RGB32					(7)		// ３２ビットカラ?標?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_RGB16				(8)		// α?ャンネル付き１６ビットカラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_RGB32				(9)		// α?ャンネル付き３２ビットカラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ALPHATEST_RGB16			(10)	// αテスト付き１６ビットカラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ALPHATEST_RGB32			(11)	// αテスト付き３２ビットカラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DXT1						(12)	// DXT1
#define DX_GRAPHICSIMAGE_FORMAT_3D_DXT2						(13)	// DXT2
#define DX_GRAPHICSIMAGE_FORMAT_3D_DXT3						(14)	// DXT3
#define DX_GRAPHICSIMAGE_FORMAT_3D_DXT4						(15)	// DXT4
#define DX_GRAPHICSIMAGE_FORMAT_3D_DXT5 					(16)	// DXT5
#define DX_GRAPHICSIMAGE_FORMAT_3D_BC7_UNORM 				(17)	// BC7 UNORM
#define DX_GRAPHICSIMAGE_FORMAT_3D_BC7_UNORM_SRGB			(18)	// BC7 UNORM SRGB
#define DX_GRAPHICSIMAGE_FORMAT_3D_PLATFORM0				(19)	// プラットフォ??依存フォ??ット０
#define DX_GRAPHICSIMAGE_FORMAT_3D_PLATFORM1				(20)	// プラットフォ??依存フォ??ット１
#define DX_GRAPHICSIMAGE_FORMAT_3D_PLATFORM2				(21)	// プラットフォ??依存フォ??ット２
#define DX_GRAPHICSIMAGE_FORMAT_3D_PLATFORM3				(22)	// プラットフォ??依存フォ??ット３
#define DX_GRAPHICSIMAGE_FORMAT_3D_YUV	 					(23)	// ＹＵＶフォ??ット
#define DX_GRAPHICSIMAGE_FORMAT_3D_ABGR_I16					(24)	// ARGB整数16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ABGR_F16					(25)	// ARGB浮動小数?16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ABGR_F32					(26)	// ARGB浮動小数?32ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ONE_I8					(27)	// １?ャンネル整数8ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ONE_I16					(28)	// １?ャンネル整数16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ONE_F16					(29)	// １?ャンネル浮動少数16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_ONE_F32					(30)	// １?ャンネル浮動少数32ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_TWO_I8					(31)	// ２?ャンネル整数8ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_TWO_I16					(32)	// ２?ャンネル整数16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_TWO_F16					(33)	// ２?ャンネル浮動少数16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_TWO_F32					(34)	// ２?ャンネル浮動少数32ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_RGB16			(35)	// ?画可?１６ビットカラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_RGB32			(36)	// ?画可?３２ビットカラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ALPHA_RGB32	(37)	// ?画可?α付き３２ビットカラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_I16		(38)	// ?画可?ARGB整数16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F16		(39)	// ?画可?ARGB浮動小数?16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F32		(40)	// ?画可?ARGB浮動小数?32ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I8			(41)	// ?画可?１?ャンネル整数8ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I16		(42)	// ?画可?１?ャンネル整数16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F16		(43)	// ?画可?１?ャンネル浮動少数16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F32		(44)	// ?画可?１?ャンネル浮動少数32ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I8			(45)	// ?画可?２?ャンネル整数8ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I16		(46)	// ?画可?２?ャンネル整数16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F16		(47)	// ?画可?２?ャンネル浮動少数16ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F32		(48)	// ?画可?２?ャンネル浮動少数32ビット?カラ?
#define DX_GRAPHICSIMAGE_FORMAT_3D_NUM						(49)
#define DX_GRAPHICSIMAGE_FORMAT_2D							(50)	// 標?( DirectDrawSurface の場合はこれのみ )
#define DX_GRAPHICSIMAGE_FORMAT_R5G6B5						(51)	// R5G6B5( MEMIMG 用 )
#define DX_GRAPHICSIMAGE_FORMAT_X8A8R5G6B5					(52)	// X8A8R5G6B5( MEMIMG 用 )
#define DX_GRAPHICSIMAGE_FORMAT_X8R8G8B8					(53)	// X8R8G8B8( MEMIMG 用 )
#define DX_GRAPHICSIMAGE_FORMAT_A8R8G8B8					(54)	// A8R8G8B8( MEMIMG 用 )

#define DX_GRAPHICSIMAGE_FORMAT_NUM							(55)	// グラフィックフォ??ットの種類の数

// 基?イメ?ジのピクセルフォ??ット
#define DX_BASEIMAGE_FORMAT_NORMAL					(0)				// 普通の画像
#define DX_BASEIMAGE_FORMAT_DXT1					(1)				// ＤＸＴ１
#define DX_BASEIMAGE_FORMAT_DXT2					(2)				// ＤＸＴ２
#define DX_BASEIMAGE_FORMAT_DXT3					(3)				// ＤＸＴ３
#define DX_BASEIMAGE_FORMAT_DXT4					(4)				// ＤＸＴ４
#define DX_BASEIMAGE_FORMAT_DXT5					(5)				// ＤＸＴ５
#define DX_BASEIMAGE_FORMAT_BC7_UNORM				(6)				// ＢＣ７ UNORM
#define DX_BASEIMAGE_FORMAT_BC7_UNORM_SRGB			(7)				// ＢＣ７ UNORM SRGB
#define DX_BASEIMAGE_FORMAT_PLATFORM0				(8)				// プラットフォ??依存フォ??ット０
#define DX_BASEIMAGE_FORMAT_PLATFORM1				(9)				// プラットフォ??依存フォ??ット１
#define DX_BASEIMAGE_FORMAT_PLATFORM2				(10)			// プラットフォ??依存フォ??ット２
#define DX_BASEIMAGE_FORMAT_PLATFORM3				(11)			// プラットフォ??依存フォ??ット３
#define DX_BASEIMAGE_FORMAT_YUV						(12)			// ＹＵＶ

// ??ビ?のサ?フェスモ?ド
#define DX_MOVIESURFACE_NORMAL						(0)
#define DX_MOVIESURFACE_OVERLAY						(1)
#define DX_MOVIESURFACE_FULLCOLOR					(2)

// SetUserScreenImage で渡す画面のピクセルフォ??ット
#define DX_USER_SCREEN_PIXEL_FORMAT_R5G6B5			(0)				// R5G6B5( ＤＸライブラリの?画関数も使用できます( ?フトウェアレン?リングモ?ド相当 ) )
#define DX_USER_SCREEN_PIXEL_FORMAT_R5G5B5X1		(1)				// R5G5B5X1( ＤＸライブラリの?画関数は使用できなくなります )
#define DX_USER_SCREEN_PIXEL_FORMAT_X1R5G5B5		(2)				// X1R5G5B5( ＤＸライブラリの?画関数は使用できなくなります )
#define DX_USER_SCREEN_PIXEL_FORMAT_X8B8G8R8		(3)				// X8B8G8R8( ＤＸライブラリの?画関数は使用できなくなります )
#define DX_USER_SCREEN_PIXEL_FORMAT_X8R8G8B8		(4)				// X8R8G8B8( ＤＸライブラリの?画関数も使用できます( ?フトウェアレン?リングモ?ド相当 ) )
#define DX_USER_SCREEN_PIXEL_FORMAT_NUM				(5)

// ウインドウの奥行き位置設定?イプ
#define DX_WIN_ZTYPE_NORMAL							(0)				// 通常設定
#define DX_WIN_ZTYPE_BOTTOM							(1)				// 全てのウインドウの一番奥に配置する
#define DX_WIN_ZTYPE_TOP							(2)				// 全てのウインドウの一番手前に配置する
#define DX_WIN_ZTYPE_TOPMOST						(3)				// 全てのウインドウの一番手前に配置する( ウインドウがアクティブではないときも最前面に?示される )

// ツ?ルバ?の??ンの状態
#define TOOLBUTTON_STATE_ENABLE						(0)				// 入力可?な状態
#define TOOLBUTTON_STATE_PRESSED					(1)				// 押されている状態
#define TOOLBUTTON_STATE_DISABLE					(2)				// 入力不可?な状態
#define TOOLBUTTON_STATE_PRESSED_DISABLE			(3)				// 押されている状態で、入力不可?な状態
#define TOOLBUTTON_STATE_NUM						(4)				// ツ?ルバ?の??ンの状態の数

// ツ?ルバ?の??ンの?イプ
#define TOOLBUTTON_TYPE_NORMAL						(0)				// 普通の??ン
#define TOOLBUTTON_TYPE_CHECK						(1)				// 押すごとにＯＮ?ＯＦＦが切り替わる??ン
#define TOOLBUTTON_TYPE_GROUP						(2)				// 別の TOOLBUTTON_TYPE_GROUP ?イプの??ンが押されるとＯＦＦになる?イプの??ン(グル?プの区切りは隙間で)
#define TOOLBUTTON_TYPE_SEP							(3)				// 隙間(??ンではありません)
#define TOOLBUTTON_TYPE_NUM							(4)				// ツ?ルバ?の??ンの?イプの数

// 親メニュ?のＩＤ
#define MENUITEM_IDTOP								(0xabababab)

// メニュ?に追加する際の?イプ
#define MENUITEM_ADD_CHILD							(0)				// 指定の?目の子として追加する
#define MENUITEM_ADD_INSERT							(1)				// 指定の?目と指定の?目より一つ上の?目の間に追加する

// メニュ?の横に付く??ク?イプ
#define MENUITEM_MARK_NONE							(0)				// 何も付け無い
#define MENUITEM_MARK_CHECK							(1)				// ?ェック??ク
#define MENUITEM_MARK_RADIO							(2)				// ラジオ??ン

// 文字変換?イプ定?
#define DX_NUMMODE_10								(0)				// １０進数
#define DX_NUMMODE_16								(1)				// １６進数
#define DX_STRMODE_NOT0								(2)				// 空きを０で埋めない
#define DX_STRMODE_USE0								(3)				// 空きを０で埋める

// CheckHitKeyAll で調べる入力?イプ
#define DX_CHECKINPUT_KEY							(0x0001)		// キ?入力を調べる
#define DX_CHECKINPUT_PAD							(0x0002)		// パッド入力を調べる
#define DX_CHECKINPUT_MOUSE							(0x0004)		// ?ウス??ン入力を調べる
#define DX_CHECKINPUT_ALL							(DX_CHECKINPUT_KEY | DX_CHECKINPUT_PAD | DX_CHECKINPUT_MOUSE)	// すべての入力を調べる

// パッド入力取得パラメ??
#define DX_INPUT_KEY_PAD1							(0x1001)		// キ?入力とパッド１入力
#define DX_INPUT_PAD1								(0x0001)		// パッド１入力
#define DX_INPUT_PAD2								(0x0002)		// パッド２入力
#define DX_INPUT_PAD3								(0x0003)		// パッド３入力
#define DX_INPUT_PAD4								(0x0004)		// パッド４入力
#define DX_INPUT_PAD5								(0x0005)		// パッド５入力
#define DX_INPUT_PAD6								(0x0006)		// パッド６入力
#define DX_INPUT_PAD7								(0x0007)		// パッド７入力
#define DX_INPUT_PAD8								(0x0008)		// パッド８入力
#define DX_INPUT_PAD9								(0x0009)		// パッド９入力
#define DX_INPUT_PAD10								(0x000a)		// パッド１０入力
#define DX_INPUT_PAD11								(0x000b)		// パッド１１入力
#define DX_INPUT_PAD12								(0x000c)		// パッド１２入力
#define DX_INPUT_PAD13								(0x000d)		// パッド１３入力
#define DX_INPUT_PAD14								(0x000e)		// パッド１４入力
#define DX_INPUT_PAD15								(0x000f)		// パッド１５入力
#define DX_INPUT_PAD16								(0x0010)		// パッド１６入力
#define DX_INPUT_KEY								(0x1000)		// キ?入力

// ゲ??コントロ?ラ??イプ
#define DX_PADTYPE_OTHER							(0)
#define DX_PADTYPE_XBOX_360							(1)
#define DX_PADTYPE_XBOX_ONE							(2)
#define DX_PADTYPE_DUAL_SHOCK_3						(3)
#define DX_PADTYPE_DUAL_SHOCK_4						(4)
#define DX_PADTYPE_DUAL_SENSE						(5)
#define DX_PADTYPE_SWITCH_JOY_CON_L					(6)
#define DX_PADTYPE_SWITCH_JOY_CON_R					(7)
#define DX_PADTYPE_SWITCH_PRO_CTRL					(8)
#define DX_PADTYPE_SWITCH_HORI_PAD					(9)
#define DX_PADTYPE_NUM								(10)

// ?ッ?の同時接触検出対応最大数
#define TOUCHINPUTPOINT_MAX							(16)

// パッド入力定?
#define PAD_INPUT_DOWN								(0x00000001)	// ↓?ェック?スク
#define PAD_INPUT_LEFT								(0x00000002)	// ←?ェック?スク
#define PAD_INPUT_RIGHT								(0x00000004)	// →?ェック?スク
#define PAD_INPUT_UP								(0x00000008)	// ↑?ェック?スク
#define PAD_INPUT_A									(0x00000010)	// ???ン?ェック?スク
#define PAD_INPUT_B									(0x00000020)	// Ｂ??ン?ェック?スク
#define PAD_INPUT_C									(0x00000040)	// Ｃ??ン?ェック?スク
#define PAD_INPUT_X									(0x00000080)	// Ｘ??ン?ェック?スク
#define PAD_INPUT_Y									(0x00000100)	// Ｙ??ン?ェック?スク
#define PAD_INPUT_Z									(0x00000200)	// Ｚ??ン?ェック?スク
#define PAD_INPUT_L									(0x00000400)	// Ｌ??ン?ェック?スク
#define PAD_INPUT_R									(0x00000800)	// Ｒ??ン?ェック?スク
#define PAD_INPUT_START								(0x00001000)	// ＳＴ?ＲＴ??ン?ェック?スク
#define PAD_INPUT_M									(0x00002000)	// Ｍ??ン?ェック?スク
#define PAD_INPUT_D									(0x00004000)
#define PAD_INPUT_F									(0x00008000)
#define PAD_INPUT_G									(0x00010000)
#define PAD_INPUT_H									(0x00020000)
#define PAD_INPUT_I									(0x00040000)
#define PAD_INPUT_J									(0x00080000)
#define PAD_INPUT_K									(0x00100000)
#define PAD_INPUT_LL								(0x00200000)
#define PAD_INPUT_N									(0x00400000)
#define PAD_INPUT_O									(0x00800000)
#define PAD_INPUT_P									(0x01000000)
#define PAD_INPUT_RR								(0x02000000)
#define PAD_INPUT_S									(0x04000000)
#define PAD_INPUT_T									(0x08000000)
#define PAD_INPUT_U									(0x10000000)
#define PAD_INPUT_V									(0x20000000)
#define PAD_INPUT_W									(0x40000000)
#define PAD_INPUT_XX								(0x80000000)

#define PAD_INPUT_1									(0x00000010)
#define PAD_INPUT_2									(0x00000020)
#define PAD_INPUT_3									(0x00000040)
#define PAD_INPUT_4									(0x00000080)
#define PAD_INPUT_5									(0x00000100)
#define PAD_INPUT_6									(0x00000200)
#define PAD_INPUT_7									(0x00000400)
#define PAD_INPUT_8									(0x00000800)
#define PAD_INPUT_9									(0x00001000)
#define PAD_INPUT_10								(0x00002000)
#define PAD_INPUT_11								(0x00004000)
#define PAD_INPUT_12								(0x00008000)
#define PAD_INPUT_13								(0x00010000)
#define PAD_INPUT_14								(0x00020000)
#define PAD_INPUT_15								(0x00040000)
#define PAD_INPUT_16								(0x00080000)
#define PAD_INPUT_17								(0x00100000)
#define PAD_INPUT_18								(0x00200000)
#define PAD_INPUT_19								(0x00400000)
#define PAD_INPUT_20								(0x00800000)
#define PAD_INPUT_21								(0x01000000)
#define PAD_INPUT_22								(0x02000000)
#define PAD_INPUT_23								(0x04000000)
#define PAD_INPUT_24								(0x08000000)
#define PAD_INPUT_25								(0x10000000)
#define PAD_INPUT_26								(0x20000000)
#define PAD_INPUT_27								(0x40000000)
#define PAD_INPUT_28								(0x80000000)

// XInput??ン入力定?
#define XINPUT_BUTTON_DPAD_UP						(0)				// デジ?ル方向??ン上
#define XINPUT_BUTTON_DPAD_DOWN						(1)				// デジ?ル方向??ン下
#define XINPUT_BUTTON_DPAD_LEFT						(2)				// デジ?ル方向??ン左
#define XINPUT_BUTTON_DPAD_RIGHT					(3)				// デジ?ル方向??ン右
#define XINPUT_BUTTON_START							(4)				// START??ン
#define XINPUT_BUTTON_BACK							(5)				// BACK??ン
#define XINPUT_BUTTON_LEFT_THUMB					(6)				// 左スティック押し込み
#define XINPUT_BUTTON_RIGHT_THUMB					(7)				// 右スティック押し込み
#define XINPUT_BUTTON_LEFT_SHOULDER					(8)				// LB??ン
#define XINPUT_BUTTON_RIGHT_SHOULDER				(9)				// RB??ン
#define XINPUT_BUTTON_A								(12)			// A??ン
#define XINPUT_BUTTON_B								(13)			// B??ン
#define XINPUT_BUTTON_X								(14)			// X??ン
#define XINPUT_BUTTON_Y								(15)			// Y??ン

// ?ウス入力定?
#define MOUSE_INPUT_LEFT							(0x0001)		// ?ウス左??ン
#define MOUSE_INPUT_RIGHT							(0x0002)		// ?ウス右??ン
#define MOUSE_INPUT_MIDDLE							(0x0004)		// ?ウス中央??ン
#define MOUSE_INPUT_1								(0x0001)		// ?ウス１??ン
#define MOUSE_INPUT_2								(0x0002)		// ?ウス２??ン
#define MOUSE_INPUT_3								(0x0004)		// ?ウス３??ン
#define MOUSE_INPUT_4								(0x0008)		// ?ウス４??ン
#define MOUSE_INPUT_5								(0x0010)		// ?ウス５??ン
#define MOUSE_INPUT_6								(0x0020)		// ?ウス６??ン
#define MOUSE_INPUT_7								(0x0040)		// ?ウス７??ン
#define MOUSE_INPUT_8								(0x0080)		// ?ウス８??ン

// ?ウスのログ情報?イプ
#define MOUSE_INPUT_LOG_DOWN						(0)				// ??ンを押した
#define MOUSE_INPUT_LOG_UP							(1)				// ??ンを離した

// キ?定?
#define KEY_INPUT_BACK								(0x0E)			// BackSpaceキ?	D_DIK_BACK
#define KEY_INPUT_TAB								(0x0F)			// Tabキ?			D_DIK_TAB
#define KEY_INPUT_RETURN							(0x1C)			// Enterキ?		D_DIK_RETURN

#define KEY_INPUT_LSHIFT							(0x2A)			// 左Shiftキ?		D_DIK_LSHIFT
#define KEY_INPUT_RSHIFT							(0x36)			// 右Shiftキ?		D_DIK_RSHIFT
#define KEY_INPUT_LCONTROL							(0x1D)			// 左Ctrlキ?		D_DIK_LCONTROL
#define KEY_INPUT_RCONTROL							(0x9D)			// 右Ctrlキ?		D_DIK_RCONTROL
#define KEY_INPUT_ESCAPE							(0x01)			// Escキ?			D_DIK_ESCAPE
#define KEY_INPUT_SPACE								(0x39)			// スペ?スキ?		D_DIK_SPACE
#define KEY_INPUT_PGUP								(0xC9)			// PageUpキ?		D_DIK_PGUP
#define KEY_INPUT_PGDN								(0xD1)			// PageDownキ?		D_DIK_PGDN
#define KEY_INPUT_END								(0xCF)			// Endキ?			D_DIK_END
#define KEY_INPUT_HOME								(0xC7)			// Homeキ?			D_DIK_HOME
#define KEY_INPUT_LEFT								(0xCB)			// 左キ?			D_DIK_LEFT
#define KEY_INPUT_UP								(0xC8)			// 上キ?			D_DIK_UP
#define KEY_INPUT_RIGHT								(0xCD)			// 右キ?			D_DIK_RIGHT
#define KEY_INPUT_DOWN								(0xD0)			// 下キ?			D_DIK_DOWN
#define KEY_INPUT_INSERT							(0xD2)			// Insertキ?		D_DIK_INSERT
#define KEY_INPUT_DELETE							(0xD3)			// Deleteキ?		D_DIK_DELETE

#define KEY_INPUT_MINUS								(0x0C)			// ?キ?			D_DIK_MINUS
#define KEY_INPUT_YEN								(0x7D)			// ￥キ?			D_DIK_YEN
#define KEY_INPUT_PREVTRACK							(0x90)			// ＾キ?			D_DIK_PREVTRACK
#define KEY_INPUT_PERIOD							(0x34)			// ．キ?			D_DIK_PERIOD
#define KEY_INPUT_SLASH								(0x35)			// ?キ?			D_DIK_SLASH
#define KEY_INPUT_LALT								(0x38)			// 左Altキ?		D_DIK_LALT
#define KEY_INPUT_RALT								(0xB8)			// 右Altキ?		D_DIK_RALT
#define KEY_INPUT_SCROLL							(0x46)			// ScrollLockキ?	D_DIK_SCROLL
#define KEY_INPUT_SEMICOLON							(0x27)			// ；キ?			D_DIK_SEMICOLON
#define KEY_INPUT_COLON								(0x92)			// ：キ?			D_DIK_COLON
#define KEY_INPUT_LBRACKET							(0x1A)			// ［キ?			D_DIK_LBRACKET
#define KEY_INPUT_RBRACKET							(0x1B)			// ］キ?			D_DIK_RBRACKET
#define KEY_INPUT_AT								(0x91)			// ＠キ?			D_DIK_AT
#define KEY_INPUT_BACKSLASH							(0x2B)			// ?キ?			D_DIK_BACKSLASH
#define KEY_INPUT_COMMA								(0x33)			// ，キ?			D_DIK_COMMA
#define KEY_INPUT_KANJI								(0x94)			// 漢字キ?			D_DIK_KANJI
#define KEY_INPUT_CONVERT							(0x79)			// 変換キ?			D_DIK_CONVERT
#define KEY_INPUT_NOCONVERT							(0x7B)			// 無変換キ?		D_DIK_NOCONVERT
#define KEY_INPUT_KANA								(0x70)			// カナキ?			D_DIK_KANA
#define KEY_INPUT_APPS								(0xDD)			// アプリケ?ションメニュ?キ?		D_DIK_APPS
#define KEY_INPUT_CAPSLOCK							(0x3A)			// CaspLockキ?		D_DIK_CAPSLOCK
#define KEY_INPUT_SYSRQ								(0xB7)			// PrintScreenキ?	D_DIK_SYSRQ
#define KEY_INPUT_PAUSE								(0xC5)			// PauseBreakキ?	D_DIK_PAUSE
#define KEY_INPUT_LWIN								(0xDB)			// 左Winキ?		D_DIK_LWIN
#define KEY_INPUT_RWIN								(0xDC)			// 右Winキ?		D_DIK_RWIN

#define KEY_INPUT_NUMLOCK							(0x45)			// テンキ?NumLockキ?		D_DIK_NUMLOCK
#define KEY_INPUT_NUMPAD0							(0x52)			// テンキ?０				D_DIK_NUMPAD0
#define KEY_INPUT_NUMPAD1							(0x4F)			// テンキ?１				D_DIK_NUMPAD1
#define KEY_INPUT_NUMPAD2							(0x50)			// テンキ?２				D_DIK_NUMPAD2
#define KEY_INPUT_NUMPAD3							(0x51)			// テンキ?３				D_DIK_NUMPAD3
#define KEY_INPUT_NUMPAD4							(0x4B)			// テンキ?４				D_DIK_NUMPAD4
#define KEY_INPUT_NUMPAD5							(0x4C)			// テンキ?５				D_DIK_NUMPAD5
#define KEY_INPUT_NUMPAD6							(0x4D)			// テンキ?６				D_DIK_NUMPAD6
#define KEY_INPUT_NUMPAD7							(0x47)			// テンキ?７				D_DIK_NUMPAD7
#define KEY_INPUT_NUMPAD8							(0x48)			// テンキ?８				D_DIK_NUMPAD8
#define KEY_INPUT_NUMPAD9							(0x49)			// テンキ?９				D_DIK_NUMPAD9
#define KEY_INPUT_MULTIPLY							(0x37)			// テンキ?＊キ?			D_DIK_MULTIPLY
#define KEY_INPUT_ADD								(0x4E)			// テンキ??キ?			D_DIK_ADD
#define KEY_INPUT_SUBTRACT							(0x4A)			// テンキ??キ?			D_DIK_SUBTRACT
#define KEY_INPUT_DECIMAL							(0x53)			// テンキ?．キ?			D_DIK_DECIMAL
#define KEY_INPUT_DIVIDE							(0xB5)			// テンキ??キ?			D_DIK_DIVIDE
#define KEY_INPUT_NUMPADENTER						(0x9C)			// テンキ?のエン??キ?	D_DIK_NUMPADENTER

#define KEY_INPUT_F1								(0x3B)			// Ｆ１キ?			D_DIK_F1
#define KEY_INPUT_F2								(0x3C)			// Ｆ２キ?			D_DIK_F2
#define KEY_INPUT_F3								(0x3D)			// Ｆ３キ?			D_DIK_F3
#define KEY_INPUT_F4								(0x3E)			// Ｆ４キ?			D_DIK_F4
#define KEY_INPUT_F5								(0x3F)			// Ｆ５キ?			D_DIK_F5
#define KEY_INPUT_F6								(0x40)			// Ｆ６キ?			D_DIK_F6
#define KEY_INPUT_F7								(0x41)			// Ｆ７キ?			D_DIK_F7
#define KEY_INPUT_F8								(0x42)			// Ｆ８キ?			D_DIK_F8
#define KEY_INPUT_F9								(0x43)			// Ｆ９キ?			D_DIK_F9
#define KEY_INPUT_F10								(0x44)			// Ｆ１０キ?		D_DIK_F10
#define KEY_INPUT_F11								(0x57)			// Ｆ１１キ?		D_DIK_F11
#define KEY_INPUT_F12								(0x58)			// Ｆ１２キ?		D_DIK_F12

#define KEY_INPUT_A									(0x1E)			// ?キ?			D_DIK_A
#define KEY_INPUT_B									(0x30)			// Ｂキ?			D_DIK_B
#define KEY_INPUT_C									(0x2E)			// Ｃキ?			D_DIK_C
#define KEY_INPUT_D									(0x20)			// Ｄキ?			D_DIK_D
#define KEY_INPUT_E									(0x12)			// Ｅキ?			D_DIK_E
#define KEY_INPUT_F									(0x21)			// Ｆキ?			D_DIK_F
#define KEY_INPUT_G									(0x22)			// Ｇキ?			D_DIK_G
#define KEY_INPUT_H									(0x23)			// Ｈキ?			D_DIK_H
#define KEY_INPUT_I									(0x17)			// Ｉキ?			D_DIK_I
#define KEY_INPUT_J									(0x24)			// Ｊキ?			D_DIK_J
#define KEY_INPUT_K									(0x25)			// Ｋキ?			D_DIK_K
#define KEY_INPUT_L									(0x26)			// Ｌキ?			D_DIK_L
#define KEY_INPUT_M									(0x32)			// Ｍキ?			D_DIK_M
#define KEY_INPUT_N									(0x31)			// Ｎキ?			D_DIK_N
#define KEY_INPUT_O									(0x18)			// Ｏキ?			D_DIK_O
#define KEY_INPUT_P									(0x19)			// Ｐキ?			D_DIK_P
#define KEY_INPUT_Q									(0x10)			// Ｑキ?			D_DIK_Q
#define KEY_INPUT_R									(0x13)			// Ｒキ?			D_DIK_R
#define KEY_INPUT_S									(0x1F)			// Ｓキ?			D_DIK_S
#define KEY_INPUT_T									(0x14)			// Ｔキ?			D_DIK_T
#define KEY_INPUT_U									(0x16)			// Ｕキ?			D_DIK_U
#define KEY_INPUT_V									(0x2F)			// Ｖキ?			D_DIK_V
#define KEY_INPUT_W									(0x11)			// Ｗキ?			D_DIK_W
#define KEY_INPUT_X									(0x2D)			// Ｘキ?			D_DIK_X
#define KEY_INPUT_Y									(0x15)			// Ｙキ?			D_DIK_Y
#define KEY_INPUT_Z									(0x2C)			// Ｚキ?			D_DIK_Z

#define KEY_INPUT_0 								(0x0B)			// ０キ?			D_DIK_0
#define KEY_INPUT_1									(0x02)			// １キ?			D_DIK_1
#define KEY_INPUT_2									(0x03)			// ２キ?			D_DIK_2
#define KEY_INPUT_3									(0x04)			// ３キ?			D_DIK_3
#define KEY_INPUT_4									(0x05)			// ４キ?			D_DIK_4
#define KEY_INPUT_5									(0x06)			// ５キ?			D_DIK_5
#define KEY_INPUT_6									(0x07)			// ６キ?			D_DIK_6
#define KEY_INPUT_7									(0x08)			// ７キ?			D_DIK_7
#define KEY_INPUT_8									(0x09)			// ８キ?			D_DIK_8
#define KEY_INPUT_9									(0x0A)			// ９キ?			D_DIK_9

// アスキ?コントロ?ルキ?コ?ド
#define CTRL_CODE_BS								(0x08)			// バックスペ?ス
#define CTRL_CODE_TAB								(0x09)			// ?ブ
#define CTRL_CODE_CR								(0x0d)			// 改行
#define CTRL_CODE_DEL								(0x10)			// ＤＥＬキ?

#define CTRL_CODE_COPY								(0x03)			// コピ?
#define CTRL_CODE_PASTE								(0x16)			// ペ?スト
#define CTRL_CODE_CUT								(0x18)			// カット
#define CTRL_CODE_ALL								(0x01)			// 全て選択

#define CTRL_CODE_LEFT								(0x1d)			// ←キ?
#define CTRL_CODE_RIGHT								(0x1c)			// →キ?
#define CTRL_CODE_UP								(0x1e)			// ↑キ?
#define CTRL_CODE_DOWN								(0x1f)			// ↓キ?

#define CTRL_CODE_HOME								(0x1a)			// ＨＯＭＥ??ン
#define CTRL_CODE_END								(0x19)			// ＥＮＤ??ン
#define CTRL_CODE_PAGE_UP							(0x17)			// Ｐ?ＧＥ ＵＰ
#define CTRL_CODE_PAGE_DOWN							(0x15)			// Ｐ?ＧＥ ＤＯＷＮ

#define CTRL_CODE_ESC								(0x1b)			// ＥＳＣキ?
#define CTRL_CODE_CMP								(0x20)			// 制御コ?ド?居値

// SetKeyInputStringColor2 に渡す色変更対象を指定するための識別子
#define DX_KEYINPSTRCOLOR_NORMAL_STR					(0)			// 入力文字列の色
#define DX_KEYINPSTRCOLOR_NORMAL_STR_EDGE				(1)			// 入力文字列の縁の色
#define DX_KEYINPSTRCOLOR_NORMAL_CURSOR					(2)			// ＩＭＥ非使用時のカ??ルの色
#define DX_KEYINPSTRCOLOR_SELECT_STR					(3)			// 入力文字列の選択部分( SHIFTキ?を押しながら左右キ?で選択 )の色
#define DX_KEYINPSTRCOLOR_SELECT_STR_EDGE				(4)			// 入力文字列の選択部分( SHIFTキ?を押しながら左右キ?で選択 )の縁の色
#define DX_KEYINPSTRCOLOR_SELECT_STR_BACK				(5)			// 入力文字列の選択部分( SHIFTキ?を押しながら左右キ?で選択 )の周りの色
#define DX_KEYINPSTRCOLOR_IME_STR						(6)			// ＩＭＥ使用時の入力文字列の色
#define DX_KEYINPSTRCOLOR_IME_STR_EDGE					(7)			// ＩＭＥ使用時の入力文字列の縁の色
#define DX_KEYINPSTRCOLOR_IME_STR_BACK					(8)			// ＩＭＥ使用時の入力文字列の周りの色
#define DX_KEYINPSTRCOLOR_IME_CURSOR					(9)			// ＩＭＥ使用時のカ??ルの色
#define DX_KEYINPSTRCOLOR_IME_LINE						(10)		// ＩＭＥ使用時の変換文字列の下線の色
#define DX_KEYINPSTRCOLOR_IME_SELECT_STR				(11)		// ＩＭＥ使用時の選択対象の変換候補文字列の色
#define DX_KEYINPSTRCOLOR_IME_SELECT_STR_EDGE			(12)		// ＩＭＥ使用時の選択対象の変換候補文字列の縁の色
#define DX_KEYINPSTRCOLOR_IME_SELECT_STR_BACK			(13)		// ＩＭＥ使用時の選択対象の変換候補文字列の周りの色
#define DX_KEYINPSTRCOLOR_IME_CONV_WIN_STR				(14)		// ＩＭＥ使用時の変換候補ウインドウ内の文字列の色
#define DX_KEYINPSTRCOLOR_IME_CONV_WIN_STR_EDGE			(15)		// ＩＭＥ使用時の変換候補ウインドウ内の文字列の縁の色
#define DX_KEYINPSTRCOLOR_IME_CONV_WIN_SELECT_STR		(16)		// ＩＭＥ使用時の変換候補ウインドウ内で選択している文字列の色
#define DX_KEYINPSTRCOLOR_IME_CONV_WIN_SELECT_STR_EDGE	(17)		// ＩＭＥ使用時の変換候補ウインドウ内で選択している文字列の縁の色
#define DX_KEYINPSTRCOLOR_IME_CONV_WIN_SELECT_STR_BACK	(18)		// ＩＭＥ使用時の変換候補ウインドウ内で選択している文字列の周りの色
#define DX_KEYINPSTRCOLOR_IME_CONV_WIN_EDGE				(19)		// ＩＭＥ使用時の変換候補ウインドウの縁の色
#define DX_KEYINPSTRCOLOR_IME_CONV_WIN_BACK				(20)		// ＩＭＥ使用時の変換候補ウインドウの下地の色
#define DX_KEYINPSTRCOLOR_IME_MODE_STR					(21)		// ＩＭＥ使用時の入力モ?ド文字列の色(『全角ひらがな』等)
#define DX_KEYINPSTRCOLOR_IME_MODE_STR_EDGE				(22)		// ＩＭＥ使用時の入力モ?ド文字列の縁の色
#define DX_KEYINPSTRCOLOR_NUM							(23)

// 文字列入力処理の入力文字数が限界に達している状態で、文字列の末?部分で入力が行われた場合の処理モ?ド
#define DX_KEYINPSTR_ENDCHARAMODE_OVERWRITE				(0)			// 文字数が限界に達している状態で文字列の末?で文字が入力された場合は、最後の文字を上書き( デフォルト )
#define DX_KEYINPSTR_ENDCHARAMODE_NOTCHANGE				(1)			// 文字数が限界に達している状態で文字列の末?で文字が入力された場合は、何も変化しない

// ?ッ?パネルデ??のツ?ル?イプ
#define DX_TOUCHINPUT_TOOL_TYPE_UNKNOWN					(0)			// 不明
#define DX_TOUCHINPUT_TOOL_TYPE_FINGER					(1)			// 指
#define DX_TOUCHINPUT_TOOL_TYPE_STYLUS					(2)			// ス?イラスペン
#define DX_TOUCHINPUT_TOOL_TYPE_MOUSE					(3)			// ?ウス
#define DX_TOUCHINPUT_TOOL_TYPE_ERASER					(4)			// 消しゴ?

// フルスクリ?ン解像度モ?ド定?
#define DX_FSRESOLUTIONMODE_BORDERLESS_WINDOW		(0)				// 仮想フルスクリ?ンモ?ド( 実際にはフルスクリ?ンモ?ドにせず、????レスウィンドウをデスクトップ画面いっぱいに拡大して仮想的にフルスクリ?ンモ?ドを実現するモ?ド )
#define DX_FSRESOLUTIONMODE_DESKTOP					(1)				// モニ??の画面モ?ドをデスクトップ画面と同じにしてＤＸライブラリ画面を拡大して?示するモ?ド
#define DX_FSRESOLUTIONMODE_NATIVE					(2)				// モニ??の解像度をＤＸライブラリ画面の解像度に合わせるモ?ド
#define DX_FSRESOLUTIONMODE_MAXIMUM					(3)				// モニ??の解像度を最大にしてＤＸライブラリ画面を拡大して?示するモ?ド

// フルスクリ?ン拡大モ?ド定?
#define DX_FSSCALINGMODE_BILINEAR					(0)				// バイリニアモ?ド( ピクセルが滲んでピクセルとピクセルの区切りがはっきりしない )
#define DX_FSSCALINGMODE_NEAREST					(1)				// 最近?モ?ド( ピクセルが四角くくっきり?示される )

// SetGraphMode 戻り値定?
#define DX_CHANGESCREEN_OK							(0)				// 画面変更は成功した
#define DX_CHANGESCREEN_RETURN						(-1)			// 画面の変更は失敗し、元の画面モ?ドに戻された
#define DX_CHANGESCREEN_DEFAULT						(-2)			// 画面の変更は失敗しデフォルトの画面モ?ドに変更された
#define DX_CHANGESCREEN_REFRESHNORMAL				(-3)			// 画面の変更は成功したが、リフレッシュレ?トの変更は失敗した

// ストリ??デ??読み込み処理コ?ド簡略化関連
#define STTELL( st )								((st)->ReadShred.Tell( (st)->DataPoint ))
#define STSEEK( st, pos, type )						((st)->ReadShred.Seek( (st)->DataPoint, (pos), (type) ))
#define STREAD( buf, length, num, st )				((st)->ReadShred.Read( (buf), (length), (num), (st)->DataPoint ))
#define STWRITE( buf, length, num, st )				((st)->ReadShred.Write( (buf), (length), (num), (st)->DataPoint ))
#define STEOF( st )									((st)->ReadShred.Eof( (st)->DataPoint ))
#define STCLOSE( st )								((st)->ReadShred.Close( (st)->DataPoint ))

// ストリ??デ??制御のシ?ク?イプ定?
#define STREAM_SEEKTYPE_SET							(SEEK_SET)
#define STREAM_SEEKTYPE_END							(SEEK_END)
#define STREAM_SEEKTYPE_CUR							(SEEK_CUR)

// グラフィックロ?ド時のイメ?ジ?イプ
#define LOADIMAGE_TYPE_FILE							(0)				// イメ?ジはフ?イルである
#define LOADIMAGE_TYPE_MEM							(1)				// イメ?ジはメモリである
#define LOADIMAGE_TYPE_NONE							(-1)			// イメ?ジは無い



#ifndef DX_NON_NETWORK

// HTTP エラ?
#define HTTP_ERR_SERVER								(0)				// サ?バ?エラ?
#define HTTP_ERR_NOTFOUND							(1)				// フ?イルが見つからなかった
#define HTTP_ERR_MEMORY								(2)				// メモリ確保の失敗
#define HTTP_ERR_LOST								(3)				// 途中で切断された
#define HTTP_ERR_NONE								(-1)			// エラ?は報告されていない

// HTTP 処理の結果
#define HTTP_RES_COMPLETE							(0)				// 処理完了
#define HTTP_RES_STOP								(1)				// 処理中?
#define HTTP_RES_ERROR								(2)				// エラ?終了
#define HTTP_RES_NOW								(-1)			// 現在進行中

#endif // DX_NON_NETWORK

#define DX_DEFINE_END

// デ???定?-------------------------------------------------------------------

#if defined( __cplusplus ) && defined( DX_COMPILE_TYPE_C_LANGUAGE )
extern "C" {
#endif

// WAVEFORMATEX の定?
#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_

typedef struct tWAVEFORMATEX
{
	WORD					wFormatTag ;					// フォ??ット( WAVE_FORMAT_PCM( 値は 1 ) 等 )
	WORD					nChannels ;						// ?ャンネル数
	DWORD					nSamplesPerSec ;				// １秒辺りのサンプル数
	DWORD					nAvgBytesPerSec ;				// １秒辺りのバイト数( PCMの場合 nSamplesPerSec * nBlockAlign )
	WORD					nBlockAlign ;					// 全?ャンネルの１サンプルを合わせたバイト数( wBitsPerSample / 8 * nChannels )
	WORD					wBitsPerSample ;				// １サンプル辺りのビット数
	WORD					cbSize ;						// 拡張情報のバイト数( 拡張情報が無い場合は 0 )
} WAVEFORMATEX, *PWAVEFORMATEX, NEAR *NPWAVEFORMATEX, FAR *LPWAVEFORMATEX ;

#endif // _WAVEFORMATEX_

// WAVEFORMAT の定?
#ifndef WAVE_FORMAT_PCM

typedef struct waveformat_tag
{
	WORD					wFormatTag ;					// フォ??ット( WAVE_FORMAT_PCM( 値は 1 ) 等 )
	WORD					nChannels ;						// ?ャンネル数
	DWORD					nSamplesPerSec ;				// １秒辺りのサンプル数
	DWORD					nAvgBytesPerSec ;				// １秒辺りのバイト数( PCMの場合 nSamplesPerSec * nBlockAlign )
	WORD					nBlockAlign ;					// 全?ャンネルの１サンプルを合わせたバイト数( wBitsPerSample / 8 * nChannels )
} WAVEFORMAT, *PWAVEFORMAT, NEAR *NPWAVEFORMAT, FAR *LPWAVEFORMAT ;

#define WAVE_FORMAT_PCM		1

#endif // WAVE_FORMAT_PCM

#ifndef DX_NON_NAMESPACE

namespace DxLib
{

#endif // DX_NON_NAMESPACE

#define DX_STRUCT_START

// ＩＭＥ入力文字列の?画に必要な情報の内の文節情報
typedef struct tagIMEINPUTCLAUSEDATA
{
	int						Position ;						// 何文字目から
	int						Length ;						// 何文字か
} IMEINPUTCLAUSEDATA, *LPIMEINPUTCLAUSEDATA ;

// ＩＭＥ入力文字列の?画に必要な情報
typedef struct tagIMEINPUTDATA
{
	const TCHAR *				InputString ;				// 入力中の文字列

	int							CursorPosition ;			// カ??ルの入力文字列中の位置(バイト単位)

	const IMEINPUTCLAUSEDATA *	ClauseData ;				// 文節情報
	int							ClauseNum ;					// 文節情報の数
	int							SelectClause ;				// 選択中の分節( -1 の場合はどの文節にも属していない( 末尾にカ??ルがある ) )

	int							CandidateNum ;				// 変換候補の数( 0の場合は変換中ではない )
	const TCHAR **				CandidateList ;				// 変換候補文字列リスト( 例：ｎ番目の候補を?画する場合  DrawString( 0, 0, data.CandidateList[ n ], GetColor(255,255,255) ); )
	int							SelectCandidate ;			// 選択中の変換候補

	int							ConvertFlag ;				// 文字変換中かどうか( TRUE:変換中  FALSE:変換中ではない( 文字単位でカ??ルが移動できる状態 ) )
} IMEINPUTDATA, *LPIMEINPUTDATA ;

// ?画文字列一文字の情報
typedef struct tagDRAWCHARINFO
{
	TCHAR					Char[ 14 ] ;					// 文字
	WORD					Bytes ;							// 文字のバイト数
	float					DrawX, DrawY ;					// ?画位置
	float					SizeX, SizeY ;					// ?画サイズ
} DRAWCHARINFO, *LPDRAWCHARINFO ;

// 画面モ?ド情報デ???
typedef struct tagDISPLAYMODEDATA
{
	int						Width ;							// 水平解像度
	int						Height ;						// 垂直解像度
	int						ColorBitDepth ;					// 色ビット?度
	int						RefreshRate ;					// リフレッシュレ?ト( -1 の場合は規定値 )
} DISPLAYMODEDATA, *LPDISPLAYMODEDATA ;

// ?イ?デ???
typedef struct tagDATEDATA
{
	int						Year ;							// 年
	int						Mon ;							// 月
	int						Day ;							// 日
	int						Hour ;							// 時間
	int						Min ;							// 分
	int						Sec ;							// 秒
} DATEDATA, *LPDATEDATA ;

// フ?イル情報?造体
typedef struct tagFILEINFO
{
	TCHAR					Name[ 260 ] ;					// オブジェクト名
	int						DirFlag ;						// ディレクトリかどうか( TRUE:ディレクトリ  FALSE:フ?イル )
	LONGLONG				Size ;							// サイズ
	DATEDATA				CreationTime ;					// 作成日時
	DATEDATA				LastWriteTime ;					// 最終更新日時
} FILEINFO, *LPFILEINFO ;

// フ?イル情報?造体（ wchar_t 版 ）
typedef struct tagFILEINFOW
{
	wchar_t					Name[ 260 ] ;					// オブジェクト名
	int						DirFlag ;						// ディレクトリかどうか( TRUE:ディレクトリ  FALSE:フ?イル )
	LONGLONG				Size ;							// サイズ
	DATEDATA				CreationTime ;					// 作成日時
	DATEDATA				LastWriteTime ;					// 最終更新日時
} FILEINFOW, *LPFILEINFOW ;

// 行列?造体
typedef struct tagMATRIX
{
	float					m[4][4] ;
} MATRIX, *LPMATRIX ;

typedef struct tagMATRIX_D
{
	double					m[4][4] ;
} MATRIX_D, *LPMATRIX_D ;

// ベクトルデ???
typedef struct tagVECTOR
{
	float					x, y, z ;
} VECTOR, *LPVECTOR, FLOAT3, *LPFLOAT3 ;

typedef struct tagVECTOR_D
{
	double					x, y, z ;
} VECTOR_D, *LPVECTOR_D, DOUBLE3, *LPDOUBLE3 ;

// FLOAT2個デ???
typedef struct tagFLOAT2
{
	float					u, v ;
} FLOAT2 ;

// float ?のカラ?値
typedef struct tagCOLOR_F
{
	float					r, g, b, a ;
} COLOR_F, *LPCOLOR_F ;

// unsigned char ?のカラ?値
typedef struct tagCOLOR_U8
{
	BYTE					b, g, r, a ;
} COLOR_U8 ;

// FLOAT4個デ???
typedef struct tagFLOAT4
{
	float					x, y, z, w ;
} FLOAT4, *LPFLOAT4 ;

// DOUBLE4個デ???
typedef struct tagDOUBLE4
{
	double					x, y, z, w ;
} DOUBLE4, *LPDOUBLE4 ;

// INT4個デ???
typedef struct tagINT4
{
	int						x, y, z, w ;
} INT4 ;

#ifndef DX_NOTUSE_DRAWFUNCTION

// ２Ｄ?画に使用する頂?デ???(DrawPrimitive2D用)
typedef struct tagVERTEX2D
{
	VECTOR					pos ;
	float					rhw ;
	COLOR_U8				dif ;
	float					u, v ;
} VERTEX2D, *LPVERTEX2D ; 

// ２Ｄ?画に使用する頂?デ???(DrawPrimitive2DToShader用)
typedef struct tagVERTEX2DSHADER
{
	VECTOR					pos ;
	float					rhw ;
	COLOR_U8				dif ;
	COLOR_U8				spc ;
	float					u, v ;
	float					su, sv ;
} VERTEX2DSHADER, *LPVERTEX2DSHADER ; 

// ２Ｄ?画に使用する頂?デ???(公開用)
typedef struct tagVERTEX
{
	float					x, y ;
	float					u, v ;
	unsigned char			b, g, r, a ;
} VERTEX ;

// ３Ｄ?画に使用する頂?デ???( 旧バ?ジョンのもの )
typedef struct tagVERTEX_3D
{
	VECTOR					pos ;
	unsigned char			b, g, r, a ;
	float					u, v ;
} VERTEX_3D, *LPVERTEX_3D ;

// ３Ｄ?画に使用する頂?デ???
typedef struct tagVERTEX3D
{
	VECTOR					pos ;							// 座標
	VECTOR					norm ;							// ?線
	COLOR_U8				dif ;							// ディフュ?ズカラ?
	COLOR_U8				spc ;							// スペキュラカラ?
	float					u, v ;							// テクス?ャ座標
	float					su, sv ;						// 補助テクス?ャ座標
} VERTEX3D, *LPVERTEX3D ;

// ３Ｄ?画に使用する頂?デ???( DrawPrimitive3DToShader用 )
// 注意…メンバ変数に追加があるかもしれませんので、宣言時の初期化( VERTEX3DSHADER Vertex = { 0.0f, 0.0f, ... というようなもの )はしない方が良いです
typedef struct tagVERTEX3DSHADER
{
	VECTOR					pos ;							// 座標
	FLOAT4					spos ;							// 補助座標
	VECTOR					norm ;							// ?線
	VECTOR					tan ;							// 接線
	VECTOR					binorm ;						// ??線
	COLOR_U8				dif ;							// ディフュ?ズカラ?
	COLOR_U8				spc ;							// スペキュラカラ?
	float					u, v ;							// テクス?ャ座標
	float					su, sv ;						// 補助テクス?ャ座標
} VERTEX3DSHADER, *LPVERTEX3DSHADER ;

// ライトパラメ??
typedef struct tagLIGHTPARAM
{
	int						LightType ;						// ライトの?イプ( DX_LIGHTTYPE_D3DLIGHT_POINT 等 )
	COLOR_F					Diffuse ;						// ディフュ?ズカラ?
	COLOR_F					Specular ;						// スペキュラカラ?
	COLOR_F					Ambient ;						// アンビエント色
	VECTOR					Position ;						// 位置
	VECTOR					Direction ;						// 方向
	float					Range ;							// 有効距離
	float					Falloff ;						// フォ?ルオフ 1.0f にしておくのが好ましい
	float					Attenuation0 ;					// 距離による減衰係数０
	float					Attenuation1 ;					// 距離による減衰係数１
	float					Attenuation2 ;					// 距離による減衰係数２
	float					Theta ;							// ス?ットライトの内部コ?ンの照明角度( ラジアン )
	float					Phi ;							// ス?ットライトの外部コ?ンの照明角度
} LIGHTPARAM ;

// ?テリアルパラメ??
typedef struct tagMATERIALPARAM
{
	COLOR_F					Diffuse ;						// ディフュ?ズカラ?
	COLOR_F					Ambient ;						// アンビエントカラ?
	COLOR_F					Specular ;						// スペキュラカラ?
	COLOR_F					Emissive ;						// エ?ッシブカラ?
	float					Power ;							// スペキュラハイライトの鮮明度
} MATERIALPARAM ;

#endif // DX_NOTUSE_DRAWFUNCTION

// ラインヒット?ェック結果格?用?造体
typedef struct tagHITRESULT_LINE
{
	int						HitFlag ;						// 当たったかどうか( 1:当たった  0:当たらなかった )
	VECTOR					Position ;						// 当たった座標
} HITRESULT_LINE ;

// ラインヒット?ェック結果格?用?造体
typedef struct tagHITRESULT_LINE_D
{
	int						HitFlag ;						// 当たったかどうか( 1:当たった  0:当たらなかった )
	VECTOR_D				Position ;						// 当たった座標
} HITRESULT_LINE_D ;

// 関数 Segment_Segment_Analyse の結果を受け取る為の?造体
typedef struct tagSEGMENT_SEGMENT_RESULT
{
	float					SegA_SegB_MinDist_Square ;		// 線分?と線分Ｂが最も接近する座標間の距離の二乗

	float					SegA_MinDist_Pos1_Pos2_t ;		// 線分?と線分Ｂに最も接近する座標の線分?の t ( 0.0f ? 1.0f 、最近?座標 = ( SegAPos2 - SegAPos1 ) * t + SegAPos1 )
	float					SegB_MinDist_Pos1_Pos2_t ;		// 線分Ｂが線分?に最も接近する座標の線分Ｂの t ( 0.0f ? 1.0f 、最近?座標 = ( SegBPos2 - SegBPos1 ) * t + SegBPos1 )

	VECTOR					SegA_MinDist_Pos ;				// 線分?が線分Ｂに最も接近する線分?上の座標
	VECTOR					SegB_MinDist_Pos ;				// 線分Ｂが線分?に最も接近する線分Ｂ上の座標
} SEGMENT_SEGMENT_RESULT ;

// 関数 Segment_Segment_Analyse の結果を受け取る為の?造体
typedef struct tagSEGMENT_SEGMENT_RESULT_D
{
	double					SegA_SegB_MinDist_Square ;		// 線分?と線分Ｂが最も接近する座標間の距離の二乗

	double					SegA_MinDist_Pos1_Pos2_t ;		// 線分?と線分Ｂに最も接近する座標の線分?の t ( 0.0 ? 1.0 、最近?座標 = ( SegAPos2 - SegAPos1 ) * t + SegAPos1 )
	double					SegB_MinDist_Pos1_Pos2_t ;		// 線分Ｂが線分?に最も接近する座標の線分Ｂの t ( 0.0 ? 1.0 、最近?座標 = ( SegBPos2 - SegBPos1 ) * t + SegBPos1 )

	VECTOR_D				SegA_MinDist_Pos ;				// 線分?が線分Ｂに最も接近する線分?上の座標
	VECTOR_D				SegB_MinDist_Pos ;				// 線分Ｂが線分?に最も接近する線分Ｂ上の座標
} SEGMENT_SEGMENT_RESULT_D ;

// 関数 Segment_Point_Analyse の結果を受け取る為の?造体
typedef struct tagSEGMENT_POINT_RESULT
{
	float					Seg_Point_MinDist_Square ;		// 線分と?が最も接近する座標間の距離の二乗
	float					Seg_MinDist_Pos1_Pos2_t ;		// 線分が?に最も接近する座標の線分の t ( 0.0f ? 1.0f 、最近?座標 = ( SegPos2 - SegPos1 ) * t + SegPos1 )
	VECTOR					Seg_MinDist_Pos ;				// 線分が?に最も接近する線分上の座標
} SEGMENT_POINT_RESULT ;

// 関数 Segment_Point_Analyse の結果を受け取る為の?造体
typedef struct tagSEGMENT_POINT_RESULT_D
{
	double					Seg_Point_MinDist_Square ;		// 線分と?が最も接近する座標間の距離の二乗
	double					Seg_MinDist_Pos1_Pos2_t ;		// 線分が?に最も接近する座標の線分の t ( 0.0 ? 1.0 、最近?座標 = ( SegPos2 - SegPos1 ) * t + SegPos1 )
	VECTOR_D				Seg_MinDist_Pos ;				// 線分が?に最も接近する線分上の座標
} SEGMENT_POINT_RESULT_D ;

// 関数 Segment_Triangle_Analyse の結果を受け取る為の?造体
typedef struct tagSEGMENT_TRIANGLE_RESULT
{
	float					Seg_Tri_MinDist_Square ;		// 線分と三角?が最も接近する座標間の距離の二乗

	float					Seg_MinDist_Pos1_Pos2_t ;		// 線分が三角?に最も接近する座標の線分の t ( 0.0f ? 1.0f 、最近?座標 = ( SegPos2 - SegPos1 ) * t + SegPos1 )
	VECTOR					Seg_MinDist_Pos ;				// 線分が三角?に最も接近する線分上の座標

	float					Tri_MinDist_Pos1_w ;			// 三角?が線分に最も接近する座標の三角?座標１の重み( 最近?座標 = TriPos1 * TriPos1_w + TriPos2 * TriPos2_w + TriPos3 * TriPos3_w )
	float					Tri_MinDist_Pos2_w ;			// 三角?が線分に最も接近する座標の三角?座標２の重み
	float					Tri_MinDist_Pos3_w ;			// 三角?が線分に最も接近する座標の三角?座標３の重み
	VECTOR					Tri_MinDist_Pos ;				// 三角?が線分に最も接近する三角?上の座標
} SEGMENT_TRIANGLE_RESULT ;

// 関数 Segment_Triangle_Analyse の結果を受け取る為の?造体
typedef struct tagSEGMENT_TRIANGLE_RESULT_D
{
	double					Seg_Tri_MinDist_Square ;		// 線分と三角?が最も接近する座標間の距離の二乗

	double					Seg_MinDist_Pos1_Pos2_t ;		// 線分が三角?に最も接近する座標の線分の t ( 0.0 ? 1.0 、最近?座標 = ( SegPos2 - SegPos1 ) * t + SegPos1 )
	VECTOR_D				Seg_MinDist_Pos ;				// 線分が三角?に最も接近する線分上の座標

	double					Tri_MinDist_Pos1_w ;			// 三角?が線分に最も接近する座標の三角?座標１の重み( 最近?座標 = TriPos1 * TriPos1_w + TriPos2 * TriPos2_w + TriPos3 * TriPos3_w )
	double					Tri_MinDist_Pos2_w ;			// 三角?が線分に最も接近する座標の三角?座標２の重み
	double					Tri_MinDist_Pos3_w ;			// 三角?が線分に最も接近する座標の三角?座標３の重み
	VECTOR_D				Tri_MinDist_Pos ;				// 三角?が線分に最も接近する三角?上の座標
} SEGMENT_TRIANGLE_RESULT_D ;

// 関数 Triangle_Point_Analyse の結果を受け取る為の?造体
typedef struct tagTRIANGLE_POINT_RESULT
{
	float					Tri_Pnt_MinDist_Square ;		// 三角?と?が最も接近する座標間の距離の二乗

	float					Tri_MinDist_Pos1_w ;			// 三角?が?に最も接近する座標の三角?座標１の重み( 最近?座標 = TriPos1 * TriPos1_w + TriPos2 * TriPos2_w + TriPos3 * TriPos3_w )
	float					Tri_MinDist_Pos2_w ;			// 三角?が?に最も接近する座標の三角?座標２の重み
	float					Tri_MinDist_Pos3_w ;			// 三角?が?に最も接近する座標の三角?座標３の重み
	VECTOR					Tri_MinDist_Pos ;				// 三角?が?に最も接近する三角?上の座標
} TRIANGLE_POINT_RESULT ;

// 関数 Triangle_Point_Analyse の結果を受け取る為の?造体
typedef struct tagTRIANGLE_POINT_RESULT_D
{
	double					Tri_Pnt_MinDist_Square ;		// 三角?と?が最も接近する座標間の距離の二乗

	double					Tri_MinDist_Pos1_w ;			// 三角?が?に最も接近する座標の三角?座標１の重み( 最近?座標 = TriPos1 * TriPos1_w + TriPos2 * TriPos2_w + TriPos3 * TriPos3_w )
	double					Tri_MinDist_Pos2_w ;			// 三角?が?に最も接近する座標の三角?座標２の重み
	double					Tri_MinDist_Pos3_w ;			// 三角?が?に最も接近する座標の三角?座標３の重み
	VECTOR_D				Tri_MinDist_Pos ;				// 三角?が?に最も接近する三角?上の座標
} TRIANGLE_POINT_RESULT_D ;

// 関数 Plane_Point_Analyse の結果を受け取る為の?造体
typedef struct tagPLANE_POINT_RESULT
{
	int						Pnt_Plane_Normal_Side ;			// ?が平面の?線の側にあるかどうか( 1:?線の側にある  0:?線と反対側にある )
	float					Plane_Pnt_MinDist_Square ;		// 平面と?の距離
	VECTOR					Plane_MinDist_Pos ;				// 平面上の?との最近?座標
} PLANE_POINT_RESULT ;

// 関数 Plane_Point_Analyse の結果を受け取る為の?造体
typedef struct tagPLANE_POINT_RESULT_D
{
	int						Pnt_Plane_Normal_Side ;			// ?が平面の?線の側にあるかどうか( 1:?線の側にある  0:?線と反対側にある )
	double					Plane_Pnt_MinDist_Square ;		// 平面と?の距離
	VECTOR_D				Plane_MinDist_Pos ;				// 平面上の?との最近?座標
} PLANE_POINT_RESULT_D ;

#ifndef DX_NOTUSE_DRAWFUNCTION

// コリジョン結果代入用?リゴン
typedef struct tagMV1_COLL_RESULT_POLY
{
	int						HitFlag ;						// ( MV1CollCheck_Line でのみ有効 )ヒットフラグ( 1:ヒットした  0:ヒットしなかった )
	VECTOR					HitPosition ;					// ( MV1CollCheck_Line でのみ有効 )ヒット座標

	int						FrameIndex ;					// 当たった?リゴンが含まれるフレ??の番号
	int						MeshIndex ;						// 当たった?リゴンが含まれるメッシュの番号( メッシュ単位で判定した場合のみ有効 )
	int						PolygonIndex ;					// 当たった?リゴンの番号
	int						MaterialIndex ;					// 当たった?リゴンが使用している?テリアルの番号
	VECTOR					Position[ 3 ] ;					// 当たった?リゴンを?成する三?の座標
	VECTOR					Normal ;						// 当たった?リゴンの?線
	float					PositionWeight[ 3 ] ;			// 当たった座標は、当たった?リゴンの三?それぞれどの割合で影響しているか、の値( 当たった座標 = Position[ 0 ] * PositionWeight[ 0 ] + Position[ 1 ] * PositionWeight[ 1 ] + Position[ 2 ] * PositionWeight[ 2 ] )
	int						PosMaxWeightFrameIndex[ 3 ] ;	// 当たった?リゴンの座標がそれぞれ最も影響を受けているフレ??の番号
} MV1_COLL_RESULT_POLY ;

// コリジョン結果代入用?リゴン配列
typedef struct tagMV1_COLL_RESULT_POLY_DIM
{
	int						HitNum ;						// ヒットした?リゴンの数
	MV1_COLL_RESULT_POLY *	Dim ;							// ヒットした?リゴンの配列( HitNum個分存在する )
} MV1_COLL_RESULT_POLY_DIM ;

// 参照用頂??造体
typedef struct tagMV1_REF_VERTEX
{
	VECTOR					Position ;						// 位置
	VECTOR					Normal ;						// ?線
	FLOAT2					TexCoord[ 2 ] ;					// テクス?ャ座標
	COLOR_U8				DiffuseColor ;					// ディフュ?ズカラ?
	COLOR_U8				SpecularColor ;					// スペキュラカラ?
	int						MaxWeightFrameIndex ;			// 最も大きな影響を?えているフレ??( スキニングメッシュ用 )
} MV1_REF_VERTEX ;

// 参照用?リゴン?造体
typedef struct tagMV1_REF_POLYGON
{
	unsigned short			FrameIndex ;					// この?リゴンが属しているフレ??
	unsigned short			MeshIndex ;						// この?リゴンが属しているメッシュ
	unsigned short			MaterialIndex ;					// 使用している?テリアル
	unsigned short			VIndexTarget ;					// VIndex が指すインデックスの参照先( 1:フレ??  0:モデル全体 )
	int						VIndex[ 3 ] ;					// ３角??リゴンを成す参照頂?のインデックス
	VECTOR					MinPosition ;					// ?リゴンを成す頂?座標の最小値
	VECTOR					MaxPosition ;					// ?リゴンを成す頂?座標の最大値
} MV1_REF_POLYGON ;

// 参照用?リゴンデ???造体
typedef struct tagMV1_REF_POLYGONLIST
{
	int						PolygonNum ;					// 参照用?リゴンの数
	int						VertexNum ;						// 頂?の数
	VECTOR					MinPosition ;					// 頂?座標の最小値
	VECTOR					MaxPosition ;					// 頂?座標の最大値
	MV1_REF_POLYGON	*		Polygons ;						// 参照用?リゴン配列
	MV1_REF_VERTEX *		Vertexs ;						// 参照用頂?配列
} MV1_REF_POLYGONLIST ;

#endif // DX_NOTUSE_DRAWFUNCTION




// ３Ｄサウンドリバ?ブエフェクトパラメ???造体
// ( 注釈は MSDN の XAUDIO2FX_REVERB_PARAMETERS ?造体の解説をほぼ引用しています )
typedef struct tagSOUND3D_REVERB_PARAM
{
	float					WetDryMix ;						// リバ?ブとなる出力の割合( 指定可?範囲 0.0f ? 100.0f )

	unsigned int			ReflectionsDelay ;				// ?イレクト パスに対する初期反射の遅延時間、単位は?リ秒( 指定可?範囲 0 ? 300 )
	BYTE					ReverbDelay ;					// 初期反射に対するリバ?ブの遅延時間、単位は?リ秒( 指定可?範囲 0 ? 85 )
	BYTE					RearDelay ;						// 左後方出力および右後方出力の遅延時間、単位は?リ秒( 指定可?範囲 0 ? 5 )

	BYTE					PositionLeft ;					// シ?ュレ?ション空間における視聴者に対する左入力の位置( 指定可?範囲 0 ? 30 )
															// PositionLeft を最小値に設定した場合、左入力は視聴者の近くに配置されます。
															// この位置では、サウンド フィ?ルドにおいて初期反射が優勢になり、残響減衰は弱まって、振幅が小さくなります。
															// PositionLeft を最大値に設定した場合、左入力はシ?ュレ?ション室内で視聴者から最大限遠い位置に配置されます。
															// PositionLeft は残響減衰時間 (部屋の残響効果) に影響せず、視聴者に対する音源の見かけの位置のみに影響します。
	BYTE					PositionRight ;					// PositionLeft と同効果の右入力値( 指定可?範囲 0 ? 30 )、右入力にのみ影響を?える
	BYTE					PositionMatrixLeft ;			// 音源から視聴者までの距離によるインプレッションを増減させる値( 指定可?範囲 0 ? 30 )
	BYTE					PositionMatrixRight ;			// 音源から視聴者までの距離によるインプレッションを増減させま値( 指定可?範囲 0 ? 30 )
	BYTE					EarlyDiffusion ;				// 個々の壁の反射特性値( 指定可?範囲 0 ? 15 )、( 堅く平らな?面をシ?ュレ?トするには小さな値を設定し、散乱性の?面をシ?ュレ?トするには大きな値を設定します。)
	BYTE					LateDiffusion ;					// 個々の壁のリバ?ブ特性値( 指定可?範囲 0 ? 15 )、( 堅く平らな?面をシ?ュレ?トするには小さな値を設定し、散乱性の?面をシ?ュレ?トするには大きな値を設定します。)
	BYTE					LowEQGain ;						// 1 kHz における減衰時間を基?にして低周波数の減衰時間調整値( 指定可?範囲 0 ? 12 )
															// 値とゲイン (dB) の関係
															// 値          0  1  2  3  4  5  6  7  8  9 10 11 12
															// ゲイン(dB) -8 -7 -6 -5 -4 -3 -2 -1  0 +1 +2 +3 +4
															// LowEQGain の値が 8 の場合、低周波数の減衰時間と 1 kHz における減衰時間が等しくなることに注意してください
	BYTE					LowEQCutoff ;					// LowEQGain パラメ???により制御されるロ?パス フィル??の折?周波数の設定値( 指定可?範囲 0 ? 9 )
															// 値と周波数 (Hz) の関係
															// 値          0   1   2   3   4   5   6   7   8   9
															// 周波数(Hz) 50 100 150 200 250 300 350 400 450 500
	BYTE					HighEQGain ;					// 1 kHz における減衰時間を基?にして高周波数の減衰時間調整値( 指定可?範囲 0 ? 8 )
															// 値とゲイン (dB) の関係
															// 値          0  1  2  3  4  5  6  7 8
															// ゲイン(dB) -8 -7 -6 -5 -4 -3 -2 -1 0
															// 0 に設定すると、高周波数の音が 1 kHz の場合と同じ割合で減衰します。最大値に設定すると、高周波数の音が 1 kHz の場合よりもはるかに高い割合で減衰します。
	BYTE					HighEQCutoff ;					// HighEQGain パラメ???により制御されるハイパス フィル??の折?周波数設定値( 指定可?範囲 0 ? 14 )
															// 値と周波数 (kHz) の関係
															// 値          0    1    2     3    4     5    6     7    8     9   10    11   12    13   14
															// 周波数(kHz) 1  1.5    2   2.5    3   3.5    4   4.5    5   5.5    6   6.5    7   7.5    8

	float					RoomFilterFreq ;				// 室内エフェクトのロ?パス フィル??の折?周波数、単位は Hz ( 指定可?範囲 20.0f ? 20000.0f )
	float					RoomFilterMain ;				// 初期反射と後期フィ?ルド残響の両方に適用されるロ?パス フィル??のパス バンド強度レベル、単位は dB ( 指定可?範囲 -100.0f ? 0.0f )
	float					RoomFilterHF ;					// 折?周波数 (RoomFilterFreq) での初期反射と後期フィ?ルド残響の両方に適用されるロ?パス フィル??のパス バンド強度レベル、単位は dB ( 指定可?範囲 -100.0f ? 0.0f )
	float					ReflectionsGain ;				// 初期反射の強度/レベルを調整値、単位は dB ( 指定可?範囲 -100.0f ? 20.0f )
	float					ReverbGain ;					// リバ?ブの強度/レベルを調整値、単位は dB ( 指定可?範囲 -100.0f ? 20.0f )
	float					DecayTime ;						// 1 kHz における残響減衰時間、単位は秒 ( 指定可?範囲 0.1f ? 上限値特になし )、これは、フル スケ?ルの入力信号が 60 dB 減衰するまでの時間です。
	float					Density ;						// 後期フィ?ルド残響のモ?ド密度を制御値、単位はパ?セント( 指定可?範囲 0.0f ? 100.0f )
															// 無色 (colorless) の空間では、Density を最大値 (100.0f ) に設定する必要があります。
															// Density を小さくすると、サウンドはくぐもった音 (くし?フィル??が適用された音) になります。
															// これはサイロをシ?ュレ?ションするときに有効なエフェクトです。
	float					RoomSize ;						// 音響空間の見かけ上のサイズ、単位はフィ?ト( 指定可?範囲 1.0f (30.48 cm) ? 100.0f (30.48 m) )
} SOUND3D_REVERB_PARAM ;





// ストリ??デ??制御用関数?イン??造体?イプ２
typedef struct tagSTREAMDATASHREDTYPE2
{
	DWORD_PTR				(*Open		)( const TCHAR *Path, int UseCacheFlag, int BlockReadFlag, int UseASyncReadFlag ) ;
	int						(*Close		)( DWORD_PTR Handle ) ;
	LONGLONG				(*Tell		)( DWORD_PTR Handle ) ;
	int						(*Seek		)( DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType ) ;
	size_t					(*Read		)( void *Buffer, size_t BlockSize, size_t DataNum, DWORD_PTR Handle ) ;
	int						(*Eof		)( DWORD_PTR Handle ) ;
	int						(*IdleCheck	)( DWORD_PTR Handle ) ;
	int						(*ChDir		)( const TCHAR *Path ) ;
	int						(*GetDir	)( TCHAR *Buffer ) ;
	int						(*GetDirS	)( TCHAR *Buffer, size_t BufferSize ) ;
	DWORD_PTR				(*FindFirst	)( const TCHAR *FilePath, FILEINFO *Buffer ) ;		// 戻り値: -1=エラ?  -1以外=FindHandle
	int						(*FindNext	)( DWORD_PTR FindHandle, FILEINFO *Buffer ) ;		// 戻り値: -1=エラ?  0=成功
	int						(*FindClose	)( DWORD_PTR FindHandle ) ;							// 戻り値: -1=エラ?  0=成功
} STREAMDATASHREDTYPE2 ;

// ストリ??デ??制御用関数?イン??造体?イプ２の wchar_t 使用版
typedef struct tagSTREAMDATASHREDTYPE2W
{
	DWORD_PTR				(*Open		)( const wchar_t *Path, int UseCacheFlag, int BlockReadFlag, int UseASyncReadFlag ) ;
	int						(*Close		)( DWORD_PTR Handle ) ;
	LONGLONG				(*Tell		)( DWORD_PTR Handle ) ;
	int						(*Seek		)( DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType ) ;
	size_t					(*Read		)( void *Buffer, size_t BlockSize, size_t DataNum, DWORD_PTR Handle ) ;
	int						(*Eof		)( DWORD_PTR Handle ) ;
	int						(*IdleCheck	)( DWORD_PTR Handle ) ;
	int						(*IsDXA		)( DWORD_PTR Handle ) ;									// 戻り値: -1=エラ?  0=ＤＸア?カイブフ?イル内のフ?イルではない  1=ＤＸア?カイブフ?イル内のフ?イル
	int						(*ChDir		)( const wchar_t *Path ) ;
	int						(*GetDir	)( wchar_t *Buffer ) ;
	int						(*GetDirS	)( wchar_t *Buffer, size_t BufferSize ) ;
	DWORD_PTR				(*FindFirst	)( const wchar_t *FilePath, FILEINFOW *Buffer ) ;		// 戻り値: -1=エラ?  -1以外=FindHandle
	int						(*FindNext	)( DWORD_PTR FindHandle, FILEINFOW *Buffer ) ;			// 戻り値: -1=エラ?  0=成功
	int						(*FindClose	)( DWORD_PTR FindHandle ) ;								// 戻り値: -1=エラ?  0=成功
} STREAMDATASHREDTYPE2W ;

// ストリ??デ??制御用関数?イン??造体
typedef struct tagSTREAMDATASHRED
{
	LONGLONG				(*Tell		)( DWORD_PTR StreamDataPoint ) ;
	int						(*Seek		)( DWORD_PTR StreamDataPoint, LONGLONG SeekPoint, int SeekType ) ;
	size_t					(*Read		)( void *Buffer, size_t BlockSize, size_t DataNum, DWORD_PTR StreamDataPoint ) ;
//	size_t					(*Write		)( void *Buffer, size_t BlockSize, size_t DataNum, DWORD_PTR StreamDataPoint ) ;
	int						(*Eof		)( DWORD_PTR StreamDataPoint ) ;
	int						(*IdleCheck	)( DWORD_PTR StreamDataPoint ) ;
	int						(*Close		)( DWORD_PTR StreamDataPoint ) ;
} STREAMDATASHRED, *LPSTREAMDATASHRED ;

// ストリ??デ??制御用デ???造体
typedef struct tagSTREAMDATA
{
	STREAMDATASHRED			ReadShred ;
	DWORD_PTR				DataPoint ;
} STREAMDATA ;





// パレット情報?造体
typedef struct tagCOLORPALETTEDATA
{
	unsigned char			Blue ;
	unsigned char			Green ;
	unsigned char			Red ;
	unsigned char			Alpha ;
} COLORPALETTEDATA ;

// カラ??造情報?造体
typedef struct tagCOLORDATA
{
	unsigned char			Format ;										// フォ??ット( DX_BASEIMAGE_FORMAT_NORMAL 等 )

	unsigned char			ChannelNum ;									// ?ャンネル数
	unsigned char			ChannelBitDepth ;								// １?ャンネル辺りのビット?度
	unsigned char			FloatTypeFlag ;									// 浮動小数??かどうか( TRUE:浮動小数??  FALSE:整数? )
	unsigned char			PixelByte ;										// １ピクセルあたりのバイト数

	// 以下は ChannelNum 又は ChannelBitDepth が 0 の時のみ有効
	unsigned char			ColorBitDepth ;									// ビット?度
	unsigned char			NoneLoc, NoneWidth ;							// 使われていないビットのアドレスと幅
	unsigned char			RedWidth, GreenWidth, BlueWidth, AlphaWidth ;	// 各色のビット幅
	unsigned char			RedLoc	, GreenLoc  , BlueLoc  , AlphaLoc   ;	// 各色の配置されているビットアドレス
	unsigned int			RedMask , GreenMask , BlueMask , AlphaMask  ;	// 各色のビット?スク
	unsigned int			NoneMask ;										// 使われていないビットの?スク
	int						MaxPaletteNo ;									// 使用しているパレット番号の最大値( 0 の場合は 255 とみなす )

	// memo : ここより上にメンバ?変数を追加したら DxBaseImage.cpp の NS_GraphColorMatchBltVer2 のアセンブラでのパレットデ??参照のリテラル値を修正する必要あり
	COLORPALETTEDATA		Palette[ 256 ] ;								// パレット( ColorBitDepth が８以下の場合のみ有効 )
} COLORDATA, *LPCOLORDATA ;

// 基?イメ?ジデ???造体
typedef struct tagBASEIMAGE
{
	COLORDATA				ColorData ;						// 色情報
	int						Width, Height, Pitch ;			// 幅、高さ、ピッ?
	void					*GraphData ;					// グラフィックイメ?ジ
	int						MipMapCount ;					// ?ップ?ップの数
	int						GraphDataCount ;				// グラフィックイメ?ジの数
} BASEIMAGE, GRAPHIMAGE, *LPGRAPHIMAGE ;

// 矩?デ???
typedef struct tagRECTDATA
{
	int						x1, y1, x2, y2 ;				// 座標
	unsigned int			color ;							// 色
	int						pal ;							// パラメ??
} RECTDATA, *LPRECTDATA ;

// ラインデ???
typedef struct tagLINEDATA
{
	int						x1, y1, x2, y2 ;				// 座標
	unsigned int			color ;							// 色
	int						pal ;							// パラメ??
} LINEDATA, *LPLINEDATA ;

// 座標デ???
typedef struct tagPOINTDATA
{
	int						x, y ;							// 座標
	unsigned int			color ;							// 色
	int						pal ;							// パラメ??
} POINTDATA, *LPPOINTDATA ;

// 立方体デ???
typedef struct tagCUBEDATA
{
	VECTOR					Pos1 ;							// 座標1
	VECTOR					Pos2 ;							// 座標2
	COLOR_U8				DifColor ;						// ディフュ?ズカラ?
	COLOR_U8				SpcColor ;						// スペキュラカラ?
} CUBEDATA, *LPCUBEDATA ;

#ifndef DX_NOTUSE_DRAWFUNCTION

// イメ?ジフォ??ットデ??
typedef struct tagIMAGEFORMATDESC
{
	unsigned char			TextureFlag ;					// テクス?ャか、フラグ( TRUE:テクス?ャ  FALSE:標?サ?フェス )
	unsigned char			CubeMapTextureFlag ;			// キュ?ブ?ップテクス?ャか、フラグ( TRUE:キュ?ブ?ップテクス?ャ?FALSE:それ以外 )
	unsigned char			AlphaChFlag ;					// α?ャンネルはあるか、フラグ	( TRUE:ある  FALSE:ない )
	unsigned char			DrawValidFlag ;					// ?画可?か、フラグ( TRUE:可?  FALSE:不可? )
	unsigned char			SystemMemFlag ;					// システ?メモリ上に存在しているか、フラグ( TRUE:システ?メモリ上  FALSE:ＶＲ?Ｍ上 )( 標?サ?フェスの時のみ有効 )
	unsigned char			UseManagedTextureFlag ;			// ?ネ?ジドテクス?ャを使用するか、フラグ
	unsigned char			UseLinearMapTextureFlag ;		// テクス?ャのメモリデ??配置にリニアが選択できる場合はデ??配置方式をリニアにするかどうか( TRUE:リニアが可?な場合はリニアにする  FALSE:リニアが可?な場合も特にリニアを指定しない )
	unsigned char			PlatformTextureFormat ;			// 環境依存のテクス?ャフォ??ットを直接指定するために使用するための変数( DX_TEXTUREFORMAT_DIRECT3D9_R8G8B8 など )

	unsigned char			BaseFormat ;					// 基?フォ??ット( DX_BASEIMAGE_FORMAT_NORMAL 等 )
	unsigned char			MipMapCount ;					// ?ップ?ップの数
	unsigned char			AlphaTestFlag ;					// αテスト?ャンネルはあるか、フラグ( TRUE:ある  FALSE:ない )( テクス?ャの場合のみ有効 )
	unsigned char			FloatTypeFlag ;					// 浮動小数??かどうか
	unsigned char			ColorBitDepth ;					// 色?度( テクス?ャの場合のみ有効 )
	unsigned char			ChannelNum ;					// ?ャンネルの数
	unsigned char			ChannelBitDepth ;				// １?ャンネル辺りのビット?度( テクス?ャの場合のみ有効、0 の場合は ColorBitDepth が使用される )
	unsigned char			BlendGraphFlag ;				// ブレンド用画像か、フラグ
	unsigned char			UsePaletteFlag ;				// パレットを使用しているか、フラグ( SystemMemFlag が TRUE の場合のみ有効 )

	unsigned char			MSSamples ;						// ?ル?サンプリング数( ?画対象の場合使用 )
	unsigned char			MSQuality ;						// ?ル?サンプリングクオリティ( ?画対象の場合使用 )
} IMAGEFORMATDESC ;

#endif // DX_NOTUSE_DRAWFUNCTION

// DirectInput のジョイパッド入力情報
typedef struct tagDINPUT_JOYSTATE
{
	int						X ;								// スティックのＸ軸パラメ??( -1000?1000 )
	int						Y ;								// スティックのＹ軸パラメ??( -1000?1000 )
	int						Z ;								// スティックのＺ軸パラメ??( -1000?1000 )
	int						Rx ;							// スティックのＸ軸回?パラメ??( -1000?1000 )
	int						Ry ;							// スティックのＹ軸回?パラメ??( -1000?1000 )
	int						Rz ;							// スティックのＺ軸回?パラメ??( -1000?1000 )
	int						Slider[ 2 ] ;					// スライ??二つ( 0?65535 )
	unsigned int			POV[ 4 ] ;						// ハットスイッ?４つ( 0xffffffff:入力なし 0:上 4500:右上 9000:右 13500:右下 18000:下 22500:左下 27000:左 31500:左上 )
	unsigned char			Buttons[ 32 ] ;					// ??ン３２個( 押された??ンは 128 になる )
} DINPUT_JOYSTATE ;

// XInput のジョイパッド入力情報
typedef struct tagXINPUT_STATE
{
	unsigned char			Buttons[ 16 ] ;					// ??ン１６個( 添字には XINPUT_BUTTON_DPAD_UP 等を使用する、0:押されていない  1:押されている )
	unsigned char			LeftTrigger ;					// 左トリガ?( 0?255 )
	unsigned char			RightTrigger ;					// 右トリガ?( 0?255 )
	short					ThumbLX ;						// 左スティックの横軸値( -32768 ? 32767 )
	short					ThumbLY ;						// 左スティックの縦軸値( -32768 ? 32767 )
	short					ThumbRX ;						// 右スティックの横軸値( -32768 ? 32767 )
	short					ThumbRY ;						// 右スティックの縦軸値( -32768 ? 32767 )
} XINPUT_STATE ;

// ?ッ?パネルの１箇所分の?ッ?の情報
typedef struct tagTOUCHINPUTPOINT
{
	DWORD					Device ;						// ?ッ?されたデバイス
	DWORD					ID ;							// ?ッ?を判別するためのＩＤ
	int						PositionX ;						// ?ッ?された座標X
	int						PositionY ;						// ?ッ?された座標Y
	float					Pressure ;						// 圧力
	float					Orientation ;					// 画面の上?に対する接?の方位角
	float					Tilt ;							// 画面の垂線に対する接?の高度角
	int						ToolType ;						// ツ?ル?イプ( DX_TOUCHINPUT_TOOL_TYPE_UNKNOWN など )
} TOUCHINPUTPOINT ;

// ?ッ?パネルの?ッ?の情報
typedef struct tagTOUCHINPUTDATA
{
	LONGLONG				Time ;							// 情報の時間

	unsigned int			Source ;
	int						PointNum ;						// 有効な?ッ?情報の数
	TOUCHINPUTPOINT			Point[ TOUCHINPUTPOINT_MAX ] ;	// ?ッ?情報
} TOUCHINPUTDATA ;






// WinSockets使用時のアドレス指定用?造体
typedef struct tagIPDATA
{
	unsigned char			d1, d2, d3, d4 ;				// アドレス値
} IPDATA, *LPIPDATA ;

typedef struct tagIPDATA_IPv6
{
	union
	{
		unsigned char			Byte[ 16 ] ;
		unsigned short			Word[ 8 ] ;
	} ;
	unsigned long				ScopeID ;
} IPDATA_IPv6 ;

#define DX_STRUCT_END


#ifndef DX_NON_NAMESPACE

}

#endif // DX_NON_NAMESPACE

// 関数プロト?イプ宣言------------------------------------------------------------------

#ifdef WINDOWS_DESKTOP_OS
#include "DxFunctionWin.h"
#endif // WINDOWS_DESKTOP_OS

#ifdef __ANDROID__
#include "DxFunctionAndroid.h"
#endif // __ANDROID__

#ifdef __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
		#include "DxFunctioniOS.h"
	#endif
#endif // __APPLE__






#ifndef DX_NON_NAMESPACE

namespace DxLib
{

#endif // DX_NON_NAMESPACE

#define DX_FUNCTION_START

// DxSystem.cpp関数プロト?イプ宣言

// 初期化終了系関数
extern	int			DxLib_Init( void ) ;													// ライブラリ初期化を行う
extern	int			DxLib_End( void ) ;														// ライブラリ使用の後始末を行う

extern	int			DxLib_GlobalStructInitialize( void ) ;									// ライブラリの内部で使用している?造体を?ロ初期化して、DxLib_Init の前に行った設定を無効化する( DxLib_Init の前でのみ有効 )
extern	int			DxLib_IsInit( void ) ;													// ライブラリが初期化されているかどうかを取得する( 戻り値: TRUE=初期化されている  FALSE=されていない )

// エラ?関係関数
extern	int			GetLastErrorCode( void ) ;												// 最後に発生したエラ?のエラ?コ?ドを取得する( 戻り値?0:エラ?が発生していない、又はエラ?コ?ド出力に対応したエラ?が発生していない??0以外：エラ?コ?ド、DX_ERRORCODE_WIN_DESKTOP_24BIT_COLOR など )
extern	int			GetLastErrorMessage( TCHAR *StringBuffer, int StringBufferBytes ) ;		// 最後に発生したエラ?のエラ?メッセ?ジを指定の文字列バッフ?に取得する

// メッセ?ジ処理関数
extern	int			ProcessMessage( void ) ;												// ウインドウズのメッセ?ジル?プに代わる処理を行う

// 設定系関数
extern	int			SetAlwaysRunFlag( int Flag ) ;											// アプリが非アクティブ状態でも処理を実行するかどうかを設定する( TRUE:実行する  FALSE:停?する( デフォルト ) )
extern	int			SetNotDrawFlag( int Flag ) ;											// ＤＸライブラリの?画??を使うかどうかを設定する( TRUE:使用しない  FALSE:使用する( デフォルト ) )
extern	int			SetNotSoundFlag( int Flag ) ;											// ＤＸライブラリのサウンド??を使うかどうかを設定する( TRUE:使用しない  FALSE:使用する( デフォルト ) )
extern	int			SetNotInputFlag( int Flag ) ;											// ＤＸライブラリの入力状態の取得??を使うかどうかを設定する( TRUE:使用しない  FALSE:使用する( デフォルト ) )

// ウエイト系関数
extern	int			WaitTimer( int WaitTime ) ;												// 指定の時間だけ処理をとめる
#ifndef DX_NON_INPUT
extern	int			WaitKey( void ) ;														// キ?の入力待ちを行う
#endif // DX_NON_INPUT
extern	int			SleepThread( int WaitTime ) ;											// 指定の時間だけスレッドを眠らせる

// カウン?及び時刻取得系関数
extern	int			GetNowCount(							int UseRDTSCFlag DEFAULTPARAM( = FALSE ) ) ;	// ?リ秒単位の精度を持つカウン?の現在の値を得る
extern	LONGLONG	GetNowHiPerformanceCount(				int UseRDTSCFlag DEFAULTPARAM( = FALSE ) ) ;	// GetNowCountの高精度バ?ジョン( μ秒単位の精度を持つカウン?の現在の値を得る )
extern	ULONGLONG	GetNowSysPerformanceCount(				void ) ;						// OSが提供する高精度カウン?の現在の値を得る
extern	ULONGLONG	GetSysPerformanceFrequency(				void ) ;						// OSが提供する高精度カウン?の周波数( 1秒辺りのカウント数 )を得る
extern	ULONGLONG	ConvSysPerformanceCountToSeconds(		ULONGLONG Count ) ;				// OSが提供する高精度カウン?の値を秒の値に変換する
extern	ULONGLONG	ConvSysPerformanceCountToMilliSeconds(	ULONGLONG Count ) ;				// OSが提供する高精度カウン?の値を?リ秒の値に変換する
extern	ULONGLONG	ConvSysPerformanceCountToMicroSeconds(	ULONGLONG Count ) ;				// OSが提供する高精度カウン?の値を?イクロ秒の値に変換する
extern	ULONGLONG	ConvSysPerformanceCountToNanoSeconds(	ULONGLONG Count ) ;				// OSが提供する高精度カウン?の値をナノ秒の値に変換する
extern	ULONGLONG	ConvSecondsToSysPerformanceCount(		ULONGLONG Seconds ) ;			// 秒の値をOSが提供する高精度カウン?の値に変換する
extern	ULONGLONG	ConvMilliSecondsToSysPerformanceCount(	ULONGLONG MilliSeconds ) ;		// ?リ秒の値をOSが提供する高精度カウン?の値に変換する
extern	ULONGLONG	ConvMicroSecondsToSysPerformanceCount(	ULONGLONG MicroSeconds ) ;		// ?イクロ秒の値をOSが提供する高精度カウン?の値に変換する
extern	ULONGLONG	ConvNanoSecondsToSysPerformanceCount(	ULONGLONG NanoSeconds ) ;		// ナノ秒の値をOSが提供する高精度カウン?の値に変換する
extern	int			GetDateTime(							DATEDATA *DateBuf ) ;			// 現在時刻を取得する 

// 乱数取得
extern	int			GetRand( int RandMax ) ;												// 乱数を取得する( RandMax : 返って来る値の最大値 )
extern	int			SRand(	 int Seed ) ;													// 乱数の初期値を設定する

#ifndef DX_NON_MERSENNE_TWISTER
extern	DWORD		GetMersenneTwisterRand( void ) ;										// メルセンヌ・ツイス??アルゴリズ?で生成された乱数値を無加工で取得する

extern	DWORD_PTR	CreateRandHandle( int Seed DEFAULTPARAM( = -1 ) ) ;						// 乱数ハンドルを作成する( 戻り値?0以外:乱数ハンドル?0:エラ? )
extern	int			DeleteRandHandle( DWORD_PTR RandHandle ) ;								// 乱数ハンドルを削除する
extern	int			SRandHandle( DWORD_PTR RandHandle, int Seed ) ;							// 乱数ハンドルの初期値を再設定する
extern	int			GetRandHandle( DWORD_PTR RandHandle, int RandMax ) ;					// 乱数ハンドルを使用して乱数を取得する( RandMax : 返って来る値の最大値 )
extern	DWORD		GetMersenneTwisterRandHandle( DWORD_PTR RandHandle ) ;					// 乱数ハンドルを使用してメルセンヌ・ツイス??アルゴリズ?で生成された乱数値を無加工で取得する

#endif // DX_NON_MERSENNE_TWISTER

// バッテリ?関連
extern	int			GetBatteryLifePercent( void ) ;											// 電池の残量を % で取得する( 戻り値： 100=フル?電状態  0=?電残量無し )

// クリップ??ド関係
extern	int			GetClipboardText(			TCHAR *DestBuffer, int DestBufferBytes DEFAULTPARAM( = -1 ) ) ;		// クリップ??ドに格?されているテキストデ??を読み出す( DestBuffer:文字列を格?するバッフ?の先頭アドレス   戻り値  -1:クリップ??ドにテキストデ??が無い  -1以外:クリップ??ドに格?されている文字列デ??のサイズ( 単位:byte ) ) 
extern	int			SetClipboardText(			const TCHAR *Text                    ) ;	// クリップ??ドにテキストデ??を格?する
extern	int			SetClipboardTextWithStrLen(	const TCHAR *Text, size_t TextLength ) ;	// クリップ??ドにテキストデ??を格?する

// iniフ?イル関係
extern	int			GetPrivateProfileStringDx(                 const TCHAR *AppName,                       const TCHAR *KeyName,                       const TCHAR *Default,                       TCHAR *ReturnedStringBuffer, size_t ReturnedStringBufferBytes, const TCHAR *IniFilePath,                           int IniFileCharCodeFormat DEFAULTPARAM( = -1 ) /* DX_CHARCODEFORMAT_SHIFTJIS 等、-1 でデフォルト */ ) ;		// GetPrivateProfileString のＤＸライブラリ版
extern	int			GetPrivateProfileStringDxWithStrLen(       const TCHAR *AppName, size_t AppNameLength, const TCHAR *KeyName, size_t KeyNameLength, const TCHAR *Default, size_t DefaultLength, TCHAR *ReturnedStringBuffer, size_t ReturnedStringBufferBytes, const TCHAR *IniFilePath, size_t IniFilePathLength, int IniFileCharCodeFormat DEFAULTPARAM( = -1 ) /* DX_CHARCODEFORMAT_SHIFTJIS 等、-1 でデフォルト */ ) ;		// GetPrivateProfileString のＤＸライブラリ版
extern	int			GetPrivateProfileIntDx(                    const TCHAR *AppName,                       const TCHAR *KeyName,                       int          Default,                                                                                      const TCHAR *IniFilePath,                           int IniFileCharCodeFormat DEFAULTPARAM( = -1 ) /* DX_CHARCODEFORMAT_SHIFTJIS 等、-1 でデフォルト */ ) ;		// GetPrivateProfileInt のＤＸライブラリ版
extern	int			GetPrivateProfileIntDxWithStrLen(          const TCHAR *AppName, size_t AppNameLength, const TCHAR *KeyName, size_t KeyNameLength, int          Default,                                                                                      const TCHAR *IniFilePath, size_t IniFilePathLength, int IniFileCharCodeFormat DEFAULTPARAM( = -1 ) /* DX_CHARCODEFORMAT_SHIFTJIS 等、-1 でデフォルト */ ) ;		// GetPrivateProfileInt のＤＸライブラリ版
extern	int			GetPrivateProfileStringDxForMem(           const TCHAR *AppName,                       const TCHAR *KeyName,                       const TCHAR *Default,                       TCHAR *ReturnedStringBuffer, size_t ReturnedStringBufferBytes, const void *IniFileImage, size_t IniFileImageBytes, int IniFileCharCodeFormat DEFAULTPARAM( = -1 ) /* DX_CHARCODEFORMAT_SHIFTJIS 等、-1 でデフォルト */ ) ;		// GetPrivateProfileStringDx のメモリから読み込む版
extern	int			GetPrivateProfileStringDxForMemWithStrLen( const TCHAR *AppName, size_t AppNameLength, const TCHAR *KeyName, size_t KeyNameLength, const TCHAR *Default, size_t DefaultLength, TCHAR *ReturnedStringBuffer, size_t ReturnedStringBufferBytes, const void *IniFileImage, size_t IniFileImageBytes, int IniFileCharCodeFormat DEFAULTPARAM( = -1 ) /* DX_CHARCODEFORMAT_SHIFTJIS 等、-1 でデフォルト */ ) ;		// GetPrivateProfileStringDx のメモリから読み込む版
extern	int			GetPrivateProfileIntDxForMem(              const TCHAR *AppName,                       const TCHAR *KeyName,                       int          Default,                                                                                      const void *IniFileImage, size_t IniFileImageBytes, int IniFileCharCodeFormat DEFAULTPARAM( = -1 ) /* DX_CHARCODEFORMAT_SHIFTJIS 等、-1 でデフォルト */ ) ;		// GetPrivateProfileIntDx のメモリから読み込む版
extern	int			GetPrivateProfileIntDxForMemWithStrLen(    const TCHAR *AppName, size_t AppNameLength, const TCHAR *KeyName, size_t KeyNameLength, int          Default,                                                                                      const void *IniFileImage, size_t IniFileImageBytes, int IniFileCharCodeFormat DEFAULTPARAM( = -1 ) /* DX_CHARCODEFORMAT_SHIFTJIS 等、-1 でデフォルト */ ) ;		// GetPrivateProfileIntDx のメモリから読み込む版

#if defined( __APPLE__ ) || defined( __ANDROID__ )

// メ?ルアプリを送信メ?ル編集状態で起動する
// MailAddr    : 宛先( NULL で無効 )、メ?ルアドレスが複数ある場合はカン?『,』で区切ってください
// MailCCAddr  : CC の宛先( NULL で無効 )、メ?ルアドレスが複数ある場合はカン?『,』で区切ってください
// MailBCCAddr : BCC の宛先( NULL で無効 )、メ?ルアドレスが複数ある場合はカン?『,』で区切ってください
// Subject     : ?イトル( NULL で無効 )、メ?ルアドレスが複数ある場合はカン?『,』で区切ってください
// Text        : ?文( NULL で無効 )、メ?ルアドレスが複数ある場合はカン?『,』で区切ってください
extern	int			MailApp_Send(           const TCHAR *MailAddr DEFAULTPARAM( = NULL ) ,                                             const TCHAR *MailCCAddr DEFAULTPARAM( = NULL ) ,                                               const TCHAR *MailBCCAddr DEFAULTPARAM( = NULL ) ,                                                const TCHAR *Subject DEFAULTPARAM( = NULL ) ,                                            const TCHAR *Text DEFAULTPARAM( = NULL )                                         ) ;
extern	int			MailApp_SendWithStrLen( const TCHAR *MailAddr DEFAULTPARAM( = NULL ) , size_t MailAddrLength DEFAULTPARAM( = 0 ) , const TCHAR *MailCCAddr DEFAULTPARAM( = NULL ) , size_t MailCCAddrLength DEFAULTPARAM( = 0 ) , const TCHAR *MailBCCAddr DEFAULTPARAM( = NULL ) , size_t MailBCCAddrLength DEFAULTPARAM( = 0 ) , const TCHAR *Subject DEFAULTPARAM( = NULL ) , size_t SubjectLength DEFAULTPARAM( = 0 ) , const TCHAR *Text DEFAULTPARAM( = NULL ) , size_t TextLength DEFAULTPARAM( = 0 ) ) ;

#endif // defined( __APPLE__ ) || defined( __ANDROID__ )











// DxLog.cpp関数プロト?イプ宣言

#ifndef DX_NON_LOG
// ログフ?イル関数
extern	int			LogFileAdd(				const TCHAR *String ) ;							// ログフ?イル( Log.txt ) に文字列を出力する
extern	int			LogFileAddWithStrLen(	const TCHAR *String, size_t StringLength ) ;	// ログフ?イル( Log.txt ) に文字列を出力する
extern 	int			LogFileFmtAdd(			const TCHAR *FormatString , ... ) ;				// 書式付きで ログフ?イル( Log.txt ) に文字列を出力する( 書式は printf と同じ )
extern	int			LogFileTabAdd(			void ) ;										// ログフ?イル( Log.txt ) に出力する文字列の前に付ける?ブの数を一つ増やす
extern	int			LogFileTabSub(			void ) ;										// ログフ?イル( Log.txt ) に出力する文字列の前に付ける?ブの数を一つ減らす
extern	int			ErrorLogAdd(			const TCHAR *String ) ;							// LogFileAdd の旧名称関数
extern 	int			ErrorLogFmtAdd(			const TCHAR *FormatString , ... ) ;				// LogFileFmtAdd の旧名称関数
extern	int			ErrorLogTabAdd(			void ) ;										// LogFileTabAdd の旧名称関数
extern	int			ErrorLogTabSub(			void ) ;										// LogFileTabSub の旧名称関数
extern	int			SetUseTimeStampFlag(	int UseFlag ) ;									// ログフ?イル( Log.txt ) に出力する文字列の前に起動してからの時間を付けるかどうかを設定する( TRUE:付ける( デフォルト)  FALSE:付けない )
extern 	int			AppLogAdd(				const TCHAR *String , ... ) ;					// LogFileFmtAdd と同じ??の関数

// ログ出力設定関数
extern	int			SetOutApplicationLogValidFlag(	          int Flag ) ;													// ログフ?イル( Log.txt ) を作成するかどうかを設定する( TRUE:作成する( デフォルト )  FALSE:作成しない )、DxLib_Init の前でのみ使用可??
extern	int			SetOutApplicationSystemLogValidFlag(      int Flag ) ;													// ログフ?イル( Log.txt ) にＤＸライブラリ内部のログ出力を行うかどうか設定する( TRUE:ＤＸライブラリ内部のログ出力を行う( デフォルト )  FALSE:ＤＸライブラリ内部のログ出力を行わない )
extern	int			SetApplicationLogFileName(                const TCHAR *FileName ) ;										// ログフ?イルの名前を設定する( Log.txt 以外にしたい場合に使用 )
extern	int			SetApplicationLogFileNameWithStrLen(      const TCHAR *FileName, size_t FileNameLength ) ;				// ログフ?イルの名前を設定する( Log.txt 以外にしたい場合に使用 )
extern	int			SetApplicationLogSaveDirectory(	          const TCHAR *DirectoryPath ) ;								// ログフ?イル( Log.txt ) を保存するディレクトリパスを設定する
extern	int			SetApplicationLogSaveDirectoryWithStrLen( const TCHAR *DirectoryPath, size_t DirectoryPathLength ) ;	// ログフ?イル( Log.txt ) を保存するディレクトリパスを設定する
extern	int			SetUseDateNameLogFile(			          int Flag ) ;													// ログフ?イル名に日付をつけるかどうかをセットする( TRUE:付ける  FALSE:付けない( デフォルト ) )

#ifndef DX_NON_PRINTF_DX

// ログ出力??関数
extern	int			SetLogDrawOutFlag(	 int DrawFlag ) ;									// printfDx の結果を画面に出力するかどうかを設定する、TRUE:出力を行う  FALSE:出力を行わない( printfDx を実行すると内部で SetLogDrawOutFlag( TRUE ) ; が呼ばれます )
extern 	int			GetLogDrawFlag(		 void ) ;											// printfDx の結果を画面に出力するかどうかの設定を取得する( 戻り値  TRUE:出力を行う  FALSE:出力を行わない )
extern	int			SetLogFontSize(		 int Size ) ;										// printfDx の結果を画面に出力する際に使用するフォントのサイズを設定する
extern	int			SetLogFontHandle(	 int FontHandle ) ;									// printfDx の結果を画面に出力する際に使用するフォントのハンドルを変更する
extern	int			SetLogDrawArea(		 int x1, int y1, int x2, int y2 ) ;					// printfDx の結果を画面に出力する際の?画する領域を設定する

// 簡易画面出力関数
extern 	int			printfDx(			 const TCHAR *FormatString , ... ) ;												// printf と同じ引数で画面に文字列を?示するための関数
extern	int			putsDx(				 const TCHAR *String, int NewLine DEFAULTPARAM( = TRUE ) ) ;						// puts と同じ引数で画面に文字列を?示するための関数
extern	int			putsDxWithStrLen(	 const TCHAR *String, size_t StringLength, int NewLine DEFAULTPARAM( = TRUE ) ) ;	// puts と同じ引数で画面に文字列を?示するための関数
extern	int			clsDx(				 void ) ;																			// printfDx の結果をリセットするための関数
extern	int			setPrintColorDx(     int Color, int EdgeColor DEFAULTPARAM( = 0 ) ) ;													// printf や puts で?示する文字列の色を指定する
#endif // DX_NON_PRINTF_DX

#endif // DX_NON_LOG












#ifndef DX_NON_ASYNCLOAD

// DxASyncLoad.cpp関数プロト?イプ宣言

// 非同期読み込み関係
extern	int			SetUseASyncLoadFlag(			int Flag ) ;										// 読み込み処理系の関数で非同期読み込みを行うかどうかを設定する( 非同期読み込みに対応している関数のみ有効 )( TRUE:非同期読み込みを行う  FALSE:非同期読み込みを行わない( デフォルト ) )
extern	int			GetUseASyncLoadFlag(			void ) ;											// 読み込み処理系の関数で非同期読み込みを行うかどうかを取得する( 非同期読み込みに対応している関数のみ有効 )( TRUE:非同期読み込みを行う  FALSE:非同期読み込みを行わない( デフォルト ) )
extern	int			CheckHandleASyncLoad(			int Handle ) ;										// ハンドルの非同期読み込みが完了しているかどうかを取得する( TRUE:まだ完了していない  FALSE:完了している  -1:エラ? )
extern	int			GetHandleASyncLoadResult(		int Handle ) ;										// ハンドルの非同期読み込み処理の戻り値を取得する( 非同期読み込み中の場合は一つ前の非同期読み込み処理の戻り値が返ってきます )
extern	int			SetASyncLoadFinishDeleteFlag(	int Handle ) ;										// ハンドルの非同期読み込み処理が完了したらハンドルを削除するフラグを立てる
extern	int			SetASyncLoadFinishCallback(		int Handle, void ( *Callback )( int Handle, void *Data ), void *Data ) ;	// ハンドルの非同期読み込み処理が完了したら呼ばれる関数をセットする
extern	int			WaitHandleASyncLoad(			int Handle ) ;										// 指定のハンドルの非同期読み込み処理が終了するまで待つ
extern	int			WaitHandleASyncLoadAll(			void ) ;											// 全ての非同期読み込みデ??が読み込み終わるまで待つ
extern	int			GetASyncLoadNum(				void ) ;											// 非同期読み込み中の処理の数を取得する
extern	int			SetASyncLoadThreadNum(			int ThreadNum ) ;									// 非同期読み込み処理を行うスレッドの数を設定する( ThreadNum に指定できる数は 1 ? 32 )

#endif // DX_NON_ASYNCLOAD











// DxHandle.cpp関数プロト?イプ宣言

extern	int			SetDeleteHandleFlag(		int Handle, int *DeleteFlag ) ;							// ハンドルが削除されたときに?１が設定される変数を登?する
extern	int			GetHandleNum(				int HandleType /* DX_HANDLETYPE_GRAPH等 */ ) ;			// 指定の?イプのハンドルの数を取得する
extern	int			GetMaxHandleNum(			int HandleType /* DX_HANDLETYPE_GRAPH等 */ ) ;			// 指定の?イプのハンドルが最大で幾つ作成できるかを取得する
extern	int			DumpHandleInfo(				int HandleType /* DX_HANDLETYPE_GRAPH等 */ ) ;			// 指定の?イプの全ハンドルの情報をログに出力する














// ?ウス関係関数
extern	int			SetMouseDispFlag(		int DispFlag ) ;												// ?ウス?イン?の?示状態を設定する( DispFlag:?ウス?イン?を?示するかどうか( TRUE:?示する  FALSE:?示しない )
extern	int			SetMouseDispIgnoreMenuFlag(	int IgnoreMenuFlag ) ;										// ?ウス?イン?の?示状態についてメニュ?の?示状態を無視するかどうかを設定する( TRUE:メニュ?の?示状態を無視する  FALSE:メニュ?の?示状態を考慮する( デフォルト ) )
#ifndef DX_NON_INPUT
extern	int			GetMousePoint(			int *XBuf, int *YBuf ) ;										// ?ウス?イン?の位置を取得する
extern	int			SetMousePoint(			int PointX, int PointY ) ;										// ?ウス?イン?の位置を設定する
extern	int			GetMouseInput(			void ) ;														// ?ウスの??ンの押下状態を取得する
extern	int			GetMouseWheelRotVol(	int CounterReset DEFAULTPARAM( = TRUE ) ) ;						// 垂直?ウスホイ?ルの回?量を取得する
extern	int			GetMouseHWheelRotVol(	int CounterReset DEFAULTPARAM( = TRUE ) ) ;						// 水平?ウスホイ?ルの回?量を取得する
extern	float		GetMouseWheelRotVolF(	int CounterReset DEFAULTPARAM( = TRUE ) ) ;						// 垂直?ウスホイ?ルの回?量を取得する( 戻り値が float ? )
extern	float		GetMouseHWheelRotVolF(	int CounterReset DEFAULTPARAM( = TRUE ) ) ;						// 水平?ウスホイ?ルの回?量を取得する( 戻り値が float ? )
extern	int			GetMouseInputLog(		int *Button, int *ClickX, int *ClickY, int LogDelete DEFAULTPARAM( = TRUE ) ) ;					// ?ウスの??ンを押した情報を一つ取得する( Button:押された??ン( MOUSE_INPUT_LEFT 等 )を格?する変数のアドレス  ClickX:押された時のＸ座標を格?する変数のアドレス  ClickY:押された時のＹ座標を格?する変数のアドレス   LogDelete:取得した押下情報一つ分をログから削除するかどうか( TRUE:削除する  FALSE:削除しない、つまり次にこの関数が呼ばれたときに同じ値を取得することになる )??戻り値  0:押された情報取得できた?-1:押された情報が無かった、つまり前回の呼び出し( または起動時から最初の呼び出し )の間に一度も?ウスの??ンが押されなかった )
extern	int			GetMouseInputLog2(		int *Button, int *ClickX, int *ClickY, int *LogType, int LogDelete DEFAULTPARAM( = TRUE ) ) ;	// ?ウスの??ンを押したり離したりした情報を一つ取得する( Button:押されたり離されたりした??ン( MOUSE_INPUT_LEFT 等 )を格?する変数のアドレス  ClickX:押されたり離されたりした時のＸ座標を格?する変数のアドレス  ClickY:押されたり離されたりした時のＹ座標を格?する変数のアドレス  LogType：押されたのか( MOUSE_INPUT_LOG_DOWN )離されたのか( MOUSE_INPUT_LOG_UP )、等の情報を格?する変数のアドレス  LogDelete:取得した押されたり離されたりした情報一つ分をログから削除するかどうか( TRUE:削除する  FALSE:削除しない、つまり次にこの関数が呼ばれたときに同じ値を取得することになる )??戻り値  0:押されたり離されたりした情報取得できた?-1:押されたり離されたりした情報が無かった、つまり前回の呼び出し( または起動時から最初の呼び出し )の間に一度も?ウスの??ンが押されたり離されたりしなかった )
#endif // DX_NON_INPUT












// ?ッ?パネル入力関係関数
#ifndef DX_NON_INPUT
extern	int				GetTouchInputNum( void ) ;																				// ?ッ?されている数を取得する
extern	int				GetTouchInput( int InputNo, int *PositionX, int *PositionY, int *ID DEFAULTPARAM( = NULL ) , int *Device DEFAULTPARAM( = NULL ) , float *Pressure DEFAULTPARAM( = NULL ) ) ;		// ?ッ?の情報を取得する

extern	int				GetTouchInputLogNum( void ) ;																			// ストックされている?ッ?情報の数を取得する
extern	int				ClearTouchInputLog( void ) ;																			// ストックされている?ッ?情報をクリアする
extern	TOUCHINPUTDATA	GetTouchInputLogOne( int PeekFlag DEFAULTPARAM( = FALSE ) ) ;											// ストックされている?ッ?情報から一番古い情報をひとつ取得する
extern	int				GetTouchInputLog( TOUCHINPUTDATA *TouchData, int GetNum, int PeekFlag DEFAULTPARAM( = FALSE ) ) ;		// ストックされている?ッ?情報から古い順に指定数バッフ?に取得する( 戻り値  -1:エラ?  0以上:取得した情報の数 )

extern	int				GetTouchInputDownLogNum( void ) ;																		// ストックされている?ッ?され始めた情報の数を取得する
extern	int				ClearTouchInputDownLog( void ) ;																		// ストックされている?ッ?され始めた情報をクリアする
extern	TOUCHINPUTPOINT	GetTouchInputDownLogOne( int PeekFlag DEFAULTPARAM( = FALSE ) ) ;										// ストックされている?ッ?され始めた情報から一番古い情報をひとつ取得する
extern	int				GetTouchInputDownLog( TOUCHINPUTPOINT *PointData, int GetNum, int PeekFlag DEFAULTPARAM( = FALSE ) ) ;	// ストックされている?ッ?され始めた情報から古い順に指定数バッフ?に取得する( 戻り値  -1:エラ?  0以上:取得した情報の数 )

extern	int				GetTouchInputUpLogNum( void ) ;																			// ストックされている?ッ?が離された情報の数を取得する
extern	int				ClearTouchInputUpLog( void ) ;																			// ストックされている?ッ?が離された情報をクリアする
extern	TOUCHINPUTPOINT	GetTouchInputUpLogOne( int PeekFlag DEFAULTPARAM( = FALSE ) ) ;											// ストックされている?ッ?が離された情報から一番古い情報をひとつ取得する
extern	int				GetTouchInputUpLog( TOUCHINPUTPOINT *PointData, int GetNum, int PeekFlag DEFAULTPARAM( = FALSE ) ) ;	// ストックされている?ッ?が離された情報から古い順に指定数バッフ?に取得する( 戻り値  -1:エラ?  0以上:取得した情報の数 )

#endif // DX_NON_INPUT















// DxMemory.cpp関数プロト?イプ宣言

// メモリ確保系関数
extern	void*		DxAlloc(						               size_t AllocSize ,                    const char *File DEFAULTPARAM( = NULL ) , int Line DEFAULTPARAM( = -1 ) ) ;	// 指定のサイズのメモリを確保する( AllocSize:確保するメモリのサイズ( 単位:byte )  File:DxAllocを呼んだ??スフ?イル名( デバッグ用 )  Line:DxAllocを呼んだ??スフ?イル中の行番号( デバッグ用 )??戻り値  NULL:メモリの確保失敗   NULL以外:確保したメモリ領域の先頭アドレス )
extern	void*		DxAllocAligned(					               size_t AllocSize , size_t Alignment , const char *File DEFAULTPARAM( = NULL ) , int Line DEFAULTPARAM( = -1 ) ) ;	// 指定のサイズのメモリを確保する( アライン指定版 )( AllocSize:確保するメモリのサイズ( 単位:byte )  Alignment:確保するメモリ領域のアドレス値の?数  File:DxAllocを呼んだ??スフ?イル名( デバッグ用 )  Line:DxAllocを呼んだ??スフ?イル中の行番号( デバッグ用 )??戻り値  NULL:メモリの確保失敗   NULL以外:確保したメモリ領域の先頭アドレス )
extern	void*		DxCalloc(						               size_t AllocSize ,                    const char *File DEFAULTPARAM( = NULL ) , int Line DEFAULTPARAM( = -1 ) ) ;	// 指定のサイズのメモリを確保して、０で埋める、初期化後に０で埋める以外は DxAlloc と動作は同じ
extern	void*		DxCallocAligned(				               size_t AllocSize , size_t Alignment , const char *File DEFAULTPARAM( = NULL ) , int Line DEFAULTPARAM( = -1 ) ) ;	// 指定のサイズのメモリを確保して、０で埋める、初期化後に０で埋める以外は DxAllocAligned と動作は同じ
extern	void*		DxRealloc(						void *Memory , size_t AllocSize ,                    const char *File DEFAULTPARAM( = NULL ) , int Line DEFAULTPARAM( = -1 ) ) ;	// メモリの再確保を行う( Memory:再確保を行うメモリ領域の先頭アドレス( DxAlloc の戻り値 )  AllocSize:新しい確保サイズ?FileとLine の説明は DxAlloc の注釈の通り ?戻り値 NULL:メモリの再確保失敗?NULL以外:再確保した新しいメモリ領域の先頭アドレス?)
extern	void*		DxReallocAligned(				void *Memory , size_t AllocSize , size_t Alignment , const char *File DEFAULTPARAM( = NULL ) , int Line DEFAULTPARAM( = -1 ) ) ;	// メモリの再確保を行う( アライン指定版 )( Memory:再確保を行うメモリ領域の先頭アドレス( DxAlloc の戻り値 )  AllocSize:新しい確保サイズ   Alignment:新しく確保するメモリ領域のアドレス値の?数?FileとLine の説明は DxAlloc の注釈の通り ?戻り値 NULL:メモリの再確保失敗?NULL以外:再確保した新しいメモリ領域の先頭アドレス?)
extern	void		DxFree(							void *Memory ) ;																				// メモリを解放する( Memory:解放するメモリ領域の先頭アドレス( DxAlloc の戻り値 ) )
extern	size_t		DxSetAllocSizeTrap(				size_t Size ) ;																					// 列挙対象にするメモリの確保容量をセットする
extern	int			DxSetAllocPrintFlag(			int Flag ) ;																					// ＤＸライブラリ内でメモリ確保が行われる時に情報を出力するかどうかをセットする
extern	size_t		DxGetAllocSize(					void ) ;																						// DxAlloc や DxCalloc で確保しているメモリサイズを取得する
extern	int			DxGetAllocNum(					void ) ;																						// DxAlloc や DxCalloc で確保しているメモリの数を取得する
extern	void		DxDumpAlloc(					void ) ;																						// DxAlloc や DxCalloc で確保しているメモリを列挙する
extern	void		DxDrawAlloc(					int x, int y, int Width, int Height ) ;															// DxAlloc や DxCalloc で確保しているメモリの状況を?画する
extern	int			DxErrorCheckAlloc(				void ) ;																						// 確保したメモリ情報が破壊されていないか調べる( -1:破壊あり  0:なし )
extern	int			DxSetAllocSizeOutFlag(			int Flag ) ;																					// メモリが確保、解放が行われる度に確保しているメモリの容量を出力するかどうかのフラグをセットする
extern	int			DxSetAllocMemoryErrorCheckFlag(	int Flag ) ;																					// メモリの確保、解放が行われる度に確保しているメモリ確保情報が破損していないか調べるかどうかのフラグをセットする














// DxBaseFunc.cpp 関数プロト?イプ宣言

// 文字コ?ド関係
extern	int				GetCharBytes( int CharCodeFormat /* DX_CHARCODEFORMAT_SHIFTJIS 等 */ , const void *String ) ;	// 文字列の先頭の文字のバイト数を取得する
extern	int				ConvertStringCharCodeFormat( int SrcCharCodeFormat /* DX_CHARCODEFORMAT_SHIFTJIS 等 */ , const void *SrcString, int DestCharCodeFormat /* DX_CHARCODEFORMAT_SHIFTJIS 等 */ , void *DestStringBuffer ) ;		// 文字列の文字コ?ド?式を別の文字コ?ド?式に変換する
extern	int				SetUseCharCodeFormat( int CharCodeFormat /* DX_CHARCODEFORMAT_SHIFTJIS 等 */ ) ;		// 文字列の引数の文字コ?ド?式を設定する( 文字列?画系関数とその他一部関数を除く )( UNICODE版では無効 )
extern	int				GetUseCharCodeFormat( void ) ;															// 文字列の引数の文字コ?ド?式を取得する( 戻り値：文字コ?ド?式( DX_CHARCODEFORMAT_SHIFTJIS 等 ) )( UNICODE版では無効 )
extern	int				Get_wchar_t_CharCodeFormat( void ) ;													// wchar_t?の文字コ?ド?式を取得する( 戻り値： DX_CHARCODEFORMAT_UTF16LE など )

// 文字列関係
extern	void			strcpyDx(      TCHAR *Dest,                   const TCHAR *Src ) ;						// strcpy と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strcpy_sDx(    TCHAR *Dest, size_t DestBytes, const TCHAR *Src ) ;						// strcpy_s と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strpcpyDx(     TCHAR *Dest,                   const TCHAR *Src, int Pos ) ;				// 位置指定付き strcpy、Pos はコピ?開始位置?( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strpcpy_sDx(   TCHAR *Dest, size_t DestBytes, const TCHAR *Src, int Pos ) ;				// 位置指定付き strcpy_s、Pos はコピ?開始位置?( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strpcpy2Dx(    TCHAR *Dest,                   const TCHAR *Src, int Pos ) ;				// 位置指定付き strcpy、Pos はコピ?開始位置( 全角文字も 1 扱い )?( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strpcpy2_sDx(  TCHAR *Dest, size_t DestBytes, const TCHAR *Src, int Pos ) ;				// 位置指定付き strcpy_s、Pos はコピ?開始位置( 全角文字も 1 扱い )?( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strncpyDx(     TCHAR *Dest,                   const TCHAR *Src, int Num ) ;				// strncpy と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strncpy_sDx(   TCHAR *Dest, size_t DestBytes, const TCHAR *Src, int Num ) ;				// strncpy_s と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strncpy2Dx(    TCHAR *Dest,                   const TCHAR *Src, int Num ) ;				// strncpy の Num が文字数( 全角文字も 1 扱い )になったもの、終?に必ずヌル文字が代入される( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strncpy2_sDx(  TCHAR *Dest, size_t DestBytes, const TCHAR *Src, int Num ) ;				// strncpy_s の Num が文字数( 全角文字も 1 扱い )になったもの、終?に必ずヌル文字が代入される( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strrncpyDx(    TCHAR *Dest,                   const TCHAR *Src, int Num ) ;				// strncpy の文字列の終?からの文字数指定版( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strrncpy_sDx(  TCHAR *Dest, size_t DestBytes, const TCHAR *Src, int Num ) ;				// strncpy_s の文字列の終?からの文字数指定版( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strrncpy2Dx(   TCHAR *Dest,                   const TCHAR *Src, int Num ) ;				// strncpy の文字列の終?からの文字数( 全角文字も 1 扱い )指定版、終?に必ずヌル文字が代入される( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strrncpy2_sDx( TCHAR *Dest, size_t DestBytes, const TCHAR *Src, int Num ) ;				// strncpy_s の文字列の終?からの文字数( 全角文字も 1 扱い )指定版、終?に必ずヌル文字が代入される( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strpncpyDx(    TCHAR *Dest,                   const TCHAR *Src, int Pos, int Num ) ;	// strncpy のコピ?開始位置指定版、Pos はコピ?開始位置、Num は文字数( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strpncpy_sDx(  TCHAR *Dest, size_t DestBytes, const TCHAR *Src, int Pos, int Num ) ;	// strncpy_s のコピ?開始位置指定版、Pos はコピ?開始位置、Num は文字数( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strpncpy2Dx(   TCHAR *Dest,                   const TCHAR *Src, int Pos, int Num ) ;	// strncpy のコピ?開始位置指定版、Pos はコピ?開始位置( 全角文字も 1 扱い )、Num は文字数( 全角文字も 1 扱い )、終?に必ずヌル文字が代入される( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strpncpy2_sDx( TCHAR *Dest, size_t DestBytes, const TCHAR *Src, int Pos, int Num ) ;	// strncpy_s のコピ?開始位置指定版、Pos はコピ?開始位置( 全角文字も 1 扱い )、Num は文字数( 全角文字も 1 扱い )、終?に必ずヌル文字が代入される( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strcatDx(      TCHAR *Dest,                   const TCHAR *Src ) ;						// strcat と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	void			strcat_sDx(    TCHAR *Dest, size_t DestBytes, const TCHAR *Src ) ;						// strcat_s と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	size_t			strlenDx(      const TCHAR *Str ) ;												// strlen と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	size_t			strlen2Dx(     const TCHAR *Str ) ;												// strlen の戻り値が文字数( 全角文字も 1 扱い )になったもの( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				strcmpDx(      const TCHAR *Str1, const TCHAR *Str2 ) ;							// strcmp と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				stricmpDx(     const TCHAR *Str1, const TCHAR *Str2 ) ;							// stricmp と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				strncmpDx(     const TCHAR *Str1, const TCHAR *Str2, int Num ) ;				// strncmp と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				strncmp2Dx(    const TCHAR *Str1, const TCHAR *Str2, int Num ) ;				// strncmp の Num が文字数( 全角文字も 1 扱い )になったもの( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				strpncmpDx(    const TCHAR *Str1, const TCHAR *Str2, int Pos, int Num ) ;		// strncmp の比較開始位置指定版、Pos が Str1 の比較開始位置、Num が文字数( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				strpncmp2Dx(   const TCHAR *Str1, const TCHAR *Str2, int Pos, int Num ) ;		// strncmp の比較開始位置指定版、Pos が Str1 の比較開始位置( 全角文字も 1 扱い )、Num が文字数( 全角文字も 1 扱い )( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	DWORD			strgetchrDx(   const TCHAR *Str, int Pos, int *CharNums DEFAULTPARAM( = NULL ) ) ;	// 文字列の指定の位置の文字コ?ドを取得する、Pos は取得する位置、CharNums は文字数を代入する変数のアドレス、戻り値は文字コ?ド( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	DWORD			strgetchr2Dx(  const TCHAR *Str, int Pos, int *CharNums DEFAULTPARAM( = NULL ) ) ;	// 文字列の指定の位置の文字コ?ドを取得する、Pos は取得する位置( 全角文字も 1 扱い )、CharNums は文字数を代入する変数のアドレス、戻り値は文字コ?ド( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				strputchrDx(   TCHAR *Str, int Pos, DWORD CharCode ) ;							// 文字列の指定の位置に文字コ?ドを書き込む、Pos は書き込む位置、CharCode は文字コ?ド、戻り値は書き込んだ文字数( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				strputchr2Dx(  TCHAR *Str, int Pos, DWORD CharCode ) ;							// 文字列の指定の位置に文字コ?ドを書き込む、Pos は書き込む位置( 全角文字も 1 扱い )、CharCode は文字コ?ド、戻り値は書き込んだ文字数( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	const TCHAR *	strposDx(      const TCHAR *Str, int Pos ) ;									// 文字列の指定の位置のアドレスを取得する、Pos は取得する位置?( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	const TCHAR *	strpos2Dx(     const TCHAR *Str, int Pos ) ;									// 文字列の指定の位置のアドレスを取得する、Pos は取得する位置( 全角文字も 1 扱い )?( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	const TCHAR *	strstrDx(      const TCHAR *Str1, const TCHAR *Str2 ) ;							// strstr と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				strstr2Dx(     const TCHAR *Str1, const TCHAR *Str2 ) ;							// strstr の戻り値が文字列先頭からの文字数( 全角文字も 1 扱い ) になったもの( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	const TCHAR *	strrstrDx(     const TCHAR *Str1, const TCHAR *Str2 ) ;							// strrstr と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				strrstr2Dx(    const TCHAR *Str1, const TCHAR *Str2 ) ;							// strrstr の戻り値が文字列先頭からの文字数( 全角文字も 1 扱い ) になったもの( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	const TCHAR *	strchrDx(      const TCHAR *Str, DWORD CharCode ) ;								// strchr と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				strchr2Dx(     const TCHAR *Str, DWORD CharCode ) ;								// strchr の戻り値が文字列先頭からの文字数( 全角文字も 1 扱い ) になったもの( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	const TCHAR *	strrchrDx(     const TCHAR *Str, DWORD CharCode ) ;								// strrchr と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				strrchr2Dx(    const TCHAR *Str, DWORD CharCode ) ;								// strrchr の戻り値が文字列先頭からの文字数( 全角文字も 1 扱い ) になったもの( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	TCHAR *			struprDx(      TCHAR *Str ) ;													// strupr と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				vsprintfDx(    TCHAR *Buffer,                    const TCHAR *FormatString, va_list Arg ) ;	// vsprintf と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				vsnprintfDx(   TCHAR *Buffer, size_t BufferSize, const TCHAR *FormatString, va_list Arg ) ;	// vsnprintf と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				sprintfDx(     TCHAR *Buffer,                    const TCHAR *FormatString, ... ) ;			// sprintf と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				snprintfDx(    TCHAR *Buffer, size_t BufferSize, const TCHAR *FormatString, ... ) ;			// snprintf と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	TCHAR *			itoaDx(        int Value, TCHAR *Buffer,                     int Radix ) ;		// itoa と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	TCHAR *			itoa_sDx(      int Value, TCHAR *Buffer, size_t BufferBytes, int Radix ) ;		// itoa_s と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				atoiDx(        const TCHAR *Str ) ;												// atoi と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます ) 
extern	double			atofDx(        const TCHAR *Str ) ;												// atof と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				vsscanfDx(     const TCHAR *String, const TCHAR *FormatString, va_list Arg ) ;	// vsscanf と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )
extern	int				sscanfDx(      const TCHAR *String, const TCHAR *FormatString, ... ) ;			// sscanf と同等の??( ?ル?バイト文字列版では文字コ?ド?式として SetUseCharCodeFormat で設定した?式が使用されます )


















// DxNetwork.cpp関数プロト?イプ宣言

#ifndef DX_NON_NETWORK

// 通信関係
extern	int			ProcessNetMessage(				int RunReleaseProcess DEFAULTPARAM( = FALSE ) ) ;										// 通信メッセ?ジの処理をする関数

extern	int			GetHostIPbyName(				const TCHAR *HostName,							IPDATA      *IPDataBuf, int IPDataBufLength DEFAULTPARAM( = 1 ) , int *IPDataGetNum DEFAULTPARAM( = NULL ) ) ;		// ＤＮＳサ?バ?を使ってホスト名からＩＰアドレスを取得する( IPv4版 )
extern	int			GetHostIPbyNameWithStrLen(		const TCHAR *HostName, size_t HostNameLength,	IPDATA      *IPDataBuf, int IPDataBufLength DEFAULTPARAM( = 1 ) , int *IPDataGetNum DEFAULTPARAM( = NULL ) ) ;		// ＤＮＳサ?バ?を使ってホスト名からＩＰアドレスを取得する( IPv4版 )
extern	int			GetHostIPbyName_IPv6(			const TCHAR *HostName,							IPDATA_IPv6 *IPDataBuf, int IPDataBufLength DEFAULTPARAM( = 1 ) , int *IPDataGetNum DEFAULTPARAM( = NULL ) ) ;		// ＤＮＳサ?バ?を使ってホスト名からＩＰアドレスを取得する( IPv6版 )
extern	int			GetHostIPbyName_IPv6WithStrLen(	const TCHAR *HostName, size_t HostNameLength,	IPDATA_IPv6 *IPDataBuf, int IPDataBufLength DEFAULTPARAM( = 1 ) , int *IPDataGetNum DEFAULTPARAM( = NULL ) ) ;		// ＤＮＳサ?バ?を使ってホスト名からＩＰアドレスを取得する( IPv6版 )
extern 	int			ConnectNetWork(					IPDATA      IPData, int Port DEFAULTPARAM( = -1 ) ) ;									// 他?シンに接続する( IPv4版 )
extern	int			ConnectNetWork_IPv6(			IPDATA_IPv6 IPData, int Port DEFAULTPARAM( = -1 ) ) ;									// 他?シンに接続する( IPv6版 )
extern 	int			ConnectNetWork_ASync(			IPDATA      IPData, int Port DEFAULTPARAM( = -1 ) ) ;									// 他?シンに接続する( IPv4版 )、非同期版
extern	int			ConnectNetWork_IPv6_ASync(		IPDATA_IPv6 IPData, int Port DEFAULTPARAM( = -1 ) ) ;									// 他?シンに接続する( IPv6版 )、非同期版
extern 	int			PreparationListenNetWork(		int Port DEFAULTPARAM( = -1 ) ) ;														// 接続を受けられる状態にする( IPv4版 )
extern 	int			PreparationListenNetWork_IPv6(	int Port DEFAULTPARAM( = -1 ) ) ;														// 接続を受けられる状態にする( IPv6版 )
extern 	int			StopListenNetWork(				void ) ;																				// 接続を受けつけ状態の解除
extern 	int			CloseNetWork(					int NetHandle ) ;																		// 接続を終了する

extern 	int			GetNetWorkAcceptState(			int NetHandle ) ;																		// 接続状態を取得する
extern 	int			GetNetWorkDataLength(			int NetHandle ) ;																		// 受信デ??の量を得る
extern	int			GetNetWorkSendDataLength(		int NetHandle ) ;																		// 未送信のデ??の量を得る 
extern 	int			GetNewAcceptNetWork(			void ) ;																				// 新たに接続した通信回線を得る
extern 	int			GetLostNetWork(					void ) ;																				// 接続を切断された通信回線を得る
extern 	int			GetNetWorkIP(					int NetHandle, IPDATA      *IpBuf ) ;													// 接続先のＩＰを得る( IPv4版 )
extern 	int			GetNetWorkIP_IPv6(				int NetHandle, IPDATA_IPv6 *IpBuf ) ;													// 接続先のＩＰを得る( IPv6版 )
extern	int			GetMyIPAddress(					IPDATA      *IpBuf, int IpBufLength DEFAULTPARAM( = 1 ) , int *IpNum DEFAULTPARAM( = NULL ) ) ;			// 自分のIPv4を得る
extern	int			GetMyIPAddress_IPv6(			IPDATA_IPv6 *IpBuf, int IpBufLength DEFAULTPARAM( = 1 ) , int *IpNum DEFAULTPARAM( = NULL ) ) ;			// 自分のIPv6を得る
extern	int			SetConnectTimeOutWait(			int Time ) ;																			// 接続の?イ?アウトまでの時間を設定する
extern	int			SetUseDXNetWorkProtocol(		int Flag ) ;																			// ＤＸライブラリの通信?態を使うかどうかをセットする
extern	int			GetUseDXNetWorkProtocol(		void ) ; 																				// ＤＸライブラリの通信?態を使うかどうかを取得する
extern	int			SetUseDXProtocol(				int Flag ) ;																			// SetUseDXNetWorkProtocol の別名
extern	int			GetUseDXProtocol(				void ) ; 																				// GetUseDXNetWorkProtocol の別名
extern	int			SetNetWorkCloseAfterLostFlag(	int Flag ) ;																			// 接続が切断された直後に接続ハンドルを解放するかどうかのフラグをセットする
extern	int			GetNetWorkCloseAfterLostFlag(	void ) ;																				// 接続が切断された直後に接続ハンドルを解放するかどうかのフラグを取得する
//extern	int			SetProxySetting( int UseFlag, const char *Address, int Port ) ;														// ＨＴＴＰ通信で使用するプロキシ設定を行う
//extern	int			GetProxySetting( int *UseFlagBuffer, char *AddressBuffer, int *PortBuffer ) ;										// ＨＴＴＰ通信で使用するプロキシ設定を取得する
//extern	int			SetIEProxySetting( void ) ;																							// ＩＥのプロキシ設定を適応する

extern 	int			NetWorkRecv(			int NetHandle, void *Buffer, int Length ) ;														// 受信したデ??を読み込む
extern	int			NetWorkRecvToPeek(		int NetHandle, void *Buffer, int Length ) ;														// 受信したデ??を読み込む、読み込んだデ??はバッフ?から削除されない
extern	int			NetWorkRecvBufferClear(	int NetHandle ) ;																				// 受信したデ??をクリアする
extern 	int			NetWorkSend(			int NetHandle, const void *Buffer, int Length ) ;												// デ??を送信する

extern	int			MakeUDPSocket(			int RecvPort DEFAULTPARAM( = -1 ) ) ;															// UDPを使用した通信を行う?ケットハンドルを作成する( RecvPort を -1 にすると送信専用の?ケットハンドルになります )
extern	int			MakeUDPSocket_IPv6(		int RecvPort DEFAULTPARAM( = -1 ) ) ;															// UDPを使用した通信を行う?ケットハンドルを作成する( RecvPort を -1 にすると送信専用の?ケットハンドルになります )( IPv6版 )
extern	int			DeleteUDPSocket(		int NetUDPHandle ) ;																			// UDPを使用した通信を行う?ケットハンドルを削除する
extern	int			NetWorkSendUDP(			int NetUDPHandle, IPDATA       SendIP, int SendPort,  const void *Buffer, int Length ) ;			// UDPを使用した通信で指定のＩＰにデ??を送信する、Length は最大65507、SendPort を -1 にすると MakeUDPSocket に RecvPort で渡した??トが使用されます( 戻り値  0以上;送信できたデ??サイズ  -1:エラ?  -2:送信デ??が大きすぎる  -3:送信?備ができていない  )
extern	int			NetWorkSendUDP_IPv6(	int NetUDPHandle, IPDATA_IPv6  SendIP, int SendPort,  const void *Buffer, int Length ) ;			// UDPを使用した通信で指定のＩＰにデ??を送信する、Length は最大65507、SendPort を -1 にすると MakeUDPSocket に RecvPort で渡した??トが使用されます( 戻り値  0以上;送信できたデ??サイズ  -1:エラ?  -2:送信デ??が大きすぎる  -3:送信?備ができていない  )( IPv6版 )
extern	int			NetWorkRecvUDP(			int NetUDPHandle, IPDATA      *RecvIP, int *RecvPort,       void *Buffer, int Length, int Peek ) ;	// UDPを使用した通信でデ??を受信する、Peek に TRUE を渡すと受信に成功してもデ??を受信キュ?から削除しません( 戻り値  0以上:受信したデ??のサイズ  -1:エラ?  -2:バッフ?のサイズが足りない  -3:受信デ??がない )
extern	int			NetWorkRecvUDP_IPv6(	int NetUDPHandle, IPDATA_IPv6 *RecvIP, int *RecvPort,       void *Buffer, int Length, int Peek ) ;	// UDPを使用した通信でデ??を受信する、Peek に TRUE を渡すと受信に成功してもデ??を受信キュ?から削除しません( 戻り値  0以上:受信したデ??のサイズ  -1:エラ?  -2:バッフ?のサイズが足りない  -3:受信デ??がない )( IPv6版 )
//extern int		CheckNetWorkSendUDP(	int NetUDPHandle ) ;																			// UDPを使用した通信でデ??が送信できる状態かどうかを調べる( 戻り値  -1:エラ?  TRUE:送信可?  FALSE:送信不可? )
extern	int			CheckNetWorkRecvUDP(	int NetUDPHandle ) ;																			// UDPを使用した通信で新たな受信デ??が存在するかどうかを調べる( 戻り値  -1:エラ?  TRUE:受信デ??あり  FALSE:受信デ??なし )

/*	使用不可
extern	int			HTTP_FileDownload(			const char *FileURL, const char *SavePath DEFAULTPARAM( = NULL ) , void **SaveBufferP DEFAULTPARAM( = NULL ) , int *FileSize DEFAULTPARAM( = NULL ) , char **ParamList DEFAULTPARAM( = NULL ) ) ;						// HTTP を使用してネットワ?ク上のフ?イルを?ウンロ?ドする
extern	int			HTTP_GetFileSize(			const char *FileURL ) ;																		// HTTP を使用してネットワ?ク上のフ?イルのサイズを得る

extern	int			HTTP_StartFileDownload(		const char *FileURL, const char *SavePath, void **SaveBufferP DEFAULTPARAM( = NULL ) , char **ParamList DEFAULTPARAM( = NULL ) ) ;	// HTTP を使用したネットワ?ク上のフ?イルを?ウンロ?ドする処理を開始する
extern	int			HTTP_StartGetFileSize(		const char *FileURL ) ;																		// HTTP を使用したネットワ?ク上のフ?イルのサイズを得る処理を開始する
extern	int			HTTP_Close(					int HttpHandle ) ;																			// HTTP の処理を終了し、ハンドルを解放する
extern	int			HTTP_CloseAll(				void ) ;																					// 全てのハンドルに対して HTTP_Close を行う
extern	int			HTTP_GetState(				int HttpHandle ) ;																			// HTTP 処理の現在の状態を得る( NET_RES_COMPLETE 等 )
extern	int			HTTP_GetError(				int HttpHandle ) ;																			// HTTP 処理でエラ?が発生した場合、エラ?の内容を得る( HTTP_ERR_NONE 等 )
extern	int			HTTP_GetDownloadFileSize(	int HttpHandle ) ;																			// HTTP 処理で対象となっているフ?イルのサイズを得る( 戻り値: -1 = エラ?・若しくはまだフ?イルのサイズを取得していない  0以上 = フ?イルのサイズ )
extern	int			HTTP_GetDownloadedFileSize( int HttpHandle ) ;																			// HTTP 処理で既に?ウンロ?ドしたフ?イルのサイズを取得する

extern	int			fgetsForNetHandle(			int NetHandle, char *strbuffer ) ;															// fgets のネットワ?クハンドル版( -1:取得できず 0:取得できた )
extern	int			URLAnalys(					const char *URL, char *HostBuf DEFAULTPARAM( = NULL ) , char *PathBuf DEFAULTPARAM( = NULL ) , char *FileNameBuf DEFAULTPARAM( = NULL ) , int *PortBuf DEFAULTPARAM( = NULL ) ) ;	// ＵＲＬを解析する
extern	int			URLConvert(					char *URL, int ParamConvert DEFAULTPARAM( = TRUE ) , int NonConvert DEFAULTPARAM( = FALSE ) ) ;				// HTTP に渡せない記号が使われた文字列を渡せるような文字列に変換する( 戻り値: -1 = エラ?  0以上 = 変換後の文字列のサイズ )
extern	int			URLParamAnalysis(			char **ParamList, char **ParamStringP ) ;													// HTTP 用パラメ??リストから一つのパラメ??文字列を作成する( 戻り値:  -1 = エラ?  0以上 = パラメ??の文字列の長さ )
*/

#endif // DX_NON_NETWORK
















// DxInputString.cpp関数プロト?イプ宣言

#ifndef DX_NON_INPUTSTRING

// 文字コ?ドバッフ??作関係
extern	int			StockInputChar(		TCHAR CharCode ) ;								// 文字コ?ドバッフ?に文字コ?ドをストックする
extern	int			ClearInputCharBuf(	void ) ;										// 文字コ?ドバッフ?をクリアする
extern	TCHAR		GetInputChar(		int DeleteFlag ) ;								// 文字コ?ドバッフ?に溜まったデ??から文字コ?ドを一つ取得する
extern	TCHAR		GetInputCharWait(	int DeleteFlag ) ;								// 文字コ?ドバッフ?に溜まったデ??から文字コ?ドを一つ取得する、バッフ?になにも文字コ?ドがない場合は文字コ?ドがバッフ?に一文字分溜まるまで待つ

extern	int			GetOneChar(			TCHAR *CharBuffer, int DeleteFlag ) ;			// 文字コ?ドバッフ?に溜まったデ??から１文字分取得する
extern	int			GetOneCharWait(		TCHAR *CharBuffer, int DeleteFlag ) ;			// 文字コ?ドバッフ?に溜まったデ??から１文字分取得する、バッフ?に何も文字コ?ドがない場合は文字コ?ドがバッフ?に一文字分溜まるまで待つ
extern	int			GetCtrlCodeCmp(		TCHAR Char ) ;									// 指定の文字コ?ドがアスキ?コントロ?ルコ?ドか調べる

#ifndef DX_NON_KEYEX

extern	int			DrawIMEInputString(				int x, int y,                                 int SelectStringNum , int DrawCandidateList DEFAULTPARAM( = TRUE ) ) ;	// 画面上に入力中の文字列を?画する
extern	int			DrawIMEInputExtendString(		int x, int y, double ExRateX, double ExRateY, int SelectStringNum , int DrawCandidateList DEFAULTPARAM( = TRUE ) ) ;	// 画面上に入力中の文字列を?画する( 拡大率付き )
extern	int			SetUseIMEFlag(					int UseFlag ) ;							// ＩＭＥを使用するかどうかを設定する
extern	int			GetUseIMEFlag(					void ) ;								// ＩＭＥを使用するかどうかの設定を取得する
extern	int			SetInputStringMaxLengthIMESync(	int Flag ) ;							// ＩＭＥで入力できる最大文字数を MakeKeyInput の設定に合わせるかどうかをセットする( TRUE:あわせる  FALSE:あわせない(デフォルト) )
extern	int			SetIMEInputStringMaxLength(		int Length ) ;							// ＩＭＥで一度に入力できる最大文字数を設定する( 0:制限なし  1以上:指定の文字数で制限 )

#endif // DX_NON_KEYEX

#endif // DX_NON_INPUTSTRING

extern	int			GetStringPoint(				const TCHAR *String,                      int Point ) ;		// 全角文字、半角文字入り乱れる中から指定の文字数での半角文字数を得る
extern	int			GetStringPointWithStrLen(	const TCHAR *String, size_t StringLength, int Point ) ;		// 全角文字、半角文字入り乱れる中から指定の文字数での半角文字数を得る
extern	int			GetStringPoint2(			const TCHAR *String,                      int Point ) ;		// 全角文字、半角文字入り乱れる中から指定の半角文字数での文字数を得る
extern	int			GetStringPoint2WithStrLen(	const TCHAR *String, size_t StringLength, int Point ) ;		// 全角文字、半角文字入り乱れる中から指定の半角文字数での文字数を得る
extern	int			GetStringLength(			const TCHAR *String ) ;										// 全角文字、半角文字入り乱れる中から文字数を取得する

#ifndef DX_NON_FONT
extern	int			DrawObtainsString(						int x, int y, int AddY, const TCHAR *String,                      unsigned int StrColor, unsigned int StrEdgeColor DEFAULTPARAM( = 0 ) , int FontHandle DEFAULTPARAM( = -1 ) , unsigned int SelectBackColor DEFAULTPARAM( = 0xffffffff ) , unsigned int SelectStrColor DEFAULTPARAM( = 0 ) , unsigned int SelectStrEdgeColor DEFAULTPARAM( = 0xffffffff ) , int SelectStart DEFAULTPARAM( = -1 ) , int SelectEnd DEFAULTPARAM( = -1 ) , int *LineCount DEFAULTPARAM( = NULL ) ) ;		// ?画可?領域に収まるように改行しながら文字列を?画
extern	int			DrawObtainsNString(						int x, int y, int AddY, const TCHAR *String, size_t StringLength, unsigned int StrColor, unsigned int StrEdgeColor DEFAULTPARAM( = 0 ) , int FontHandle DEFAULTPARAM( = -1 ) , unsigned int SelectBackColor DEFAULTPARAM( = 0xffffffff ) , unsigned int SelectStrColor DEFAULTPARAM( = 0 ) , unsigned int SelectStrEdgeColor DEFAULTPARAM( = 0xffffffff ) , int SelectStart DEFAULTPARAM( = -1 ) , int SelectEnd DEFAULTPARAM( = -1 ) , int *LineCount DEFAULTPARAM( = NULL ) ) ;		// ?画可?領域に収まるように改行しながら文字列を?画
extern	int			DrawObtainsString_CharClip(				int x, int y, int AddY, const TCHAR *String,                      unsigned int StrColor, unsigned int StrEdgeColor DEFAULTPARAM( = 0 ) , int FontHandle DEFAULTPARAM( = -1 ) , unsigned int SelectBackColor DEFAULTPARAM( = 0xffffffff ) , unsigned int SelectStrColor DEFAULTPARAM( = 0 ) , unsigned int SelectStrEdgeColor DEFAULTPARAM( = 0xffffffff ) , int SelectStart DEFAULTPARAM( = -1 ) , int SelectEnd DEFAULTPARAM( = -1 ) , int *LineCount DEFAULTPARAM( = NULL ) ) ;		// ?画可?領域に収まるように改行しながら文字列を?画( クリップが文字単位 )
extern	int			DrawObtainsNString_CharClip(			int x, int y, int AddY, const TCHAR *String, size_t StringLength, unsigned int StrColor, unsigned int StrEdgeColor DEFAULTPARAM( = 0 ) , int FontHandle DEFAULTPARAM( = -1 ) , unsigned int SelectBackColor DEFAULTPARAM( = 0xffffffff ) , unsigned int SelectStrColor DEFAULTPARAM( = 0 ) , unsigned int SelectStrEdgeColor DEFAULTPARAM( = 0xffffffff ) , int SelectStart DEFAULTPARAM( = -1 ) , int SelectEnd DEFAULTPARAM( = -1 ) , int *LineCount DEFAULTPARAM( = NULL ) ) ;		// ?画可?領域に収まるように改行しながら文字列を?画( クリップが文字単位 )
extern	int			DrawObtainsString_WordClip(				int x, int y, int AddY, const TCHAR *String,                      unsigned int StrColor, unsigned int StrEdgeColor DEFAULTPARAM( = 0 ) , int FontHandle DEFAULTPARAM( = -1 ) , unsigned int SelectBackColor DEFAULTPARAM( = 0xffffffff ) , unsigned int SelectStrColor DEFAULTPARAM( = 0 ) , unsigned int SelectStrEdgeColor DEFAULTPARAM( = 0xffffffff ) , int SelectStart DEFAULTPARAM( = -1 ) , int SelectEnd DEFAULTPARAM( = -1 ) , int *LineCount DEFAULTPARAM( = NULL ) ) ;		// ?画可?領域に収まるように改行しながら文字列を?画( クリップが単語単位 )
extern	int			DrawObtainsNString_WordClip(			int x, int y, int AddY, const TCHAR *String, size_t StringLength, unsigned int StrColor, unsigned int StrEdgeColor DEFAULTPARAM( = 0 ) , int FontHandle DEFAULTPARAM( = -1 ) , unsigned int SelectBackColor DEFAULTPARAM( = 0xffffffff ) , unsigned int SelectStrColor DEFAULTPARAM( = 0 ) , unsigned int SelectStrEdgeColor DEFAULTPARAM( = 0xffffffff ) , int SelectStart DEFAULTPARAM( = -1 ) , int SelectEnd DEFAULTPARAM( = -1 ) , int *LineCount DEFAULTPARAM( = NULL ) ) ;		// ?画可?領域に収まるように改行しながら文字列を?画( クリップが単語単位 )
extern	int			GetObtainsStringCharPosition(			int x, int y, int AddY, const TCHAR *String, int StrLen, int *PosX, int *PosY, int FontHandle DEFAULTPARAM( = -1 ) , int *LineCount DEFAULTPARAM( = NULL ) ) ;		// ?画可?領域に収まるように改行しながら文字列を?画した場合の文字列の末?の座標を取得する
extern	int			GetObtainsStringCharPosition_CharClip(	int x, int y, int AddY, const TCHAR *String, int StrLen, int *PosX, int *PosY, int FontHandle DEFAULTPARAM( = -1 ) , int *LineCount DEFAULTPARAM( = NULL ) ) ;		// ?画可?領域に収まるように改行しながら文字列を?画した場合の文字列の末?の座標を取得する( クリップが文字単位 )
extern	int			GetObtainsStringCharPosition_WordClip(	int x, int y, int AddY, const TCHAR *String, int StrLen, int *PosX, int *PosY, int FontHandle DEFAULTPARAM( = -1 ) , int *LineCount DEFAULTPARAM( = NULL ) ) ;		// ?画可?領域に収まるように改行しながら文字列を?画した場合の文字列の末?の座標を取得する( クリップが単語単位 )
#endif // DX_NON_FONT
extern	int			DrawObtainsBox(					int x1, int y1, int x2, int y2, int AddY, unsigned int Color, int FillFlag ) ;																																										// ?画可?領域に収まるように補正を加えながら矩?を?画

#ifndef DX_NON_INPUTSTRING

#ifndef DX_NON_KEYEX

extern	int			InputStringToCustom(			int x, int y, size_t BufLength, TCHAR *StrBuffer, int CancelValidFlag, int SingleCharOnlyFlag, int NumCharOnlyFlag, int DoubleCharOnlyFlag DEFAULTPARAM( = FALSE ) , int EnableNewLineFlag DEFAULTPARAM( = FALSE ) , int DisplayCandidateList DEFAULTPARAM( = TRUE ) ) ;		// 文字列の入力取得

extern	int			KeyInputString(					int x, int y, size_t CharMaxLength, TCHAR *StrBuffer, int CancelValidFlag ) ;													// 文字列の入力取得
extern	int			KeyInputSingleCharString(		int x, int y, size_t CharMaxLength, TCHAR *StrBuffer, int CancelValidFlag ) ;													// 半角文字列のみの入力取得
extern	int			KeyInputNumber(					int x, int y, int MaxNum, int MinNum, int CancelValidFlag ) ;																	// 数値の入力取得

extern	int			GetIMEInputModeStr(				TCHAR *GetBuffer ) ;																											// IMEの入力モ?ド文字列を取得する
extern	const IMEINPUTDATA* GetIMEInputData(		void ) ;																														// IMEで入力中の文字列の情報を取得する
extern	int			SetIMEInputString(				const TCHAR *String ) ;																											// IMEで入力中の文字列を変更する( IMEで文字列を入力中ではなかった場合は何も起こりません )
extern	int			SetIMEInputStringWithStrLen(	const TCHAR *String, size_t StringLength ) ;																					// IMEで入力中の文字列を変更する( IMEで文字列を入力中ではなかった場合は何も起こりません )
extern	int			SetKeyInputStringColor(			ULONGLONG NmlStr, ULONGLONG NmlCur, ULONGLONG IMEStrBack, ULONGLONG IMECur, ULONGLONG IMELine, ULONGLONG IMESelectStr, ULONGLONG IMEModeStr , ULONGLONG NmlStrE DEFAULTPARAM( = 0 ) , ULONGLONG IMESelectStrE DEFAULTPARAM( = 0 ) , ULONGLONG IMEModeStrE DEFAULTPARAM( = 0 ) , ULONGLONG IMESelectWinE DEFAULTPARAM( = ULL_PARAM( 0xffffffffffffffff ) ) ,	ULONGLONG IMESelectWinF DEFAULTPARAM( = ULL_PARAM( 0xffffffffffffffff ) ) , ULONGLONG SelectStrBackColor DEFAULTPARAM( = ULL_PARAM( 0xffffffffffffffff ) ) , ULONGLONG SelectStrColor DEFAULTPARAM( = ULL_PARAM( 0xffffffffffffffff ) ) , ULONGLONG SelectStrEdgeColor DEFAULTPARAM( = ULL_PARAM( 0xffffffffffffffff ) ) , ULONGLONG IMEStr DEFAULTPARAM( = ULL_PARAM( 0xffffffffffffffff ) ) , ULONGLONG IMEStrE DEFAULTPARAM( = ULL_PARAM( 0xffffffffffffffff ) ) ) ;	// ( SetKeyInputStringColor2 の旧関数 )InputString関数使用時の文字の各色を変更する
extern	int			SetKeyInputStringColor2(		int TargetColor /* DX_KEYINPSTRCOLOR_NORMAL_STR 等 */ , unsigned int Color ) ;													// InputString関数使用時の文字の各色を変更する
extern	int			ResetKeyInputStringColor2(		int TargetColor /* DX_KEYINPSTRCOLOR_NORMAL_STR 等 */ ) ;																		// SetKeyInputStringColor2 で設定した色をデフォルトに戻す
extern	int			SetKeyInputStringFont(			int FontHandle ) ;																												// キ?入力文字列?画関連で使用するフォントのハンドルを変更する(-1でデフォルトのフォントハンドル)
extern	int			SetKeyInputStringEndCharaMode(	int EndCharaMode /* DX_KEYINPSTR_ENDCHARAMODE_OVERWRITE 等 */ ) ;																// キ?入力文字列処理の入力文字数が限界に達している状態で、文字列の末?部分で入力が行われた場合の処理モ?ドを変更する
extern	int			DrawKeyInputModeString(			int x, int y ) ;																												// 入力モ?ド文字列を?画する

extern	int			InitKeyInput(					void ) ;																														// キ?入力デ??初期化
extern	int			MakeKeyInput(					size_t MaxStrLength, int CancelValidFlag, int SingleCharOnlyFlag, int NumCharOnlyFlag, int DoubleCharOnlyFlag DEFAULTPARAM( = FALSE ) , int EnableNewLineFlag DEFAULTPARAM( = FALSE ) ) ;			// 新しいキ?入力ハンドルの作成
extern	int			DeleteKeyInput(					int InputHandle ) ;																												// キ?入力ハンドルの削除
extern	int			SetActiveKeyInput(				int InputHandle ) ;																												// 指定のキ?入力ハンドルをアクティブにする( -1 を指定するとアクティブなキ?入力ハンドルが無い状態になります )
extern	int			GetActiveKeyInput(				void ) ;																														// 現在アクティブになっているキ?入力ハンドルを取得する
extern	int			CheckKeyInput(					int InputHandle ) ;																												// キ?入力ハンドルの入力が終了しているか取得する
extern	int			ReStartKeyInput(				int InputHandle ) ;																												// 入力が完了したキ?入力ハンドルを再度編集状態に戻す
extern	int			ProcessActKeyInput(				void ) ;																														// キ?入力ハンドル処理関数
extern	int			DrawKeyInputString(				int x, int y,                                 int InputHandle , int DrawCandidateList DEFAULTPARAM( = TRUE ) ) ;				// キ?入力ハンドルの入力中情報の?画
extern	int			DrawKeyInputExtendString(		int x, int y, double ExRateX, double ExRateY, int InputHandle , int DrawCandidateList DEFAULTPARAM( = TRUE ) ) ;				// キ?入力ハンドルの入力中情報の?画( 拡大率指定付き )
extern	int			SetKeyInputDrawArea(			int x1, int y1, int x2, int y2, int InputHandle ) ;																				// キ?入力ハンドルの入力中文字列を?画する際の?画範囲を設定する

extern	int			SetKeyInputSelectArea(			int  SelectStart, int  SelectEnd, int InputHandle ) ;																			// キ?入力ハンドルの指定の領域を選択状態にする( SelectStart と SelectEnd に -1 を指定すると選択状態が解除されます )
extern	int			GetKeyInputSelectArea(			int *SelectStart, int *SelectEnd, int InputHandle ) ;																			// キ?入力ハンドルの選択領域を取得する
extern	int			SetKeyInputDrawStartPos(		int DrawStartPos, int InputHandle ) ;																							// キ?入力ハンドルの?画開始文字位置を設定する
extern	int			GetKeyInputDrawStartPos(		int InputHandle ) ;																												// キ?入力ハンドルの?画開始文字位置を取得する
extern	int			SetKeyInputCursorBrinkTime(		int Time ) ;																													// キ?入力ハンドルのキ?入力時のカ??ルの?滅する早さをセットする
extern	int			SetKeyInputCursorBrinkFlag(		int Flag ) ;																													// キ?入力ハンドルのキ?入力時のカ??ルを?滅させるかどうかをセットする
extern	int			SetKeyInputString(				const TCHAR *String,                      int InputHandle ) ;																	// キ?入力ハンドルに指定の文字列をセットする
extern	int			SetKeyInputStringWithStrLen(	const TCHAR *String, size_t StringLength, int InputHandle ) ;																	// キ?入力ハンドルに指定の文字列をセットする
extern	int			SetKeyInputNumber(				int   Number,                             int InputHandle ) ;																	// キ?入力ハンドルに指定の数値を文字に置き換えてセットする
extern	int			SetKeyInputNumberToFloat(		float Number,                             int InputHandle ) ;																	// キ?入力ハンドルに指定の浮動小数?値を文字に置き換えてセットする
extern	int			GetKeyInputString(				TCHAR *StrBuffer,                         int InputHandle ) ;																	// キ?入力ハンドルの入力中の文字列を取得する
extern	int			GetKeyInputNumber(				int InputHandle ) ;																												// キ?入力ハンドルの入力中の文字列を整数値として取得する
extern	float		GetKeyInputNumberToFloat(		int InputHandle ) ;																												// キ?入力ハンドルの入力中の文字列を浮動小数?値として取得する
extern	int			SetKeyInputCursorPosition(		int Position,        int InputHandle ) ;																						// キ?入力ハンドルの現在のカ??ル位置を設定する
extern	int			GetKeyInputCursorPosition(		int InputHandle ) ;																												// キ?入力ハンドルの現在のカ??ル位置を取得する

#endif // DX_NON_KEYEX

#endif // DX_NON_INPUTSTRING










// DxFile.cpp関数プロト?イプ宣言

// フ?イルアクセス関数
extern	int			FileRead_open(						const TCHAR *FilePath,                        int ASync DEFAULTPARAM( = FALSE ) ) ;	// フ?イルを開く
extern	int			FileRead_open_WithStrLen(			const TCHAR *FilePath, size_t FilePathLength, int ASync DEFAULTPARAM( = FALSE ) ) ;	// フ?イルを開く
extern	int			FileRead_open_mem(					const void *FileImage, size_t FileImageSize ) ;					// メモリに展開されたフ?イルを開く
extern	LONGLONG	FileRead_size(						const TCHAR *FilePath ) ;										// フ?イルのサイズを取得する
extern	LONGLONG	FileRead_size_WithStrLen(			const TCHAR *FilePath, size_t FilePathLength ) ;				// フ?イルのサイズを取得する
extern	LONGLONG	FileRead_size_handle(				int FileHandle ) ;												// フ?イルのサイズを取得する( フ?イルハンドル使用版 )
extern	int			FileRead_close(						int FileHandle ) ;												// フ?イルを閉じる
extern	LONGLONG	FileRead_tell(						int FileHandle ) ;												// フ?イル?イン?の読み込み位置を取得する
extern	int			FileRead_seek(						int FileHandle , LONGLONG Offset , int Origin ) ;				// フ?イル?イン?の読み込み位置を変更する
extern	int			FileRead_read(						void *Buffer , int ReadSize , int FileHandle ) ;				// フ?イルからデ??を読み込む
extern	int			FileRead_idle_chk(					int FileHandle ) ;												// フ?イル読み込みが完了しているかどうかを取得する
extern	int			FileRead_eof(						int FileHandle ) ;												// フ?イルの読み込み位置が終?に達しているかどうかを取得する
extern	int			FileRead_set_format(				int FileHandle, int CharCodeFormat /* DX_CHARCODEFORMAT_SHIFTJIS 等 */ ) ;	// フ?イルの文字コ?ド?式を設定する( テキストフ?イル用 )
extern	int			FileRead_gets(						TCHAR *Buffer , int BufferSize , int FileHandle ) ;				// フ?イルから文字列を読み出す
extern	TCHAR		FileRead_getc(						int FileHandle ) ;												// フ?イルから一文字読み出す
extern	int			FileRead_scanf(						int FileHandle , const TCHAR *Format , ... ) ;					// フ?イルから書式化されたデ??を読み出す

extern	DWORD_PTR	FileRead_createInfo(				const TCHAR *ObjectPath ) ;										// フ?イル情報ハンドルを作成する( 戻り値  -1:エラ?  -1以外:フ?イル情報ハンドル )
extern	DWORD_PTR	FileRead_createInfo_WithStrLen(		const TCHAR *ObjectPath, size_t ObjectPathLength ) ;			// フ?イル情報ハンドルを作成する( 戻り値  -1:エラ?  -1以外:フ?イル情報ハンドル )
extern	int			FileRead_getInfoNum(				DWORD_PTR FileInfoHandle ) ;									// フ?イル情報ハンドル中のフ?イルの数を取得する
extern	int			FileRead_getInfo(					int Index , FILEINFO *Buffer , DWORD_PTR FileInfoHandle ) ;		// フ?イル情報ハンドル中のフ?イルの情報を取得する
extern	int			FileRead_deleteInfo(				DWORD_PTR FileInfoHandle ) ;									// フ?イル情報ハンドルを削除する

extern	DWORD_PTR	FileRead_findFirst(					const TCHAR *FilePath,                        FILEINFO *Buffer ) ; // 指定のフ?イル又はフォル?の情報を取得し、フ?イル検索ハンドルも作成する( 戻り値: -1=エラ?  -1以外=フ?イル検索ハンドル )
extern	DWORD_PTR	FileRead_findFirst_WithStrLen(		const TCHAR *FilePath, size_t FilePathLength, FILEINFO *Buffer ) ; // 指定のフ?イル又はフォル?の情報を取得し、フ?イル検索ハンドルも作成する( 戻り値: -1=エラ?  -1以外=フ?イル検索ハンドル )
extern	int			FileRead_findNext(					DWORD_PTR FindHandle, FILEINFO *Buffer ) ;						// 条件の合致する次のフ?イルの情報を取得する( 戻り値: -1=エラ?  0=成功 )
extern	int			FileRead_findClose(					DWORD_PTR FindHandle ) ;										// フ?イル検索ハンドルを閉じる( 戻り値: -1=エラ?  0=成功 )

extern	int			FileRead_fullyLoad(					const TCHAR *FilePath ) ;										// 指定のフ?イルの内容を全てメモリに読み込み、その情報のアクセスに必要なハンドルを返す( 戻り値  -1:エラ?  -1以外:ハンドル )、使い終わったらハンドルは FileRead_fullyLoad_delete で削除する必要があります
extern	int			FileRead_fullyLoad_WithStrLen(		const TCHAR *FilePath, size_t FilePathLength ) ;				// 指定のフ?イルの内容を全てメモリに読み込み、その情報のアクセスに必要なハンドルを返す( 戻り値  -1:エラ?  -1以外:ハンドル )、使い終わったらハンドルは FileRead_fullyLoad_delete で削除する必要があります
extern	int			FileRead_fullyLoad_delete(			int FLoadHandle ) ;												// FileRead_fullyLoad で読み込んだフ?イルのハンドルを削除する
extern	const void*	FileRead_fullyLoad_getImage(		int FLoadHandle ) ;												// FileRead_fullyLoad で読み込んだフ?イルの内容を格?したメモリアドレスを取得する
extern	LONGLONG	FileRead_fullyLoad_getSize(			int FLoadHandle ) ;												// FileRead_fullyLoad で読み込んだフ?イルのサイズを取得する

// 設定関係関数
extern	int			GetStreamFunctionDefault(			void ) ;														// ＤＸライブラリでストリ??デ??アクセスに使用する関数がデフォルトのものか調べる( TRUE:デフォルトのもの  FALSE:デフォルトではない )
extern	int			ChangeStreamFunction(				const STREAMDATASHREDTYPE2  *StreamThread  ) ;					// ＤＸライブラリでストリ??デ??アクセスに使用する関数を変更する
extern	int			ChangeStreamFunctionW(				const STREAMDATASHREDTYPE2W *StreamThreadW ) ;					// ＤＸライブラリでストリ??デ??アクセスに使用する関数を変更する( wchar_t 使用版 )

// 補助関係関数
extern int			ConvertFullPath(					const TCHAR *Src,                   TCHAR *Dest, const TCHAR *CurrentDir DEFAULTPARAM( = NULL )                                              ) ;	// フルパスではないパス文字列をフルパスに変換する( CurrentDir はフルパスである必要がある(語尾に『\』があっても無くても良い) )( CurrentDir が NULL の場合は現在のカレントディレクトリを使用する )
extern int			ConvertFullPathWithStrLen(			const TCHAR *Src, size_t SrcLength, TCHAR *Dest, const TCHAR *CurrentDir DEFAULTPARAM( = NULL ), size_t CurrentDirLength DEFAULTPARAM( = 0 ) ) ;












// DxInput.cpp関数プロト?イプ宣言

#ifndef DX_NON_INPUT

// 入力状態取得関数
extern	int			CheckHitKey(							int KeyCode ) ;															// キ???ドの押下状態を取得する
extern	int			CheckHitKeyAll(							int CheckType DEFAULTPARAM( = DX_CHECKINPUT_ALL ) ) ;					// どれか一つでもキ?が押されているかどうかを取得( 押されていたら戻り値が 0 以外になる )
extern	int			GetHitKeyStateAll(						DX_CHAR *KeyStateArray ) ;												// すべてのキ?の押下状態を取得する( KeyStateBuf:char?256個分の配列の先頭アドレス )
extern	int			GetHitKeyStateAllEx(					int *KeyStateArray ) ;													// すべてのキ?の押下状態を取得する( KeyStateBuf:int?256個分の配列の先頭アドレス、初回は配列の全要素に 0 が代入されている必要があります、配列の各要素の状態?0:押されていない?1：押された１回め?2以上：押され続けている回数?-1：押されて離された１回め?-2以下：押されて離されてからの回数 )
extern	int			GetJoypadNum(							void ) ;																// ジョイパッドが接続されている数を取得する
extern	int			GetJoypadButtonNum(						int InputType ) ;														// ジョイパッドの??ンの数を取得する
extern	int			GetJoypadInputState(					int InputType ) ;														// ジョイパッドの入力状態を取得する
extern	int			GetJoypadAnalogInput(					int *XBuf, int *YBuf, int InputType ) ;									// ジョイパッドのアナログ的なスティック入力情報を得る
extern	int			GetJoypadAnalogInputRight(				int *XBuf, int *YBuf, int InputType ) ;									// ( 使用非推奨 )ジョイパッドのアナログ的なスティック入力情報を得る(右スティック用)
extern	int			GetJoypadDirectInputState(				int InputType, DINPUT_JOYSTATE *DInputState ) ;							// DirectInput から得られるジョイパッドの生のデ??を取得する( DX_INPUT_KEY や DX_INPUT_KEY_PAD1 など、キ???ドが絡む?イプを InputType に渡すとエラ?となり -1 を返す )
extern	int			CheckJoypadXInput(						int InputType ) ;														// 指定の入力デバイスが XInput に対応しているかどうかを取得する( 戻り値  TRUE:XInput対応の入力デバイス  FALSE:XInput非対応の入力デバイス   -1:エラ? )( DX_INPUT_KEY や DX_INPUT_KEY_PAD1 など、キ???ドが絡む?イプを InputType に渡すとエラ?となり -1 を返す )
extern	int			GetJoypadType(							int InputType ) ;														// ジョイパッドの?イプを取得する( 戻り値  -1:エラ?  0以上:ジョイパッド?イプ( DX_PADTYPE_XBOX_360 など ) )
extern	int			GetJoypadXInputState(					int InputType, XINPUT_STATE *XInputState ) ;							// XInput から得られる入力デバイス( Xbox360コントロ?ラ等 )の生のデ??を取得する( XInput非対応のパッドの場合はエラ?となり -1 を返す、DX_INPUT_KEY や DX_INPUT_KEY_PAD1 など、キ???ドが絡む?イプを InputType に渡すとエラ?となり -1 を返す )
extern	int			SetJoypadInputToKeyInput(				int InputType, int PadInput, int KeyInput1, int KeyInput2 DEFAULTPARAM( = -1 ) , int KeyInput3 DEFAULTPARAM( = -1 ) , int KeyInput4 DEFAULTPARAM( = -1 )  ) ; // ジョイパッドの入力に対応したキ???ドの入力を設定する( InputType:設定を変更するパッドの識別子( DX_INPUT_PAD1等 )??PadInput:設定を変更するパッド??ンの識別子( PAD_INPUT_1 等 )??KeyInput1:PadInput を押下したことにするキ?コ?ド( KEY_INPUT_A など )その１??KeyInput2:その２、-1で設定なし??KeyInput3:その３、-1で設定なし??KeyInput4:その４、-1で設定なし )
extern	int			SetJoypadDeadZone(						int InputType, double Zone ) ;											// ジョイパッドの無効??ンの設定を行う( InputType:設定を変更するパッドの識別子( DX_INPUT_PAD1等 )   Zone:新しい無効??ン( 0.0 ? 1.0 )、デフォルト値は 0.35 )
extern	double		GetJoypadDeadZone(						int InputType ) ;														// ジョイパッドの無効??ンの設定を取得する( InputType:設定を変更するパッドの識別子( DX_INPUT_PAD1等 )   戻り値:無効??ン( 0.0 ? 1.0 ) )
extern	int			SetJoypadDefaultDeadZoneAll(			double Zone ) ;															// ジョイパッドのデフォルトの無効??ンを設定する( Zone:新しい無効??ン( 0.0 ? 1.0 )、デフォルト値は 0.35 )
extern	double		GetJoypadDefaultDeadZoneAll(			void ) ;																// ジョイパッドのデフォルトの無効??ンを取得する( 戻り値:無効??ン( 0.0 ? 1.0 ) )
extern	int			StartJoypadVibration(					int InputType, int Power, int Time, int EffectIndex DEFAULTPARAM( = -1 ) ) ;	// ジョイパッドの振動を開始する
extern	int			StopJoypadVibration(					int InputType, int EffectIndex DEFAULTPARAM( = -1 ) ) ;					// ジョイパッドの振動を停?する
extern	int			GetJoypadPOVState(						int InputType, int POVNumber ) ;										// ジョイパッドのＰＯＶ入力の状態を得る( 戻り値?指定のPOVデ??の角度、単位は角度の１００?( 90度なら 9000 ) 中心位置にある場合は -1 が返る )
extern	int			ReSetupJoypad(							void ) ;																// ジョイパッドの再セットアップを行う( 新たに接続されたジョイパッドがあったら検出される )
extern	int			ReSetupInputSystem(						void ) ;																// 入力システ?の再セットアップを行う( 戻り値  0:正常終了  -1:エラ?発生 )

extern	int			SetUseJoypadVibrationFlag(				int Flag ) ;															// ジョイパッドの振動??を使用するかどうかを設定する( TRUE:使用する??FALSE:使用しない )

#endif // DX_NON_INPUT







#ifndef DX_NOTUSE_DRAWFUNCTION

// 画像処理系関数プロト?イプ宣言

// グラフィックハンドル作成関係関数
extern	int			MakeGraph(							int SizeX, int SizeY, int NotUse3DFlag DEFAULTPARAM( = FALSE ) ) ;			// 指定サイズのグラフィックハンドルを作成する
extern	int			MakeScreen(							int SizeX, int SizeY, int UseAlphaChannel DEFAULTPARAM( = FALSE ) ) ;		// SetDrawScreen で?画対象にできるグラフィックハンドルを作成する
extern	int			DerivationGraph(					int   SrcX, int   SrcY, int   Width, int   Height, int SrcGraphHandle ) ;	// 指定のグラフィックハンドルの指定部分だけを抜き出して新たなグラフィックハンドルを作成する
extern	int			DerivationGraphF(					float SrcX, float SrcY, float Width, float Height, int SrcGraphHandle ) ;	// 指定のグラフィックハンドルの指定部分だけを抜き出して新たなグラフィックハンドルを作成する( float版 )
extern	int			DeleteGraph(						int GrHandle ) ;															// グラフィックハンドルを削除する
extern	int			DeleteSharingGraph(					int GrHandle ) ;															// 指定のグラフィックハンドルと、同じグラフィックハンドルから派生しているグラフィックハンドル( DerivationGraph で派生したハンドル、LoadDivGraph 読み込んで作成された複数のハンドル )を一度に削除する
extern	int			GetGraphNum(						void ) ;																	// 有効なグラフィックハンドルの数を取得する
extern	int			FillGraph(							int GrHandle, int Red, int Green, int Blue, int Alpha DEFAULTPARAM( = 255 ) ) ;											// グラフィックハンドルを指定の色で塗りつぶす
extern	int			FillRectGraph(						int GrHandle, int x, int y, int Width, int Height, int Red, int Green, int Blue, int Alpha DEFAULTPARAM( = 255 ) ) ;	// グラフィックハンドルの指定の範囲を指定の色で塗りつぶす
extern	int			SetGraphLostFlag(					int GrHandle, int *LostFlag ) ;												// 指定のグラフィックハンドルが削除された際に 1 にする変数のアドレスを設定する
extern	int			InitGraph(							void ) ;																	// すべてのグラフィックハンドルを削除する
extern	int			ReloadFileGraphAll(					void ) ;																	// フ?イルから画像を読み込んだ全てのグラフィックハンドルについて、再度フ?イルから画像を読み込む

// シャドウ?ップハンドル関係関数
extern	int			MakeShadowMap(						int SizeX, int SizeY ) ;													// シャドウ?ップハンドルを作成する
extern	int			DeleteShadowMap(					int SmHandle ) ;															// シャドウ?ップハンドルを削除する
extern	int			SetShadowMapLightDirection(			int SmHandle, VECTOR Direction ) ;											// シャドウ?ップが想定するライトの方向を設定する
extern	int			ShadowMap_DrawSetup(				int SmHandle ) ;															// シャドウ?ップへの?画の?備を行う
extern	int			ShadowMap_DrawEnd(					void ) ;																	// シャドウ?ップへの?画を終了する
extern	int			SetUseShadowMap(					int SmSlotIndex, int SmHandle ) ;											// ?画で使用するシャドウ?ップを指定する、有効なスロットは０?２、SmHandle に -1 を渡すと指定のスロットのシャドウ?ップを解除
extern	int			SetShadowMapDrawArea(				int SmHandle, VECTOR MinPosition, VECTOR MaxPosition ) ;					// シャドウ?ップに?画する際の範囲を設定する( この関数で?画範囲を設定しない場合は視錐台を拡大した範囲が?画範囲となる )
extern	int			ResetShadowMapDrawArea(				int SmHandle ) ;															// SetShadowMapDrawArea の設定を解除する
extern	int			SetShadowMapAdjustDepth(			int SmHandle, float Depth ) ;												// シャドウ?ップを使用した?画時の補正?度を設定する
extern	int			GetShadowMapViewProjectionMatrix(	int SmHandle, MATRIX *MatrixBuffer ) ;										// シャドウ?ップ作成時や適用時に使用するビュ?行列と射影行列を乗算した行列を取得する
extern	int			TestDrawShadowMap(					int SmHandle, int x1, int y1, int x2, int y2 ) ;							// シャドウ?ップを画面にテスト?画する

// グラフィックハンドルへの画像?送関数
extern	int			BltBmpToGraph(						const COLORDATA *BmpColorData, HBITMAP RgbBmp, HBITMAP AlphaBmp,                                                                      int CopyPointX, int CopyPointY,                              int  GrHandle ) ;					// ＢＭＰの内容をグラフィックハンドルに?送
extern	int			BltBmpToDivGraph(					const COLORDATA *BmpColorData, HBITMAP RgbBmp, HBITMAP AlphaBmp,                                                                      int AllNum, int XNum, int YNum, int Width, int Height, const int *GrHandle, int ReverseFlag ) ;	// ＢＭＰの内容を分割作成したグラフィックハンドルたちに?送
extern	int			BltBmpOrGraphImageToGraph(			const COLORDATA *BmpColorData, HBITMAP RgbBmp, HBITMAP AlphaBmp, int BmpFlag, const BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int CopyPointX, int CopyPointY,                              int  GrHandle ) ;					// ＢＭＰ か BASEIMAGE をグラフィックハンドルに?送
extern	int			BltBmpOrGraphImageToGraph2(			const COLORDATA *BmpColorData, HBITMAP RgbBmp, HBITMAP AlphaBmp, int BmpFlag, const BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, const RECT *SrcRect, int DestX, int DestY,                   int  GrHandle ) ;					// ＢＭＰ か BASEIMAGE の指定の領域をグラフィックハンドルに?送
extern	int			BltBmpOrGraphImageToDivGraph(		const COLORDATA *BmpColorData, HBITMAP RgbBmp, HBITMAP AlphaBmp, int BmpFlag, const BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, int   Width, int   Height, const int *GrHandle, int ReverseFlag ) ;	// ＢＭＰ か BASEIMAGE を分割作成したグラフィックハンドルたちに?送
extern	int			BltBmpOrGraphImageToDivGraphF(		const COLORDATA *BmpColorData, HBITMAP RgbBmp, HBITMAP AlphaBmp, int BmpFlag, const BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, float Width, float Height, const int *GrHandle, int ReverseFlag ) ;	// ＢＭＰ か BASEIMAGE を分割作成したグラフィックハンドルたちに?送( float? )

// 画像からグラフィックハンドルを作成する関数
extern	int			LoadBmpToGraph(						const TCHAR *FileName,                        int TextureFlag, int ReverseFlag, int SurfaceMode DEFAULTPARAM( = DX_MOVIESURFACE_NORMAL ) ) ;											// 画像フ?イルからグラフィックハンドルを作成する
extern	int			LoadBmpToGraphWithStrLen(			const TCHAR *FileName, size_t FileNameLength, int TextureFlag, int ReverseFlag, int SurfaceMode DEFAULTPARAM( = DX_MOVIESURFACE_NORMAL ) ) ;											// 画像フ?イルからグラフィックハンドルを作成する
extern	int			LoadGraph(							const TCHAR *FileName,                        int NotUse3DFlag DEFAULTPARAM( = FALSE ) ) ;																				// 画像フ?イルからグラフィックハンドルを作成する
extern	int			LoadGraphWithStrLen(				const TCHAR *FileName, size_t FileNameLength, int NotUse3DFlag DEFAULTPARAM( = FALSE ) ) ;																				// 画像フ?イルからグラフィックハンドルを作成する
extern	int			LoadReverseGraph(					const TCHAR *FileName,                        int NotUse3DFlag DEFAULTPARAM( = FALSE ) ) ;																				// 画像フ?イルを反?したものでグラフィックハンドルを作成する
extern	int			LoadReverseGraphWithStrLen(			const TCHAR *FileName, size_t FileNameLength, int NotUse3DFlag DEFAULTPARAM( = FALSE ) ) ;																				// 画像フ?イルを反?したものでグラフィックハンドルを作成する
extern	int			LoadDivGraph(						const TCHAR *FileName,                        int AllNum, int XNum, int YNum, int   XSize, int   YSize, int *HandleArray, int NotUse3DFlag DEFAULTPARAM( = FALSE ) , int   XStride DEFAULTPARAM( = 0 ) , int   YStride DEFAULTPARAM( = 0 ) ) ;	// 画像フ?イルを分割してグラフィックハンドルを作成する
extern	int			LoadDivGraphWithStrLen(				const TCHAR *FileName, size_t FileNameLength, int AllNum, int XNum, int YNum, int   XSize, int   YSize, int *HandleArray, int NotUse3DFlag DEFAULTPARAM( = FALSE ) , int   XStride DEFAULTPARAM( = 0 ) , int   YStride DEFAULTPARAM( = 0 ) ) ;	// 画像フ?イルを分割してグラフィックハンドルを作成する
extern	int			LoadDivGraphF(						const TCHAR *FileName,                        int AllNum, int XNum, int YNum, float XSize, float YSize, int *HandleArray, int NotUse3DFlag DEFAULTPARAM( = FALSE ) , float XStride DEFAULTPARAM( = 0 ) , float YStride DEFAULTPARAM( = 0 ) ) ;	// 画像フ?イルを分割してグラフィックハンドルを作成する
extern	int			LoadDivGraphFWithStrLen(			const TCHAR *FileName, size_t FileNameLength, int AllNum, int XNum, int YNum, float XSize, float YSize, int *HandleArray, int NotUse3DFlag DEFAULTPARAM( = FALSE ) , float XStride DEFAULTPARAM( = 0 ) , float YStride DEFAULTPARAM( = 0 ) ) ;	// 画像フ?イルを分割してグラフィックハンドルを作成する
extern	int			LoadDivBmpToGraph(					const TCHAR *FileName,                        int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, int *HandleArray, int TextureFlag, int ReverseFlag , int   XStride DEFAULTPARAM( = 0 ) , int   YStride DEFAULTPARAM( = 0 ) ) ;			// 画像フ?イルを分割してグラフィックハンドルを作成する
extern	int			LoadDivBmpToGraphWithStrLen(		const TCHAR *FileName, size_t FileNameLength, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, int *HandleArray, int TextureFlag, int ReverseFlag , int   XStride DEFAULTPARAM( = 0 ) , int   YStride DEFAULTPARAM( = 0 ) ) ;			// 画像フ?イルを分割してグラフィックハンドルを作成する
extern	int			LoadDivBmpToGraphF(					const TCHAR *FileName,                        int AllNum, int XNum, int YNum, float SizeX, float SizeY, int *HandleArray, int TextureFlag, int ReverseFlag , float XStride DEFAULTPARAM( = 0 ) , float YStride DEFAULTPARAM( = 0 ) ) ;			// 画像フ?イルを分割してグラフィックハンドルを作成する
extern	int			LoadDivBmpToGraphFWithStrLen(		const TCHAR *FileName, size_t FileNameLength, int AllNum, int XNum, int YNum, float SizeX, float SizeY, int *HandleArray, int TextureFlag, int ReverseFlag , float XStride DEFAULTPARAM( = 0 ) , float YStride DEFAULTPARAM( = 0 ) ) ;			// 画像フ?イルを分割してグラフィックハンドルを作成する
extern	int			LoadReverseDivGraph(				const TCHAR *FileName,                        int AllNum, int XNum, int YNum, int   XSize, int   YSize, int *HandleArray, int NotUse3DFlag DEFAULTPARAM( = FALSE ) , int   XStride DEFAULTPARAM( = 0 ) , int   YStride DEFAULTPARAM( = 0 ) ) ;	// 画像フ?イルを反?したものを分割してグラフィックハンドルを作成する
extern	int			LoadReverseDivGraphWithStrLen(		const TCHAR *FileName, size_t FileNameLength, int AllNum, int XNum, int YNum, int   XSize, int   YSize, int *HandleArray, int NotUse3DFlag DEFAULTPARAM( = FALSE ) , int   XStride DEFAULTPARAM( = 0 ) , int   YStride DEFAULTPARAM( = 0 ) ) ;	// 画像フ?イルを反?したものを分割してグラフィックハンドルを作成する
extern	int			LoadReverseDivGraphF(				const TCHAR *FileName,                        int AllNum, int XNum, int YNum, float XSize, float YSize, int *HandleArray, int NotUse3DFlag DEFAULTPARAM( = FALSE ) , float XStride DEFAULTPARAM( = 0 ) , float YStride DEFAULTPARAM( = 0 ) ) ;	// 画像フ?イルを反?したものを分割してグラフィックハンドルを作成する
extern	int			LoadReverseDivGraphFWithStrLen(		const TCHAR *FileName, size_t FileNameLength, int AllNum, int XNum, int YNum, float XSize, float YSize, int *HandleArray, int NotUse3DFlag DEFAULTPARAM( = FALSE ) , float XStride DEFAULTPARAM( = 0 ) , float YStride DEFAULTPARAM( = 0 ) ) ;	// 画像フ?イルを反?したものを分割してグラフィックハンドルを作成する
extern	int			LoadBlendGraph(						const TCHAR *FileName ) ;																																				// 画像フ?イルからブレンド用グラフィックハンドルを作成する
extern	int			LoadBlendGraphWithStrLen(			const TCHAR *FileName, size_t FileNameLength ) ;																														// 画像フ?イルからブレンド用グラフィックハンドルを作成する

extern	int			CreateGraphFromMem(					const void *RGBFileImage, int RGBFileImageSize,               const void *AlphaFileImage DEFAULTPARAM( = NULL ) , int AlphaFileImageSize DEFAULTPARAM( = 0 ) ,                  int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																			// メモリ上の画像イメ?ジからグラフィックハンドルを作成する
extern	int			ReCreateGraphFromMem(				const void *RGBFileImage, int RGBFileImageSize, int GrHandle, const void *AlphaFileImage DEFAULTPARAM( = NULL ) , int AlphaFileImageSize DEFAULTPARAM( = 0 ) ,                  int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																			// メモリ上の画像イメ?ジから既存のグラフィックハンドルにデ??を?送する
extern	int			CreateDivGraphFromMem(				const void *RGBFileImage, int RGBFileImageSize, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY,       int *HandleArray,               int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) , const void *AlphaFileImage DEFAULTPARAM( = NULL ) , int AlphaFileImageSize DEFAULTPARAM( = 0 ) ) ;			// メモリ上の画像イメ?ジから分割グラフィックハンドルを作成する
extern	int			CreateDivGraphFFromMem(				const void *RGBFileImage, int RGBFileImageSize, int AllNum, int XNum, int YNum, float SizeX, float SizeY,       int *HandleArray,               int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) , const void *AlphaFileImage DEFAULTPARAM( = NULL ) , int AlphaFileImageSize DEFAULTPARAM( = 0 ) ) ;			// メモリ上の画像イメ?ジから分割グラフィックハンドルを作成する( float版 )
extern	int			ReCreateDivGraphFromMem(			const void *RGBFileImage, int RGBFileImageSize, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, const int *HandleArray,               int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) , const void *AlphaFileImage DEFAULTPARAM( = NULL ) , int AlphaFileImageSize DEFAULTPARAM( = 0 ) ) ;			// メモリ上の画像イメ?ジから既存の分割グラフィックハンドルにデ??を?送する
extern	int			ReCreateDivGraphFFromMem(			const void *RGBFileImage, int RGBFileImageSize, int AllNum, int XNum, int YNum, float SizeX, float SizeY, const int *HandleArray,               int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) , const void *AlphaFileImage DEFAULTPARAM( = NULL ) , int AlphaFileImageSize DEFAULTPARAM( = 0 ) ) ;			// メモリ上の画像イメ?ジから既存の分割グラフィックハンドルにデ??を?送する( float版 )
extern	int			CreateGraphFromBmp(					const BITMAPINFO *RGBBmpInfo, const void *RGBBmpImage,               const BITMAPINFO *AlphaBmpInfo DEFAULTPARAM( = NULL ) , const void *AlphaBmpImage DEFAULTPARAM( = NULL ) , int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																			// ビット?ップデ??からグラフィックハンドルを作成する
extern	int			ReCreateGraphFromBmp(				const BITMAPINFO *RGBBmpInfo, const void *RGBBmpImage, int GrHandle, const BITMAPINFO *AlphaBmpInfo DEFAULTPARAM( = NULL ) , const void *AlphaBmpImage DEFAULTPARAM( = NULL ) , int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																			// ビット?ップデ??から既存のグラフィックハンドルにデ??を?送する
extern	int			CreateDivGraphFromBmp(				const BITMAPINFO *RGBBmpInfo, const void *RGBBmpImage, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY,       int *HandleArray,        int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) , const BITMAPINFO *AlphaBmpInfo DEFAULTPARAM( = NULL ) , const void *AlphaBmpImage DEFAULTPARAM( = NULL ) ) ;	// ビット?ップデ??から分割グラフィックハンドルを作成する
extern	int			CreateDivGraphFFromBmp(				const BITMAPINFO *RGBBmpInfo, const void *RGBBmpImage, int AllNum, int XNum, int YNum, float SizeX, float SizeY,       int *HandleArray,        int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) , const BITMAPINFO *AlphaBmpInfo DEFAULTPARAM( = NULL ) , const void *AlphaBmpImage DEFAULTPARAM( = NULL ) ) ;	// ビット?ップデ??から分割グラフィックハンドルを作成する( float版 )
extern	int			ReCreateDivGraphFromBmp(			const BITMAPINFO *RGBBmpInfo, const void *RGBBmpImage, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, const int *HandleArray,        int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) , const BITMAPINFO *AlphaBmpInfo DEFAULTPARAM( = NULL ) , const void *AlphaBmpImage DEFAULTPARAM( = NULL ) ) ;	// ビット?ップデ??から既存の分割グラフィックハンドルにデ??を?送する
extern	int			ReCreateDivGraphFFromBmp(			const BITMAPINFO *RGBBmpInfo, const void *RGBBmpImage, int AllNum, int XNum, int YNum, float SizeX, float SizeY, const int *HandleArray,        int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) , const BITMAPINFO *AlphaBmpInfo DEFAULTPARAM( = NULL ) , const void *AlphaBmpImage DEFAULTPARAM( = NULL ) ) ;	// ビット?ップデ??から既存の分割グラフィックハンドルにデ??を?送する( float版 )
extern	int			CreateDXGraph(						const BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage,                                                                                 int TextureFlag ) ;																									// 基?イメ?ジデ??からサイズを割り出し、それに合ったグラフィックハンドルを作成する
extern	int			CreateGraphFromGraphImage(			const BASEIMAGE *RgbBaseImage,                                                                                                                  int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??からグラフィックハンドルを作成する
extern	int			ReCreateGraphFromGraphImage(		const BASEIMAGE *RgbBaseImage,                                  int GrHandle,                                                                   int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から既存のグラフィックハンドルにデ??を?送する
extern	int			CreateDivGraphFromGraphImage(		      BASEIMAGE *RgbBaseImage,                                  int AllNum, int XNum, int YNum, int   SizeX, int   SizeY,       int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から分割グラフィックハンドルを作成する
extern	int			CreateDivGraphFFromGraphImage(		      BASEIMAGE *RgbBaseImage,                                  int AllNum, int XNum, int YNum, float SizeX, float SizeY,       int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から分割グラフィックハンドルを作成する( float版 )
extern	int			ReCreateDivGraphFromGraphImage(		      BASEIMAGE *RgbBaseImage,                                  int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, const int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から既存の分割グラフィックハンドルにデ??を?送する
extern	int			ReCreateDivGraphFFromGraphImage(	      BASEIMAGE *RgbBaseImage,                                  int AllNum, int XNum, int YNum, float SizeX, float SizeY, const int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から既存の分割グラフィックハンドルにデ??を?送する( float版 )
#ifndef DX_COMPILE_TYPE_C_LANGUAGE
extern	int			CreateGraphFromGraphImage(			const BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage,                                                                                 int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??からグラフィックハンドルを作成する
extern	int			ReCreateGraphFromGraphImage(		const BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int GrHandle,                                                                   int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から既存のグラフィックハンドルにデ??を?送する
extern	int			CreateDivGraphFromGraphImage(		      BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY,       int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から分割グラフィックハンドルを作成する
extern	int			CreateDivGraphFFromGraphImage(		      BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, float SizeX, float SizeY,       int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から分割グラフィックハンドルを作成する( float版 )
extern	int			ReCreateDivGraphFromGraphImage(		      BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, const int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から既存の分割グラフィックハンドルにデ??を?送する
extern	int			ReCreateDivGraphFFromGraphImage(	      BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, float SizeX, float SizeY, const int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から既存の分割グラフィックハンドルにデ??を?送する( float版 )
#endif // DX_COMPILE_TYPE_C_LANGUAGE
extern	int			CreateGraphFromGraphImage2(			const BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage,                                                                                 int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??からグラフィックハンドルを作成する
extern	int			ReCreateGraphFromGraphImage2(		const BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int GrHandle,                                                                   int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から既存のグラフィックハンドルにデ??を?送する
extern	int			CreateDivGraphFromGraphImage2(		      BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY,       int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から分割グラフィックハンドルを作成する
extern	int			CreateDivGraphFFromGraphImage2(		      BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, float SizeX, float SizeY,       int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から分割グラフィックハンドルを作成する( float版 )
extern	int			ReCreateDivGraphFromGraphImage2(	      BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, const int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から既存の分割グラフィックハンドルにデ??を?送する
extern	int			ReCreateDivGraphFFromGraphImage2(	      BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, float SizeX, float SizeY, const int *HandleArray, int TextureFlag DEFAULTPARAM( = TRUE ) , int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																// 基?イメ?ジデ??から既存の分割グラフィックハンドルにデ??を?送する( float版 )
extern	int			CreateGraph(						int Width, int Height, int Pitch, const void *RGBImage, const void *AlphaImage DEFAULTPARAM( = NULL ) , int GrHandle DEFAULTPARAM( = -1 ) ) ;																																		// メモリ上のビット?ップイメ?ジからグラフィックハンドルを作成する
extern	int			CreateDivGraph(						int Width, int Height, int Pitch, const void *RGBImage, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, int *HandleArray, const void *AlphaImage DEFAULTPARAM( = NULL ) ) ;																								// メモリ上のビット?ップイメ?ジから分割グラフィックハンドルを作成する
extern	int			CreateDivGraphF(					int Width, int Height, int Pitch, const void *RGBImage, int AllNum, int XNum, int YNum, float SizeX, float SizeY, int *HandleArray, const void *AlphaImage DEFAULTPARAM( = NULL ) ) ;																								// メモリ上のビット?ップイメ?ジから分割グラフィックハンドルを作成する( float版 )
extern	int			ReCreateGraph(						int Width, int Height, int Pitch, const void *RGBImage, int GrHandle, const void *AlphaImage DEFAULTPARAM( = NULL ) ) ;																																								// メモリ上のビット?ップイメ?ジからグラフィックハンドルを再作成する
#ifndef DX_NON_SOFTIMAGE
extern	int			CreateBlendGraphFromSoftImage(		int SIHandle ) ;																														// ?フトウエアで扱うイメ?ジからブレンド用画像グラフィックハンドルを作成する( -1:エラ?  -1以外:ブレンド用グラフィックハンドル )
extern	int			CreateGraphFromSoftImage(			int SIHandle ) ;																														// ?フトウエアで扱うイメ?ジからグラフィックハンドルを作成する( -1:エラ?  -1以外:グラフィックハンドル )
extern	int			CreateGraphFromRectSoftImage(		int SIHandle, int x, int y, int SizeX, int SizeY ) ;																					// ?フトウエアで扱うイメ?ジの指定の領域を使ってグラフィックハンドルを作成する( -1:エラ?  -1以外:グラフィックハンドル )
extern	int			ReCreateGraphFromSoftImage(			int SIHandle, int GrHandle ) ;																											// ?フトウエアで扱うイメ?ジから既存のグラフィックハンドルに画像デ??を?送する
extern	int			ReCreateGraphFromRectSoftImage(		int SIHandle, int x, int y, int SizeX, int SizeY, int GrHandle ) ;																		// ?フトウエアで扱うイメ?ジから既存のグラフィックハンドルに画像デ??を?送する
extern	int			CreateDivGraphFromSoftImage(		int SIHandle, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, int *HandleArray ) ;											// ?フトウエアで扱うイメ?ジから分割グラフィックハンドルを作成する
extern	int			CreateDivGraphFFromSoftImage(		int SIHandle, int AllNum, int XNum, int YNum, float SizeX, float SizeY, int *HandleArray ) ;											// ?フトウエアで扱うイメ?ジから分割グラフィックハンドルを作成する( float版 )
extern	int			ReCreateDivGraphFromSoftImage(		int SIHandle, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, const int *HandleArray ) ;										// ?フトウエアで扱うイメ?ジから既存の分割グラフィックハンドルにデ??を?送する
extern	int			ReCreateDivGraphFFromSoftImage(	    int SIHandle, int AllNum, int XNum, int YNum, float SizeX, float SizeY, const int *HandleArray ) ;										// ?フトウエアで扱うイメ?ジから既存の分割グラフィックハンドルにデ??を?送する( float版 )
#endif // DX_NON_SOFTIMAGE
extern	int			CreateGraphFromBaseImage(			const BASEIMAGE *BaseImage ) ;																											// 基?イメ?ジデ??からグラフィックハンドルを作成する
extern	int			CreateGraphFromRectBaseImage(		const BASEIMAGE *BaseImage, int x, int y, int SizeX, int SizeY ) ;																		// 基?イメ?ジデ??の指定の領域を使ってグラフィックハンドルを作成する
extern	int			ReCreateGraphFromBaseImage(			const BASEIMAGE *BaseImage,                                     int GrHandle ) ;														// 基?イメ?ジデ??から既存のグラフィックハンドルに画像デ??を?送する
extern	int			ReCreateGraphFromRectBaseImage(		const BASEIMAGE *BaseImage, int x, int y, int SizeX, int SizeY, int GrHandle ) ;														// 基?イメ?ジデ??の指定の領域を使って既存のグラフィックハンドルに画像デ??を?送する
extern	int			CreateDivGraphFromBaseImage(		      BASEIMAGE *BaseImage, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, int *HandleArray ) ;								// 基?イメ?ジデ??から分割グラフィックハンドルを作成する
extern	int			CreateDivGraphFFromBaseImage(		      BASEIMAGE *BaseImage, int AllNum, int XNum, int YNum, float SizeX, float SizeY, int *HandleArray ) ;								// 基?イメ?ジデ??から分割グラフィックハンドルを作成する( float版 )
extern	int			ReCreateDivGraphFromBaseImage(		      BASEIMAGE *BaseImage, int AllNum, int XNum, int YNum, int   SizeX, int   SizeY, const int *HandleArray ) ;						// 基?イメ?ジデ??から既存の分割グラフィックハンドルにデ??を?送する
extern	int			ReCreateDivGraphFFromBaseImage(	          BASEIMAGE *BaseImage, int AllNum, int XNum, int YNum, float SizeX, float SizeY, const int *HandleArray ) ;						// 基?イメ?ジデ??から既存の分割グラフィックハンドルにデ??を?送する( float版 )
extern	int			ReloadGraph(						const TCHAR *FileName,                        int GrHandle, int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																		// 画像フ?イルからグラフィックハンドルへ画像デ??を?送する
extern	int			ReloadGraphWithStrLen(				const TCHAR *FileName, size_t FileNameLength, int GrHandle, int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;																		// 画像フ?イルからグラフィックハンドルへ画像デ??を?送する
extern	int			ReloadDivGraph(						const TCHAR *FileName,                        int AllNum, int XNum, int YNum, int   XSize, int   YSize, const int *HandleArray, int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像フ?イルからグラフィックハンドルたちへ画像デ??を分割?送する
extern	int			ReloadDivGraphWithStrLen(			const TCHAR *FileName, size_t FileNameLength, int AllNum, int XNum, int YNum, int   XSize, int   YSize, const int *HandleArray, int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像フ?イルからグラフィックハンドルたちへ画像デ??を分割?送する
extern	int			ReloadDivGraphF(					const TCHAR *FileName,                        int AllNum, int XNum, int YNum, float XSize, float YSize, const int *HandleArray, int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像フ?イルからグラフィックハンドルたちへ画像デ??を分割?送する( float版 )
extern	int			ReloadDivGraphFWithStrLen(			const TCHAR *FileName, size_t FileNameLength, int AllNum, int XNum, int YNum, float XSize, float YSize, const int *HandleArray, int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像フ?イルからグラフィックハンドルたちへ画像デ??を分割?送する( float版 )
extern	int			ReloadReverseGraph(					const TCHAR *FileName,                        int GrHandle ) ;																			// ReloadGraph の画像反?処理追加版
extern	int			ReloadReverseGraphWithStrLen(		const TCHAR *FileName, size_t FileNameLength, int GrHandle ) ;																			// ReloadGraph の画像反?処理追加版
extern	int			ReloadReverseDivGraph(				const TCHAR *FileName,                        int AllNum, int XNum, int YNum, int   XSize, int   YSize, const int *HandleArray ) ;		// ReloadDivGraph の画像反?処理追加版
extern	int			ReloadReverseDivGraphWithStrLen(	const TCHAR *FileName, size_t FileNameLength, int AllNum, int XNum, int YNum, int   XSize, int   YSize, const int *HandleArray ) ;		// ReloadDivGraph の画像反?処理追加版
extern	int			ReloadReverseDivGraphF(				const TCHAR *FileName,                        int AllNum, int XNum, int YNum, float XSize, float YSize, const int *HandleArray ) ;		// ReloadDivGraph の画像反?処理追加版( float版 )
extern	int			ReloadReverseDivGraphFWithStrLen(	const TCHAR *FileName, size_t FileNameLength, int AllNum, int XNum, int YNum, float XSize, float YSize, const int *HandleArray ) ;		// ReloadDivGraph の画像反?処理追加版( float版 )

// グラフィックハンドル作成時設定係関数
extern	int			SetGraphColorBitDepth(						int ColorBitDepth ) ;							// SetCreateGraphColorBitDepth の旧名称
extern 	int			GetGraphColorBitDepth(						void ) ;										// GetCreateGraphColorBitDepth の旧名称
extern	int			SetCreateGraphColorBitDepth(				int BitDepth ) ;								// 作成するグラフィックハンドルの色?度を設定する
extern	int			GetCreateGraphColorBitDepth(				void ) ;										// 作成するグラフィックハンドルの色?度を取得する
extern	int			SetCreateGraphChannelBitDepth(				int BitDepth ) ;								// 作成するグラフィックハンドルの１?ャンネル辺りのビット?度を設定する
extern	int			GetCreateGraphChannelBitDepth(				void ) ;										// 作成するグラフィックハンドルの１?ャンネル辺りのビット?度を取得する
extern	int			SetCreateGraphInitGraphDelete(				int Flag ) ;									// 作成するグラフィックハンドルを InitGraph() で削除されるかを設定する( Flag  TRUE:InitGraphで削除される(デフォルト)  FALSE:InitGraphで削除されない )
extern	int			GetCreateGraphInitGraphDelete(				void ) ;										// 作成するグラフィックハンドルを InitGraph() で削除されるかを取得する( Flag  TRUE:InitGraphで削除される(デフォルト)  FALSE:InitGraphで削除されない )
extern	int			SetCreateGraphHandle(						int GrHandle ) ;								// 作成するグラフィックハンドルのハンドル値を設定する、０以下の値を渡すと設定解除( 存在しないグラフィックハンドルの値の場合のみ有効 )
extern	int			GetCreateGraphHandle(						void ) ;										// 作成するグラフィックハンドルのハンドル値を取得する
extern	int			SetCreateDivGraphHandle(					const int *HandleArray, int HandleNum ) ;		// 作成するグラフィックハンドルのハンドル値を設定する、LoadDivGraph 等の分割画像読み込み用、HandleArray に NULL を渡すと設定解除( 存在しないグラフィックハンドルの値の場合のみ有効 )
extern	int			GetCreateDivGraphHandle(					int *HandleArray ) ;							// 作成するグラフィックハンドルのハンドル値を取得する、LoadDivGraph 等の分割画像読み込み用、戻り値は SetCreateDivGraphHandle の引数 HandleNum に渡した値、HandleArray は NULL でも可
extern	int			SetDrawValidGraphCreateFlag(				int Flag ) ;									// SetDrawScreen に引数として渡せる( ?画対象として使用できる )グラフィックハンドルを作成するかどうかを設定する( TRUE:?画可?グラフィックハンドルを作成する  FLASE:通常のグラフィックハンドルを作成する( デフォルト ) )
extern	int			GetDrawValidGraphCreateFlag(				void ) ;										// SetDrawScreen に引数として渡せる( ?画対象として使用できる )グラフィックハンドルを作成するかどうかを設定を取得する
extern	int			SetDrawValidFlagOf3DGraph(					int Flag ) ;									// SetDrawValidGraphCreateFlag の旧名称
extern	int			SetLeftUpColorIsTransColorFlag(				int Flag ) ;									// 画像フ?イルからグラフィックハンドルを作成する際に画像左上の色を透過色として扱うかどうかを設定する( TRUE:透過色として扱う ?FALSE:透過色として扱わない( デフォルト ) )
extern	int			SetUsePaletteGraphFlag(						int Flag ) ;									// 読み込む画像がパレット画像の場合、パレット画像として使用できる場合はパレット画像として使用するかどうかを設定する( TRUE:パレット画像として使用できる場合はパレット画像として使用する( デフォルト )  FALSE:パレット画像として使用できる場合もパレット画像としては使用しない( 通常?イプの画像に変換して使用する ) )
extern	int			SetUseBlendGraphCreateFlag(					int Flag ) ;									// ブレンド処理用画像を作成するかどうか( 要は画像の赤成分をα成分として扱うかどうか )の設定を行う( TRUE:ブレンド画像として読み込む  FALSE:通常画像として読み込む( デフォルト ) )
extern	int			GetUseBlendGraphCreateFlag(					void ) ;										// ブレンド処理用画像を作成するかどうか( 要は画像の赤成分をα成分として扱うかどうか )の設定を取得する
extern	int			SetUseAlphaTestGraphCreateFlag(				int Flag ) ;									// アルフ?テストを使用するグラフィックハンドルを作成するかどうかを設定する( TRUE:アルフ?テストを使用する( デフォルト )  FALSE:アルフ?テストを使用しない )
extern	int			GetUseAlphaTestGraphCreateFlag(				void ) ;										// アルフ?テストを使用するグラフィックハンドルを作成するかどうかを取得する
extern	int			SetUseAlphaTestFlag(						int Flag ) ;									// SetUseAlphaTestGraphCreateFlag の旧名称
extern	int			GetUseAlphaTestFlag(						void ) ;										// GetUseAlphaTestGraphCreateFlag の旧名称
extern	int			SetCubeMapTextureCreateFlag(				int Flag ) ;									// キュ?ブ?ップテクス?ャを作成するかどうかのフラグを設定する
extern	int			GetCubeMapTextureCreateFlag(				void ) ;										// キュ?ブ?ップテクス?ャを作成するかどうかのフラグを取得する
extern	int			SetUseNoBlendModeParam(						int Flag ) ;									// SetDrawBlendMode 関数の第一引数に DX_BLENDMODE_NOBLEND を代入した際に、デフォルトでは第二引数は内部で２５５を指定したことになるが、その自動２５５化をしないかどうかを設定する( TRUE:しない(第二引数の値が使用される)   FALSE:する(第二引数の値は無視されて 255 が常に使用される)(デフォルト) )α?ャンネル付き画像に対して?画を行う場合のみ意味がある関数
extern	int			SetDrawValidAlphaChannelGraphCreateFlag(	int Flag ) ;									// SetDrawScreen の引数として渡せる( ?画対象として使用できる )α?ャンネル付きグラフィックハンドルを作成するかどうかを設定する( SetDrawValidGraphCreateFlag 関数で?画対象として使用できるグラフィックハンドルを作成するように設定されていないと効果ありません )( TRUE:α?ャンネル付き   FALSE:α?ャンネルなし( デフォルト ) )
extern	int			GetDrawValidAlphaChannelGraphCreateFlag(	void ) ;										// SetDrawScreen の引数として渡せる( ?画対象として使用できる )α?ャンネル付きグラフィックハンドルを作成するかどうかを取得する
extern	int			SetDrawValidFloatTypeGraphCreateFlag(		int Flag ) ;									// SetDrawScreen の引数として渡せる( ?画対象として使用できる )ピクセルフォ??ットが浮動小数??のグラフィックハンドルを作成するかどうかを設定する( SetDrawValidGraphCreateFlag 関数で?画対象として使用できるグラフィックハンドルを作成するように設定されていないと効果ありません )、グラフィックスデバイスが浮動小数??のピクセルフォ??ットに対応していない場合はグラフィックハンドルの作成に失敗する( TRUE:浮動小数????FALSE:整数?( デフォルト ) )
extern	int			GetDrawValidFloatTypeGraphCreateFlag(		void ) ;										// SetDrawScreen の引数として渡せる( ?画対象として使用できる )ピクセルフォ??ットが浮動小数??のグラフィックハンドルを作成するかどうかを取得する
extern	int			SetDrawValidGraphCreateZBufferFlag(			int Flag ) ;									// SetDrawScreen の引数として渡せる( ?画対象として使用できる )グラフィックハンドルを作成する際に専用のＺバッフ?も作成するかどうかを設定する( TRUE:専用のＺバッフ?を作成する( デフォルト )  FALSE:専用のＺバッフ?は作成しない )
extern	int			GetDrawValidGraphCreateZBufferFlag(			void ) ;										// SetDrawScreen の引数として渡せる( ?画対象として使用できる )グラフィックハンドルを作成する際に専用のＺバッフ?も作成するかどうかを取得する
extern	int			SetCreateDrawValidGraphZBufferBitDepth(		int BitDepth ) ;								// SetDrawScreen の引数として渡せる( ?画対象として使用できる )グラフィックハンドルに適用するＺバッフ?のビット?度を設定する( BitDepth:ビット?度( 指定可?な値は 16, 24, 32 の何れか( SetDrawValidGraphCreateFlag 関数で?画対象として使用できるグラフィックハンドルを作成するように設定されていないと効果ありません )
extern	int			GetCreateDrawValidGraphZBufferBitDepth(		void ) ;										// SetDrawScreen の引数として渡せる( ?画対象として使用できる )グラフィックハンドルに適用するＺバッフ?のビット?度を取得する
extern	int			SetCreateDrawValidGraphMipLevels(			int MipLevels ) ;								// SetDrawScreen の引数として渡せる( ?画対象として使用できる )グラフィックハンドルに適用するMipMapのレベルを設定する
extern	int			GetCreateDrawValidGraphMipLevels(			void ) ;										// SetDrawScreen の引数として渡せる( ?画対象として使用できる )グラフィックハンドルに適用するMipMapのレベルを取得する
extern	int			SetCreateDrawValidGraphChannelNum(			int ChannelNum ) ;								// SetDrawScreen の引数として渡せる( ?画対象として使用できる )グラフィックハンドルに適用する色の?ャンネル数を設定する( ChannelNum:?ャンネル数( 指定可?な値は 1, 2, 4 の何れか( SetDrawValidGraphCreateFlag 関数で?画対象として使用できるグラフィックハンドルを作成するように設定されていないと効果ありません )
extern	int			GetCreateDrawValidGraphChannelNum(			void ) ;										// SetDrawScreen の引数として渡せる( ?画対象として使用できる )グラフィックハンドルに適用する色の?ャンネル数を取得する
extern	int			SetCreateDrawValidGraphMultiSample(			int Samples, int Quality ) ;					// SetDrawScreen の引数として渡せる( ?画対象として使用できる )グラフィックハンドルに適用する?ル?サンプリング( アン?エイリアシング )設定を行う( Samples:?ル?サンプル処理に使用するドット数( 多いほど重くなります )  Quality:?ル?サンプル処理の品質 )
extern	int			SetDrawValidMultiSample(					int Samples, int Quality ) ;					// SetCreateDrawValidGraphMultiSample の旧名称
extern	int			GetMultiSampleQuality(						int Samples ) ;									// 指定の?ル?サンプル数で使用できる最大クオリティ値を取得する( 戻り値が?イナスの場合は引数のサンプル数が使用できないことを示します )
extern	int			SetUseTransColor(							int Flag ) ;									// 透過色??を使用するかどうかを設定する( TRUE:使用する( デフォルト )  FALSE:使用しない )
extern	int			SetUseTransColorGraphCreateFlag(			int Flag ) ;									// 透過色??を使用することを前提とした画像デ??の読み込み処理を行うかどうかを設定する( TRUE にすると SetDrawMode( DX_DRAWMODE_BILINEAR ); をした状態で DrawGraphF 等の浮動小数??座標を受け取る関数で小数?以下の値を指定した場合に発生する?画結果の不自然を緩和する効果があります ( デフォルトは FALSE ) )
extern 	int			SetUseGraphAlphaChannel(					int Flag ) ;									// SetUseAlphaChannelGraphCreateFlag の旧名称
extern 	int			GetUseGraphAlphaChannel(					void ) ;										// GetUseAlphaChannelGraphCreateFlag の旧名称
extern 	int			SetUseAlphaChannelGraphCreateFlag(			int Flag ) ;									// α?ャンネル付きグラフィックハンドルを作成するかどうかを設定する( TRUE:α?ャンネル付き   FALSE:α?ャンネル無し )
extern 	int			GetUseAlphaChannelGraphCreateFlag(			void ) ;										// α?ャンネル付きグラフィックハンドルを作成するかどうかを取得する( TRUE:α?ャンネル付き   FALSE:α?ャンネル無し )
extern	int			SetUseNotManageTextureFlag(					int Flag ) ;									// Direct3D の管理テクス?ャ??を使用するグラフィックハンドルを作成するかどうかを設定する( TRUE:管理??を使用する( デフォルト )  FALSE:管理??を使用しない )、管理??を使用するとグラフィックスデバイスのＶＲ?Ｍ容量以上の画像を扱うことができる代わりにシステ?メモリの使用量が増えます
extern	int			GetUseNotManageTextureFlag(					void ) ;										// Direct3D の管理テクス?ャ??を使用するグラフィックハンドルを作成するかどうかを取得する
extern	int			SetUsePlatformTextureFormat(				int PlatformTextureFormat ) ;					// 作成するグラフィックハンドルで使用する環境依存のテクス?ャフォ??ットを指定する( Direct3D9環境なら DX_TEXTUREFORMAT_DIRECT3D9_R8G8B8 など、0 を渡すと解除 )
extern	int			GetUsePlatformTextureFormat(				void ) ;										// 作成するグラフィックハンドルで使用する環境依存のテクス?ャフォ??ットを取得する
extern	int			SetTransColor(								int Red, int Green, int Blue ) ;				// 作成するグラフィックハンドルに適用する透過色を設定する( Red,Green,Blue:透過色を光の３原色で?したもの( 各色０?２５５ ) )
extern	int			GetTransColor(								int *Red, int *Green, int *Blue ) ;				// 作成するグラフィックハンドルに適用する透過色を取得する
extern	int			SetUseDivGraphFlag(							int Flag ) ;									// ２のｎ乗ではないサイズの画像を複数のテクス?ャを使用してＶＲ?Ｍの無駄を省くかどうかを設定する( TRUE:複数のテクス?ャを使用する   FALSE:なるべく一枚のテクス?ャで済ます( デフォルト ) )、複数のテクス?ャを使用する場合はＶＲ?Ｍ容量の節約ができる代わりに速度の低下やバイリニアフィル?リング?画時にテクス?ャとテクス?ャの境目が良く見るとわかる等の弊害があります
extern	int			SetUseAlphaImageLoadFlag(					int Flag ) ;									// LoadGraph などの際にフ?イル名の末尾に _a が付いたアルフ??ャンネル用の画像フ?イルを追加で読み込む処理を行うかどうかを設定する( TRUE:行う( デフォルト )  FALSE:行わない )
extern	int			SetUseMaxTextureSize(						int Size ) ;									// 使用するテクス?ャ?の最大サイズを設定する( デフォルトではグラフィックスデバイスが対応している最大テクス?ャ?サイズ、引数に 0 を渡すとデフォルト設定になります )
extern	int			SetUseGraphBaseDataBackup(					int Flag ) ;									// グラフィックハンドルを作成する際に使用した画像デ??のバックアップをして Direct3DDevice のデバイスロスト時に使用するかどうかを設定する( TRUE:バックアップをする( デフォルト )  FALSE:バックアップをしない )、バックアップをしないとメモリの節約になりますが、復帰に?かる時間が長くなり、メモリ上のフ?イルイメ?ジからグラフィックハンドルを作成した場合は自動復帰ができないなどの弊害があります
extern	int			GetUseGraphBaseDataBackup(					void ) ;										// グラフィックハンドルを作成する際に使用した画像デ??のバックアップをして Direct3DDevice のデバイスロスト時に使用するかどうかを取得する
extern	int			SetUseSystemMemGraphCreateFlag(				int Flag ) ;									// ( 現在効果なし )グラフィックハンドルが持つ画像デ??をシステ?メモリ上に作成するかどうかを設定する( TRUE:システ?メモリ上に作成  FALSE:ＶＲ?Ｍ上に作成( デフォルト ) )
extern	int			GetUseSystemMemGraphCreateFlag(				void ) ;										// ( 現在効果なし )グラフィックハンドルが持つ画像デ??をシステ?メモリ上に作成するかどうかを取得する
extern	int			SetUseLoadDivGraphSizeCheckFlag(			int Flag ) ;									// LoadDivGraph 系の分割画像読み込み関数でサイズの?ェックを行うかどうかを設定する( Flag:TRUE( ?ェックを行う(デフォルト) )  FALSE:?ェックを行わない )
extern	int			GetUseLoadDivGraphSizeCheckFlag(			void ) ;										// LoadDivGraph 系の分割画像読み込み関数でサイズの?ェックを行うかどうかの設定を取得する

// 画像情報関係関数
extern	const unsigned int* GetFullColorImage(				int GrHandle ) ;																// 指定のグラフィックハンドルの?ＲＧＢ８イメ?ジを取得する( 現在動画フ?イルをグラフィックハンドルで読み込んだ場合のみ使用可? )

extern	int			GraphLock(						int GrHandle, int *PitchBuf, void **DataPointBuf, COLORDATA **ColorDataPP DEFAULTPARAM( = NULL ), int WriteOnly DEFAULTPARAM( = FALSE ) ) ;	// グラフィックメモリ領域のロック
extern	int			GraphUnLock(					int GrHandle ) ;																							// グラフィックメモリ領域のロック解除

extern	int			SetUseGraphZBuffer(				int GrHandle, int UseFlag, int BitDepth DEFAULTPARAM( = -1 ) ) ;						// グラフィックハンドル専用のＺバッフ?を持つかどうかを設定する( GrHandle:対象となるグラフィックハンドル( ?画対象として使用可?なグラフィックハンドルのみ有効 )  UseFlag:専用のＺバッフ?を持つかどうか( TRUE:持つ( デフォルト )  FALSE:持たない )  BitDepth:ビット?度( 16 or 24 or 32 ) )
extern	int			CopyGraphZBufferImage(			int DestGrHandle, int SrcGrHandle ) ;													// グラフィックハンドルのＺバッフ?の状態を別のグラフィックハンドルのＺバッフ?にコピ?する( DestGrHandle も SrcGrHandle もＺバッフ?を持っている?画対象にできるグラフィックハンドルで、サイズが同じであり、且つ?ル?サンプリング( アン?エイリアス )設定が無いことが条件 )

extern	int			SetDeviceLostDeleteGraphFlag(	int GrHandle, int DeleteFlag ) ;														// グラフィックスデバイスのデバイスロスト発生時に指定のグラフィックハンドルを削除するかどうかを設定する( TRUE:デバイスロスト時に削除する  FALSE:デバイスロストが発生しても削除しない )

extern	int			GetGraphSize(					int GrHandle, int   *SizeXBuf, int   *SizeYBuf ) ;										// グラフィックハンドルが持つ画像のサイズを得る
extern	int			GetGraphSizeF(					int GrHandle, float *SizeXBuf, float *SizeYBuf ) ;										// グラフィックハンドルが持つ画像のサイズを得る( float? )
extern	int			GetGraphTextureSize(			int GrHandle, int   *SizeXBuf, int   *SizeYBuf ) ;										// グラフィックハンドルが持つ一つ目のテクス?ャのサイズを得る
extern	int			GetGraphUseBaseGraphArea(		int GrHandle, int   *UseX,     int   *UseY,    int *UseSizeX, int *UseSizeY ) ;			// LoadDivGraph や DerivationGraph で元画像の一部分を使用している場合に、指定のグラフィックハンドルが使用している元画像の範囲を取得する
extern	int			GetGraphMipmapCount(			int GrHandle ) ;																		// グラフィックハンドルが持つテクス?ャの?ップ?ップレベル数を取得する
extern	int			GetGraphFilePath(				int GrHandle, TCHAR *FilePathBuffer ) ;													// グラフィックハンドルが画像フ?イルから読み込まれていた場合、その画像のフ?イルパスを取得する
extern	int			CheckDrawValidGraph(			int GrHandle ) ;																		// 指定のグラフィックハンドルが?画対象にできる( SetDrawScreen の引数に渡せる )グラフィックハンドルかどうかを取得する( 戻り値?TRUE:?画対象にできるグラフィックハンドル?FALSE:?画対象にできないグラフィックハンドル )

extern	const COLORDATA* GetTexColorData(			int AlphaCh, int AlphaTest, int ColorBitDepth, int DrawValid DEFAULTPARAM( = FALSE ) ) ;	// カラ?デ??を得る
#ifndef DX_COMPILE_TYPE_C_LANGUAGE
extern	const COLORDATA* GetTexColorData(			const IMAGEFORMATDESC *Format ) ;														// フォ??ットに基づいたカラ?デ??を得る
extern	const COLORDATA* GetTexColorData(			int FormatIndex /* DX_GRAPHICSIMAGE_FORMAT_3D_RGB32 等 */ ) ;							// 指定のフォ??ットインデックスのカラ?デ??を得る
#endif // DX_COMPILE_TYPE_C_LANGUAGE
extern	const COLORDATA* GetTexColorData2(			const IMAGEFORMATDESC *Format ) ;														// フォ??ットに基づいたカラ?デ??を得る
extern	const COLORDATA* GetTexColorData3(			int FormatIndex /* DX_GRAPHICSIMAGE_FORMAT_3D_RGB32 等 */ ) ;							// 指定のフォ??ットインデックスのカラ?デ??を得る
extern	int			GetMaxGraphTextureSize(			int *SizeX, int *SizeY ) ;																// グラフィックスデバイスが対応している最大テクス?ャサイズを取得する
extern	int			GetValidRestoreShredPoint(		void ) ;																				// グラフィックハンドルの画像を復元する関数が登?されているかどうかを取得する( TRUE:登?されている  FALSE:登?されていない )
extern	int			GetCreateGraphColorData(		COLORDATA *ColorData, IMAGEFORMATDESC *Format ) ;										// ( 現在効果なし )これから新たにグラフィックを作成する場合に使用するカラ?情報を取得する

// 画像パレット?作関係関数( ?フトウエア画像のみ使用可? )
extern	int			GetGraphPalette(				int GrHandle, int ColorIndex, int *Red, int *Green, int *Blue ) ;						// グラフィックハンドルのパレットを取得する( ?フトウエアレン?リングモ?ドで、且つパレット画像の場合のみ使用可? )
extern  int			GetGraphOriginalPalette(		int GrHandle, int ColorIndex, int *Red, int *Green, int *Blue ) ;						// グラフィックハンドルの SetGraphPalette で変更する前のパレットを取得する( ?フトウエアレン?リングモ?ドで、且つパレット画像の場合のみ使用可? )
extern	int			SetGraphPalette(				int GrHandle, int ColorIndex, unsigned int Color ) ;									// グラフィックハンドルのパレットを変更する( ?フトウエアレン?リングモ?ドで、且つパレット画像の場合のみ使用可? )
extern	int			ResetGraphPalette(				int GrHandle ) ;																		// SetGraphPalette で変更したパレットを全て元に戻す( ?フトウエアレン?リングモ?ドで、且つパレット画像の場合のみ使用可? )

// ???画関数
extern	int			DrawLine(         int   x1, int   y1, int   x2, int   y2,                                         unsigned int Color, int   Thickness DEFAULTPARAM( = 1 )    ) ;							// 線を?画する
extern	int			DrawLineAA(       float x1, float y1, float x2, float y2,                                         unsigned int Color, float Thickness DEFAULTPARAM( = 1.0f ) ) ;							// 線を?画する( アン?エイリアス付き )
extern	int			DrawBox(          int   x1, int   y1, int   x2, int   y2,                                         unsigned int Color, int FillFlag, int   LineThickness DEFAULTPARAM( = 1 )    ) ;			// 四角?を?画する
extern	int			DrawBoxAA(        float x1, float y1, float x2, float y2,                                         unsigned int Color, int FillFlag, float LineThickness DEFAULTPARAM( = 1.0f ) ) ;			// 四角?を?画する( アン?エイリアス付き )
extern	int			DrawFillBox(      int   x1, int   y1, int   x2, int   y2,                                         unsigned int Color ) ;																	// 中身を塗りつぶす四角?を?画する
extern	int			DrawLineBox(      int   x1, int   y1, int   x2, int   y2,                                         unsigned int Color, int LineThickness DEFAULTPARAM( = 1 ) ) ;								// 枠だけの四角?の?画 する
extern	int			DrawCircle(       int   x,  int   y,  int   r,                                                    unsigned int Color, int FillFlag DEFAULTPARAM( = TRUE ), int   LineThickness DEFAULTPARAM( = 1 )    ) ;	// ?を?画する
extern	int			DrawCircleAA(     float x,  float y,  float r,            int posnum,                             unsigned int Color, int FillFlag DEFAULTPARAM( = TRUE ), float LineThickness DEFAULTPARAM( = 1.0f ), double Angle DEFAULTPARAM( = 0.0 ) ) ;	// ?を?画する( アン?エイリアス付き )
extern	int			DrawOval(         int   x,  int   y,  int   rx, int   ry,                                         unsigned int Color, int FillFlag,        int   LineThickness DEFAULTPARAM( = 1 )    ) ;	// 楕?を?画する
extern	int			DrawOvalAA(       float x,  float y,  float rx, float ry, int posnum,                             unsigned int Color, int FillFlag,        float LineThickness DEFAULTPARAM( = 1.0f ) ) ;	// 楕?を?画する( アン?エイリアス付き )
extern	int			DrawOval_Rect(    int   x1, int   y1, int   x2, int   y2,                                         unsigned int Color, int FillFlag ) ;														// 指定の矩?に収まる?( 楕? )を?画する
extern	int			DrawTriangle(     int   x1, int   y1, int   x2, int   y2, int   x3, int   y3,                     unsigned int Color, int FillFlag ) ;														// 三角?を?画する
extern	int			DrawTriangleAA(   float x1, float y1, float x2, float y2, float x3, float y3,                     unsigned int Color, int FillFlag, float LineThickness DEFAULTPARAM( = 1.0f ) ) ;			// 三角?を?画する( アン?エイリアス付き )
extern	int			DrawQuadrangle(   int   x1, int   y1, int   x2, int   y2, int   x3, int   y3, int   x4, int   y4, unsigned int Color, int FillFlag ) ;														// 四角?を?画する
extern	int			DrawQuadrangleAA( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, unsigned int Color, int FillFlag, float LineThickness DEFAULTPARAM( = 1.0f ) ) ;			// 四角?を?画する( アン?エイリアス付き )
extern	int			DrawRoundRect(    int   x1, int   y1, int   x2, int   y2, int   rx, int   ry,                     unsigned int Color, int FillFlag ) ;														// 角の丸い四角?を?画する
extern	int			DrawRoundRectAA(  float x1, float y1, float x2, float y2, float rx, float ry, int posnum,         unsigned int Color, int FillFlag, float LineThickness DEFAULTPARAM( = 1.0f ) ) ;			// 角の丸い四角?を?画する( アン?エイリアス付き )
extern	int			BeginAADraw(      void ) ;																																									// DrawTriangleAA などのアン?エイリアス付き???画の?備を行う
extern	int			EndAADraw(        void ) ;																																									// DrawTriangleAA などのアン?エイリアス付き???画の後始末を行う
extern 	int			DrawPixel(        int   x,  int   y,                                                              unsigned int Color ) ;																	// ?を?画する

extern	int			Paint(			int x, int y, unsigned int FillColor, ULONGLONG BoundaryColor DEFAULTPARAM( = ULL_PARAM( 0xffffffffffffffff ) ) ) ;				// 指定?から境界色があるところまで塗りつぶす(境界色を -1 にすると指定?の色の領域を塗りつぶす)

extern 	int			DrawPixelSet(   const POINTDATA *PointDataArray, int Num ) ;																					// ?の集合を?画する
extern	int			DrawLineSet(    const LINEDATA *LineDataArray,   int Num ) ;																					// 線の集合を?画する
extern	int			DrawBoxSet(     const RECTDATA *RectDataArray,   int Num ) ;																					// 矩?の集合を?画する

extern	int			DrawPixel3D(     VECTOR   Pos,                                                                 unsigned int Color ) ;							// ３Ｄの?を?画する
extern	int			DrawPixel3DD(    VECTOR_D Pos,                                                                 unsigned int Color ) ;							// ３Ｄの?を?画する
extern	int			DrawLine3D(      VECTOR   Pos1,   VECTOR   Pos2,                                               unsigned int Color ) ;							// ３Ｄの線分を?画する
extern	int			DrawLine3DD(     VECTOR_D Pos1,   VECTOR_D Pos2,                                               unsigned int Color ) ;							// ３Ｄの線分を?画する
extern	int			DrawTriangle3D(  VECTOR   Pos1,   VECTOR   Pos2, VECTOR   Pos3,                                unsigned int Color, int FillFlag ) ;				// ３Ｄの三角?を?画する
extern	int			DrawTriangle3DD( VECTOR_D Pos1,   VECTOR_D Pos2, VECTOR_D Pos3,                                unsigned int Color, int FillFlag ) ;				// ３Ｄの三角?を?画する
extern	int			DrawCube3D(      VECTOR   Pos1,   VECTOR   Pos2,                            unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄの立方体を?画する
extern	int			DrawCube3DD(     VECTOR_D Pos1,   VECTOR_D Pos2,                            unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄの立方体を?画する
extern	int			DrawCubeSet3D(   CUBEDATA *CubeDataArray, int Num, int FillFlag ) ;																				// ３Ｄの立方体の集合を?画する
extern	int			DrawSphere3D(    VECTOR   CenterPos,                  float  r,             int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄの球体を?画する
extern	int			DrawSphere3DD(   VECTOR_D CenterPos,                  double r,             int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄの球体を?画する
extern	int			DrawCapsule3D(   VECTOR   Pos1,   VECTOR   Pos2,      float  r,             int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄのカプセルを?画する
extern	int			DrawCapsule3DD(  VECTOR_D Pos1,   VECTOR_D Pos2,      double r,             int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄのカプセルを?画する
extern	int			DrawCylinder3D(  VECTOR   Pos1,   VECTOR   Pos2,      float  r,             int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄの?柱を?画する
extern	int			DrawCylinder3DD( VECTOR_D Pos1,   VECTOR_D Pos2,      double r,             int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄの?柱を?画する
extern	int			DrawTube3D(      VECTOR   Pos1,   VECTOR   Pos2,      float  r1, float  r2, int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄの筒を?画する
extern	int			DrawTube3DD(     VECTOR_D Pos1,   VECTOR_D Pos2,      double r1, double r2, int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄの筒を?画する
extern	int			DrawCone3D(      VECTOR   TopPos, VECTOR   BottomPos, float  r,             int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄの?錐を?画する
extern	int			DrawCone3DD(     VECTOR_D TopPos, VECTOR_D BottomPos, double r,             int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag ) ;		// ３Ｄの?錐を?画する

// 画像?画関数
extern	int			LoadGraphScreen(           int x, int y, const TCHAR *GraphName,                         int TransFlag ) ;										// 画像フ?イルを読みこんで画面に?画する
extern	int			LoadGraphScreenWithStrLen( int x, int y, const TCHAR *GraphName, size_t GraphNameLength, int TransFlag ) ;										// 画像フ?イルを読みこんで画面に?画する

extern	int			DrawGraph(                int x, int y,                                                                 int GrHandle, int TransFlag ) ;																							// 画像の等??画
extern	int			DrawExtendGraph(          int x1, int y1, int x2, int y2,                                               int GrHandle, int TransFlag ) ;																							// 画像の拡大?画
extern	int			DrawRotaGraph(            int x, int y,                 double ExRate,                    double Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の回??画
extern	int			DrawRotaGraph2(           int x, int y, int cx, int cy, double ExtRate,                   double Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の回??画２( 回?中心指定付き )
extern	int			DrawRotaGraph3(           int x, int y, int cx, int cy, double ExtRateX, double ExtRateY, double Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ; 	// 画像の回??画３( 回?中心指定付き?縦横拡大率別指定版 )
extern	int			DrawRotaGraphFast(        int x, int y,                 float  ExRate,                    float  Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の回??画( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRotaGraphFast2(       int x, int y, int cx, int cy, float  ExtRate,                   float  Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の回??画２( 回?中心指定付き )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRotaGraphFast3(       int x, int y, int cx, int cy, float  ExtRateX, float  ExtRateY, float  Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ; 	// 画像の回??画３( 回?中心指定付き?縦横拡大率別指定版 )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawModiGraph(            int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,               int GrHandle, int TransFlag ) ;																							// 画像の自由変??画
extern	int			DrawTurnGraph(            int x, int y,                                                                 int GrHandle, int TransFlag ) ;																							// 画像の左右反??画
extern	int			DrawReverseGraph(         int x, int y,                                                                 int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の反??画

extern	int			DrawGraphF(               float xf, float yf,                                                                       int GrHandle, int TransFlag ) ;															// 画像の?画( 座標指定が float 版 )
extern	int			DrawExtendGraphF(         float x1f, float y1f, float x2f, float y2f,                                               int GrHandle, int TransFlag ) ;															// 画像の拡大?画( 座標指定が float 版 )
extern	int			DrawRotaGraphF(           float xf, float yf,                       double ExRate,                    double Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の回??画( 座標指定が float 版 )
extern	int			DrawRotaGraph2F(          float xf, float yf, float cxf, float cyf, double ExtRate,                   double Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の回??画２( 回?中心指定付き )( 座標指定が float 版 )
extern	int			DrawRotaGraph3F(          float xf, float yf, float cxf, float cyf, double ExtRateX, double ExtRateY, double Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ; 	// 画像の回??画３( 回?中心指定付き?縦横拡大率別指定版 )( 座標指定が float 版 )
extern	int			DrawRotaGraphFastF(       float xf, float yf,                       float  ExRate,                    float  Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の回??画( 座標指定が float 版 )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRotaGraphFast2F(      float xf, float yf, float cxf, float cyf, float  ExtRate,                   float  Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の回??画２( 回?中心指定付き )( 座標指定が float 版 )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRotaGraphFast3F(      float xf, float yf, float cxf, float cyf, float  ExtRateX, float  ExtRateY, float  Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ; 	// 画像の回??画３( 回?中心指定付き?縦横拡大率別指定版 )( 座標指定が float 版 )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawModiGraphF(           float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,           int GrHandle, int TransFlag ) ;															// 画像の自由変??画( 座標指定が float 版 )
extern	int			DrawTurnGraphF(           float xf, float yf,                                                                       int GrHandle, int TransFlag ) ;															// 画像の左右反??画( 座標指定が float 版 )
extern	int			DrawReverseGraphF(        float xf, float yf,                                                                       int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の反??画( 座標指定が float 版 )

extern	int			DrawChipMap(              int Sx, int Sy, int XNum, int YNum, const int *MapData, int ChipTypeNum, int MapDataPitch, const int *ChipGrHandle, int TransFlag ) ;																											// ?ップ画像を使った２Ｄ?ップ?画
#ifndef DX_COMPILE_TYPE_C_LANGUAGE
extern	int			DrawChipMap(              int MapWidth, int MapHeight,        const int *MapData, int ChipTypeNum,                   const int *ChipGrHandle, int TransFlag, int MapDrawPointX, int MapDrawPointY, int MapDrawWidth, int MapDrawHeight, int ScreenX, int ScreenY ) ;	// ?ップ画像を使った２Ｄ?ップ?画
#endif // DX_COMPILE_TYPE_C_LANGUAGE
extern	int			DrawChipMap2(             int MapWidth, int MapHeight,        const int *MapData, int ChipTypeNum,                   const int *ChipGrHandle, int TransFlag, int MapDrawPointX, int MapDrawPointY, int MapDrawWidth, int MapDrawHeight, int ScreenX, int ScreenY ) ;	// ?ップ画像を使った２Ｄ?ップ?画
extern	int			DrawTile(                 int x1, int y1, int x2, int y2, int Tx, int Ty, double ExtRate, double Angle, int GrHandle, int TransFlag ) ;																																	// 画像を指定領域に?イル状に?画する

extern	int			DrawRectGraph(            int DestX,  int DestY,                          int SrcX, int SrcY, int    Width, int    Height,                         int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;				// 画像の指定矩?部分のみを等??画
extern	int			DrawRectExtendGraph(      int DestX1, int DestY1, int DestX2, int DestY2, int SrcX, int SrcY, int SrcWidth, int SrcHeight,                         int GraphHandle, int TransFlag ) ;																	// 画像の指定矩?部分のみを拡大?画
extern	int			DrawRectRotaGraph(        int x, int y, int SrcX, int SrcY, int Width, int Height, double ExtRate, double Angle,                                   int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;				// 画像の指定矩?部分のみを回??画
extern	int			DrawRectRotaGraph2(       int x, int y, int SrcX, int SrcY, int Width, int Height, int cx, int cy, double ExtRate,  double Angle,                  int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;				// 画像の指定矩?部分のみを回??画２( 回?中心指定付き )
extern	int			DrawRectRotaGraph3(       int x, int y, int SrcX, int SrcY, int Width, int Height, int cx, int cy, double ExtRateX, double ExtRateY, double Angle, int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;				// 画像の指定矩?部分のみを回??画３( 回?中心指定付き?縦横拡大率別指定版 )
extern	int			DrawRectRotaGraphFast(    int x, int y, int SrcX, int SrcY, int Width, int Height, float ExtRate, float Angle,                                     int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;				// 画像の指定矩?部分のみを回??画( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRectRotaGraphFast2(   int x, int y, int SrcX, int SrcY, int Width, int Height, int cx, int cy, float ExtRate,  float Angle,                    int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;				// 画像の指定矩?部分のみを回??画２( 回?中心指定付き )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRectRotaGraphFast3(   int x, int y, int SrcX, int SrcY, int Width, int Height, int cx, int cy, float ExtRateX, float ExtRateY, float Angle,    int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;				// 画像の指定矩?部分のみを回??画３( 回?中心指定付き?縦横拡大率別指定版 )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRectModiGraph(        int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int SrcX, int SrcY, int Width, int Height,               int GraphHandle, int TransFlag ) ;																	// 画像の指定矩?部分のみを自由変??画

extern	int			DrawRectGraphF(           float DestX,  float DestY,                              int   SrcX, int   SrcY, int    Width, int    Height,                           int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の指定矩?部分のみを等??画( 座標指定が float 版 )
extern	int			DrawRectGraphF2(          float DestX,  float DestY,                              float SrcX, float SrcY, float  Width, float  Height,                           int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の指定矩?部分のみを等??画( 座標指定が float 版( 画像内座標の指定も float 版 ) )
extern	int			DrawRectExtendGraphF(     float DestX1, float DestY1, float DestX2, float DestY2, int   SrcX, int   SrcY, int SrcWidth, int SrcHeight,                           int GraphHandle, int TransFlag ) ;														// 画像の指定矩?部分のみを拡大?画( 座標指定が float 版 )
extern	int			DrawRectExtendGraphF2(    float DestX1, float DestY1, float DestX2, float DestY2, float SrcX, float SrcY, float SrcWidth, float SrcHeight,                   int GraphHandle, int TransFlag ) ;															// 画像の指定矩?部分のみを拡大?画( 座標指定が float 版( 画像内座標の指定も float 版 ) )
extern	int			DrawRectRotaGraphF(       float x, float y, int SrcX, int SrcY, int Width, int Height,                       double ExtRate,                   double Angle, int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の指定矩?部分のみを回??画( 座標指定が float 版 )
extern	int			DrawRectRotaGraph2F(      float x, float y, int SrcX, int SrcY, int Width, int Height, float cxf, float cyf, double ExtRate,                   double Angle, int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の指定矩?部分のみを回??画２( 回?中心指定付き )( 座標指定が float 版 )
extern	int			DrawRectRotaGraph3F(      float x, float y, int SrcX, int SrcY, int Width, int Height, float cxf, float cyf, double ExtRateX, double ExtRateY, double Angle, int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の指定矩?部分のみを回??画３( 回?中心指定付き?縦横拡大率別指定版 )( 座標指定が float 版 )
extern	int			DrawRectRotaGraphFastF(   float x, float y, int SrcX, int SrcY, int Width, int Height,                       float ExtRate,                    float  Angle, int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の指定矩?部分のみを回??画( 座標指定が float 版 )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRectRotaGraphFast2F(  float x, float y, int SrcX, int SrcY, int Width, int Height, float cxf, float cyf, float ExtRate,                    float  Angle, int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の指定矩?部分のみを回??画２( 回?中心指定付き )( 座標指定が float 版 )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRectRotaGraphFast3F(  float x, float y, int SrcX, int SrcY, int Width, int Height, float cxf, float cyf, float ExtRateX,  float  ExtRateY, float  Angle, int GraphHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像の指定矩?部分のみを回??画３( 回?中心指定付き?縦横拡大率別指定版 )( 座標指定が float 版 )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRectModiGraphF(       float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int SrcX, int SrcY, int Width, int Height,         int GraphHandle, int TransFlag ) ;															// 画像の指定矩?部分のみを自由変??画( 座標指定が float 版 )

extern	int			DrawBlendGraph(           int   x, int   y, int GrHandle, int TransFlag,                 int BlendGraph, int BorderParam, int BorderRange ) ;									// ブレンド画像と合成して画像を等??画する
extern	int			DrawBlendGraphF(          float x, float y, int GrHandle, int TransFlag,                 int BlendGraph, int BorderParam, int BorderRange ) ;									// ブレンド画像と合成して画像を等??画する( 座標指定が float 版 )
extern	int			DrawBlendGraphPos(        int   x, int   y, int GrHandle, int TransFlag, int bx, int by, int BlendGraph, int BorderParam, int BorderRange ) ;									// ブレンド画像と合成して画像を等??画する( ブレンド画像の起?座標を指定する引数付き )

extern	int			DrawCircleGauge(          int   CenterX, int   CenterY, double Percent, int GrHandle, double StartPercent DEFAULTPARAM( = 0.0 ) , double Scale DEFAULTPARAM( = 1.0 ) , int ReverseX DEFAULTPARAM( = FALSE ) , int ReverseY DEFAULTPARAM( = FALSE ) ) ;										// ?グラフ的な?画を行う( GrHandle の画像の上下左右の?は透過色にしておく必要があります )
extern	int			DrawCircleGaugeF(         float CenterX, float CenterY, double Percent, int GrHandle, double StartPercent DEFAULTPARAM( = 0.0 ) , double Scale DEFAULTPARAM( = 1.0 ) , int ReverseX DEFAULTPARAM( = FALSE ) , int ReverseY DEFAULTPARAM( = FALSE ) ) ;										// ?グラフ的な?画を行う( GrHandle の画像の上下左右の?は透過色にしておく必要があります )( 座標指定が float 版 )

extern	int			DrawGraphToZBuffer(       int X, int Y,                                                                 int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;														// Ｚバッフ?に対して画像の等??画
extern	int			DrawTurnGraphToZBuffer(   int x, int y,                                                                 int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;														// Ｚバッフ?に対して画像の左右反??画
extern	int			DrawReverseGraphToZBuffer( int x, int y,                                                                int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// Ｚバッフ?に対して画像の反??画
extern	int			DrawExtendGraphToZBuffer( int x1, int y1, int x2, int y2,                                               int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;														// Ｚバッフ?に対して画像の拡大?画
extern	int			DrawRotaGraphToZBuffer(   int x, int y, double ExRate, double Angle,                                    int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// Ｚバッフ?に対して画像の回??画
extern	int			DrawRotaGraph2ToZBuffer(  int x, int y, int cx, int cy, double ExtRate,                   double Angle, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// Ｚバッフ?に対して画像の回??画２( 回?中心指定付き )
extern	int			DrawRotaGraph3ToZBuffer(  int x, int y, int cx, int cy, double ExtRateX, double ExtRateY, double Angle, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ; 	// Ｚバッフ?に対して画像の回??画３( 回?中心指定付き?縦横拡大率別指定版 )
extern	int			DrawRotaGraphFastToZBuffer(  int x, int y, float ExRate, float Angle,                                   int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// Ｚバッフ?に対して画像の回??画( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRotaGraphFast2ToZBuffer( int x, int y, int cx, int cy, float ExtRate,                  float Angle, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;	// Ｚバッフ?に対して画像の回??画２( 回?中心指定付き )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawRotaGraphFast3ToZBuffer( int x, int y, int cx, int cy, float ExtRateX, float ExtRateY, float Angle, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ; 	// Ｚバッフ?に対して画像の回??画３( 回?中心指定付き?縦横拡大率別指定版 )( 高速版、座標計算のアルゴリズ?が簡略化されています、?画結果に不都合が無ければこちらの方が高速です )
extern	int			DrawModiGraphToZBuffer(   int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,               int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;														// Ｚバッフ?に対して画像の自由変??画
extern	int			DrawBoxToZBuffer(         int x1, int y1, int x2, int y2,                                               int FillFlag, int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;														// Ｚバッフ?に対して矩?の?画
extern	int			DrawCircleToZBuffer(      int x, int y, int r,                                                          int FillFlag, int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;														// Ｚバッフ?に対して?の?画
extern	int			DrawTriangleToZBuffer(    int x1, int y1, int x2, int y2, int x3, int y3,                               int FillFlag, int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;														// Ｚバッフ?に対して三角?を?画する 
extern	int			DrawQuadrangleToZBuffer(  int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,               int FillFlag, int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;														// Ｚバッフ?に対して四角?を?画する
extern	int			DrawRoundRectToZBuffer(   int x1, int y1, int x2, int y2, int rx, int ry,                               int FillFlag, int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;														// Ｚバッフ?に対して角の丸い四角?を?画する

extern	int			DrawPolygon(                             const VERTEX    *VertexArray, int PolygonNum,                                                                                                       int GrHandle, int TransFlag, int UVScaling DEFAULTPARAM( = FALSE ) ) ;		// ２Ｄ?リゴンを?画する( Vertex:三角?を?成する頂?配列の先頭アドレス( 頂?の数は?リゴンの数?３ )  PolygonNum:?画する?リゴンの数  GrHandle:使用するグラフィックハンドル  TransFlag:透過色処理を行うかどうか( TRUE:行う  FALSE:行わない )  UVScaling:基?FALSEでOK )
extern	int			DrawPolygon2D(                           const VERTEX2D  *VertexArray, int PolygonNum,                                                                                                       int GrHandle, int TransFlag ) ;							// ２Ｄ?リゴンを?画する
extern	int			DrawPolygon3D(                           const VERTEX3D  *VertexArray, int PolygonNum,                                                                                                       int GrHandle, int TransFlag ) ;							// ３Ｄ?リゴンを?画する
extern	int			DrawPolygonIndexed2D(                    const VERTEX2D  *VertexArray, int VertexNum, const unsigned short *IndexArray, int PolygonNum,                                                      int GrHandle, int TransFlag ) ;							// ２Ｄ?リゴンを?画する( 頂?インデックスを使用 )
extern	int			DrawPolygon32bitIndexed2D(               const VERTEX2D  *VertexArray, int VertexNum, const unsigned int   *IndexArray, int PolygonNum,                                                      int GrHandle, int TransFlag ) ;							// ２Ｄ?リゴンを?画する( 頂?インデックスを使用 )
extern	int			DrawPolygonIndexed3D(                    const VERTEX3D  *VertexArray, int VertexNum, const unsigned short *IndexArray, int PolygonNum,                                                      int GrHandle, int TransFlag ) ;							// ３Ｄ?リゴンを?画する( 頂?インデックスを使用 )
extern	int			DrawPolygon32bitIndexed3D(               const VERTEX3D  *VertexArray, int VertexNum, const unsigned int   *IndexArray, int PolygonNum,                                                      int GrHandle, int TransFlag ) ;							// ３Ｄ?リゴンを?画する( 頂?インデックスを使用 )
extern	int			DrawPolygonIndexed3DBase(                const VERTEX_3D *VertexArray, int VertexNum, const unsigned short *IndexArray, int IndexNum,   int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag ) ;							// ３Ｄ?リゴンを?画する( 頂?インデックスを使用 )( 旧バ?ジョン用 )
extern	int			DrawPolygon32bitIndexed3DBase(           const VERTEX_3D *VertexArray, int VertexNum, const unsigned int   *IndexArray, int IndexNum,   int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag ) ;							// ３Ｄ?リゴンを?画する( 頂?インデックスを使用 )( 旧バ?ジョン用 )
extern	int			DrawPolygon3DBase(                       const VERTEX_3D *VertexArray, int VertexNum,                                                   int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag ) ;							// ３Ｄ?リゴンを?画する( 旧バ?ジョン用 )
#ifndef DX_COMPILE_TYPE_C_LANGUAGE
extern	int			DrawPolygon3D(                           const VERTEX_3D *VertexArray, int PolygonNum,                                                                                                       int GrHandle, int TransFlag ) ;							// ３Ｄ?リゴンを?画する( 旧バ?ジョン用 )
#endif // DX_COMPILE_TYPE_C_LANGUAGE
extern	int			DrawPolygon3D2(                          const VERTEX_3D *VertexArray, int PolygonNum,                                                                                                       int GrHandle, int TransFlag ) ;							// ３Ｄ?リゴンを?画する( 旧バ?ジョン用 )

extern	int			DrawPolygonBase(                         const VERTEX    *VertexArray, int VertexNum,                                                   int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag, int UVScaling DEFAULTPARAM( = FALSE ) ) ;		// ２Ｄプリ?ティブを?画する
extern	int			DrawPrimitive2D(                         const VERTEX2D  *VertexArray, int VertexNum,                                                   int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag ) ;							// ２Ｄプリ?ティブを?画する
extern	int			DrawPrimitive3D(                         const VERTEX3D  *VertexArray, int VertexNum,                                                   int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag ) ;							// ３Ｄプリ?ティブを?画する
extern	int			DrawPrimitiveIndexed2D(                  const VERTEX2D  *VertexArray, int VertexNum, const unsigned short *IndexArray, int IndexNum,   int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag ) ;							// ２Ｄプリ?ティブを?画する(頂?インデックス使用)
extern	int			DrawPrimitive32bitIndexed2D(             const VERTEX2D  *VertexArray, int VertexNum, const unsigned int   *IndexArray, int IndexNum,   int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag ) ;							// ２Ｄプリ?ティブを?画する(頂?インデックス使用)
extern	int			DrawPrimitiveIndexed3D(                  const VERTEX3D  *VertexArray, int VertexNum, const unsigned short *IndexArray, int IndexNum,   int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag ) ;							// ３Ｄプリ?ティブを?画する(頂?インデックス使用)
extern	int			DrawPrimitive32bitIndexed3D(             const VERTEX3D  *VertexArray, int VertexNum, const unsigned int   *IndexArray, int IndexNum,   int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag ) ;							// ３Ｄプリ?ティブを?画する(頂?インデックス使用)

extern	int			DrawPolygon3D_UseVertexBuffer(           int VertexBufHandle,                                                                                                                                                               int GrHandle, int TransFlag ) ;		// 頂?バッフ?を使用して３Ｄ?リゴンを?画する
extern	int			DrawPrimitive3D_UseVertexBuffer(         int VertexBufHandle,                     int PrimitiveType  /* DX_PRIMTYPE_TRIANGLELIST 等 */,                                                                                     int GrHandle, int TransFlag ) ;		// 頂?バッフ?を使用して３Ｄプリ?ティブを?画する
extern	int			DrawPrimitive3D_UseVertexBuffer2(        int VertexBufHandle,                     int PrimitiveType  /* DX_PRIMTYPE_TRIANGLELIST 等 */,                 int StartVertex, int UseVertexNum,                                  int GrHandle, int TransFlag ) ;		// 頂?バッフ?を使用して３Ｄプリ?ティブを?画する
extern	int			DrawPolygonIndexed3D_UseVertexBuffer(    int VertexBufHandle, int IndexBufHandle,                                                                                                                                           int GrHandle, int TransFlag ) ;		// 頂?バッフ?とインデックスバッフ?を使用して３Ｄ?リゴンを?画する
extern	int			DrawPrimitiveIndexed3D_UseVertexBuffer(  int VertexBufHandle, int IndexBufHandle, int PrimitiveType  /* DX_PRIMTYPE_TRIANGLELIST 等 */,                                                                                     int GrHandle, int TransFlag ) ;		// 頂?バッフ?とインデックスバッフ?を使用して３Ｄプリ?ティブを?画する
extern	int			DrawPrimitiveIndexed3D_UseVertexBuffer2( int VertexBufHandle, int IndexBufHandle, int PrimitiveType  /* DX_PRIMTYPE_TRIANGLELIST 等 */, int BaseVertex, int StartVertex, int UseVertexNum, int StartIndex, int UseIndexNum, int GrHandle, int TransFlag ) ;		// 頂?バッフ?とインデックスバッフ?を使用して３Ｄプリ?ティブを?画する

extern	int			DrawGraph3D(                             float x, float y, float z,                                                                     int GrHandle, int TransFlag ) ;																// 画像の３Ｄ?画
extern	int			DrawExtendGraph3D(                       float x, float y, float z, double ExRateX, double ExRateY,                                     int GrHandle, int TransFlag ) ;																// 画像の拡大３Ｄ?画
extern	int			DrawRotaGraph3D(                         float x, float y, float z, double ExRate, double Angle,                                        int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;		// 画像の回?３Ｄ?画
extern	int			DrawRota2Graph3D(                        float x, float y, float z, float cx, float cy, double ExtRateX, double ExtRateY, double Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;		// 画像の回?３Ｄ?画(回?中心指定?)
extern	int			DrawModiBillboard3D(                     VECTOR Pos, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,    int GrHandle, int TransFlag ) ;																// 画像の自由変?３Ｄ?画

extern	int			DrawBillboard3D(                         VECTOR Pos,                                                  float cx, float cy, float Size,               float Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;		// ３Ｄ空間上に画像を?画
extern	int			DrawRectBillboard3D(                     VECTOR Pos, int SrcX, int SrcY, int SrcWidth, int SrcHeight, float cx, float cy, float Size,               float Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;		// ３Ｄ空間上に画像を?画( 画像内矩?指定あり )


// ?画設定関係関数
extern	int			SetDrawMode(						int DrawMode ) ;												// ?画モ?ドを設定する
extern	int			GetDrawMode(						void ) ;														// ?画モ?ドを取得する
extern	int			SetDrawBlendMode(					int BlendMode, int BlendParam ) ;								// ?画ブレンドモ?ドを設定する
extern	int			GetDrawBlendMode(					int *BlendMode, int *BlendParam ) ;								// ?画ブレンドモ?ドを取得する
extern	int			SetDrawCustomBlendMode(				int BlendEnable, int SrcBlendRGB /* DX_BLEND_SRC_COLOR 等 */, int DestBlendRGB /* DX_BLEND_SRC_COLOR 等 */, int BlendOpRGB /* DX_BLENDOP_ADD 等 */, int SrcBlendA /* DX_BLEND_SRC_COLOR 等 */, int DestBlendA /* DX_BLEND_SRC_COLOR 等 */, int BlendOpA /* DX_BLENDOP_ADD 等 */, int BlendParam ) ;		// カス??ブレンドモ?ドを設定する
extern	int			GetDrawCustomBlendMode(				int *BlendEnable, int *SrcBlendRGB, int *DestBlendRGB, int *BlendOpRGB, int *SrcBlendA, int *DestBlendA, int *BlendOpA, int *BlendParam ) ;																																								// カス??ブレンドモ?ドを取得する
extern	int			SetDrawAlphaTest(					int TestMode, int TestParam ) ;									// ?画時のアルフ?テストの設定を行う( TestMode:テストモ?ド( DX_CMP_GREATER等 -1でデフォルト動作に戻す )  TestParam:?画アルフ?値との比較に使用する値( 0?255 ) )
extern	int			GetDrawAlphaTest(					int *TestMode, int *TestParam ) ;								// ?画時のアルフ?テストの設定を取得する( TestMode:テストモ?ド( DX_CMP_GREATER等 -1でデフォルト動作に戻す )  TestParam:?画アルフ?値との比較に使用する値( 0?255 ) )
extern	int			SetBlendGraph(						int BlendGraph, int BorderParam, int BorderRange ) ;			// ( SetBlendGraphParam の BlendType = DX_BLENDGRAPHTYPE_WIPE の処理を行う旧関数 )?画処理時に?画する画像とブレンドするα?ャンネル付き画像をセットする( BlendGraph を -1 でブレンド??を無効 )
extern	int			SetBlendGraphParam(					int BlendGraph, int BlendType, ... ) ;							// ?画処理時に?画する画像とブレンドする画像のブレンド設定を行う、BlendGraph を -1 にすれば設定を解除、その場合 BlendType とその後ろのパラメ??は無視される
//		int			SetBlendGraphParam(					int BlendGraph, int BlendType = DX_BLENDGRAPHTYPE_NORMAL, int Ratio = ( 0( ブレンド率０％ )?255( ブレンド率１００％ ) ) ) ;
//		int			SetBlendGraphParam(					int BlendGraph, int BlendType = DX_BLENDGRAPHTYPE_WIPE, int BorderParam = 境界位置(０?２５５), int BorderRange = 境界幅(指定できる値は１、６４、１２８、２５５の４つ) ) ;
//		int			SetBlendGraphParam(					int BlendGraph, int BlendType = DX_BLENDGRAPHTYPE_ALPHA ) ;
extern	int			SetBlendGraphPosition(				int x, int y ) ;												// ブレンド画像の起?座標をセットする
extern	int			SetBlendGraphPositionMode(			int BlendGraphPositionMode /* DX_BLENDGRAPH_POSMODE_DRAWGRAPH など */ ) ;	// ブレンド画像の適応座標モ?ドを設定する
extern	int			SetDrawBright(						int RedBright, int GreenBright, int BlueBright ) ;				// ?画輝度を設定する
extern	int			GetDrawBright(						int *Red, int *Green, int *Blue ) ;								// ?画輝度を取得する
extern	int			SetDrawAddColor(					int Red, int Green, int Blue ) ;								// ?画カラ?に加算する色を設定する( Red, Green, Blue : 加算する色( 0 = 0%  255 = 100%  -255 = -100% ) ( ?イナスの値で減算もできます ) )
extern	int			GetDrawAddColor(					int *Red, int *Green, int *Blue ) ;								// ?画カラ?に加算する色を取得する
extern	int			SetWriteAlphaChannelFlag(			int Flag ) ;													// ?画先のアルフ??ャンネルの内容を書き換えるかを設定する( FALSE:書き換えない  TRUE:書き換える( デフォルト ) )
extern	int			GetWriteAlphaChannelFlag(			void ) ;														// ?画先のアルフ??ャンネルの内容を書き換えるかを取得する( FALSE:書き換えない  TRUE:書き換える( デフォルト ) )
extern	int			CheckSeparateAlphaBlendEnable(		void ) ;														// ?画先のアルフ??ャンネルの内容を書き換えないことができるかどうかを取得する( TRUE:書き換えないことができる  FALSE:書き換えないことができない )
extern	int			SetIgnoreDrawGraphColor(			int EnableFlag ) ;												// ?画する画像のＲＧＢ成分を無視するかどうかを指定する( EnableFlag:この??を使うかどうか( TRUE:使う  FALSE:使わない( デフォルト ) )
extern	int			GetIgnoreDrawGraphColor(			void ) ;														// ?画する画像のＲＧＢ成分を無視するかどうかを取得する( 戻り値  この??を使うかどうか( TRUE:使う  FALSE:使わない )
extern	int			SetMaxAnisotropy(					int MaxAnisotropy ) ;											// 最大異方性値を設定する
extern	int			GetMaxAnisotropy(					void ) ;														// 最大異方性値を取得する
extern	int			SetUseLarge3DPositionSupport(		int UseFlag ) ;													// ３Ｄ処理で使用する座標値が 10000000.0f などの大きな値になっても?画の崩れを小さく?える処理を使用するかどうかを設定する、DxLib_Init の呼び出し前でのみ使用可?( TRUE:?画の崩れを?える処理を使用する( CPU負荷が上がります )??FALSE:?画の崩れを?える処理は使用しない( デフォルト ) )

extern	int			SetUseZBufferFlag(					int Flag ) ;													// Ｚバッフ?を使用するかどうかを設定する( ２Ｄと３Ｄ?画に影響 )( TRUE:Ｚバッフ?を使用する  FALSE:Ｚバッフ?を使用しない( デフォルト ) )
extern	int			SetWriteZBufferFlag(				int Flag ) ;													// Ｚバッフ?に書き込みを行うかどうかを設定する( ２Ｄと３Ｄ?画に影響 )( TRUE:書き込みを行う  FALSE:書き込みを行わない( デフォルト ) )
extern	int			SetZBufferCmpType(					int CmpType /* DX_CMP_NEVER 等 */ ) ;							// Ｚバッフ?のＺ値と書き込むＺ値との比較モ?ドを設定する( ２Ｄと３Ｄ?画に影響 )( CmpType:DX_CMP_NEVER等( デフォルト:DX_CMP_LESSEQUAL ) )
extern	int			SetZBias(							int Bias ) ;													// 書き込むＺ値のバイアスを設定する( ２Ｄと３Ｄ?画に影響 )( Bias:バイアス値( デフォルト:0 ) )
extern	int			SetUseZBuffer3D(					int Flag ) ;													// Ｚバッフ?を使用するかどうかを設定する( ３Ｄ?画のみに影響 )( TRUE:Ｚバッフ?を使用する  FALSE:Ｚバッフ?を使用しない( デフォルト ) )
extern	int			SetWriteZBuffer3D(					int Flag ) ;													// Ｚバッフ?に書き込みを行うかどうかを設定する( ３Ｄ?画のみに影響 )( TRUE:書き込みを行う  FALSE:書き込みを行わない( デフォルト ) )
extern	int			SetZBufferCmpType3D(				int CmpType /* DX_CMP_NEVER 等 */ ) ;							// Ｚバッフ?のＺ値と書き込むＺ値との比較モ?ドを設定する( ３Ｄ?画のみに影響 )( CmpType:DX_CMP_NEVER等( デフォルト:DX_CMP_LESSEQUAL ) )
extern	int			SetZBias3D(							int Bias ) ;													// 書き込むＺ値のバイアスを設定する( ３Ｄ?画のみに影響 )( Bias:バイアス値( デフォルト:0 ) )
extern	int			SetDrawZ(							float Z ) ;														// ２Ｄ?画でＺバッフ?に書き込むＺ値を設定する( Z:書き込むＺ値( デフォルト:0.2f ) )
extern	int			SetUseReversedZ(					int Flag ) ;													// Ｚバッフ?に書き込むＺ値を標?方式と反?した値( リバ?スＺ )にするかどうかを設定する、DxLib_Init実行前のみ使用可?( TRUE:反?した値にする?FALSE:通常の値にする( デフォルト ) )

extern	int			SetDrawArea(						int x1, int y1, int x2, int y2 ) ;								// ?画可?領域の設定する
extern	int			GetDrawArea(						RECT *Rect ) ;													// ?画可?領域を取得する
extern	int			SetDrawAreaFull(					void ) ;														// ?画可?領域を?画対象画面全体にする
extern	int			SetDraw3DScale(						float Scale ) ;													// ３Ｄ?画の拡大率を設定する

extern	int			SetRestoreShredPoint(				void (* ShredPoint )( void ) ) ;								// SetRestoreGraphCallback の旧名
extern	int			SetRestoreGraphCallback(			void (* Callback )( void ) ) ;									// グラフィックハンドル復元関数を登?する
extern	int			RunRestoreShred(					void ) ;														// グラフィック復元関数を実行する
extern	int			SetGraphicsDeviceRestoreCallbackFunction( void (* Callback )( void *Data ), void *CallbackData ) ;	// グラフィックスデバイスがロストから復帰した際に呼ばれるコ?ルバック関数を設定する
extern	int			SetGraphicsDeviceLostCallbackFunction(    void (* Callback )( void *Data ), void *CallbackData ) ;	// グラフィックスデバイスがロストから復帰する前に呼ばれるコ?ルバック関数を設定する

extern	int			SetTransformTo2D(					const MATRIX   *Matrix ) ;										// ２Ｄ?画に使用される変換行列を設定する
extern	int			SetTransformTo2DD(					const MATRIX_D *Matrix ) ;										// ２Ｄ?画に使用される変換行列を設定する
extern	int			ResetTransformTo2D(					void ) ;														// ２Ｄ?画用に使用する変換行列の設定を初期状態に戻す
extern	int			SetTransformToWorld(				const MATRIX   *Matrix ) ;										// ロ?カル座標からワ?ルド座標に変換するための行列を設定する
extern	int			SetTransformToWorldD(				const MATRIX_D *Matrix ) ;										// ロ?カル座標からワ?ルド座標に変換するための行列を設定する
extern	int			GetTransformToWorldMatrix(			      MATRIX   *MatBuf ) ;										// ロ?カル座標からワ?ルド座標に変換するための行列を取得する
extern	int			GetTransformToWorldMatrixD(			      MATRIX_D *MatBuf ) ;										// ロ?カル座標からワ?ルド座標に変換するための行列を取得する
extern	int			SetTransformToView(					const MATRIX   *Matrix ) ;										// ワ?ルド座標からビュ?座標に変換するための行列を設定する
extern	int			SetTransformToViewD(				const MATRIX_D *Matrix ) ;										// ワ?ルド座標からビュ?座標に変換するための行列を設定する
extern	int			GetTransformToViewMatrix(			      MATRIX   *MatBuf ) ;										// ワ?ルド座標からビュ?座標に変換するための行列を取得する
extern	int			GetTransformToViewMatrixD(			      MATRIX_D *MatBuf ) ;										// ワ?ルド座標からビュ?座標に変換するための行列を取得する
extern	int			SetTransformToProjection(			const MATRIX   *Matrix ) ;										// ビュ?座標からプロジェクション座標に変換するための行列を設定する
extern	int			SetTransformToProjectionD(			const MATRIX_D *Matrix ) ;										// ビュ?座標からプロジェクション座標に変換するための行列を設定する
extern	int			GetTransformToProjectionMatrix(		      MATRIX   *MatBuf ) ;										// ビュ?座標からプロジェクション座標に変換するための行列を取得する
extern	int			GetTransformToProjectionMatrixD(	      MATRIX_D *MatBuf ) ;										// ビュ?座標からプロジェクション座標に変換するための行列を取得する
extern	int			SetTransformToViewport(				const MATRIX   *Matrix ) ;										// ビュ???ト行列を設定する
extern	int			SetTransformToViewportD(			const MATRIX_D *Matrix ) ;										// ビュ???ト行列を設定する
extern	int			GetTransformToViewportMatrix(	    MATRIX   *MatBuf ) ;											// ビュ???ト行列を取得する
extern	int			GetTransformToViewportMatrixD(	    MATRIX_D *MatBuf ) ;											// ビュ???ト行列を取得する
extern	int			GetTransformToAPIViewportMatrix(    MATRIX   *MatBuf ) ;											// Direct3Dで自動適用されるビュ???ト行列を取得する
extern	int			GetTransformToAPIViewportMatrixD(   MATRIX_D *MatBuf ) ;											// Direct3Dで自動適用されるビュ???ト行列を取得する
extern	int			SetDefTransformMatrix(				void ) ;														// デフォルトの変換行列を設定する
extern	int			GetTransformPosition(				VECTOR   *LocalPos, float  *x, float  *y ) ;					// ロ?カル座標からスクリ?ン座標を取得する
extern	int			GetTransformPositionD(				VECTOR_D *LocalPos, double *x, double *y ) ;					// ロ?カル座標からスクリ?ン座標を取得する
extern	float		GetBillboardPixelSize(				VECTOR   WorldPos, float  WorldSize ) ;							// ワ?ルド空間上のビル??ドのサイズからスクリ?ンに投影した場合のピクセル単位のサイズを取得する
extern	double		GetBillboardPixelSizeD(				VECTOR_D WorldPos, double WorldSize ) ;							// ワ?ルド空間上のビル??ドのサイズからスクリ?ンに投影した場合のピクセル単位のサイズを取得する
extern	VECTOR		ConvWorldPosToViewPos(				VECTOR   WorldPos ) ;											// ワ?ルド座標をビュ?座標に変換する
extern	VECTOR_D	ConvWorldPosToViewPosD(				VECTOR_D WorldPos ) ;											// ワ?ルド座標をビュ?座標に変換する
extern	VECTOR		ConvWorldPosToScreenPos(			VECTOR   WorldPos ) ;											// ワ?ルド座標をスクリ?ン座標に変換する
extern	VECTOR_D	ConvWorldPosToScreenPosD(			VECTOR_D WorldPos ) ;											// ワ?ルド座標をスクリ?ン座標に変換する
extern	FLOAT4		ConvWorldPosToScreenPosPlusW(		VECTOR   WorldPos ) ;											// ワ?ルド座標をスクリ?ン座標に変換する、最後のＸＹＺ座標をＷで割る前の値を得る
extern	DOUBLE4		ConvWorldPosToScreenPosPlusWD(		VECTOR_D WorldPos ) ;											// ワ?ルド座標をスクリ?ン座標に変換する、最後のＸＹＺ座標をＷで割る前の値を得る
extern	VECTOR		ConvScreenPosToWorldPos(			VECTOR   ScreenPos ) ;											// スクリ?ン座標をワ?ルド座標に変換する
extern	VECTOR_D	ConvScreenPosToWorldPosD(			VECTOR_D ScreenPos ) ;											// スクリ?ン座標をワ?ルド座標に変換する
extern	VECTOR		ConvScreenPosToWorldPos_ZLinear(	VECTOR   ScreenPos ) ;											// スクリ?ン座標をワ?ルド座標に変換する( Z座標が線? )
extern	VECTOR_D	ConvScreenPosToWorldPos_ZLinearD(	VECTOR_D ScreenPos ) ;											// スクリ?ン座標をワ?ルド座標に変換する( Z座標が線? )

extern	int			SetUseCullingFlag(					int Flag ) ;													// SetUseBackCulling の旧名称
extern	int			SetUseBackCulling(					int Flag /* DX_CULLING_LEFT 等 */ ) ;							// ?リゴンカリングモ?ドを設定する
extern	int			GetUseBackCulling(					void ) ;														// ?リゴンカリングモ?ドを取得する
extern	int			SetUseRightHandClippingProcess(		int Flag ) ;													// 右手座標系のクリッピング処理を行うかを設定する( TRUE:右手座標系のクリッピング処理を行う  FALSE:左手座標系のクリッピング処理を行う( デフォルト ) )
extern	int			GetUseRightHandClippingProcess(		void ) ;														// 右手座標系のクリッピング処理を行うかを取得する( TRUE:右手座標系のクリッピング処理を行う  FALSE:左手座標系のクリッピング処理を行う( デフォルト ) )

extern	int			SetTextureAddressMode(				int Mode /* DX_TEXADDRESS_WRAP 等 */ , int Stage DEFAULTPARAM( = -1 ) ) ;	// テクス?ャアドレスモ?ドを設定する
extern	int			SetTextureAddressModeUV(			int ModeU, int ModeV, int Stage DEFAULTPARAM( = -1 ) ) ;					// テクス?ャアドレスモ?ドを設定する( U と V を別々に設定する )
extern	int			SetTextureAddressTransform(			float TransU, float TransV, float ScaleU, float ScaleV, float RotCenterU, float RotCenterV, float Rotate ) ;	// テクス?ャ座標変換パラメ??を設定する
extern	int			SetTextureAddressTransformMatrix(	MATRIX Matrix ) ;												// テクス?ャ座標変換行列を設定する
extern	int			ResetTextureAddressTransform(		void ) ;														// テクス?ャ座標変換設定をリセットする

extern	int			SetFogEnable(						int Flag ) ;													// フォグを有効にするかどうかを設定する( TRUE:有効  FALSE:無効 )
extern	int			GetFogEnable(						void ) ;														// フォグが有効かどうかを取得する( TRUE:有効  FALSE:無効 )
extern	int			SetFogMode(							int Mode /* DX_FOGMODE_NONE 等 */ ) ;							// フォグモ?ドを設定する
extern	int			GetFogMode(							void ) ;														// フォグモ?ドを取得する
extern	int			SetFogColor(						int  r, int  g, int  b ) ;										// フォグカラ?を設定する
extern	int			GetFogColor(						int *r, int *g, int *b ) ;										// フォグカラ?を取得する
extern	int			SetFogStartEnd(						float  start, float  end ) ;									// フォグが始まる距離と終了する距離を設定する( 0.0f ? 1.0f )
extern	int			GetFogStartEnd(						float *start, float *end ) ;									// フォグが始まる距離と終了する距離を取得する( 0.0f ? 1.0f )
extern	int			SetFogDensity(						float density ) ;												// フォグの密度を設定する( 0.0f ? 1.0f )
extern	float		GetFogDensity(						void ) ;														// フォグの密度を取得する( 0.0f ? 1.0f )

extern	int			SetVerticalFogEnable(				int Flag ) ;													// 高さフォグを有効にするかどうかを設定する( TRUE:有効  FALSE:無効 )
extern	int			GetVerticalFogEnable(				void ) ;														// 高さフォグが有効かどうかを取得する( TRUE:有効  FALSE:無効 )
extern	int			SetVerticalFogMode(					int Mode /* DX_FOGMODE_NONE 等 */ ) ;							// 高さフォグモ?ドを設定する
extern	int			GetVerticalFogMode(					void ) ;														// 高さフォグモ?ドを取得する
extern	int			SetVerticalFogColor(				int  r, int  g, int  b ) ;										// 高さフォグカラ?を設定する
extern	int			GetVerticalFogColor(				int *r, int *g, int *b ) ;										// 高さフォグカラ?を取得する
extern	int			SetVerticalFogStartEnd(				float  start, float  end ) ;									// 高さフォグが始まる距離と終了する距離を設定する( 0.0f ? 1.0f )
extern	int			GetVerticalFogStartEnd(				float *start, float *end ) ;									// 高さフォグが始まる距離と終了する距離を取得する( 0.0f ? 1.0f )
extern	int			SetVerticalFogDensity(				float start, float density ) ;									// 高さフォグが始まる処理と密度を設定する( 0.0f ? 1.0f )
extern	int			GetVerticalFogDensity(				float *start, float *density ) ;								// 高さフォグの始まる処理と密度を取得する( 0.0f ? 1.0f )


// 画面関係関数
#ifndef DX_COMPILE_TYPE_C_LANGUAGE
extern	unsigned int	GetPixel(									int x, int y ) ;																// 指定座標の色を取得する
#endif // DX_COMPILE_TYPE_C_LANGUAGE
extern	unsigned int	GetPixelDX(									int x, int y ) ;																// 指定座標の色を取得する
extern	COLOR_F			GetPixelF(									int x, int y ) ;																// 指定座標の色を取得する( float? )
extern	int				SetBackgroundColor(							int Red,  int  Green, int  Blue, int  Alpha DEFAULTPARAM( = 0 )    ) ;			// メインウインドウの背景色を設定する( Red,Green,Blue,Alpha:それぞれ ０?２５５ )
extern	int				GetBackgroundColor(							int *Red, int *Green, int *Blue, int *Alpha DEFAULTPARAM( = NULL ) ) ;			// メインウインドウの背景色を取得する( Red,Green,Blue,Alpha:それぞれ ０?２５５ )
extern	int				GetDrawScreenGraph(							                             int x1, int y1, int x2, int y2,                       int GrHandle, int UseClientFlag DEFAULTPARAM( = TRUE ) ) ;	// ?画先の画面から指定領域の画像情報をグラフィックハンドルに?送する
extern	int				BltDrawValidGraph(							int TargetDrawValidGrHandle, int x1, int y1, int x2, int y2, int DestX, int DestY, int DestGrHandle ) ;							// SetDrawScreen で?画対象にできるグラフィックハンドルから指定領域の画像情報を別のグラフィックハンドルに?送する
extern	int				ScreenFlip(									void ) ;																		// 裏画面と?画面の内容を交換する
extern 	int				ScreenCopy(									void ) ;																		// 裏画面の内容を?画面に?送する
extern	int				WaitVSync(									int SyncNum ) ;																	// 垂直同期信号を待つ
extern	int				ClearDrawScreen(					        const RECT *ClearRect DEFAULTPARAM( = NULL ) ) ;												// 画面をクリアする
extern	int				ClearDrawScreenZBuffer(						const RECT *ClearRect DEFAULTPARAM( = NULL ) ) ;												// 画面のＺバッフ?をクリアする
extern	int				ClsDrawScreen(								void ) ;																		// ClearDrawScreenの旧名称
extern	int				SetDrawScreen(								int DrawScreen ) ;																// ?画先画面を設定する( MakeScreen で作成したグラフィックハンドルも渡すことができます )
extern	int				GetDrawScreen(								void ) ;																		// ?画先画面を取得する
extern	int				GetActiveGraph(								void ) ;																		// GetDrawScreen の旧名称
extern	int				SetUseSetDrawScreenSettingReset(			int UseFlag ) ;																	// SetDrawScreen を実行した際にカメラや?画範囲の設定をリセットするかを設定する( UseFlag  TRUE:リセットする( デフォルト )  FALSE:リセットしない )
extern	int				GetUseSetDrawScreenSettingReset(			void ) ;																		// SetDrawScreen を実行した際にカメラや?画範囲の設定をリセットするかを取得する
extern	int				SetDrawZBuffer(								int DrawScreen ) ;																// ?画先Ｚバッフ?のセット( DrawScreen 付属のＺバッフ?を?画先Ｚバッフ?にする、DrawScreen を -1 にするとデフォルトの?画先Ｚバッフ?に戻る )
extern	int				SetGraphMode(								int ScreenSizeX, int ScreenSizeY, int ColorBitDepth, int RefreshRate DEFAULTPARAM( = 60 ) ) ;	// 画面モ?ドを設定する
extern	int				SetUserScreenImage(							void *Image, int PixelFormat /* DX_USER_SCREEN_PIXEL_FORMAT_R5G6B5 等 */ ) ;	// 画面のメモリイメ?ジをセットする( DxLib_Init の前で呼ぶ必要がある( DxLib_Init の前に一度でも呼んでいれば、DxLib_Init 後は Image のアドレスのみの変更目的で呼ぶことは可? )、PixelFormat に DX_USER_SCREEN_PIXEL_FORMAT_R5G6B5 又は DX_USER_SCREEN_PIXEL_FORMAT_X8R8G8B8 の二つ以外を指定した場合はＤＸライブラリの?画関数は一切使用できなくなります )
extern	int				SetFullScreenResolutionMode(				int ResolutionMode /* DX_FSRESOLUTIONMODE_NATIVE 等 */ ) ;						// フルスクリ?ン解像度モ?ドを設定する
extern	int				GetFullScreenResolutionMode(				int *ResolutionMode, int *UseResolutionMode ) ;									// フルスクリ?ン解像度モ?ドを取得する( UseResolutionMode は実際に使用されている解像度モ?ド( 例えば DX_FSRESOLUTIONMODE_NATIVE を指定していてもモニ?が指定の解像度に対応していない場合は UseResolutionMode が DX_FSRESOLUTIONMODE_DESKTOP や DX_FSRESOLUTIONMODE_MAXIMUM になります ) )
extern	int				GetUseFullScreenResolutionMode(				void ) ;																		// フルスクリ?ン解像度モ?ドを取得する( GetFullScreenResolutionMode の UseResolutionMode で取得できる値を返す関数 )
extern	int				SetFullScreenScalingMode(					int ScalingMode /* DX_FSSCALINGMODE_NEAREST 等 */ , int FitScaling DEFAULTPARAM( = FALSE ) ) ;	// フルスクリ?ンモ?ド時の画面拡大モ?ドを設定する
extern	int				SetEmulation320x240(						int Flag ) ;																	// ６４０ｘ４８０の画面で３２０ｘ２４０の画面解像度にするかどうかを設定する、６４０ｘ４８０以外の解像度では無効( TRUE:有効  FALSE:無効 )
extern	int				SetZBufferSize(								int ZBufferSizeX, int ZBufferSizeY ) ;											// 画面用のＺバッフ?のサイズを設定する
extern	int				SetZBufferBitDepth(							int BitDepth ) ;																// 画面用のＺバッフ?のビット?度を設定する( 16 or 24 or 32 )
extern	int				SetWaitVSyncFlag(							int Flag ) ;																	// ScreenFlip 実行時にＶＳＹＮＣ待ちをするかどうかを設定する
extern	int				GetWaitVSyncFlag(							void ) ;																		// ScreenFlip 実行時にＶＳＹＮＣ待ちをするかどうかを取得する
extern	int				SetFullSceneAntiAliasingMode(				int Samples, int Quality ) ;													// 画面のフルスクリ?ンアン?エイリアスモ?ドの設定を行う( DxLib_Init の前でのみ使用可? )
extern	int				SetGraphDisplayArea(						int x1, int y1, int x2, int y2 ) ;												// ScreenFlip 時に?画面全体に?送する裏画面の領域を設定する( DxLib_Init の前でのみ使用可? )
extern	int				SetChangeScreenModeGraphicsSystemResetFlag(	int Flag ) ;																	// 画面モ?ド変更時( とウインドウモ?ド変更時 )にグラフィックスシステ?の設定やグラフィックハンドルをリセットするかどうかを設定する( TRUE:リセットする( デフォルト )  FALSE:リセットしない )
extern	int				GetScreenState(								int *SizeX, int *SizeY, int *ColorBitDepth ) ;									// 現在の画面の解像度とカラ?ビット数を得る 
extern	int				GetDrawScreenSize(							int *XBuf, int *YBuf ) ;														// ?画先のサイズを取得する
extern	int				GetScreenBitDepth(							void ) ;																		// 画面のカラ?ビット数を取得する
extern	int				GetColorBitDepth(							void ) ;																		// GetScreenBitDepth の旧名称
extern	int				GetChangeDisplayFlag(						void ) ;																		// 画面モ?ドが変更されているかどうかを取得する
extern	int				GetVideoMemorySize(							int *AllSize, int *FreeSize ) ;													// ビデオメモリの容量を得る
extern	int				GetVideoMemorySizeEx(						ULONGLONG *TotalSize, ULONGLONG *UseSize ) ;									// ビデオメモリの容量を得る( 64bit版 )
extern	int				GetRefreshRate(								void ) ;																		// 現在の画面のリフレッシュレ?トを取得する
extern	int				GetDisplayNum(								void ) ;																		// ディスプレイの数を取得
extern	int				GetDisplayInfo(								int DisplayIndex, int *DesktopRectX, int *DesktopRectY, int *DesktopSizeX, int *DesktopSizeY, int *IsPrimary, int *DesktopRefreshRate DEFAULTPARAM( = NULL ) ) ;	// ディスプレイのデスクトップ上での矩?位置を取得する
extern	int				GetDisplayModeNum(							int DisplayIndex DEFAULTPARAM( = 0 ) ) ;										// 変更可?なディスプレイモ?ドの数を取得する
extern	DISPLAYMODEDATA	GetDisplayMode(								int ModeIndex, int DisplayIndex DEFAULTPARAM( = 0 ) ) ;							// 変更可?なディスプレイモ?ドの情報を取得する( ModeIndex は 0 ? GetDisplayModeNum の戻り値-1 )
extern	DISPLAYMODEDATA	GetFullScreenUseDisplayMode(				void ) ;																		// フルスクリ?ンモ?ドで起動している場合の使用しているディスプレイモ?ドの情報を取得する( 仮想フルスクリ?ンモ?ドの場合は取得できない )
extern	int				GetDisplayMaxResolution(					int *SizeX, int *SizeY, int DisplayIndex DEFAULTPARAM( = 0 ) ) ;				// ディスプレイの最大解像度を取得する
extern	const COLORDATA* GetDispColorData(							void ) ;																		// ディスプレイのカラ?デ??アドレスを取得する
extern	int				GetMultiDrawScreenNum(						void ) ;																		// 同時に?画を行うことができる画面の数を取得する
extern	int				GetDrawFloatCoordType(						void ) ;																		// DrawGraphF 等の浮動小数?値で座標を指定する関数における座標?イプを取得する( 戻り値 : DX_DRAWFLOATCOORDTYPE_DIRECT3D9 など )

// その他設定関係関数
extern	int			SetUseNormalDrawShader(						int Flag ) ;									// 通常?画にプログラ?ブルシェ???を使用するかどうかを設定する( TRUE:使用する( デフォルト )  FALSE:使用しない )
extern	int			SetUseSoftwareRenderModeFlag(				int Flag ) ;									// ?フトウエアレン?リングモ?ドを使用するかどうかを設定する( TRUE:使用する  FALSE:使用しない( デフォルト ) )( DxLib_Init の前に呼ぶ必要があります )
extern	int			SetNotUse3DFlag(							int Flag ) ;									// ( 同効果のSetUseSoftwareRenderModeFlag を使用して下さい )３Ｄ??を使わないかどうかを設定する
extern	int			SetUse3DFlag(								int Flag ) ;									// ( 同効果のSetUseSoftwareRenderModeFlag を使用して下さい )３Ｄ??を使うかどうかを設定する
extern	int			GetUse3DFlag(								void ) ;										// ?画に３Ｄ??を使うかどうかを取得する
extern	int			SetScreenMemToVramFlag(						int Flag ) ;									// ( 同効果のSetUseSoftwareRenderModeFlag を使用して下さい )画面のピクセルデ??をＶＲ?Ｍに置くかどうかを設定する
extern	int			GetScreenMemToSystemMemFlag(				void ) ;										// 画面のピクセルデ??がシステ?メモリ上に存在するかを取得する

extern	int			SetWindowDrawRect(							const RECT *DrawRect ) ;						// 通常使用しない
extern	int			RestoreGraphSystem(							void ) ;										// ＤＸライブラリのグラフィックス処理関連の復帰処理を行う
extern	int			SetUseHardwareVertexProcessing(				int Flag ) ;									// ハ?ドウエアの頂?演算処理??を使用するかどうかを設定する( TRUE:使用する( デフォルト )  FALSE:使用しない )( DxLib_Init の前に呼ぶ必要があります )
extern	int			SetUsePixelLighting(						int Flag ) ;									// ピクセル単位でライティングを行うかどうかを設定する、要 ShaderModel 3.0( TRUE:ピクセル単位のライティングを行う  FALSE:頂?単位のライティングを行う( デフォルト ) )
extern	int			SetUseOldDrawModiGraphCodeFlag(				int Flag ) ;									// 古いバ?ジョンの DrawModiGraph 関数のコ?ドを使用するかどうかを設定する
extern	int			SetUseVramFlag(								int Flag ) ;									// ( 現在効果なし )ＶＲ?Ｍを使用するかのフラグをセットする
extern	int			GetUseVramFlag(								void ) ;										// ( 現在効果なし )２Ｄグラフィックサ?フェス作成時にシステ?メモリ?を使用するかのフラグ取得
extern	int			SetBasicBlendFlag(							int Flag ) ;									// ( 現在効果なし )簡略化ブレンド処理を行うか否かのフラグをセットする
extern	int			SetUseBasicGraphDraw3DDeviceMethodFlag(		int Flag ) ;									// ( 現在効果なし )単純??の?画に３Ｄデバイスの??を使用するかどうかを設定する
extern	int			SetUseDisplayIndex(							int Index ) ;									// ＤＸライブラリのウインドウを?示するディスプレイデバイスを設定する( -1 を指定すると?ウスカ??ルがあるディスプレイデバイスにＤＸライブラリのウインドウを?示する )
extern	int			RenderVertex(								void ) ;										// 頂?バッフ?に溜まった頂?デ??を?画する( 特殊用途 )

// ?画パフォ??ンス関係関数
extern	int			GetDrawCallCount(							void ) ;										// 前々回の ScreenFlip 呼び出しから、前回の ScreenFlip 呼び出しまでの間に行われた?画コ?ルの回数を取得する
extern	float		GetFPS(										void ) ;										// フレ??レ?ト( １秒間に呼ばれる ScreenFlip の回数 )を取得する

#ifndef DX_NON_SAVEFUNCTION

// ?画先画面保存関数
// Jpeg_Quality         = 0:低画質?100:高画質
// Png_CompressionLevel = 0:無圧縮?  9:最高圧縮
extern	int			SaveDrawScreen(                 int x1, int y1, int x2, int y2, const TCHAR *FileName,                        int SaveType DEFAULTPARAM( = DX_IMAGESAVETYPE_BMP ) , int Jpeg_Quality DEFAULTPARAM( = 80 ) , int Jpeg_Sample2x1 DEFAULTPARAM( = TRUE ) , int Png_CompressionLevel DEFAULTPARAM( = -1 ) ) ;	// 現在?画対象になっている画面をフ?イルで保存する
extern	int			SaveDrawScreenWithStrLen(       int x1, int y1, int x2, int y2, const TCHAR *FileName, size_t FileNameLength, int SaveType DEFAULTPARAM( = DX_IMAGESAVETYPE_BMP ) , int Jpeg_Quality DEFAULTPARAM( = 80 ) , int Jpeg_Sample2x1 DEFAULTPARAM( = TRUE ) , int Png_CompressionLevel DEFAULTPARAM( = -1 ) ) ;	// 現在?画対象になっている画面をフ?イルで保存する
extern	int			SaveDrawScreenToBMP(            int x1, int y1, int x2, int y2, const TCHAR *FileName                        ) ;																																// 現在?画対象になっている画面をＢＭＰ?式で保存する
extern	int			SaveDrawScreenToBMPWithStrLen(  int x1, int y1, int x2, int y2, const TCHAR *FileName, size_t FileNameLength ) ;																																// 現在?画対象になっている画面をＢＭＰ?式で保存する
extern	int			SaveDrawScreenToDDS(            int x1, int y1, int x2, int y2, const TCHAR *FileName                        ) ;																																// 現在?画対象になっている画面をＤＤＳ?式で保存する
extern	int			SaveDrawScreenToDDSWithStrLen(  int x1, int y1, int x2, int y2, const TCHAR *FileName, size_t FileNameLength ) ;																																// 現在?画対象になっている画面をＤＤＳ?式で保存する
extern	int			SaveDrawScreenToJPEG(           int x1, int y1, int x2, int y2, const TCHAR *FileName,                        int Quality DEFAULTPARAM( = 80 ) , int Sample2x1 DEFAULTPARAM( = TRUE ) ) ;																						// 現在?画対象になっている画面をＪＰＥＧ?式で保存する Quality = 画質、値が大きいほど低圧縮高画質,0?100 
extern	int			SaveDrawScreenToJPEGWithStrLen( int x1, int y1, int x2, int y2, const TCHAR *FileName, size_t FileNameLength, int Quality DEFAULTPARAM( = 80 ) , int Sample2x1 DEFAULTPARAM( = TRUE ) ) ;																						// 現在?画対象になっている画面をＪＰＥＧ?式で保存する Quality = 画質、値が大きいほど低圧縮高画質,0?100 
extern	int			SaveDrawScreenToPNG(            int x1, int y1, int x2, int y2, const TCHAR *FileName,                        int CompressionLevel DEFAULTPARAM( = -1 ) ) ;																						// 現在?画対象になっている画面をＰＮＧ?式で保存する CompressionLevel = 圧縮率、値が大きいほど高圧縮率高負荷、０は無圧縮,0?9
extern	int			SaveDrawScreenToPNGWithStrLen(  int x1, int y1, int x2, int y2, const TCHAR *FileName, size_t FileNameLength, int CompressionLevel DEFAULTPARAM( = -1 ) ) ;																						// 現在?画対象になっている画面をＰＮＧ?式で保存する CompressionLevel = 圧縮率、値が大きいほど高圧縮率高負荷、０は無圧縮,0?9

// ?画対象にできるグラフィックハンドル保存関数
// Jpeg_Quality         = 0:低画質?100:高画質
// Png_CompressionLevel = 0:無圧縮?  9:最高圧縮
extern	int			SaveDrawValidGraph(                 int GrHandle, int x1, int y1, int x2, int y2, const TCHAR *FileName,                        int SaveType DEFAULTPARAM( = DX_IMAGESAVETYPE_BMP ) , int Jpeg_Quality DEFAULTPARAM( = 80 ) , int Jpeg_Sample2x1 DEFAULTPARAM( = TRUE ) , int Png_CompressionLevel DEFAULTPARAM( = -1 ) ) ;		// ?画対象にできるグラフィックハンドルをフ?イルで保存する
extern	int			SaveDrawValidGraphWithStrLen(       int GrHandle, int x1, int y1, int x2, int y2, const TCHAR *FileName, size_t FileNameLength, int SaveType DEFAULTPARAM( = DX_IMAGESAVETYPE_BMP ) , int Jpeg_Quality DEFAULTPARAM( = 80 ) , int Jpeg_Sample2x1 DEFAULTPARAM( = TRUE ) , int Png_CompressionLevel DEFAULTPARAM( = -1 ) ) ;		// ?画対象にできるグラフィックハンドルをフ?イルで保存する
extern	int			SaveDrawValidGraphToBMP(            int GrHandle, int x1, int y1, int x2, int y2, const TCHAR *FileName                        ) ;																																// ?画対象にできるグラフィックハンドルをＢＭＰ?式で保存する
extern	int			SaveDrawValidGraphToBMPWithStrLen(  int GrHandle, int x1, int y1, int x2, int y2, const TCHAR *FileName, size_t FileNameLength ) ;																																// ?画対象にできるグラフィックハンドルをＢＭＰ?式で保存する
extern	int			SaveDrawValidGraphToDDS(            int GrHandle, int x1, int y1, int x2, int y2, const TCHAR *FileName                        ) ;																																// ?画対象にできるグラフィックハンドルをＤＤＳ?式で保存する
extern	int			SaveDrawValidGraphToDDSWithStrLen(  int GrHandle, int x1, int y1, int x2, int y2, const TCHAR *FileName, size_t FileNameLength ) ;																																// ?画対象にできるグラフィックハンドルをＤＤＳ?式で保存する
extern	int			SaveDrawValidGraphToJPEG(           int GrHandle, int x1, int y1, int x2, int y2, const TCHAR *FileName,                        int Quality DEFAULTPARAM( = 80 ) , int Sample2x1 DEFAULTPARAM( = TRUE ) ) ;																						// ?画対象にできるグラフィックハンドルをＪＰＥＧ?式で保存する Quality = 画質、値が大きいほど低圧縮高画質,0?100 
extern	int			SaveDrawValidGraphToJPEGWithStrLen( int GrHandle, int x1, int y1, int x2, int y2, const TCHAR *FileName, size_t FileNameLength, int Quality DEFAULTPARAM( = 80 ) , int Sample2x1 DEFAULTPARAM( = TRUE ) ) ;																						// ?画対象にできるグラフィックハンドルをＪＰＥＧ?式で保存する Quality = 画質、値が大きいほど低圧縮高画質,0?100 
extern	int			SaveDrawValidGraphToPNG(            int GrHandle, int x1, int y1, int x2, int y2, const TCHAR *FileName,                        int CompressionLevel DEFAULTPARAM( = -1 ) ) ;																					// ?画対象にできるグラフィックハンドルをＰＮＧ?式で保存する CompressionLevel = 圧縮率、値が大きいほど高圧縮率高負荷、０は無圧縮,0?9
extern	int			SaveDrawValidGraphToPNGWithStrLen(  int GrHandle, int x1, int y1, int x2, int y2, const TCHAR *FileName, size_t FileNameLength, int CompressionLevel DEFAULTPARAM( = -1 ) ) ;																					// ?画対象にできるグラフィックハンドルをＰＮＧ?式で保存する CompressionLevel = 圧縮率、値が大きいほど高圧縮率高負荷、０は無圧縮,0?9

#endif // DX_NON_SAVEFUNCTION

// 頂?バッフ?関係関数
extern	int			CreateVertexBuffer(		int VertexNum, int VertexType /* DX_VERTEX_TYPE_NORMAL_3D 等 */ ) ;						// 頂?バッフ?を作成する( -1:エラ?  0以上:頂?バッフ?ハンドル )
extern	int			DeleteVertexBuffer(		int VertexBufHandle ) ;																	// 頂?バッフ?を削除する
extern	int			InitVertexBuffer(		void ) ;																				// すべての頂?バッフ?を削除する
extern	int			SetVertexBufferData(	int SetIndex, const void *VertexArray, int VertexNum, int VertexBufHandle ) ;			// 頂?バッフ?に頂?デ??を?送する
extern	void *		GetBufferVertexBuffer(	int VertexBufHandle ) ;																	// 頂?バッフ?ハンドルの頂?バッフ?のアドレスを取得する
extern	int			UpdateVertexBuffer(		int VertexBufHandle, int UpdateStartIndex, int UpdateVertexNum ) ;						// 頂?バッフ?ハンドルの頂?バッフ?への変更を適用する( GetBufferVertexBuffer で取得したバッフ?への変更を反映する )
extern	int			CreateIndexBuffer(		int IndexNum, int IndexType /* DX_INDEX_TYPE_16BIT 等 */  ) ;							// インデックスバッフ?を作成する( -1:エラ??0以上：インデックスバッフ?ハンドル )
extern	int			DeleteIndexBuffer(		int IndexBufHandle ) ;																	// インデックスバッフ?を削除する
extern	int			InitIndexBuffer(		void ) ;																				// すべてのインデックスバッフ?を削除する
extern	int			SetIndexBufferData(		int SetIndex, const void *IndexArray, int IndexNum, int IndexBufHandle ) ;				// インデックスバッフ?にインデックスデ??を?送する
extern	void *		GetBufferIndexBuffer(	int IndexBufHandle ) ;																	// インデックスバッフ?ハンドルのインデックスバッフ?のアドレスを取得する
extern	int			UpdateIndexBuffer(		int IndexBufHandle, int UpdateStartIndex, int UpdateIndexNum ) ;						// インデックスバッフ?ハンドルのインデックスバッフ?への変更を適用する( GetBufferIndexBuffer で取得したバッフ?への変更を反映する )
extern	int			GetMaxPrimitiveCount(	void ) ;																				// グラフィックスデバイスが対応している一度に?画できるプリ?ティブの最大数を取得する
extern	int			GetMaxVertexIndex(		void ) ;																				// グラフィックスデバイスが対応している一度に使用することのできる最大頂?数を取得する

// シェ???関係関数
extern	int			GetValidShaderVersion(			void ) ;												// 使用できるシェ???のバ?ジョンを取得する( 0=使えない  200=シェ???モデル２．０が使用可?  300=シェ???モデル３．０が使用可? )

extern	int			LoadVertexShader(				const TCHAR *FileName                        ) ;		// 頂?シェ???バイナリをフ?イルから読み込み頂?シェ???ハンドルを作成する( 戻り値 -1:エラ?  -1以外:シェ???ハンドル )
extern	int			LoadVertexShaderWithStrLen(		const TCHAR *FileName, size_t FileNameLength ) ;		// 頂?シェ???バイナリをフ?イルから読み込み頂?シェ???ハンドルを作成する( 戻り値 -1:エラ?  -1以外:シェ???ハンドル )
extern	int			LoadGeometryShader(				const TCHAR *FileName                        ) ;		// ジオメトリシェ???バイナリをフ?イルから読み込みジオメトリシェ???ハンドルを作成する( 戻り値 -1:エラ?  -1以外:シェ???ハンドル )
extern	int			LoadGeometryShaderWithStrLen(	const TCHAR *FileName, size_t FileNameLength ) ;		// ジオメトリシェ???バイナリをフ?イルから読み込みジオメトリシェ???ハンドルを作成する( 戻り値 -1:エラ?  -1以外:シェ???ハンドル )
extern	int			LoadPixelShader(				const TCHAR *FileName                        ) ;		// ピクセルシェ???バイナリをフ?イルから読み込みピクセルシェ???ハンドルを作成する( 戻り値 -1:エラ?  -1以外:シェ???ハンドル )
extern	int			LoadPixelShaderWithStrLen(		const TCHAR *FileName, size_t FileNameLength ) ;		// ピクセルシェ???バイナリをフ?イルから読み込みピクセルシェ???ハンドルを作成する( 戻り値 -1:エラ?  -1以外:シェ???ハンドル )
extern	int			LoadVertexShaderFromMem(		const void *ImageAddress, int ImageSize ) ;				// メモリに読み込まれた頂?シェ???バイナリから頂?シェ???ハンドルを作成する( 戻り値 -1:エラ?  -1以外:シェ???ハンドル )
extern	int			LoadGeometryShaderFromMem(		const void *ImageAddress, int ImageSize ) ;				// メモリに読み込まれたジオメトリシェ???バイナリからジオメトリシェ???ハンドルを作成する( 戻り値 -1:エラ?  -1以外:シェ???ハンドル )
extern	int			LoadPixelShaderFromMem(			const void *ImageAddress, int ImageSize ) ;				// メモリに読み込まれたピクセルシェ???バイナリからピクセルシェ???ハンドルを作成する( 戻り値 -1:エラ?  -1以外:シェ???ハンドル )
extern	int			DeleteShader(					int ShaderHandle ) ;									// シェ???ハンドルの削除( 頂?シェ???・ピクセルシェ???共通 )
extern	int			InitShader(						void ) ;												// シェ???ハンドルを全て削除する( 頂?シェ???ハンドル・ピクセルシェ???ハンドルどちらもすべて削除 )

extern	int			GetConstIndexToShader(                     const TCHAR *ConstantName,                            int ShaderHandle ) ;	// 指定の名前を持つ定数が使用するシェ???定数の番号を取得する
extern	int			GetConstIndexToShaderWithStrLen(           const TCHAR *ConstantName, size_t ConstantNameLength, int ShaderHandle ) ;	// 指定の名前を持つ定数が使用するシェ???定数の番号を取得する
extern	int			GetConstCountToShader(                     const TCHAR *ConstantName,                            int ShaderHandle ) ;	// 指定の名前を持つ定数が使用するシェ???定数の数を取得する
extern	int			GetConstCountToShaderWithStrLen(           const TCHAR *ConstantName, size_t ConstantNameLength, int ShaderHandle ) ;	// 指定の名前を持つ定数が使用するシェ???定数の数を取得する
extern	const FLOAT4 *GetConstDefaultParamFToShader(           const TCHAR *ConstantName,                            int ShaderHandle ) ;	// 指定の名前を持つ浮動小数?数定数のデフォルトパラメ??が格?されているメモリアドレスを取得する
extern	const FLOAT4 *GetConstDefaultParamFToShaderWithStrLen( const TCHAR *ConstantName, size_t ConstantNameLength, int ShaderHandle ) ;	// 指定の名前を持つ浮動小数?数定数のデフォルトパラメ??が格?されているメモリアドレスを取得する
extern	int			SetVSConstSF(         int ConstantIndex,       float  Param ) ;						// 頂?シェ???の float ?定数を設定する
extern	int			SetVSConstF(          int ConstantIndex,       FLOAT4 Param ) ;						// 頂?シェ???の float ?定数を設定する
extern	int			SetVSConstFMtx(       int ConstantIndex,       MATRIX Param ) ;						// 頂?シェ???の float ?定数に行列を設定する
extern	int			SetVSConstFMtxT(      int ConstantIndex,       MATRIX Param ) ;						// 頂?シェ???の float ?定数に?置した行列を設定する
extern	int			SetVSConstSI(         int ConstantIndex,       int    Param ) ;						// 頂?シェ???の int   ?定数を設定する
extern	int			SetVSConstI(          int ConstantIndex,       INT4   Param ) ;						// 頂?シェ???の int   ?定数を設定する
extern	int			SetVSConstB(          int ConstantIndex,       BOOL   Param ) ;						// 頂?シェ???の BOOL  ?定数を設定する
extern	int			SetVSConstSFArray(    int ConstantIndex, const float  *ParamArray, int ParamNum ) ;	// 頂?シェ???の float ?定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetVSConstFArray(     int ConstantIndex, const FLOAT4 *ParamArray, int ParamNum ) ;	// 頂?シェ???の float ?定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetVSConstFMtxArray(  int ConstantIndex, const MATRIX *ParamArray, int ParamNum ) ;	// 頂?シェ???の float ?定数に行列を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetVSConstFMtxTArray( int ConstantIndex, const MATRIX *ParamArray, int ParamNum ) ;	// 頂?シェ???の float ?定数に?置した行列を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetVSConstSIArray(    int ConstantIndex, const int    *ParamArray, int ParamNum ) ;	// 頂?シェ???の int   ?定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetVSConstIArray(     int ConstantIndex, const INT4   *ParamArray, int ParamNum ) ;	// 頂?シェ???の int   ?定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetVSConstBArray(     int ConstantIndex, const BOOL   *ParamArray, int ParamNum ) ;	// 頂?シェ???の BOOL  ?定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			ResetVSConstF(        int ConstantIndex, int ParamNum ) ;							// 頂?シェ???の float ?定数の設定をリセットする
extern	int			ResetVSConstI(        int ConstantIndex, int ParamNum ) ;							// 頂?シェ???の int   ?定数の設定をリセットする
extern	int			ResetVSConstB(        int ConstantIndex, int ParamNum ) ;							// 頂?シェ???の BOOL  ?定数の設定をリセットする

extern	int			SetPSConstSF(         int ConstantIndex,       float  Param ) ;						// ピクセルシェ???の float ?定数を設定する
extern	int			SetPSConstF(          int ConstantIndex,       FLOAT4 Param ) ;						// ピクセルシェ???の float ?定数を設定する
extern	int			SetPSConstFMtx(       int ConstantIndex,       MATRIX Param ) ;						// ピクセルシェ???の float ?定数に行列を設定する
extern	int			SetPSConstFMtxT(      int ConstantIndex,       MATRIX Param ) ;						// ピクセルシェ???の float ?定数に?置した行列を設定する
extern	int			SetPSConstSI(         int ConstantIndex,       int    Param ) ;						// ピクセルシェ???の int   ?定数を設定する
extern	int			SetPSConstI(          int ConstantIndex,       INT4   Param ) ;						// ピクセルシェ???の int   ?定数を設定する
extern	int			SetPSConstB(          int ConstantIndex,       BOOL   Param ) ;						// ピクセルシェ???の BOOL  ?定数を設定する
extern	int			SetPSConstSFArray(    int ConstantIndex, const float  *ParamArray, int ParamNum ) ;	// ピクセルシェ???の float ?定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetPSConstFArray(     int ConstantIndex, const FLOAT4 *ParamArray, int ParamNum ) ;	// ピクセルシェ???の float ?定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetPSConstFMtxArray(  int ConstantIndex, const MATRIX *ParamArray, int ParamNum ) ;	// ピクセルシェ???の float ?定数に行列を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetPSConstFMtxTArray( int ConstantIndex, const MATRIX *ParamArray, int ParamNum ) ;	// ピクセルシェ???の float ?定数に?置した行列を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetPSConstSIArray(    int ConstantIndex, const int    *ParamArray, int ParamNum ) ;	// ピクセルシェ???の int   ?定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetPSConstIArray(     int ConstantIndex, const INT4   *ParamArray, int ParamNum ) ;	// ピクセルシェ???の int   ?定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			SetPSConstBArray(     int ConstantIndex, const BOOL   *ParamArray, int ParamNum ) ;	// ピクセルシェ???の BOOL  ?定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			ResetPSConstF(        int ConstantIndex, int ParamNum ) ;							// ピクセルシェ???の float ?定数の設定をリセットする
extern	int			ResetPSConstI(        int ConstantIndex, int ParamNum ) ;							// ピクセルシェ???の int   ?定数の設定をリセットする
extern	int			ResetPSConstB(        int ConstantIndex, int ParamNum ) ;							// ピクセルシェ???の BOOL  ?定数の設定をリセットする

extern	int			SetRenderTargetToShader( int TargetIndex, int DrawScreen, int SurfaceIndex DEFAULTPARAM( = 0 ) , int MipLevel DEFAULTPARAM( = 0 ) ) ;	// シェ???を使用した?画での?画先を設定する( DrawScreen に -1 を渡すと無効化 )
extern	int			SetUseTextureToShader(   int StageIndex, int GraphHandle ) ;						// シェ???を使用した?画で使用するグラフィックハンドルを設定する
extern	int			SetUseVertexShader(      int ShaderHandle ) ;										// シェ???を使用した?画に使用する頂?シェ???を設定する( -1を渡すと解除 )
extern	int			SetUseGeometryShader(    int ShaderHandle ) ;										// シェ???を使用した?画に使用するジオメトリシェ???を設定する( -1を渡すと解除 )
extern	int			SetUsePixelShader(       int ShaderHandle ) ;										// シェ???を使用した?画に使用するピクセルシェ???を設定する( -1を渡すと解除 )

extern	int			CalcPolygonBinormalAndTangentsToShader(             VERTEX3DSHADER *VertexArray, int PolygonNum ) ;														// ?リゴンの頂?の接線と??線をＵＶ座標から計算してセットする
extern	int			CalcPolygonIndexedBinormalAndTangentsToShader(      VERTEX3DSHADER *VertexArray, int VertexNum, const unsigned short *IndexArray, int PolygonNum ) ;	// ?リゴンの頂?の接線と??線をＵＶ座標から計算してセットする( 頂?インデックスを使用する )
extern	int			CalcPolygon32bitIndexedBinormalAndTangentsToShader( VERTEX3DSHADER *VertexArray, int VertexNum, const unsigned int   *IndexArray, int PolygonNum ) ;	// ?リゴンの頂?の接線と??線をＵＶ座標から計算してセットする( 頂?インデックスを使用する )

extern	int			DrawBillboard3DToShader( VECTOR Pos, float cx, float cy, float Size, float Angle, int GrHandle, int TransFlag, int ReverseXFlag DEFAULTPARAM( = FALSE ) , int ReverseYFlag DEFAULTPARAM( = FALSE ) ) ;							// シェ???を使ってビル??ドを?画する
extern	int			DrawPolygon2DToShader(                const VERTEX2DSHADER *VertexArray, int PolygonNum ) ;																											// シェ???を使って２Ｄ?リゴンを?画する
extern	int			DrawPolygon3DToShader(                const VERTEX3DSHADER *VertexArray, int PolygonNum ) ;																											// シェ???を使って３Ｄ?リゴンを?画する
extern	int			DrawPolygonIndexed2DToShader(         const VERTEX2DSHADER *VertexArray, int VertexNum, const unsigned short *IndexArray, int PolygonNum ) ;															// シェ???を使って２Ｄ?リゴンを?画する( 頂?インデックスを使用する )
extern	int			DrawPolygon32bitIndexed2DToShader(    const VERTEX2DSHADER *VertexArray, int VertexNum, const unsigned int   *IndexArray, int PolygonNum ) ;															// シェ???を使って２Ｄ?リゴンを?画する( 頂?インデックスを使用する )
extern	int			DrawPolygonIndexed3DToShader(         const VERTEX3DSHADER *VertexArray, int VertexNum, const unsigned short *IndexArray, int PolygonNum ) ;															// シェ???を使って３Ｄ?リゴンを?画する( 頂?インデックスを使用する )
extern	int			DrawPolygon32bitIndexed3DToShader(    const VERTEX3DSHADER *VertexArray, int VertexNum, const unsigned int   *IndexArray, int PolygonNum ) ;															// シェ???を使って３Ｄ?リゴンを?画する( 頂?インデックスを使用する )
extern	int			DrawPrimitive2DToShader(              const VERTEX2DSHADER *VertexArray, int VertexNum,                                                 int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;		// シェ???を使って２Ｄプリ?ティブを?画する
extern	int			DrawPrimitive3DToShader(              const VERTEX3DSHADER *VertexArray, int VertexNum,                                                 int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;		// シェ???を使って３Ｄプリ?ティブを?画する
extern	int			DrawPrimitiveIndexed2DToShader(       const VERTEX2DSHADER *VertexArray, int VertexNum, const unsigned short *IndexArray, int IndexNum, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;		// シェ???を使って２Ｄプリ?ティブを?画する( 頂?インデックスを使用する )
extern	int			DrawPrimitive32bitIndexed2DToShader(  const VERTEX2DSHADER *VertexArray, int VertexNum, const unsigned int   *IndexArray, int IndexNum, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;		// シェ???を使って２Ｄプリ?ティブを?画する( 頂?インデックスを使用する )
extern	int			DrawPrimitiveIndexed3DToShader(       const VERTEX3DSHADER *VertexArray, int VertexNum, const unsigned short *IndexArray, int IndexNum, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;		// シェ???を使って３Ｄプリ?ティブを?画する( 頂?インデックスを使用する )
extern	int			DrawPrimitive32bitIndexed3DToShader(  const VERTEX3DSHADER *VertexArray, int VertexNum, const unsigned int   *IndexArray, int IndexNum, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;		// シェ???を使って３Ｄプリ?ティブを?画する( 頂?インデックスを使用する )
extern	int			DrawPolygon2DToShader2(               const VERTEX2D *VertexArray, int PolygonNum ) ;																												// シェ???を使って２Ｄ?リゴンを?画する( 頂?デ??が VERTEX2D版 )
extern	int			DrawPolygon3DToShader2(               const VERTEX3D *VertexArray, int PolygonNum ) ;																												// シェ???を使って３Ｄ?リゴンを?画する( 頂?デ??が VERTEX3D版 )
extern	int			DrawPolygonIndexed2DToShader2(        const VERTEX2D *VertexArray, int VertexNum, const unsigned short *IndexArray, int PolygonNum ) ;																// シェ???を使って２Ｄ?リゴンを?画する( 頂?インデックスを使用する )( 頂?デ??が VERTEX2D版 )
extern	int			DrawPolygon32bitIndexed2DToShader2(   const VERTEX2D *VertexArray, int VertexNum, const unsigned int   *IndexArray, int PolygonNum ) ;																// シェ???を使って２Ｄ?リゴンを?画する( 頂?インデックスを使用する )( 頂?デ??が VERTEX2D版 )
extern	int			DrawPolygonIndexed3DToShader2(        const VERTEX3D *VertexArray, int VertexNum, const unsigned short *IndexArray, int PolygonNum ) ;																// シェ???を使って３Ｄ?リゴンを?画する( 頂?インデックスを使用する )( 頂?デ??が VERTEX3D版 )
extern	int			DrawPolygon32bitIndexed3DToShader2(   const VERTEX3D *VertexArray, int VertexNum, const unsigned int   *IndexArray, int PolygonNum ) ;																// シェ???を使って３Ｄ?リゴンを?画する( 頂?インデックスを使用する )( 頂?デ??が VERTEX3D版 )
extern	int			DrawPrimitive2DToShader2(             const VERTEX2D *VertexArray, int VertexNum,                                                 int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;			// シェ???を使って２Ｄプリ?ティブを?画する( 頂?デ??が VERTEX2D版 )
extern	int			DrawPrimitive3DToShader2(             const VERTEX3D *VertexArray, int VertexNum,                                                 int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;			// シェ???を使って３Ｄプリ?ティブを?画する( 頂?デ??が VERTEX3D版 )
extern	int			DrawPrimitiveIndexed2DToShader2(      const VERTEX2D *VertexArray, int VertexNum, const unsigned short *IndexArray, int IndexNum, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;			// シェ???を使って２Ｄプリ?ティブを?画する( 頂?インデックスを使用する )( 頂?デ??が VERTEX2D版 )
extern	int			DrawPrimitive32bitIndexed2DToShader2( const VERTEX2D *VertexArray, int VertexNum, const unsigned int   *IndexArray, int IndexNum, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;			// シェ???を使って２Ｄプリ?ティブを?画する( 頂?インデックスを使用する )( 頂?デ??が VERTEX2D版 )
extern	int			DrawPrimitiveIndexed3DToShader2(      const VERTEX3D *VertexArray, int VertexNum, const unsigned short *IndexArray, int IndexNum, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;			// シェ???を使って３Ｄプリ?ティブを?画する( 頂?インデックスを使用する )( 頂?デ??が VERTEX3D版 )
extern	int			DrawPrimitive32bitIndexed3DToShader2( const VERTEX3D *VertexArray, int VertexNum, const unsigned int   *IndexArray, int IndexNum, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;			// シェ???を使って３Ｄプリ?ティブを?画する( 頂?インデックスを使用する )( 頂?デ??が VERTEX3D版 )
extern	int			DrawPolygon3DToShader_UseVertexBuffer(           int VertexBufHandle ) ;																															// シェ???を使って３Ｄ?リゴンを?画する( 頂?バッフ?使用版 )
extern	int			DrawPolygonIndexed3DToShader_UseVertexBuffer(    int VertexBufHandle, int IndexBufHandle ) ;																										// シェ???を使って３Ｄ?リゴンを?画する( 頂?バッフ?とインデックスバッフ?使用版 )
extern	int			DrawPrimitive3DToShader_UseVertexBuffer(         int VertexBufHandle,                     int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;													// シェ???を使って３Ｄプリ?ティブを?画する( 頂?バッフ?使用版 )
extern	int			DrawPrimitive3DToShader_UseVertexBuffer2(        int VertexBufHandle,                     int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int StartVertex, int UseVertexNum ) ;				// シェ???を使って３Ｄプリ?ティブを?画する( 頂?バッフ?使用版 )
extern	int			DrawPrimitiveIndexed3DToShader_UseVertexBuffer(  int VertexBufHandle, int IndexBufHandle, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;													// シェ???を使って３Ｄプリ?ティブを?画する( 頂?バッフ?とインデックスバッフ?使用版 )
extern	int			DrawPrimitiveIndexed3DToShader_UseVertexBuffer2( int VertexBufHandle, int IndexBufHandle, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int BaseVertex, int StartVertex, int UseVertexNum, int StartIndex, int UseIndexNum ) ;		// シェ???を使って３Ｄプリ?ティブを?画する( 頂?バッフ?とインデックスバッフ?使用版 )

// シェ???用定数バッフ?関係関数
extern	int			InitShaderConstantBuffer(		void ) ;																					// 全てのシェ???用定数バッフ?ハンドルを削除する
extern	int			CreateShaderConstantBuffer(		int BufferSize ) ;																			// シェ???用定数バッフ?ハンドルを初期化する
extern	int			DeleteShaderConstantBuffer(		int SConstBufHandle ) ;																		// シェ???用定数バッフ?ハンドルを削除する
extern	void *		GetBufferShaderConstantBuffer(	int SConstBufHandle ) ;																		// シェ???用定数バッフ?ハンドルの定数バッフ?のアドレスを取得する
extern	int			UpdateShaderConstantBuffer(		int SConstBufHandle ) ;																		// シェ???用定数バッフ?ハンドルの定数バッフ?への変更を適用する
extern	int			SetShaderConstantBuffer(		int SConstBufHandle, int TargetShader /* DX_SHADERTYPE_VERTEX など */ , int Slot ) ;		// シェ???用定数バッフ?ハンドルの定数バッフ?を指定のシェ???の指定のスロットにセットする

// フィル??関係関数
#ifndef DX_NON_FILTER
extern	int			SetGraphFilterBltBlendMode( int BlendMode /* DX_BLENDMODE_ALPHA など */ ) ;												// GraphFilterBlt や GraphBlendBlt の結果を?送先に?送する際のブレンドモ?ドを設定する( 現状で対応しているのは DX_BLENDMODE_NOBLEND と DX_BLENDMODE_ALPHA のみ )
extern	int			SetGraphBlendScalingFilterMode( int IsBilinearFilter ) ;																// GraphBlend で GrHandle と BlendGrHandle のサイズが異なる場合に適用される拡大フィル??モ?ドを設定する( IsBilinearFilter  TRUE:バイリニアフィル??(デフォルト)  FALSE:ニアレストフィル?? )

extern	int			GraphFilter(         int    GrHandle,                                                                                                               int FilterType /* DX_GRAPH_FILTER_GAUSS 等 */ , ... ) ;		// 画像にフィル??処理を行う
extern	int			GraphFilterBlt(      int SrcGrHandle, int DestGrHandle,                                                                                             int FilterType /* DX_GRAPH_FILTER_GAUSS 等 */ , ... ) ;		// 画像のフィル??付き?送を行う
extern	int			GraphFilterRectBlt(  int SrcGrHandle, int DestGrHandle, int SrcX1, int SrcY1, int SrcX2, int SrcY2, int DestX,  int DestY,                          int FilterType /* DX_GRAPH_FILTER_GAUSS 等 */ , ... ) ;		// 画像のフィル??付き?送を行う( 矩?指定 )
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_MONO, int Cb = 青色差( -255 ? 255 ), int Cr = 赤色差( -255 ? 255 ) ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_GAUSS, int PixelWidth = 使用ピクセル幅( 8 , 16 , 32 の何れか ), int Param = ぼかしパラメ??( 100 で約1ピクセル分の幅 ) ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_DOWN_SCALE, int DivNum = 元のサイズの何分の１か、という値( 1 , 2 , 4 , 8 の何れか ) ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_BRIGHT_CLIP, int CmpType = クリップ?イプ( DX_CMP_LESS:CmpParam以下をクリップ  又は  DX_CMP_GREATER:CmpParam以上をクリップ ), int CmpParam = クリップパラメ??( 0 ? 255 ), int ClipFillFlag = クリップしたピクセルを塗りつぶすかどうか( TRUE:塗りつぶす  FALSE:塗りつぶさない ), unsigned int ClipFillColor = クリップしたピクセルに塗る色値( GetColor で取得する )( ClipFillFlag が FALSE の場合は使用しない ), int ClipFillAlpha = クリップしたピクセルに塗るα値( 0 ? 255 )( ClipFillFlag が FALSE の場合は使用しない ) ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_BRIGHT_SCALE, int MinBright = 変換後に?っ暗になる明るさ( 0 ? 255 ), int MaxBright = 変換後に?っ白になる明るさ( 0 ? 255 ) ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_HSB, int HueType = Hue の意味( 0:相対値  1:絶対値 ), int Hue = 色相パラメ??( HueType が 0 の場合 = ピクセルの色相に対する相対値( -180 ? 180 )   HueType が 1 の場合 = 色相の絶対値( 0 ? 360 ) ), int Saturation = 彩度( -255 ? ), int Bright = 輝度( -255 ? 255 ) ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_INVERT ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_LEVEL, int Min = 変換元の下限値( 0 ? 255 ), int Max = 変換元の上限値( 0 ? 255 ), int Gamma = ガン?値( 100 でガン?補正無し、0 とそれ以下の値は不可 ), int AfterMin = 変換後の最低値( 0 ? 255 ), int AfterMax = 変換後の最大値( 0 ? 255 ) ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_TWO_COLOR, int Threshold = ?値( 0 ? 255 ), unsigned int LowColor = ?値より値が低かったピクセルの変換後の色値( GetColor で取得する ), int LowAlpha = ?値より値が低かったピクセルの変換後のα値( 0 ? 255 ), unsigned int HighColor = ?値より値が高かったピクセルの変換後の色値( GetColor で取得する ), int HighAlpha = ?値より値が高かったピクセルの変換後のα値( 0 ? 255 ) ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_GRADIENT_MAP, int MapGrHandle = グラデ?ション?ップのグラフィックハンドル( 元画像の輝度からグラデ?ション?ップ画像の x 座標を算出しますので縦幅は1dotでもＯＫ ), int Reverse = グラデ?ション?ップ左右反?フラグ( TRUE : グラデ?ション?ップを左右反?して使う  FALSE : 左右反?しない ) ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_REPLACEMENT, int TargetR, int TargetG, int TargetB, int TargetA = 置換対象の色( 0?255 ), int R, int G, int B, int A = 置換後の色( 0?255 ) ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_BILATERAL_BLUR ) ;			// バイラテラルブラ?
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_PREMUL_ALPHA ) ;			// 通常のアルフ??ャンネル付き画像を乗算済みアルフ?画像に変換するフィル?
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_INTERP_ALPHA ) ;			// 乗算済みα画像を通常のアルフ??ャンネル付き画像に変換するフィル?
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_YUV_TO_RGB ) ;				// YUVカラ?をRGBカラ?に変換するフィル?
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_Y2UV1_TO_RGB, int UVGrHandle = YUVカラ?のUV成分のみで、且つYに対して解像度が半分( 若しくは 4 分の 1 )のグラフィックハンドル( U=R, V=G ) ) ;		// YUVカラ?をRGBカラ?に変換するフィル?( UV成分が Y成分の半分・又は４分の１( 横・縦片方若しくは両方 )の解像度しかない場合用 )
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_YUV_TO_RGB_RRA ) ;				// YUVカラ?をRGBカラ?に変換するフィル?( 且つ右側半分のRの値をアルフ?値として扱う )
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_Y2UV1_TO_RGB_RRA, int UVGrHandle = YUVカラ?のUV成分のみで、且つYに対して解像度が半分( 若しくは 4 分の 1 )のグラフィックハンドル( U=R, V=G ) ) ;		// YUVカラ?をRGBカラ?に変換するフィル?( UV成分が Y成分の半分・又は４分の１( 横・縦片方若しくは両方 )の解像度しかない場合用 )( 且つ右側半分のRの値をアルフ?値として扱う )
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_BICUBIC_SCALE, int DestSizeX = スケ?リング後の横ピクセル数, int DestSizeY = スケ?リング後の縦ピクセル数 ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_LANCZOS3_SCALE, int DestSizeX = スケ?リング後の横ピクセル数, int DestSizeY = スケ?リング後の縦ピクセル数 ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_SSAO, int ColorWithDepthGrHandle = Zバッフ?が含まれるカラ?バッフ?, float KernelRadius = 遮蔽物を調べる範囲, float MinDistance = 遮蔽物判定する最小?度値, float MaxDistance = 遮蔽物判定する最大?度値, float Strength = 遮蔽物の影響の強さ, int OcclusionColor = オクリュ?ジョンカラ?, float OcclusionPower = オクリュ?ジョンカラ?の強さ ) ;
//		int			GraphFilter( int GrHandle, int FilterType = DX_GRAPH_FILTER_FLOAT_COLOR_SCALE, float ColorScaleR = 赤成分へのスケ?リング値, float ColorScaleG = 緑成分へのスケ?リング値, float ColorScaleB = 青成分へのスケ?リング値, float ColorScaleA = α成分へのスケ?リング値, float ColorPreSubR = 赤成分にスケ?リングを?ける前に減算する値, float ColorPreSubG = 緑成分にスケ?リングを?ける前に減算する値, float ColorPreSubB = 青成分にスケ?リングを?ける前に減算する値, float ColorPreSubA = α成分にスケ?リングを?ける前に減算する値 ) ;

extern	int			GraphBlend(         int    GrHandle, int BlendGrHandle,                                                                                                                                         int BlendRatio /* ブレンド効果の影響度( 0:０％  255:１００％ ) */ , int BlendType /* DX_GRAPH_BLEND_ADD 等 */ , ... ) ;	// 二つの画像をブレンドする
extern	int			GraphBlendBlt(      int SrcGrHandle, int BlendGrHandle, int DestGrHandle,                                                                                                                       int BlendRatio /* ブレンド効果の影響度( 0:０％  255:１００％ ) */ , int BlendType /* DX_GRAPH_BLEND_ADD 等 */ , ... ) ;	// 二つの画像をブレンドして結果を指定の画像に出力する
extern	int			GraphBlendRectBlt(  int SrcGrHandle, int BlendGrHandle, int DestGrHandle, int SrcX1, int SrcY1, int SrcX2, int SrcY2, int BlendX,  int BlendY,                            int DestX, int DestY, int BlendRatio /* ブレンド効果の影響度( 0:０％  255:１００％ ) */ , int BlendType /* DX_GRAPH_BLEND_ADD 等 */ , ... ) ;	// 二つの画像をブレンドして結果を指定の画像に出力する( 矩?指定 )
extern	int			GraphBlendRectBlt2( int SrcGrHandle, int BlendGrHandle, int DestGrHandle, int SrcX1, int SrcY1, int SrcX2, int SrcY2, int BlendX1, int BlendY1, int BlendX2, int BlendY2, int DestX, int DestY, int BlendRatio /* ブレンド効果の影響度( 0:０％  255:１００％ ) */ , int BlendType /* DX_GRAPH_BLEND_ADD 等 */ , ... ) ;	// 二つの画像をブレンドして結果を指定の画像に出力する( 矩?指定、ブレンド画像も矩?指定 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_NORMAL ) ; // 通常
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_RGBA_SELECT_MIX, int SelectR = ( 出力の赤分となる成分 DX_RGBA_SELECT_SRC_R 等 ), int SelectG = ( 出力の緑成分となる成分 DX_RGBA_SELECT_SRC_R 等 ), int SelectB = ( 出力の青成分となる成分 DX_RGBA_SELECT_SRC_R 等 ), int SelectA = ( 出力のα成分となる成分 DX_RGBA_SELECT_SRC_R 等 ) ) ;	// RGBAの要素を選択して合成
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_MULTIPLE ) ;	// 乗算
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_DIFFERENCE ) ;	// 減算
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_ADD ) ;		  	// 加算
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_SCREEN ) ;	// スクリ?ン
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_OVERLAY ) ;	// オ?バ?レイ
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_DODGE ) ;	// 覆い焼き
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_BURN ) ;	// 焼き込み
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_DARKEN ) ;	// 比較(暗)
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_LIGHTEN ) ;	// 比較(明)
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_SOFTLIGHT ) ;	// ?フトライト
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_HARDLIGHT ) ;	// ハ?ドライト
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_EXCLUSION ) ;	// 除外
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_NORMAL_ALPHACH ) ;	// α?ャンネル付き画像の通常合成
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_ADD_ALPHACH ) ;	// α?ャンネル付き画像の加算合成
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_MULTIPLE_A_ONLY ) ;	// アルフ??ャンネルのみの乗算
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_MASK ) ;	// ?スク( SrcGrHandle に BlendGrHandle を通常?画した上で、SrcGrHandle の A を優先 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_NORMAL ) ; // 通常( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_RGBA_SELECT_MIX ) ; // RGBAの要素を選択して合成( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_MULTIPLE ) ; // 乗算( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_DIFFERENCE ) ; // 減算( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_ADD ) ; // 加算( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_SCREEN ) ; // スクリ?ン( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_OVERLAY ) ; // オ?バ?レイ( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_DODGE ) ; // 覆い焼き( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_BURN ) ; // 焼き込み( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_DARKEN ) ; // 比較(暗)( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_LIGHTEN ) ; // 比較(明)( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_SOFTLIGHT ) ; // ?フトライト( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_HARDLIGHT ) ; // ハ?ドライト( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_EXCLUSION ) ; // 除外( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_NORMAL_ALPHACH ) ; // α?ャンネル付き画像の通常合成( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_ADD_ALPHACH ) ; // α?ャンネル付き画像の加算合成( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_MULTIPLE_A_ONLY ) ;	// アルフ??ャンネルのみの乗算( 乗算済みα画像用 )
//		int			GraphBlend( int GrHandle, int BlendGrHandle, int BlendRatio, int BlendType = DX_GRAPH_BLEND_PMA_MASK ) ;	// ?スク( DX_GRAPH_BLEND_MASK の 乗算済みα画像用 )
#endif // DX_NON_FILTER

#ifndef DX_NON_MOVIE
// ??ビ?グラフィック関係関数
extern	int			PlayMovie(							const TCHAR *FileName,                        int ExRate, int PlayType ) ;	// 動画フ?イルの再生
extern	int			PlayMovieWithStrLen(				const TCHAR *FileName, size_t FileNameLength, int ExRate, int PlayType ) ;	// 動画フ?イルの再生
extern	int			GetMovieImageSize_File(             const TCHAR *FileName,                        int *SizeX, int *SizeY ) ;	// 動画フ?イルの横ピクセル数と縦ピクセル数を取得する
extern	int			GetMovieImageSize_File_WithStrLen(  const TCHAR *FileName, size_t FileNameLength, int *SizeX, int *SizeY ) ;	// 動画フ?イルの横ピクセル数と縦ピクセル数を取得する
extern	int			GetMovieImageSize_Mem(              const void *FileImage, int FileImageSize, int *SizeX, int *SizeY ) ;		// メモリ上に展開された動画フ?イルの横ピクセル数と縦ピクセル数を取得する
extern	int			OpenMovieToGraph(					const TCHAR *FileName,                        int FullColor DEFAULTPARAM( = TRUE ) ) ;		// 動画フ?イルを開く
extern	int			OpenMovieToGraphWithStrLen(			const TCHAR *FileName, size_t FileNameLength, int FullColor DEFAULTPARAM( = TRUE ) ) ;		// 動画フ?イルを開く
extern 	int			PlayMovieToGraph(					int GraphHandle, int PlayType DEFAULTPARAM( = DX_PLAYTYPE_BACK ) , int SysPlay DEFAULTPARAM( = 0 ) ) ;		// 動画フ?イルを使用するグラフィックハンドルの動画フ?イルの再生を開始する
extern 	int			PauseMovieToGraph(					int GraphHandle, int SysPause DEFAULTPARAM( = 0 ) ) ;										// 動画フ?イルを使用するグラフィックハンドルの動画フ?イルの再生を停?する
extern	int			AddMovieFrameToGraph(				int GraphHandle, unsigned int FrameNum ) ;									// 動画フ?イルの再生フレ??を進める、戻すことは出来ない( 動画フ?イルが停?状態で、且つ Ogg Theora のみ有効 )
extern	int			SeekMovieToGraph(					int GraphHandle, int Time ) ;												// 動画フ?イルの再生位置を設定する(?リ秒単位)
extern	int			SetPlaySpeedRateMovieToGraph(		int GraphHandle, double SpeedRate ) ;										// 動画フ?イルの再生速度を設定する( 1.0 = 等?速  2.0 = ２?速 )、一部のフ?イルフォ??ットのみで有効な??です
extern 	int			GetMovieStateToGraph(				int GraphHandle ) ;															// 動画フ?イルの再生状態を得る
extern	int			SetMovieVolumeToGraph(				int Volume, int GraphHandle ) ;												// 動画フ?イルの音量を設定する(0?10000)
extern	int			GetMovieVolumeToGraph(				            int GraphHandle ) ;												// 動画フ?イルの音量を取得する(0?10000)
extern	int			ChangeMovieVolumeToGraph(			int Volume, int GraphHandle ) ;												// 動画フ?イルの音量を設定する(0?255)
extern	int			GetMovieVolumeToGraph2(				            int GraphHandle ) ;												// 動画フ?イルの音量を取得する(0?255)
extern	const BASEIMAGE* GetMovieBaseImageToGraph(		int GraphHandle, int *ImageUpdateFlag DEFAULTPARAM( = NULL ) , int ImageUpdateFlagSetOnly DEFAULTPARAM( = FALSE ) ) ;	// 動画フ?イルの基?イメ?ジデ??を取得する( ImageUpdateFlag に int ?変数のアドレスを渡すと、イメ?ジが更新された場合は 1 が、更新されていない場合は 0 が格?されます、 ImageUpdateFlagSetOnly を TRUE にすると戻り値の BASEIMAGE は有効な画像デ??ではなくなりますが、BASEIMAGE の更新処理が行われませんので、ImageUpdateFlag を利用して画像が更新されたかどうかだけを?ェックしたい場合は TRUE にしてください )
extern	int			GetMovieTotalFrameToGraph(			int GraphHandle ) ;															// 動画フ?イルの総フレ??数を得る( Ogg Theora と mp4 でのみ有効 )
extern	int			TellMovieToGraph(					int GraphHandle ) ;															// 動画フ?イルの再生位置を取得する(?リ秒単位)
extern	int			TellMovieToGraphToFrame(			int GraphHandle ) ;															// 動画フ?イルの再生位置を取得する(フレ??単位)
extern	int			SeekMovieToGraphToFrame(			int GraphHandle, int Frame ) ;												// 動画フ?イルの再生位置を設定する(フレ??単位)
extern	LONGLONG	GetOneFrameTimeMovieToGraph(		int GraphHandle ) ;															// 動画フ?イルの１フレ??あたりの時間を取得する(戻り値：１フレ??の時間(単位:?イクロ秒))
extern	int			GetLastUpdateTimeMovieToGraph(		int GraphHandle ) ;															// 動画フ?イルのイメ?ジを最後に更新した時間を得る(?リ秒単位)
extern	int			UpdateMovieToGraph(					int GraphHandle ) ;															// 動画フ?イルの更新処理を行う
extern	int			SetMovieRightImageAlphaFlag(		int Flag ) ;																// 読み込む動画フ?イル映像の右半分の赤成分をα情報として扱うかどうかをセットする( TRUE:α情報として扱う  FALSE:α情報として扱わない( デフォルト ) )
extern	int			SetMovieColorA8R8G8B8Flag(			int Flag ) ;																// 読み込む動画フ?イルが32bitカラ?だった場合、A8R8G8B8 ?式として扱うかどうかをセットする、32bitカラ?ではない動画フ?イルに対しては無効( Flag  TRUE:A8R8G8B8として扱う  FALSE:X8R8G8B8として扱う( デフォルト ) )
extern	int			SetMovieUseYUVFormatSurfaceFlag(	int Flag ) ;																// ＹＵＶフォ??ットのサ?フェスが使用できる場合はＹＵＶフォ??ットのサ?フェスを使用するかどうかを設定する( TRUE:使用する( デフォルト ) FALSE:ＲＧＢフォ??ットのサ?フェスを使用する )
#endif // DX_NON_MOVIE

// カメラ関係関数
extern	int			SetCameraNearFar(					float  Near, float  Far ) ;												// カメラの Nearクリップ面と Farクリップ面の距離を設定する
extern	int			SetCameraNearFarD(					double Near, double Far ) ;												// カメラの Nearクリップ面と Farクリップ面の距離を設定する
extern	int			SetCameraPositionAndTarget_UpVecY(  VECTOR    Position, VECTOR   Target ) ;									// カメラの視?、注視?、アップベクトルを設定する( アップベクトルはＹ軸方向から導き出す )
extern	int			SetCameraPositionAndTarget_UpVecYD( VECTOR_D  Position, VECTOR_D Target ) ;									// カメラの視?、注視?、アップベクトルを設定する( アップベクトルはＹ軸方向から導き出す )
extern	int			SetCameraPositionAndTargetAndUpVec( VECTOR    Position, VECTOR   TargetPosition, VECTOR   UpVector ) ;		// カメラの視?、注視?、アップベクトルを設定する
extern	int			SetCameraPositionAndTargetAndUpVecD( VECTOR_D Position, VECTOR_D TargetPosition, VECTOR_D UpVector ) ;		// カメラの視?、注視?、アップベクトルを設定する
extern	int			SetCameraPositionAndAngle(			VECTOR   Position, float  VRotate, float  HRotate, float  TRotate ) ;	// カメラの視?、注視?、アップベクトルを設定する( 注視?とアップベクトルは垂直回?角度、水平回?角度、捻り回?角度から導き出す )
extern	int			SetCameraPositionAndAngleD(			VECTOR_D Position, double VRotate, double HRotate, double TRotate ) ;	// カメラの視?、注視?、アップベクトルを設定する( 注視?とアップベクトルは垂直回?角度、水平回?角度、捻り回?角度から導き出す )
extern	int			SetCameraViewMatrix(				MATRIX   ViewMatrix ) ;													// ビュ?行列を直接設定する
extern	int			SetCameraViewMatrixD(				MATRIX_D ViewMatrix ) ;													// ビュ?行列を直接設定する
extern	int			SetCameraScreenCenter(				float x, float y ) ;													// 画面上におけるカメラが見ている映像の中心の座標を設定する
extern	int			SetCameraScreenCenterD(				double x, double y ) ;													// 画面上におけるカメラが見ている映像の中心の座標を設定する

extern	int			SetupCamera_Perspective(			float  Fov ) ;															// 遠近?カメラをセットアップする
extern	int			SetupCamera_PerspectiveD(			double Fov ) ;															// 遠近?カメラをセットアップする
extern	int			SetupCamera_Ortho(					float  Size ) ;															// 正射影カメラをセットアップする
extern	int			SetupCamera_OrthoD(					double Size ) ;															// 正射影カメラをセットアップする
extern	int			SetupCamera_ProjectionMatrix(		MATRIX   ProjectionMatrix ) ;											// 射影行列を直接設定する
extern	int			SetupCamera_ProjectionMatrixD(		MATRIX_D ProjectionMatrix ) ;											// 射影行列を直接設定する
extern	int			SetCameraDotAspect(					float  DotAspect ) ;													// カメラのドットアスペクト比を設定する
extern	int			SetCameraDotAspectD(				double DotAspect ) ;													// カメラのドットアスペクト比を設定する

extern	int			CheckCameraViewClip(				VECTOR   CheckPos ) ;													// 指定の座標がカメラの視界に入っているかどうかを判定する( 戻り値 TRUE:視界に入っていない  FALSE:視界に入っている )
extern	int			CheckCameraViewClipD(				VECTOR_D CheckPos ) ;													// 指定の座標がカメラの視界に入っているかどうかを判定する( 戻り値 TRUE:視界に入っていない  FALSE:視界に入っている )
extern	int			CheckCameraViewClip_Dir(			VECTOR   CheckPos ) ;													// 指定の座標がカメラの視界に入っているかどうかを判定する、戻り値で外れている方向も知ることができる( 戻り値 0:視界に入っている  0以外:視界に入っていない( DX_CAMERACLIP_LEFT や DX_CAMERACLIP_RIGHT が or 演算で混合されたもの、and 演算で方向を確認できる ) )
extern	int			CheckCameraViewClip_DirD(			VECTOR_D CheckPos ) ;													// 指定の座標がカメラの視界に入っているかどうかを判定する、戻り値で外れている方向も知ることができる( 戻り値 0:視界に入っている  0以外:視界に入っていない( DX_CAMERACLIP_LEFT や DX_CAMERACLIP_RIGHT が or 演算で混合されたもの、and 演算で方向を確認できる ) )
extern	int			CheckCameraViewClip_Box(			VECTOR   BoxPos1, VECTOR   BoxPos2 ) ;									// 二つの座標で?される?ックスがカメラの視界に入っているかどうかを判定する( 戻り値 TRUE:視界に入っていない  FALSE:視界に入っている )
extern	int			CheckCameraViewClip_BoxD(			VECTOR_D BoxPos1, VECTOR_D BoxPos2 ) ;									// 二つの座標で?される?ックスがカメラの視界に入っているかどうかを判定する( 戻り値 TRUE:視界に入っていない  FALSE:視界に入っている )

extern	float		GetCameraNear(						void ) ;																// カメラの Near クリップ面の距離を取得する
extern	double		GetCameraNearD(						void ) ;																// カメラの Near クリップ面の距離を取得する
extern	float		GetCameraFar(						void ) ;																// カメラの Far クリップ面の距離を取得する
extern	double		GetCameraFarD(						void ) ;																// カメラの Far クリップ面の距離を取得する

extern	VECTOR		GetCameraPosition(					void ) ;																// カメラの位置を取得する
extern	VECTOR_D	GetCameraPositionD(					void ) ;																// カメラの位置を取得する
extern	VECTOR		GetCameraTarget(					void ) ;																// カメラの注視?を取得する
extern	VECTOR_D	GetCameraTargetD(					void ) ;																// カメラの注視?を取得する
extern	VECTOR		GetCameraUpVector(					void ) ;																// カメラの正面方向に垂直な上方向のベクトルを取得する
extern	VECTOR_D	GetCameraUpVectorD(					void ) ;																// カメラの正面方向に垂直な上方向のベクトルを取得する
extern	VECTOR		GetCameraDownVector(				void ) ;																// カメラの正面方向に垂直な下方向のベクトルを取得する
extern	VECTOR_D	GetCameraDownVectorD(				void ) ;																// カメラの正面方向に垂直な下方向のベクトルを取得する
extern	VECTOR		GetCameraRightVector(				void ) ;																// カメラの正面方向に垂直な右方向のベクトルを取得する
extern	VECTOR_D	GetCameraRightVectorD(				void ) ;																// カメラの正面方向に垂直な右方向のベクトルを取得する
extern	VECTOR		GetCameraLeftVector(				void ) ;																// カメラの正面方向に垂直な左方向のベクトルを取得する
extern	VECTOR_D	GetCameraLeftVectorD(				void ) ;																// カメラの正面方向に垂直な左方向のベクトルを取得する
extern	VECTOR		GetCameraFrontVector(				void ) ;																// カメラの正面方向のベクトルを取得する
extern	VECTOR_D	GetCameraFrontVectorD(				void ) ;																// カメラの正面方向のベクトルを取得する
extern	VECTOR		GetCameraBackVector(				void ) ;																// カメラの後ろ方向のベクトルを取得する
extern	VECTOR_D	GetCameraBackVectorD(				void ) ;																// カメラの後ろ方向のベクトルを取得する
extern	float		GetCameraAngleHRotate(				void ) ;																// カメラの水平方向の向きを取得する
extern	double		GetCameraAngleHRotateD(				void ) ;																// カメラの水平方向の向きを取得する
extern	float		GetCameraAngleVRotate(				void ) ;																// カメラの垂直方向の向きを取得する
extern	double		GetCameraAngleVRotateD(				void ) ;																// カメラの垂直方向の向きを取得する
extern	float		GetCameraAngleTRotate(				void ) ;																// カメラの向きの捻り角度を取得する
extern	double		GetCameraAngleTRotateD(				void ) ;																// カメラの向きの捻り角度を取得する

extern	MATRIX		GetCameraViewMatrix(				void ) ;																// ビュ?行列を取得する
extern	MATRIX_D	GetCameraViewMatrixD(				void ) ;																// ビュ?行列を取得する
extern	MATRIX		GetCameraBillboardMatrix(			void ) ;																// ビル??ド行列を取得する
extern	MATRIX_D	GetCameraBillboardMatrixD(			void ) ;																// ビル??ド行列を取得する
extern	int			GetCameraScreenCenter(				float  *x, float  *y ) ;												// 画面上におけるカメラが見ている映像の中心の座標を取得する
extern	int			GetCameraScreenCenterD(				double *x, double *y ) ;												// 画面上におけるカメラが見ている映像の中心の座標を取得する
extern	float		GetCameraFov(						void ) ;																// カメラの視野角を取得する
extern	double		GetCameraFovD(						void ) ;																// カメラの視野角を取得する
extern	float		GetCameraSize(						void ) ;																// カメラの視野サイズを取得する
extern	double		GetCameraSizeD(						void ) ;																// カメラの視野サイズを取得する
extern	MATRIX		GetCameraProjectionMatrix(			void ) ;																// 射影行列を取得する
extern	MATRIX_D	GetCameraProjectionMatrixD(			void ) ;																// 射影行列を取得する
extern	float		GetCameraDotAspect(					void ) ;																// カメラのドットアスペクト比を得る
extern	double		GetCameraDotAspectD(				void ) ;																// カメラのドットアスペクト比を得る
extern	MATRIX		GetCameraViewportMatrix(			void ) ;																// ビュ???ト行列を取得する
extern	MATRIX_D	GetCameraViewportMatrixD(			void ) ;																// ビュ???ト行列を取得する
extern	MATRIX		GetCameraAPIViewportMatrix(			void ) ;																// Direct3Dで自動適用されるビュ???ト行列を取得する
extern	MATRIX_D	GetCameraAPIViewportMatrixD(		void ) ;																// Direct3Dで自動適用されるビュ???ト行列を取得する

// ライト関係関数
extern	int			SetUseLighting(				int Flag ) ;																	// ライティングを使用するかどうかを設定する
extern	int			SetMaterialUseVertDifColor( int UseFlag ) ;																	// ３Ｄ?画のライティング計算で頂?カラ?のディフュ?ズカラ?を使用するかどうかを設定する
extern	int			SetMaterialUseVertSpcColor( int UseFlag ) ;																	// ３Ｄ?画のライティング計算で頂?カラ?のスペキュラカラ?を使用するかどうかを設定する
extern	int			SetMaterialParam(			MATERIALPARAM Material ) ;														// ３Ｄ?画のライティング計算で使用する?テリアルパラメ??を設定する
extern	int			SetUseSpecular(				int UseFlag ) ;																	// ３Ｄ?画にスペキュラを使用するかどうかを設定する
extern	int			SetGlobalAmbientLight(		COLOR_F Color ) ;																// グロ?バルアンビエントライトカラ?を設定する
extern	int			SetUseLightAngleAttenuation( int UseFlag ) ;																// ３Ｄ?画のライティング計算で角度減衰を行うかどうかを設定する( TRUE:角度減衰を行う( デフォルト )  FALSE:角度減衰を行わない )
extern	int			SetUseHalfLambertLighting(	 int UseFlag ) ;																	// ３Ｄ?画のライティング計算でハ?フランバ?トを使用するかどうかを設定する( TRUE:ハ?フランバ?トを使用する  FALSE:ハ?フランバ?トを使用しない( デフォルト ) )

extern	int			ChangeLightTypeDir(			VECTOR Direction ) ;															// デフォルトライトの?イプをディレクショナルライトにする
extern	int			ChangeLightTypeSpot(		VECTOR Position, VECTOR Direction, float OutAngle, float InAngle, float Range, float Atten0, float Atten1, float Atten2 ) ;	// デフォルトライトの?イプをス?ットライトにする
extern	int			ChangeLightTypePoint(		VECTOR Position, float Range, float Atten0, float Atten1, float Atten2 ) ;		// デフォルトライトの?イプを?イントライトにする
extern	int			GetLightType(				void ) ;																		// デフォルトライトの?イプを取得する( 戻り値は DX_LIGHTTYPE_DIRECTIONAL 等 )
extern	int			SetLightEnable(				int EnableFlag ) ;																// デフォルトライトを使用するかどうかを設定する
extern	int			GetLightEnable(				void ) ;																		// デフォルトライトを使用するかどうかを取得する( 戻り値?TRUE:有効  FALSE:無効 )
extern	int			SetLightDifColor(			COLOR_F Color ) ;																// デフォルトライトのディフュ?ズカラ?を設定する
extern	COLOR_F		GetLightDifColor(			void ) ;																		// デフォルトライトのディフュ?ズカラ?を取得する
extern	int			SetLightSpcColor(			COLOR_F Color ) ;																// デフォルトライトのスペキュラカラ?を設定する
extern	COLOR_F		GetLightSpcColor(			void ) ;																		// デフォルトライトのスペキュラカラ?を取得する
extern	int			SetLightAmbColor(			COLOR_F Color ) ;																// デフォルトライトのアンビエントカラ?を設定する
extern	COLOR_F		GetLightAmbColor(			void ) ;																		// デフォルトライトのアンビエントカラ?を取得する
extern	int			SetLightDirection(			VECTOR Direction ) ;															// デフォルトライトの方向を設定する
extern	VECTOR		GetLightDirection(			void ) ;																		// デフォルトライトの方向を取得する
extern	int			SetLightPosition(			VECTOR Position ) ;																// デフォルトライトの位置を設定する
extern	VECTOR		GetLightPosition(			void ) ;																		// デフォルトライトの位置を取得する
extern	int			SetLightRangeAtten(			float Range, float Atten0, float Atten1, float Atten2 ) ;						// デフォルトライトの距離減衰パラメ??を設定する( 有効距離、距離減衰係数０、１、２ )
extern	int			GetLightRangeAtten(			float *Range, float *Atten0, float *Atten1, float *Atten2 )	;					// デフォルトライトの距離減衰パラメ??を取得する( 有効距離、距離減衰係数０、１、２ )
extern	int			SetLightAngle(				float OutAngle, float InAngle ) ;												// デフォルトライトのス?ットライトのパラメ??を設定する( 外部コ?ン角度、内部コ?ン角度 )
extern	int			GetLightAngle(				float *OutAngle, float *InAngle ) ;												// デフォルトライトのス?ットライトのパラメ??を取得する( 外部コ?ン角度、内部コ?ン角度 )
extern	int			SetLightUseShadowMap(		int SmSlotIndex, int UseFlag ) ;												// デフォルトライトに SetUseShadowMap で指定したシャドウ?ップを適用するかどうかを設定する( SmSlotIndex:シャドウ?ップスロット( SetUseShadowMap の第一引数に設定する値 ) UseFlag:適用にするかどうかのフラグ( TRUE:適用する( デフォルト )  FALSE:適用しない ) )

extern	int			CreateDirLightHandle(       VECTOR Direction ) ;															// ディレクショナルライトハンドルを作成する
extern	int			CreateSpotLightHandle(      VECTOR Position, VECTOR Direction, float OutAngle, float InAngle, float Range, float Atten0, float Atten1, float Atten2 ) ;	// ス?ットライトハンドルを作成する
extern	int			CreatePointLightHandle(     VECTOR Position, float Range, float Atten0, float Atten1, float Atten2 ) ;		// ?イントライトハンドルを作成する
extern	int			DeleteLightHandle(          int LHandle ) ;																	// ライトハンドルを削除する
extern	int			DeleteLightHandleAll(       void ) ;																		// ライトハンドルを全て削除する
extern	int			SetLightTypeHandle(         int LHandle, int LightType ) ;													// ライトハンドルのライトの?イプを変更する( DX_LIGHTTYPE_DIRECTIONAL 等 )
extern	int			SetLightEnableHandle(       int LHandle, int EnableFlag ) ;													// ライトハンドルのライト効果の有効、無効を設定する( TRUE:有効  FALSE:無効 )
extern	int			SetLightDifColorHandle(     int LHandle, COLOR_F Color ) ;													// ライトハンドルのライトのディフュ?ズカラ?を設定する
extern	int			SetLightSpcColorHandle(     int LHandle, COLOR_F Color ) ;													// ライトハンドルのライトのスペキュラカラ?を設定する
extern	int			SetLightAmbColorHandle(     int LHandle, COLOR_F Color ) ;													// ライトハンドルのライトのアンビエントカラ?を設定する
extern	int			SetLightDirectionHandle(    int LHandle, VECTOR Direction ) ;												// ライトハンドルのライトの方向を設定する
extern	int			SetLightPositionHandle(     int LHandle, VECTOR Position ) ;												// ライトハンドルのライトの位置を設定する
extern	int			SetLightRangeAttenHandle(   int LHandle, float Range, float Atten0, float Atten1, float Atten2 ) ;			// ライトハンドルのライトの距離減衰パラメ??を設定する( 有効距離、距離減衰係数０、１、２ )
extern	int			SetLightAngleHandle(        int LHandle, float OutAngle, float InAngle ) ;									// ライトハンドルのライトのス?ットライトのパラメ??を設定する( 外部コ?ン角度、内部コ?ン角度 )
extern	int			SetLightUseShadowMapHandle(	int LHandle, int SmSlotIndex, int UseFlag ) ;									// ライトハンドルのライトに SetUseShadowMap で指定したシャドウ?ップを適用するかどうかを設定する( SmSlotIndex:シャドウ?ップスロット( SetUseShadowMap の第一引数に設定する値 ) UseFlag:適用にするかどうかのフラグ( TRUE:適用する( デフォルト )  FALSE:適用しない ) )
extern	int			GetLightTypeHandle(         int LHandle ) ;																	// ライトハンドルのライトの?イプを取得する( 戻り値は DX_LIGHTTYPE_DIRECTIONAL 等 )
extern	int			GetLightEnableHandle(       int LHandle ) ;																	// ライトハンドルのライト効果の有効、無効を取得する( TRUE:有効  FALSE:無効 )
extern	COLOR_F		GetLightDifColorHandle(     int LHandle ) ;																	// ライトハンドルのライトのディフュ?ズカラ?を取得する
extern	COLOR_F		GetLightSpcColorHandle(     int LHandle ) ;																	// ライトハンドルのライトのスペキュラカラ?を取得する
extern	COLOR_F		GetLightAmbColorHandle(     int LHandle ) ;																	// ライトハンドルのライトのアンビエントカラ?を取得する
extern	VECTOR		GetLightDirectionHandle(    int LHandle ) ;																	// ライトハンドルのライトの方向を取得する
extern	VECTOR		GetLightPositionHandle(     int LHandle ) ;																	// ライトハンドルのライトの位置を取得する
extern	int			GetLightRangeAttenHandle(   int LHandle, float *Range, float *Atten0, float *Atten1, float *Atten2 ) ;		// ライトハンドルのライトの距離減衰パラメ??を取得する( 有効距離、距離減衰係数０、１、２ )
extern	int			GetLightAngleHandle(        int LHandle, float *OutAngle, float *InAngle ) ;								// ライトハンドルのライトのス?ットライトのパラメ??を取得する( 外部コ?ン角度、内部コ?ン角度 )

extern	int			GetEnableLightHandleNum(	void ) ;																		// 有効になっているライトハンドルの数を取得する
extern	int			GetEnableLightHandle(		int Index ) ;																	// 有効になっているライトハンドルを取得する

// 色情報取得用関数
extern	int			GetTexFormatIndex(			const IMAGEFORMATDESC *Format ) ;												// テクス?ャフォ??ットのインデックスを得る








// DxMask.cpp 関数プロト?イプ宣言

#ifndef DX_NON_MASK

// ?スク関係 
extern	int			CreateMaskScreen(					void ) ;																						// ?スクスクリ?ンを作成する
extern	int			DeleteMaskScreen(					void ) ;																						// ?スクスクリ?ンを削除する
extern	int			DrawMaskToDirectData(				int x, int y, int Width, int Height, const void *MaskData , int TransMode ) ;					// ?スクスクリ?ンにメモリ上のビット?ップデ??を?送する( MaskData は 1byte で 1dot ?す１ラインあたり Width byte のデ??配列の先頭アドレス )
extern	int			DrawFillMaskToDirectData(			int x1, int y1, int x2, int y2,  int Width, int Height, const void *MaskData ) ;				// ?スクスクリ?ンにメモリ上のビット?ップデ??で指定矩?内を塗りつぶすように?送する

extern	int			SetUseMaskScreenFlag(				int ValidFlag ) ;																				// ?スクスクリ?ンを使用するかどうかを設定する( ValidFlag:使用するかどうか( TRUE:使用する  FALSE:使用しない( デフォルト ) )
extern	int			GetUseMaskScreenFlag(				void ) ;																						// ?スクスクリ?ンを使用するかどうかを取得する
extern	int			FillMaskScreen(						int Flag ) ;																					// ?スクスクリ?ンを指定の色で塗りつぶす
extern	int			SetMaskScreenGraph(					int GraphHandle ) ;																				// ?スクスクリ?ンとして使用するグラフィックのハンドルを設定する、-1を渡すと解除( 引数で渡すグラフィックハンドルは MakeScreen で作成した「アルフ??ャンネル付きの?画対象にできるグラフィックハンドル」である必要があります( アルフ??ャンネルが?スクに使用されます ) )
extern	int			SetMaskScreenGraphUseChannel(		int UseChannel /* DX_MASKGRAPH_CH_A 等 */ ) ;													// ?スクスクリ?ンとして使用するグラフィックの、どの?ャンネルを?スクとして使用するかを設定する( デフォルトは DX_MASKGRAPH_CH_A、 尚、DX_MASKGRAPH_CH_A以外を使用する場合はグラフィックスデバイスがシェ???モデル2.0以?に対応している必要があります )

extern	int			InitMask(							void ) ;																						// ?スクハンドルをすべて削除する
extern	int			MakeMask(							int Width, int Height ) ;																		// ?スクハンドルを作成する
extern	int			GetMaskSize(						int *WidthBuf, int *HeightBuf, int MaskHandle ) ;												// ?スクハンドルが持つ?スクイメ?ジのサイズを取得する
extern	int			GetMaskBaseImageInfo(				BASEIMAGE *BaseImage, int MaskHandle ) ;														// ?スクハンドルが持つ?スクイメ?ジのBASEIMAGEデ??を取得する( フォ??ットが変更されるような?作をすると正常に動作しなくなります、あと、ReleaseBaseImage は実行する必要はありません )
extern	int			SetDataToMask(						int Width, int Height, const void *MaskData, int MaskHandle ) ;									// ?スクハンドルにビット?ップイメ?ジを?送する
extern	int			DeleteMask(							int MaskHandle ) ;																				// ?スクハンドルを削除する
extern	int			GraphImageBltToMask(				const BASEIMAGE *BaseImage, int ImageX, int ImageY, int MaskHandle ) ;							// ?スクハンドルにBASEIMAGEデ??を?送する
extern	int			LoadMask(							const TCHAR *FileName                        ) ;																			// 画像フ?イルを読み込み?スクハンドルを作成する
extern	int			LoadMaskWithStrLen(					const TCHAR *FileName, size_t FileNameLength ) ;																			// 画像フ?イルを読み込み?スクハンドルを作成する
extern	int			LoadDivMask(						const TCHAR *FileName,                        int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleArray ) ;	// 画像フ?イルを分割読み込みして?スクハンドルを作成する
extern	int			LoadDivMaskWithStrLen(				const TCHAR *FileName, size_t FileNameLength, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleArray ) ;	// 画像フ?イルを分割読み込みして?スクハンドルを作成する
extern	int			CreateMaskFromMem(					const void *FileImage, int FileImageSize ) ;																				// メモリ上にある画像フ?イルイメ?ジを読み込み?スクハンドルを作成する
extern	int			CreateDivMaskFromMem(				const void *FileImage, int FileImageSize, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleArray ) ;		// メモリ上にある画像フ?イルイメ?ジを分割読み込みして?スクハンドルを作成する
extern	int			DrawMask(							int x, int y, int MaskHandle, int TransMode ) ;													// ?スクスクリ?ンに?スクハンドルの持つ?スクイメ?ジを書き込む
#ifndef DX_NON_FONT
extern	int			DrawFormatStringMask(				int x, int y, int Flag,                 const TCHAR *FormatString, ... ) ;						// 書式指定ありの文字列を?スクスクリ?ンに?画する
extern	int			DrawFormatStringMaskToHandle(		int x, int y, int Flag, int FontHandle, const TCHAR *FormatString, ... ) ;						// 書式指定ありの文字列を?スクスクリ?ンに?画する( フォントハンドル指定版 )( SetFontCacheToTextureFlag( FALSE ) ; にして作成したフォントハンドルのみ使用可? )
extern	int			DrawStringMask(						int x, int y, int Flag,                 const TCHAR *String ) ;									// 文字列を?スクスクリ?ンに?画する
extern	int			DrawNStringMask(					int x, int y, int Flag,                 const TCHAR *String, size_t StringLength ) ;			// 文字列を?スクスクリ?ンに?画する
extern	int			DrawStringMaskToHandle(				int x, int y, int Flag, int FontHandle, const TCHAR *String ) ;									// 文字列を?スクスクリ?ンに?画する( フォントハンドル指定版 )( SetFontCacheToTextureFlag( FALSE ) ; にして作成したフォントハンドルのみ使用可? )
extern	int			DrawNStringMaskToHandle(			int x, int y, int Flag, int FontHandle, const TCHAR *String, size_t StringLength ) ;			// 文字列を?スクスクリ?ンに?画する( フォントハンドル指定版 )( SetFontCacheToTextureFlag( FALSE ) ; にして作成したフォントハンドルのみ使用可? )
#endif // DX_NON_FONT
extern	int			DrawFillMask(						int x1, int y1, int x2, int y2, int MaskHandle ) ;												// ?スクハンドルが持つ?スクイメ?ジを?スクスクリ?ンいっぱいに?画する( ?イル状に並べる )
extern	int			SetMaskReverseEffectFlag(			int ReverseFlag ) ;																				// ?スクイメ?ジ内の数値に対する効果を逆?させる( 旧バ?ジョンとの互換性の為の?? )

extern 	int			GetMaskScreenData(					int x1, int y1, int x2, int y2, int MaskHandle ) ;												// ?スクスクリ?ンの指定矩?部分を?スクハンドルに?送する
extern	int			GetMaskUseFlag(						void ) ;																						// ?スクスクリ?ンを使用する設定になっているかどうかを取得する

#endif // DX_NON_MASK

#endif // DX_NOTUSE_DRAWFUNCTION















// DxFont.cpp 関数プロト?イプ宣言

#ifndef DX_NON_FONT

// フォント、文字列?画関係関数
extern	int			EnumFontName(                           TCHAR *NameBuffer, int NameBufferNum, int JapanOnlyFlag DEFAULTPARAM( = TRUE ) ) ;																				// 使用可?なフォントの名前を列挙する( NameBuffer に 64バイト区切りで名前が格?されます )
extern	int			EnumFontNameEx(                         TCHAR *NameBuffer, int NameBufferNum,                                                       int CharSet DEFAULTPARAM( = -1 ) /* DX_CHARSET_DEFAULT 等 */ ) ;	// 使用可?なフォントの名前を列挙する( NameBuffer に 64バイト区切りで名前が格?されます )( 文字セット指定版 )
extern	int			EnumFontNameEx2(						TCHAR *NameBuffer, int NameBufferNum, const TCHAR *EnumFontName,                            int CharSet DEFAULTPARAM( = -1 ) /* DX_CHARSET_DEFAULT 等 */ ) ;	// 指定のフォント名のフォントを列挙する
extern	int			EnumFontNameEx2WithStrLen(				TCHAR *NameBuffer, int NameBufferNum, const TCHAR *EnumFontName, size_t EnumFontNameLength, int CharSet DEFAULTPARAM( = -1 ) /* DX_CHARSET_DEFAULT 等 */ ) ;	// 指定のフォント名のフォントを列挙する
extern	int			CheckFontName(							const TCHAR *FontName,                        int CharSet DEFAULTPARAM( = -1 ) /* DX_CHARSET_DEFAULT 等 */ ) ;													// 指定のフォント名のフォントが存在するかどうかを?ェックする( 戻り値  TRUE:存在する  FALSE:存在しない )
extern	int			CheckFontNameWithStrLen(				const TCHAR *FontName, size_t FontNameLength, int CharSet DEFAULTPARAM( = -1 ) /* DX_CHARSET_DEFAULT 等 */ ) ;													// 指定のフォント名のフォントが存在するかどうかを?ェックする( 戻り値  TRUE:存在する  FALSE:存在しない )

extern	int			InitFontToHandle(                       void ) ;																						// 全てのフォントハンドルを削除する

extern	int			CreateFontToHandle(						const TCHAR *FontName,                        int Size, int Thick, int FontType DEFAULTPARAM( = -1 ) , int CharSet DEFAULTPARAM( = -1 ) , int EdgeSize DEFAULTPARAM( = -1 ) , int Italic DEFAULTPARAM( = FALSE ) , int Handle DEFAULTPARAM( = -1 ) ) ;		// フォントハンドルを作成する
extern	int			CreateFontToHandleWithStrLen(			const TCHAR *FontName, size_t FontNameLength, int Size, int Thick, int FontType DEFAULTPARAM( = -1 ) , int CharSet DEFAULTPARAM( = -1 ) , int EdgeSize DEFAULTPARAM( = -1 ) , int Italic DEFAULTPARAM( = FALSE ) , int Handle DEFAULTPARAM( = -1 ) ) ;		// フォントハンドルを作成する
extern	int			LoadFontDataToHandle(					const TCHAR *FileName,                            int EdgeSize DEFAULTPARAM( = 0 ) ) ;			// フォントデ??フ?イルからフォントハンドルを作成する
extern	int			LoadFontDataToHandleWithStrLen(			const TCHAR *FileName, size_t FileNameLength,     int EdgeSize DEFAULTPARAM( = 0 ) ) ;			// フォントデ??フ?イルからフォントハンドルを作成する
extern	int			LoadFontDataFromMemToHandle(			const void *FontDataImage, int FontDataImageSize, int EdgeSize DEFAULTPARAM( = 0 ) ) ;			// メモリ上のフォントデ??フ?イルイメ?ジからフォントハンドルを作成する
extern	int			SetFontSpaceToHandle(                   int Pixel, int FontHandle ) ;																	// フォントハンドルの字間を変更する
extern	int			SetFontLineSpaceToHandle(               int Pixel, int FontHandle ) ;																	// フォントハンドルの行間を変更する
extern	int			SetFontCharCodeFormatToHandle(			int CharCodeFormat /* DX_CHARCODEFORMAT_SHIFTJIS 等 */ , int FontHandle ) ;						// 指定のフォントハンドルを使用する関数の引数に渡す文字列の文字コ?ド?式を設定する( UNICODE版では無効 )
extern	int			DeleteFontToHandle(                     int FontHandle ) ;																				// フォントハンドルを削除する
extern	int			SetFontLostFlag(                        int FontHandle, int *LostFlag ) ;																// フォントハンドルを削除した際に TRUE を代入する変数のアドレスを設定する
extern	int			AddFontImageToHandle(					int FontHandle, const TCHAR *Char,                    int GrHandle, int DrawX, int DrawY, int AddX ) ;	// 指定の文字の代わりに?画するグラフィックハンドルを登?する
extern	int			AddFontImageToHandleWithStrLen(			int FontHandle, const TCHAR *Char, size_t CharLength, int GrHandle, int DrawX, int DrawY, int AddX ) ;	// 指定の文字の代わりに?画するグラフィックハンドルを登?する
extern	int			SubFontImageToHandle(					int FontHandle, const TCHAR *Char                    ) ;												// 指定の文字の代わりに?画するグラフィックハンドルの登?を解除する
extern	int			SubFontImageToHandleWithStrLen(			int FontHandle, const TCHAR *Char, size_t CharLength ) ;												// 指定の文字の代わりに?画するグラフィックハンドルの登?を解除する
extern	int			AddSubstitutionFontToHandle(			int FontHandle, int SubstitutionFontHandle, int DrawX, int DrawY ) ;							// 代替フォントハンドル( FontHandle に無い文字を?画しようとしたときに代わりに使用されるフォントハンドル )を登?する
extern	int			SubSubstitutionFontToHandle(			int FontHandle, int SubstitutionFontHandle ) ;													// 代替フォントハンドルの登?を解除する

extern	int			ChangeFont(                             const TCHAR *FontName,                        int CharSet DEFAULTPARAM( = -1 ) /* DX_CHARSET_SHFTJIS 等 */ ) ;	// デフォルトフォントハンドルで使用するフォントを変更
extern	int			ChangeFontWithStrLen(                   const TCHAR *FontName, size_t FontNameLength, int CharSet DEFAULTPARAM( = -1 ) /* DX_CHARSET_SHFTJIS 等 */ ) ;	// デフォルトフォントハンドルで使用するフォントを変更
extern	int			ChangeFontFromHandle(					int FontHandle ) ;																				// デフォルトフォントハンドルとして使用するフォントハンドルを変更する
extern	int			ChangeFontType(                         int FontType ) ;																				// デフォルトフォントハンドルのフォント?イプの変更
extern	const TCHAR *GetFontName(							void ) ;																						// デフォルトフォントハンドルのフォント名を取得する
extern	int			SetFontSize(                            int FontSize ) ;																				// デフォルトフォントハンドルのサイズを設定する
extern	int			GetFontSize(                            void ) ;																						// デフォルトフォントハンドルのサイズを取得する
extern	int			GetFontEdgeSize(                        void ) ;																						// デフォルトフォントハンドルの縁サイズを取得する
extern	int			SetFontThickness(                       int ThickPal ) ;																				// デフォルトフォントハンドルの太さを設定する
extern	int			SetFontSpace(                           int Pixel ) ;																					// デフォルトフォントハンドルの字間を変更する
extern	int			GetFontSpace(                           void ) ;																						// デフォルトフォントハンドルの字間を取得する
extern	int			SetFontLineSpace(                       int Pixel ) ;																					// デフォルトフォントハンドルの行間を変更する
extern	int			GetFontLineSpace(                       void ) ;																						// デフォルトフォントハンドルの行間を取得する
extern	int			SetFontCharCodeFormat(					int CharCodeFormat /* DX_CHARCODEFORMAT_SHIFTJIS 等 */ ) ;										// デフォルトフォントハンドルを使用する関数の引数に渡す文字列の文字コ?ド?式を設定する( UNICODE版では無効 )
extern	int			SetDefaultFontState(                    const TCHAR *FontName,                        int Size, int Thick, int FontType DEFAULTPARAM( = -1 ) , int CharSet DEFAULTPARAM( = -1 ) , int EdgeSize DEFAULTPARAM( = -1 ) , int Italic DEFAULTPARAM( = FALSE ) ) ;	// デフォルトフォントハンドルの設定を変更する
extern	int			SetDefaultFontStateWithStrLen(          const TCHAR *FontName, size_t FontNameLength, int Size, int Thick, int FontType DEFAULTPARAM( = -1 ) , int CharSet DEFAULTPARAM( = -1 ) , int EdgeSize DEFAULTPARAM( = -1 ) , int Italic DEFAULTPARAM( = FALSE ) ) ;	// デフォルトフォントハンドルの設定を変更する
extern	int			GetDefaultFontHandle(                   void ) ;																								// デフォルトフォントハンドルを取得する
extern	int			GetFontMaxCacheCharNum(					void ) ;																								// デフォルトフォントハンドルのキャッシュできる文字の数を取得する
extern	int			GetFontMaxWidth(                        void ) ;																								// デフォルトフォントハンドルの文字の最大幅を取得する
extern	int			GetFontAscent(                          void ) ;																								// デフォルトフォントハンドルの?画位置からベ?スラインまでの高さを取得する
extern	int			GetDrawStringWidth(                     const TCHAR *String, int StrLen,          int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;									// デフォルトフォントハンドルを使用した文字列の?画幅を取得する
extern	int			GetDrawNStringWidth(                    const TCHAR *String, size_t StringLength, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;									// デフォルトフォントハンドルを使用した文字列の?画幅を取得する
extern	int			GetDrawFormatStringWidth(               const TCHAR *FormatString, ... ) ;																		// デフォルトフォントハンドルを使用した書式付き文字列の?画幅を取得する
extern	int			GetDrawExtendStringWidth(               double ExRateX, const TCHAR *String, int StrLen,          int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;					// デフォルトフォントハンドルを使用した文字列の?画幅を取得する( 拡大率付き )
extern	int			GetDrawExtendNStringWidth(              double ExRateX, const TCHAR *String, size_t StringLength, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;					// デフォルトフォントハンドルを使用した文字列の?画幅を取得する( 拡大率付き )
extern	int			GetDrawExtendFormatStringWidth(         double ExRateX, const TCHAR *FormatString, ... ) ;														// デフォルトフォントハンドルを使用した書式付き文字列の?画幅を取得する( 拡大率付き )
extern	int			GetDrawStringSize(                      int *SizeX, int *SizeY, int *LineCount, const TCHAR *String, int StrLen,          int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;											// デフォルトフォントハンドルを使用した文字列の?画幅・高さ・行数を取得する
extern	int			GetDrawNStringSize(                     int *SizeX, int *SizeY, int *LineCount, const TCHAR *String, size_t StringLength, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;											// デフォルトフォントハンドルを使用した文字列の?画幅・高さ・行数を取得する
extern	int			GetDrawFormatStringSize(                int *SizeX, int *SizeY, int *LineCount, const TCHAR *FormatString, ... ) ;																				// デフォルトフォントハンドルを使用した書式付き文字列の?画幅・高さ・行数を取得する
extern	int			GetDrawExtendStringSize(                int *SizeX, int *SizeY, int *LineCount, double ExRateX, double ExRateY, const TCHAR *String, int StrLen,          int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;			// デフォルトフォントハンドルを使用した文字列の?画幅・高さ・行数を取得する( 拡大率付き )
extern	int			GetDrawExtendNStringSize(               int *SizeX, int *SizeY, int *LineCount, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;			// デフォルトフォントハンドルを使用した文字列の?画幅・高さ・行数を取得する( 拡大率付き )
extern	int			GetDrawExtendFormatStringSize(          int *SizeX, int *SizeY, int *LineCount, double ExRateX, double ExRateY, const TCHAR *FormatString, ... ) ;												// デフォルトフォントハンドルを使用した書式付き文字列の?画幅・高さ・行数を取得する( 拡大率付き )
extern	int			GetDrawStringCharInfo(                  DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, const TCHAR *String, int StrLen,          int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;									// デフォルトフォントハンドルを使用した文字列の１文字毎の情報を取得する
extern	int			GetDrawNStringCharInfo(                 DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, const TCHAR *String, size_t StringLength, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;									// デフォルトフォントハンドルを使用した文字列の１文字毎の情報を取得する
extern	int			GetDrawFormatStringCharInfo(            DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, const TCHAR *FormatString, ... ) ;																		// デフォルトフォントハンドルを使用した書式付き文字列の１文字毎の情報を取得する
extern	int			GetDrawExtendStringCharInfo(            DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, double ExRateX, double ExRateY, const TCHAR *String, int StrLen,          int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// デフォルトフォントハンドルを使用した文字列の１文字毎の情報を取得する
extern	int			GetDrawExtendNStringCharInfo(           DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// デフォルトフォントハンドルを使用した文字列の１文字毎の情報を取得する
extern	int			GetDrawExtendFormatStringCharInfo(      DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, double ExRateX, double ExRateY, const TCHAR *FormatString, ... ) ;										// デフォルトフォントハンドルを使用した書式付き文字列の１文字毎の情報を取得する
extern	int			GetDrawStringKerningPairInfo(			const TCHAR *PairChar,                        int *KernAmount ) ;								// デフォルトフォントハンドルを使用した二つの文字のペアのカ?ニング情報を取得する( PairChar:カ?ニング情報を調べるペアとなる2文字の文字列( 2文字以上あっても先頭の2文字だけ使用されます )  KernAmount:2文字目の文字を基?の位置からずらすドット数を代入するint?変数のアドレス )
extern	int			GetDrawStringKerningPairInfoWithStrLen(	const TCHAR *PairChar, size_t PairCharLength, int *KernAmount ) ;								// デフォルトフォントハンドルを使用した二つの文字のペアのカ?ニング情報を取得する( PairChar:カ?ニング情報を調べるペアとなる2文字の文字列( 2文字以上あっても先頭の2文字だけ使用されます )  KernAmount:2文字目の文字を基?の位置からずらすドット数を代入するint?変数のアドレス )

extern	const TCHAR *GetFontNameToHandle(					int FontHandle ) ;																				// フォントハンドルのフォント名を取得する
extern	int			GetFontMaxCacheCharNumToHandle(			int FontHandle ) ;																				// フォントハンドルのキャッシュできる文字の数を取得する
extern	int			GetFontMaxWidthToHandle(                int FontHandle ) ;																				// フォントハンドルの文字の最大幅を取得する
extern	int			GetFontAscentToHandle(                  int FontHandle ) ;																				// フォントハンドルの?画位置からベ?スラインまでの高さを取得する
extern	int			GetFontSizeToHandle(                    int FontHandle ) ;																				// フォントハンドルのサイズを取得する
extern	int			GetFontEdgeSizeToHandle(                int FontHandle ) ;																				// フォントハンドルの縁サイズを取得する
extern	int			GetFontSpaceToHandle(                   int FontHandle ) ;																				// フォントハンドルの字間を取得する
extern	int			GetFontLineSpaceToHandle(               int FontHandle ) ;																				// フォントハンドルの行間を取得する
extern	int			GetFontCharInfo(                        int FontHandle, const TCHAR *Char,                    int *DrawX, int *DrawY, int *NextCharX, int *SizeX, int *SizeY ) ;	// フォントハンドルの指定の文字の?画情報を取得する
extern	int			GetFontCharInfoWithStrLen(              int FontHandle, const TCHAR *Char, size_t CharLength, int *DrawX, int *DrawY, int *NextCharX, int *SizeX, int *SizeY ) ;	// フォントハンドルの指定の文字の?画情報を取得する
extern	int			GetDrawStringWidthToHandle(             const TCHAR   *String, int StrLen,          int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;						// フォントハンドルを使用した文字列の?画幅を取得する
extern	int			GetDrawNStringWidthToHandle(            const TCHAR   *String, size_t StringLength, int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;						// フォントハンドルを使用した文字列の?画幅を取得する
extern	int			GetDrawFormatStringWidthToHandle(       int FontHandle, const TCHAR *FormatString, ... ) ;																// フォントハンドルを使用した書式付き文字列の?画幅を取得する
extern	int			GetDrawExtendStringWidthToHandle(       double ExRateX, const TCHAR *String, int StrLen,          int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;			// フォントハンドルを使用した文字列の?画幅を取得する
extern	int			GetDrawExtendNStringWidthToHandle(      double ExRateX, const TCHAR *String, size_t StringLength, int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;			// フォントハンドルを使用した文字列の?画幅を取得する
extern	int			GetDrawExtendFormatStringWidthToHandle( double ExRateX, int FontHandle, const TCHAR *FormatString, ... ) ;												// フォントハンドルを使用した書式付き文字列の?画幅を取得する
extern	int			GetDrawStringSizeToHandle(              int *SizeX, int *SizeY, int *LineCount, const TCHAR   *String, int StrLen,          int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;											// フォントハンドルを使用した文字列の?画幅・高さ・行数を取得する
extern	int			GetDrawNStringSizeToHandle(             int *SizeX, int *SizeY, int *LineCount, const TCHAR   *String, size_t StringLength, int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;											// フォントハンドルを使用した文字列の?画幅・高さ・行数を取得する
extern	int			GetDrawFormatStringSizeToHandle(        int *SizeX, int *SizeY, int *LineCount, int FontHandle, const TCHAR *FormatString, ... ) ;																					// フォントハンドルを使用した書式付き文字列の?画幅・高さ・行数を取得する
extern	int			GetDrawExtendStringSizeToHandle(        int *SizeX, int *SizeY, int *LineCount, double ExRateX, double ExRateY, const TCHAR *String, int StrLen,          int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;				// フォントハンドルを使用した文字列の?画幅・高さ・行数を取得する
extern	int			GetDrawExtendNStringSizeToHandle(       int *SizeX, int *SizeY, int *LineCount, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength, int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;				// フォントハンドルを使用した文字列の?画幅・高さ・行数を取得する
extern	int			GetDrawExtendFormatStringSizeToHandle(  int *SizeX, int *SizeY, int *LineCount, double ExRateX, double ExRateY, int FontHandle, const TCHAR *FormatString, ... ) ;													// フォントハンドルを使用した書式付き文字列の?画幅・高さ・行数を取得する
extern	int			GetDrawStringCharInfoToHandle(              DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, const TCHAR *String, int StrLen,          int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;									// フォントハンドルを使用した文字列の１文字毎の情報を取得する
extern	int			GetDrawNStringCharInfoToHandle(             DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, const TCHAR *String, size_t StringLength, int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;									// フォントハンドルを使用した文字列の１文字毎の情報を取得する
extern	int			GetDrawFormatStringCharInfoToHandle(        DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, int FontHandle, const TCHAR *FormatString, ... ) ;																		// フォントハンドルを使用した書式付き文字列の１文字毎の情報を取得する
extern	int			GetDrawExtendStringCharInfoToHandle(        DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, double ExRateX, double ExRateY, const TCHAR *String, int StrLen,          int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// フォントハンドルを使用した文字列の１文字毎の情報を取得する
extern	int			GetDrawExtendNStringCharInfoToHandle(       DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength, int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// フォントハンドルを使用した文字列の１文字毎の情報を取得する
extern	int			GetDrawExtendFormatStringCharInfoToHandle(  DRAWCHARINFO *InfoBuffer, size_t InfoBufferSize, double ExRateX, double ExRateY, int FontHandle, const TCHAR *FormatString, ... ) ;										// フォントハンドルを使用した書式付き文字列の１文字毎の情報を取得する
extern	int			GetDrawStringKerningPairInfoToHandle(	        const TCHAR *PairChar,                        int *KernAmount, int FontHandle ) ;		// フォントハンドルを使用した二つの文字のペアのカ?ニング情報を取得する( PairChar:カ?ニング情報を調べるペアとなる2文字の文字列( 2文字以上あっても先頭の2文字だけ使用されます )  KernAmount:2文字目の文字を基?の位置からずらすドット数を代入するint?変数のアドレス )
extern	int			GetDrawStringKerningPairInfoToHandleWithStrLen(	const TCHAR *PairChar, size_t PairCharLength, int *KernAmount, int FontHandle ) ;		// フォントハンドルを使用した二つの文字のペアのカ?ニング情報を取得する( PairChar:カ?ニング情報を調べるペアとなる2文字の文字列( 2文字以上あっても先頭の2文字だけ使用されます )  KernAmount:2文字目の文字を基?の位置からずらすドット数を代入するint?変数のアドレス )
extern	int			GetFontStateToHandle(                   TCHAR   *FontName, int *Size, int *Thick, int FontHandle, int *FontType DEFAULTPARAM( = NULL ) , int *CharSet DEFAULTPARAM( = NULL ) , int *EdgeSize DEFAULTPARAM( = NULL ) , int *Italic DEFAULTPARAM( = NULL ) ) ;						// フォントハンドルの情報を取得する
extern	int			CheckFontCacheToTextureFlag(            int FontHandle ) ;																				// フォントハンドルがテクス?ャキャッシュを使用しているかどうかを取得する
extern	int			CheckFontChacheToTextureFlag(           int FontHandle ) ;																				// CheckFontCacheToTextureFlag の誤字版
extern	int			CheckFontHandleValid(                   int FontHandle ) ;																				// フォントハンドルが有効かどうかを取得する
extern	int			ClearFontCacheToHandle(					int FontHandle ) ;																				// フォントハンドルのキャッシュ情報を初期化する

extern	int			SetFontCacheToTextureFlag(              int Flag ) ;																					// フォントのキャッシュにテクス?ャを使用するかどうかを設定する( TRUE:テクス?ャを使用する( デフォルト )  FALSE:テクス?ャは使用しない )
extern	int			GetFontCacheToTextureFlag(              void ) ;																						// フォントのキャッシュにテクス?ャを使用するかどうかを設定する
extern	int			SetFontChacheToTextureFlag(             int Flag ) ;																					// SetFontCacheToTextureFlag の誤字版
extern	int			GetFontChacheToTextureFlag(             void ) ;																						// GetFontCacheToTextureFlag の誤字版
extern	int			SetFontCacheTextureColorBitDepth(		int ColorBitDepth ) ;																			// フォントのキャッシュとして使用するテクス?ャのカラ?ビット?度を設定する( 16 又は 32 のみ指定可?  デフォルトは 32 )
extern	int			GetFontCacheTextureColorBitDepth(		void ) ;																						// フォントのキャッシュとして使用するテクス?ャのカラ?ビット?度を取得する
extern	int			SetFontCacheCharNum(                    int CharNum ) ;																					// フォントキャッシュでキャッシュできる文字数を設定する
extern	int			GetFontCacheCharNum(                    void ) ;																						// フォントキャッシュでキャッシュできる文字数を取得する( 戻り値  0:デフォルト  1以上:指定文字数 )
extern	int			SetFontCacheUsePremulAlphaFlag(         int Flag ) ;																					// フォントキャッシュとして保存する画像の?式を乗算済みα?ャンネル付き画像にするかどうかを設定する( TRUE:乗算済みαを使用する  FLASE:乗算済みαを使用しない( デフォルト ) )
extern	int			GetFontCacheUsePremulAlphaFlag(         void ) ;																						// フォントキャッシュとして保存する画像の?式を乗算済みα?ャンネル付き画像にするかどうかを取得する
extern	int			SetFontUseAdjustSizeFlag(               int Flag ) ;																					// フォントのサイズを補正する処理を行うかどうかを設定する( Flag  TRUE:行う( デフォルト )  FALSE:行わない )
extern	int			GetFontUseAdjustSizeFlag(               void ) ;																						// フォントのサイズを補正する処理を行うかどうかを取得する
extern	int			SetFontOnlyDrawType(					int OnlyType ) ;																				// フォントの?画で縁のみ、又は?体のみ?画を行うかどうかを設定する( OnlyType  0:通常?画 1:?体のみ?画 2:縁のみ?画 )
extern	int			GetFontOnlyDrawType(					void ) ;																						// フォントの?画で縁のみ、又は?体のみ?画を行うかどうかを取得する( 戻り値  0:通常?画 1:?体のみ?画 2:縁のみ?画 )
extern	int			SetFontIgnoreLFFlag(					int Flag ) ;																					// DrawString などで \n を無視するかどうかを設定する( TRUE : 無視する    FALSE : 無視しない( デフォルト ) )
extern	int			GetFontIgnoreLFFlag(					void ) ;																						// DrawString などで \n を無視するかどうかを取得する( TRUE : 無視する    FALSE : 無視しない( デフォルト ) )


// FontCacheStringDraw の代わりに DrawString を使ってください
extern	int			FontCacheStringDrawToHandle(            int x, int y, const TCHAR *StrData,                       unsigned int Color, unsigned int EdgeColor, BASEIMAGE *DestImage, const RECT *ClipRect /* NULL 可 */ , int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) , SIZE *DrawSizeP DEFAULTPARAM( = NULL ) ) ;
extern	int			FontCacheStringDrawToHandleWithStrLen(  int x, int y, const TCHAR *StrData, size_t StrDataLength, unsigned int Color, unsigned int EdgeColor, BASEIMAGE *DestImage, const RECT *ClipRect /* NULL 可 */ , int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) , SIZE *DrawSizeP DEFAULTPARAM( = NULL ) ) ;
extern	int			FontBaseImageBlt(                       int x, int y, const TCHAR *StrData,                       BASEIMAGE *DestImage, BASEIMAGE *DestEdgeImage,                 int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// 基?イメ?ジに文字列を?画する( デフォルトフォントハンドルを使用する )
extern	int			FontBaseImageBltWithStrLen(             int x, int y, const TCHAR *StrData, size_t StrDataLength, BASEIMAGE *DestImage, BASEIMAGE *DestEdgeImage,                 int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// 基?イメ?ジに文字列を?画する( デフォルトフォントハンドルを使用する )
extern	int			FontBaseImageBltToHandle(               int x, int y, const TCHAR *StrData,                       BASEIMAGE *DestImage, BASEIMAGE *DestEdgeImage, int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// 基?イメ?ジに文字列を?画する
extern	int			FontBaseImageBltToHandleWithStrLen(     int x, int y, const TCHAR *StrData, size_t StrDataLength, BASEIMAGE *DestImage, BASEIMAGE *DestEdgeImage, int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// 基?イメ?ジに文字列を?画する

extern	int			MultiByteCharCheck(                     const char *Buf, int CharSet /* DX_CHARSET_SHFTJIS */ ) ;										// ２バイト文字か調べる( TRUE:２バイト文字  FALSE:１バイト文字 )

// 文字列?画関数
extern	int			DrawString(                             int x, int y,                                              const TCHAR *String,                      unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;							// デフォルトフォントハンドルを使用して文字列を?画する
extern	int			DrawNString(                            int x, int y,                                              const TCHAR *String, size_t StringLength, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;							// デフォルトフォントハンドルを使用して文字列を?画する
extern	int			DrawVString(                            int x, int y,                                              const TCHAR *String,                      unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;							// デフォルトフォントハンドルを使用して文字列を?画する( 縦書き )
extern	int			DrawNVString(                           int x, int y,                                              const TCHAR *String, size_t StringLength, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;							// デフォルトフォントハンドルを使用して文字列を?画する( 縦書き )
extern	int			DrawFormatString(                       int x, int y,                                 unsigned int Color,                         const TCHAR *FormatString, ... ) ;																	// デフォルトフォントハンドルを使用して書式指定文字列を?画する
extern	int			DrawFormatVString(                      int x, int y,                                 unsigned int Color,                         const TCHAR *FormatString, ... ) ;																	// デフォルトフォントハンドルを使用して書式指定文字列を?画する( 縦書き )
extern	int			DrawFormatString2(                      int x, int y,                                 unsigned int Color, unsigned int EdgeColor, const TCHAR *FormatString, ... ) ;																	// デフォルトフォントハンドルを使用して書式指定文字列を?画する( 縁の色引数付き )
extern	int			DrawFormatVString2(                     int x, int y,                                 unsigned int Color, unsigned int EdgeColor, const TCHAR *FormatString, ... ) ;																	// デフォルトフォントハンドルを使用して書式指定文字列を?画する( 縁の色引数付き )( 縦書き )
extern	int			DrawExtendString(                       int x, int y, double ExRateX, double ExRateY,              const TCHAR *String,                      unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;							// デフォルトフォントハンドルを使用して文字列の拡大?画
extern	int			DrawExtendNString(                      int x, int y, double ExRateX, double ExRateY,              const TCHAR *String, size_t StringLength, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;							// デフォルトフォントハンドルを使用して文字列の拡大?画
extern	int			DrawExtendVString(                      int x, int y, double ExRateX, double ExRateY,              const TCHAR *String,                      unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;							// デフォルトフォントハンドルを使用して文字列の拡大?画( 縦書き )
extern	int			DrawExtendNVString(                     int x, int y, double ExRateX, double ExRateY,              const TCHAR *String, size_t StringLength, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;							// デフォルトフォントハンドルを使用して文字列の拡大?画( 縦書き )
extern	int			DrawExtendFormatString(                 int x, int y, double ExRateX, double ExRateY, unsigned int Color,                         const TCHAR *FormatString, ... ) ;																	// デフォルトフォントハンドルを使用して書式指定文字列を拡大?画する
extern	int			DrawExtendFormatVString(                int x, int y, double ExRateX, double ExRateY, unsigned int Color,                         const TCHAR *FormatString, ... ) ;																	// デフォルトフォントハンドルを使用して書式指定文字列を拡大?画する( 縦書き )
extern	int			DrawExtendFormatString2(                int x, int y, double ExRateX, double ExRateY, unsigned int Color, unsigned int EdgeColor, const TCHAR *FormatString, ... ) ;																	// デフォルトフォントハンドルを使用して書式指定文字列を拡大?画する( 縁の色引数付き )
extern	int			DrawExtendFormatVString2(               int x, int y, double ExRateX, double ExRateY, unsigned int Color, unsigned int EdgeColor, const TCHAR *FormatString, ... ) ;																	// デフォルトフォントハンドルを使用して書式指定文字列を拡大?画する( 縁の色引数付き )( 縦書き )
extern	int			DrawRotaString(							int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *String       DEFAULTPARAM( = NULL )                           ) ;		// デフォルトフォントハンドルを使用して文字列を回??画する
extern	int			DrawRotaNString(						int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *String       DEFAULTPARAM( = NULL ) , size_t StringLength DEFAULTPARAM( = 0 ) ) ;		// デフォルトフォントハンドルを使用して文字列を回??画する
extern	int			DrawRotaFormatString(					int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *FormatString DEFAULTPARAM( = NULL ) , ...                     ) ;		// デフォルトフォントハンドルを使用して書式指定文字列を回??画する
extern	int			DrawModiString(							int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *String       DEFAULTPARAM( = NULL )                           ) ;		// デフォルトフォントハンドルを使用して文字列を変??画する
extern	int			DrawModiNString(						int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *String       DEFAULTPARAM( = NULL ) , size_t StringLength DEFAULTPARAM( = 0 ) ) ;		// デフォルトフォントハンドルを使用して文字列を変??画する
extern	int			DrawModiFormatString(					int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *FormatString DEFAULTPARAM( = NULL ) , ...                     ) ;		// デフォルトフォントハンドルを使用して書式指定文字列を変??画する

extern	int			DrawStringF(                            float x, float y,                                              const TCHAR *String,                      unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;						// デフォルトフォントハンドルを使用して文字列を?画する( 座標指定が float 版 )
extern	int			DrawNStringF(                           float x, float y,                                              const TCHAR *String, size_t StringLength, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;						// デフォルトフォントハンドルを使用して文字列を?画する( 座標指定が float 版 )
extern	int			DrawVStringF(                           float x, float y,                                              const TCHAR *String,                      unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;						// デフォルトフォントハンドルを使用して文字列を?画する( 縦書き )( 座標指定が float 版 )
extern	int			DrawNVStringF(                          float x, float y,                                              const TCHAR *String, size_t StringLength, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;						// デフォルトフォントハンドルを使用して文字列を?画する( 縦書き )( 座標指定が float 版 )
extern	int			DrawFormatStringF(                      float x, float y,                                 unsigned int Color,                         const TCHAR *FormatString, ... ) ;																// デフォルトフォントハンドルを使用して書式指定文字列を?画する( 座標指定が float 版 )
extern	int			DrawFormatVStringF(                     float x, float y,                                 unsigned int Color,                         const TCHAR *FormatString, ... ) ;																// デフォルトフォントハンドルを使用して書式指定文字列を?画する( 縦書き )( 座標指定が float 版 )
extern	int			DrawFormatString2F(                     float x, float y,                                 unsigned int Color, unsigned int EdgeColor, const TCHAR *FormatString, ... ) ;																// デフォルトフォントハンドルを使用して書式指定文字列を?画する( 縁の色引数付き )( 座標指定が float 版 )
extern	int			DrawFormatVString2F(                    float x, float y,                                 unsigned int Color, unsigned int EdgeColor, const TCHAR *FormatString, ... ) ;																// デフォルトフォントハンドルを使用して書式指定文字列を?画する( 縁の色引数付き )( 縦書き )( 座標指定が float 版 )
extern	int			DrawExtendStringF(                      float x, float y, double ExRateX, double ExRateY,              const TCHAR *String,                      unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;						// デフォルトフォントハンドルを使用して文字列の拡大?画( 座標指定が float 版 )
extern	int			DrawExtendNStringF(                     float x, float y, double ExRateX, double ExRateY,              const TCHAR *String, size_t StringLength, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;						// デフォルトフォントハンドルを使用して文字列の拡大?画( 座標指定が float 版 )
extern	int			DrawExtendVStringF(                     float x, float y, double ExRateX, double ExRateY,              const TCHAR *String,                      unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;						// デフォルトフォントハンドルを使用して文字列の拡大?画( 縦書き )( 座標指定が float 版 )
extern	int			DrawExtendNVStringF(                    float x, float y, double ExRateX, double ExRateY,              const TCHAR *String, size_t StringLength, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;						// デフォルトフォントハンドルを使用して文字列の拡大?画( 縦書き )( 座標指定が float 版 )
extern	int			DrawExtendFormatStringF(                float x, float y, double ExRateX, double ExRateY, unsigned int Color,                         const TCHAR *FormatString, ... ) ;																// デフォルトフォントハンドルを使用して書式指定文字列を拡大?画する( 座標指定が float 版 )
extern	int			DrawExtendFormatVStringF(               float x, float y, double ExRateX, double ExRateY, unsigned int Color,                         const TCHAR *FormatString, ... ) ;																// デフォルトフォントハンドルを使用して書式指定文字列を拡大?画する( 縦書き )( 座標指定が float 版 )
extern	int			DrawExtendFormatString2F(               float x, float y, double ExRateX, double ExRateY, unsigned int Color, unsigned int EdgeColor, const TCHAR *FormatString, ... ) ;																// デフォルトフォントハンドルを使用して書式指定文字列を拡大?画する( 縁の色引数付き )( 座標指定が float 版 )
extern	int			DrawExtendFormatVString2F(              float x, float y, double ExRateX, double ExRateY, unsigned int Color, unsigned int EdgeColor, const TCHAR *FormatString, ... ) ;																// デフォルトフォントハンドルを使用して書式指定文字列を拡大?画する( 縁の色引数付き )( 縦書き )( 座標指定が float 版 )
extern	int			DrawRotaStringF(						float x, float y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *String       DEFAULTPARAM( = NULL )                           ) ;		// デフォルトフォントハンドルを使用して文字列を回??画する( 座標指定が float 版 )
extern	int			DrawRotaNStringF(						float x, float y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *String       DEFAULTPARAM( = NULL ) , size_t StringLength DEFAULTPARAM( = 0 ) ) ;		// デフォルトフォントハンドルを使用して文字列を回??画する( 座標指定が float 版 )
extern	int			DrawRotaFormatStringF(					float x, float y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *FormatString DEFAULTPARAM( = NULL ) , ...                     ) ;		// デフォルトフォントハンドルを使用して書式指定文字列を回??画する( 座標指定が float 版 )
extern	int			DrawModiStringF(						float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *String       DEFAULTPARAM( = NULL )                           ) ;		// デフォルトフォントハンドルを使用して文字列を変??画する( 座標指定が float 版 )
extern	int			DrawModiNStringF(						float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *String       DEFAULTPARAM( = NULL ) , size_t StringLength DEFAULTPARAM( = 0 ) ) ;		// デフォルトフォントハンドルを使用して文字列を変??画する( 座標指定が float 版 )
extern	int			DrawModiFormatStringF(					float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *FormatString DEFAULTPARAM( = NULL ) , ...                     ) ;		// デフォルトフォントハンドルを使用して書式指定文字列を変??画する( 座標指定が float 版 )

extern	int			DrawNumberToI(                          int x, int y,                          int    Num, int RisesNum, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;											// デフォルトフォントハンドルを使用して整数?の数値を?画する
extern	int			DrawNumberToF(                          int x, int y,                          double Num, int Length,   unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;											// デフォルトフォントハンドルを使用して浮動小数??の数値を?画する
extern	int			DrawNumberPlusToI(                      int x, int y, const TCHAR *NoteString, int    Num, int RisesNum, unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;											// デフォルトフォントハンドルを使用して整数?の数値とその説明の文字列を一度に?画する
extern 	int			DrawNumberPlusToF(                      int x, int y, const TCHAR *NoteString, double Num, int Length,   unsigned int Color, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;											// デフォルトフォントハンドルを使用して浮動小数??の数値とその説明の文字列を一度に?画する

extern	int			DrawStringToZBuffer(                    int x, int y, const TCHAR *String,                                                                                        int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;												// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を?画する
extern	int			DrawNStringToZBuffer(                   int x, int y, const TCHAR *String, size_t StringLength,                                                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;												// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を?画する
extern	int			DrawVStringToZBuffer(                   int x, int y, const TCHAR *String,                                                                                        int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;												// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を?画する( 縦書き )
extern	int			DrawNVStringToZBuffer(                  int x, int y, const TCHAR *String, size_t StringLength,                                                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;												// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を?画する( 縦書き )
extern	int			DrawFormatStringToZBuffer(              int x, int y,                                                                                        int WriteZMode /* DX_ZWRITE_MASK 等 */ , const TCHAR *FormatString, ... ) ;									// デフォルトフォントハンドルを使用してＺバッフ?に対して書式指定文字列を?画する
extern	int			DrawFormatVStringToZBuffer(             int x, int y,                                                                                        int WriteZMode /* DX_ZWRITE_MASK 等 */ , const TCHAR *FormatString, ... ) ;									// デフォルトフォントハンドルを使用してＺバッフ?に対して書式指定文字列を?画する( 縦書き )
extern	int			DrawExtendStringToZBuffer(              int x, int y, double ExRateX, double ExRateY, const TCHAR *String,                                                        int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;												// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を拡大?画する
extern	int			DrawExtendNStringToZBuffer(             int x, int y, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength,                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;												// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を拡大?画する
extern	int			DrawExtendVStringToZBuffer(             int x, int y, double ExRateX, double ExRateY, const TCHAR *String,                                                        int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;												// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を拡大?画する( 縦書き )
extern	int			DrawExtendNVStringToZBuffer(            int x, int y, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength,                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;												// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を拡大?画する( 縦書き )
extern	int			DrawExtendFormatStringToZBuffer(        int x, int y, double ExRateX, double ExRateY,                                                        int WriteZMode /* DX_ZWRITE_MASK 等 */ , const TCHAR *FormatString, ... ) ;									// デフォルトフォントハンドルを使用してＺバッフ?に対して書式指定文字列を拡大?画する
extern	int			DrawExtendFormatVStringToZBuffer(       int x, int y, double ExRateX, double ExRateY,                                                        int WriteZMode /* DX_ZWRITE_MASK 等 */ , const TCHAR *FormatString, ... ) ;									// デフォルトフォントハンドルを使用してＺバッフ?に対して書式指定文字列を拡大?画する( 縦書き )
extern	int			DrawRotaStringToZBuffer(				int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag , const TCHAR *String                            ) ;	// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を回??画する
extern	int			DrawRotaNStringToZBuffer(				int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag , const TCHAR *String,       size_t StringLength ) ;	// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を回??画する
extern	int			DrawRotaFormatStringToZBuffer(			int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag , const TCHAR *FormatString , ...                ) ;	// デフォルトフォントハンドルを使用してＺバッフ?に対して書式指定文字列を回??画する
extern	int			DrawModiStringToZBuffer(				int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,                                      int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag , const TCHAR *String                            ) ;	// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を変??画する
extern	int			DrawModiNStringToZBuffer(				int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,                                      int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag , const TCHAR *String,       size_t StringLength ) ;	// デフォルトフォントハンドルを使用してＺバッフ?に対して文字列を変??画する
extern	int			DrawModiFormatStringToZBuffer(			int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,                                      int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag , const TCHAR *FormatString , ...                ) ;	// デフォルトフォントハンドルを使用してＺバッフ?に対して書式指定文字列を変??画する


extern	int			DrawStringToHandle(                     int x, int y, const TCHAR *String,                      unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;									// フォントハンドルを使用して文字列を?画する
extern	int			DrawNStringToHandle(                    int x, int y, const TCHAR *String, size_t StringLength, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;									// フォントハンドルを使用して文字列を?画する
extern	int			DrawVStringToHandle(                    int x, int y, const TCHAR *String,                      unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;																				// フォントハンドルを使用して文字列を?画する( 縦書き )
extern	int			DrawNVStringToHandle(                   int x, int y, const TCHAR *String, size_t StringLength, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;																				// フォントハンドルを使用して文字列を?画する( 縦書き )
extern	int			DrawFormatStringToHandle(               int x, int y, unsigned int Color,                         int FontHandle, const TCHAR *FormatString, ... ) ;																											// フォントハンドルを使用して書式指定文字列を?画する
extern	int			DrawFormatVStringToHandle(              int x, int y, unsigned int Color,                         int FontHandle, const TCHAR *FormatString, ... ) ;																											// フォントハンドルを使用して書式指定文字列を?画する( 縦書き )
extern	int			DrawFormatString2ToHandle(              int x, int y, unsigned int Color, unsigned int EdgeColor, int FontHandle, const TCHAR *FormatString, ... ) ;																											// フォントハンドルを使用して書式指定文字列を?画する( 縁の色引数付き )
extern	int			DrawFormatVString2ToHandle(             int x, int y, unsigned int Color, unsigned int EdgeColor, int FontHandle, const TCHAR *FormatString, ... ) ;																											// フォントハンドルを使用して書式指定文字列を?画する( 縁の色引数付き )( 縦書き )
extern	int			DrawExtendStringToHandle(               int x, int y, double ExRateX, double ExRateY, const TCHAR *String,                      unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// フォントハンドルを使用して文字列を拡大?画する
extern	int			DrawExtendNStringToHandle(              int x, int y, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// フォントハンドルを使用して文字列を拡大?画する
extern	int			DrawExtendVStringToHandle(              int x, int y, double ExRateX, double ExRateY, const TCHAR *String,                      unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;												// フォントハンドルを使用して文字列を拡大?画する( 縦書き )
extern	int			DrawExtendNVStringToHandle(             int x, int y, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;												// フォントハンドルを使用して文字列を拡大?画する( 縦書き )
extern	int			DrawExtendFormatStringToHandle(         int x, int y, double ExRateX, double ExRateY, unsigned int Color,                         int FontHandle, const TCHAR *FormatString, ... ) ;																			// フォントハンドルを使用して書式指定文字列を拡大?画する
extern	int			DrawExtendFormatVStringToHandle(        int x, int y, double ExRateX, double ExRateY, unsigned int Color,                         int FontHandle, const TCHAR *FormatString, ... ) ;																			// フォントハンドルを使用して書式指定文字列を拡大?画する( 縦書き )
extern	int			DrawExtendFormatString2ToHandle(        int x, int y, double ExRateX, double ExRateY, unsigned int Color, unsigned int EdgeColor, int FontHandle, const TCHAR *FormatString, ... ) ;																			// フォントハンドルを使用して書式指定文字列を拡大?画する( 縁の色引数付き )
extern	int			DrawExtendFormatVString2ToHandle(       int x, int y, double ExRateX, double ExRateY, unsigned int Color, unsigned int EdgeColor, int FontHandle, const TCHAR *FormatString, ... ) ;																			// フォントハンドルを使用して書式指定文字列を拡大?画する( 縁の色引数付き )( 縦書き )
extern	int			DrawRotaStringToHandle(					int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, int FontHandle, unsigned int EdgeColor, int VerticalFlag, const TCHAR *String                            ) ;		// フォントハンドルを使用して文字列を回??画する
extern	int			DrawRotaNStringToHandle(				int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, int FontHandle, unsigned int EdgeColor, int VerticalFlag, const TCHAR *String,       size_t StringLength ) ;		// フォントハンドルを使用して文字列を回??画する
extern	int			DrawRotaFormatStringToHandle(			int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, int FontHandle, unsigned int EdgeColor, int VerticalFlag, const TCHAR *FormatString, ...                 ) ;		// フォントハンドルを使用して書式指定文字列を回??画する
extern	int			DrawModiStringToHandle(					int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, unsigned int Color, int FontHandle, unsigned int EdgeColor, int VerticalFlag, const TCHAR *String                            ) ;	// フォントハンドルを使用して文字列を変??画する
extern	int			DrawModiNStringToHandle(				int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, unsigned int Color, int FontHandle, unsigned int EdgeColor, int VerticalFlag, const TCHAR *String,       size_t StringLength ) ;	// フォントハンドルを使用して文字列を変??画する
extern	int			DrawModiFormatStringToHandle(			int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, unsigned int Color, int FontHandle, unsigned int EdgeColor, int VerticalFlag, const TCHAR *FormatString, ...                 ) ;	// フォントハンドルを使用して書式指定文字列を変??画する

extern	int			DrawStringFToHandle(                    float x, float y, const TCHAR *String,                      unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;									// フォントハンドルを使用して文字列を?画する( 座標指定が float 版 )
extern	int			DrawNStringFToHandle(                   float x, float y, const TCHAR *String, size_t StringLength, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;									// フォントハンドルを使用して文字列を?画する( 座標指定が float 版 )
extern	int			DrawVStringFToHandle(                   float x, float y, const TCHAR *String,                      unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;																				// フォントハンドルを使用して文字列を?画する( 縦書き )( 座標指定が float 版 )
extern	int			DrawNVStringFToHandle(                  float x, float y, const TCHAR *String, size_t StringLength, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;																				// フォントハンドルを使用して文字列を?画する( 縦書き )( 座標指定が float 版 )
extern	int			DrawFormatStringFToHandle(              float x, float y, unsigned int Color,                         int FontHandle, const TCHAR *FormatString, ... ) ;																											// フォントハンドルを使用して書式指定文字列を?画する( 座標指定が float 版 )
extern	int			DrawFormatVStringFToHandle(             float x, float y, unsigned int Color,                         int FontHandle, const TCHAR *FormatString, ... ) ;																											// フォントハンドルを使用して書式指定文字列を?画する( 縦書き )( 座標指定が float 版 )
extern	int			DrawFormatString2FToHandle(             float x, float y, unsigned int Color, unsigned int EdgeColor, int FontHandle, const TCHAR *FormatString, ... ) ;																											// フォントハンドルを使用して書式指定文字列を?画する( 縁の色引数付き )( 座標指定が float 版 )
extern	int			DrawFormatVString2FToHandle(            float x, float y, unsigned int Color, unsigned int EdgeColor, int FontHandle, const TCHAR *FormatString, ... ) ;																											// フォントハンドルを使用して書式指定文字列を?画する( 縁の色引数付き )( 縦書き )( 座標指定が float 版 )
extern	int			DrawExtendStringFToHandle(              float x, float y, double ExRateX, double ExRateY, const TCHAR *String,                      unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// フォントハンドルを使用して文字列を拡大?画する( 座標指定が float 版 )
extern	int			DrawExtendNStringFToHandle(             float x, float y, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;	// フォントハンドルを使用して文字列を拡大?画する( 座標指定が float 版 )
extern	int			DrawExtendVStringFToHandle(             float x, float y, double ExRateX, double ExRateY, const TCHAR *String,                      unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;												// フォントハンドルを使用して文字列を拡大?画する( 縦書き )( 座標指定が float 版 )
extern	int			DrawExtendNVStringFToHandle(            float x, float y, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;												// フォントハンドルを使用して文字列を拡大?画する( 縦書き )( 座標指定が float 版 )
extern	int			DrawExtendFormatStringFToHandle(        float x, float y, double ExRateX, double ExRateY, unsigned int Color,                         int FontHandle, const TCHAR *FormatString, ... ) ;																			// フォントハンドルを使用して書式指定文字列を拡大?画する( 座標指定が float 版 )
extern	int			DrawExtendFormatVStringFToHandle(       float x, float y, double ExRateX, double ExRateY, unsigned int Color,                         int FontHandle, const TCHAR *FormatString, ... ) ;																			// フォントハンドルを使用して書式指定文字列を拡大?画する( 縦書き )( 座標指定が float 版 )
extern	int			DrawExtendFormatString2FToHandle(       float x, float y, double ExRateX, double ExRateY, unsigned int Color, unsigned int EdgeColor, int FontHandle, const TCHAR *FormatString, ... ) ;																			// フォントハンドルを使用して書式指定文字列を拡大?画する( 縁の色引数付き )( 座標指定が float 版 )
extern	int			DrawExtendFormatVString2FToHandle(      float x, float y, double ExRateX, double ExRateY, unsigned int Color, unsigned int EdgeColor, int FontHandle, const TCHAR *FormatString, ... ) ;																			// フォントハンドルを使用して書式指定文字列を拡大?画する( 縁の色引数付き )( 縦書き )( 座標指定が float 版 )
extern	int			DrawRotaStringFToHandle(				float x, float y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *String       DEFAULTPARAM( = NULL )                           ) ;		// フォントハンドルを使用して文字列を回??画する( 座標指定が float 版 )
extern	int			DrawRotaNStringFToHandle(				float x, float y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *String       DEFAULTPARAM( = NULL ) , size_t StringLength DEFAULTPARAM( = 0 ) ) ;		// フォントハンドルを使用して文字列を回??画する( 座標指定が float 版 )
extern	int			DrawRotaFormatStringFToHandle(			float x, float y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) , int VerticalFlag DEFAULTPARAM( = FALSE ) , const TCHAR *FormatString DEFAULTPARAM( = NULL ) , ...                     ) ;		// フォントハンドルを使用して書式指定文字列を回??画する( 座標指定が float 版 )
extern	int			DrawModiStringFToHandle(				float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, unsigned int Color, int FontHandle, unsigned int EdgeColor, int VerticalFlag, const TCHAR *String                            ) ;		// フォントハンドルを使用して文字列を変??画する( 座標指定が float 版 )
extern	int			DrawModiNStringFToHandle(				float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, unsigned int Color, int FontHandle, unsigned int EdgeColor, int VerticalFlag, const TCHAR *String,       size_t StringLength ) ;		// フォントハンドルを使用して文字列を変??画する( 座標指定が float 版 )
extern	int			DrawModiFormatStringFToHandle(			float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, unsigned int Color, int FontHandle, unsigned int EdgeColor, int VerticalFlag, const TCHAR *FormatString, ...                 ) ;		// フォントハンドルを使用して書式指定文字列を変??画する( 座標指定が float 版 )

extern	int			DrawNumberToIToHandle(                  int x, int y,                          int    Num, int RisesNum, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;											// フォントハンドルを使用して整数?の数値を?画する
extern	int			DrawNumberToFToHandle(                  int x, int y,                          double Num, int Length,   unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;											// フォントハンドルを使用して浮動小数??の数値を?画する
extern	int			DrawNumberPlusToIToHandle(              int x, int y, const TCHAR *NoteString, int    Num, int RisesNum, unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;											// フォントハンドルを使用して整数?の数値とその説明の文字列を一度に?画する
extern	int			DrawNumberPlusToFToHandle(              int x, int y, const TCHAR *NoteString, double Num, int Length,   unsigned int Color, int FontHandle, unsigned int EdgeColor DEFAULTPARAM( = 0 ) ) ;											// フォントハンドルを使用して浮動小数??の数値とその説明の文字列を一度に?画する

extern	int			DrawStringToHandleToZBuffer(            int x, int y, const TCHAR *String,                      int FontHandle,                                                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;			// フォントハンドルを使用してＺバッフ?に対して文字列を?画する
extern	int			DrawNStringToHandleToZBuffer(           int x, int y, const TCHAR *String, size_t StringLength, int FontHandle,                                                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;			// フォントハンドルを使用してＺバッフ?に対して文字列を?画する
extern	int			DrawVStringToHandleToZBuffer(           int x, int y, const TCHAR *String,                      int FontHandle,                                                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;													// フォントハンドルを使用してＺバッフ?に対して文字列を?画する( 縦書き )
extern	int			DrawNVStringToHandleToZBuffer(          int x, int y, const TCHAR *String, size_t StringLength, int FontHandle,                                                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;													// フォントハンドルを使用してＺバッフ?に対して文字列を?画する( 縦書き )
extern	int			DrawFormatStringToHandleToZBuffer(      int x, int y, int FontHandle,                                                                                        int WriteZMode /* DX_ZWRITE_MASK 等 */ , const TCHAR *FormatString, ... ) ;										// フォントハンドルを使用してＺバッフ?に対して書式指定文字列を?画する
extern	int			DrawFormatVStringToHandleToZBuffer(     int x, int y, int FontHandle,                                                                                        int WriteZMode /* DX_ZWRITE_MASK 等 */ , const TCHAR *FormatString, ... ) ;										// フォントハンドルを使用してＺバッフ?に対して書式指定文字列を?画する( 縦書き )
extern	int			DrawExtendStringToHandleToZBuffer(      int x, int y, double ExRateX, double ExRateY, const TCHAR *String,                      int FontHandle,                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;			// フォントハンドルを使用してＺバッフ?に対して文字列を拡大?画する
extern	int			DrawExtendNStringToHandleToZBuffer(     int x, int y, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength, int FontHandle,                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;			// フォントハンドルを使用してＺバッフ?に対して文字列を拡大?画する
extern	int			DrawExtendVStringToHandleToZBuffer(     int x, int y, double ExRateX, double ExRateY, const TCHAR *String,                      int FontHandle,                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;													// フォントハンドルを使用してＺバッフ?に対して文字列を拡大?画する( 縦書き )
extern	int			DrawExtendNVStringToHandleToZBuffer(    int x, int y, double ExRateX, double ExRateY, const TCHAR *String, size_t StringLength, int FontHandle,                                   int WriteZMode /* DX_ZWRITE_MASK 等 */ ) ;													// フォントハンドルを使用してＺバッフ?に対して文字列を拡大?画する( 縦書き )
extern	int			DrawExtendFormatStringToHandleToZBuffer(   int x, int y, double ExRateX, double ExRateY, int FontHandle,                                                      int WriteZMode /* DX_ZWRITE_MASK 等 */ , const TCHAR *FormatString, ... ) ;										// フォントハンドルを使用してＺバッフ?に対して書式指定文字列を拡大?画する
extern	int			DrawExtendFormatVStringToHandleToZBuffer(  int x, int y, double ExRateX, double ExRateY, int FontHandle,                                                      int WriteZMode /* DX_ZWRITE_MASK 等 */ , const TCHAR *FormatString, ... ) ;										// フォントハンドルを使用してＺバッフ?に対して書式指定文字列を拡大?画する( 縦書き )
extern	int			DrawRotaStringToHandleToZBuffer(		int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag, const TCHAR *String                            ) ;		// フォントハンドルを使用してＺバッフ?に対して文字列を回??画する
extern	int			DrawRotaNStringToHandleToZBuffer(		int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag, const TCHAR *String,       size_t StringLength ) ;		// フォントハンドルを使用してＺバッフ?に対して文字列を回??画する
extern	int			DrawRotaFormatStringToHandleToZBuffer(	int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag, const TCHAR *FormatString, ...                 ) ;		// フォントハンドルを使用してＺバッフ?に対して書式指定文字列を回??画する
extern	int			DrawModiStringToHandleToZBuffer(		int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag, const TCHAR *String                            ) ;											// フォントハンドルを使用してＺバッフ?に対して文字列を変??画する
extern	int			DrawModiNStringToHandleToZBuffer(		int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag, const TCHAR *String,       size_t StringLength ) ;											// フォントハンドルを使用してＺバッフ?に対して文字列を変??画する
extern	int			DrawModiFormatStringToHandleToZBuffer(	int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag, const TCHAR *FormatString, ...                 ) ;											// フォントハンドルを使用してＺバッフ?に対して書式指定文字列を変??画する

#endif // DX_NON_FONT








// DxMath.cpp 関数プロト?イプ宣言

// 演算ライブラリ
extern	int			ConvertMatrixFtoD(             MATRIX_D *Out, const MATRIX   *In ) ;															// float?要素の行列をdouble?要素の行列に変換する
extern	int			ConvertMatrixDtoF(             MATRIX   *Out, const MATRIX_D *In ) ;															// double?要素の行列をfloat?要素の行列に変換する
extern	int			CreateIdentityMatrix(          MATRIX   *Out ) ;																				// 単位行列を作成する
extern	int			CreateIdentityMatrixD(         MATRIX_D *Out ) ;																				// 単位行列を作成する
extern	int			CreateLookAtMatrix(            MATRIX   *Out, const VECTOR   *Eye, const VECTOR   *At, const VECTOR   *Up ) ;					// ビュ?行列を作成する
extern	int			CreateLookAtMatrixD(           MATRIX_D *Out, const VECTOR_D *Eye, const VECTOR_D *At, const VECTOR_D *Up ) ;					// ビュ?行列を作成する
extern	int			CreateLookAtMatrix2(           MATRIX   *Out, const VECTOR   *Eye, double XZAngle, double Oira ) ;								// ビュ?行列を作成する(方向を回?値で指定)
extern	int			CreateLookAtMatrix2D(          MATRIX_D *Out, const VECTOR_D *Eye, double XZAngle, double Oira ) ;								// ビュ?行列を作成する(方向を回?値で指定)
extern	int			CreateLookAtMatrixRH(          MATRIX   *Out, const VECTOR   *Eye, const VECTOR   *At, const VECTOR   *Up ) ;					// ビュ?行列を作成する(右手座標系用)
extern	int			CreateLookAtMatrixRHD(         MATRIX_D *Out, const VECTOR_D *Eye, const VECTOR_D *At, const VECTOR_D *Up ) ;					// ビュ?行列を作成する(右手座標系用)
extern	int			CreateMultiplyMatrix(          MATRIX   *Out, const MATRIX   *In1, const MATRIX   *In2 ) ;										// 行列の積を求める
extern	int			CreateMultiplyMatrixD(         MATRIX_D *Out, const MATRIX_D *In1, const MATRIX_D *In2 ) ;										// 行列の積を求める
extern	int			CreatePerspectiveFovMatrix(    MATRIX   *Out, float  fov,  float  zn, float  zf, float  aspect DEFAULTPARAM( = -1.0f ) ) ;		// 射影行列を作成する
extern	int			CreatePerspectiveFovMatrixD(   MATRIX_D *Out, double fov,  double zn, double zf, double aspect DEFAULTPARAM( = -1.0f ) ) ;		// 射影行列を作成する
extern	int			CreatePerspectiveFovMatrixRH(  MATRIX   *Out, float  fov,  float  zn, float  zf, float  aspect DEFAULTPARAM( = -1.0f ) ) ;		// 射影行列を作成する(右手座標系用)
extern	int			CreatePerspectiveFovMatrixRHD( MATRIX_D *Out, double fov,  double zn, double zf, double aspect DEFAULTPARAM( = -1.0f ) ) ;		// 射影行列を作成する(右手座標系用)
extern	int			CreateOrthoMatrix(             MATRIX   *Out, float  size, float  zn, float  zf, float  aspect DEFAULTPARAM( = -1.0f ) ) ;		// 正射影行列を作成する
extern	int			CreateOrthoMatrixD(            MATRIX_D *Out, double size, double zn, double zf, double aspect DEFAULTPARAM( = -1.0f ) ) ;		// 正射影行列を作成する
extern	int			CreateOrthoMatrixRH(           MATRIX   *Out, float  size, float  zn, float  zf, float  aspect DEFAULTPARAM( = -1.0f ) ) ;		// 正射影行列を作成する(右手座標系用)
extern	int			CreateOrthoMatrixRHD(          MATRIX_D *Out, double size, double zn, double zf, double aspect DEFAULTPARAM( = -1.0f ) ) ;		// 正射影行列を作成する(右手座標系用)
extern	int			CreateScalingMatrix(           MATRIX   *Out, float  sx, float  sy, float  sz ) ;												// スケ?リング行列を作成する
extern	int			CreateScalingMatrixD(          MATRIX_D *Out, double sx, double sy, double sz ) ;												// スケ?リング行列を作成する
extern	int			CreateRotationXMatrix(         MATRIX   *Out, float  Angle ) ;																	// Ｘ軸を中心とした回?行列を作成する
extern	int			CreateRotationXMatrixD(        MATRIX_D *Out, double Angle ) ;																	// Ｘ軸を中心とした回?行列を作成する
extern	int			CreateRotationYMatrix(         MATRIX   *Out, float  Angle ) ;																	// Ｙ軸を中心とした回?行列を作成する
extern	int			CreateRotationYMatrixD(        MATRIX_D *Out, double Angle ) ;																	// Ｙ軸を中心とした回?行列を作成する
extern	int			CreateRotationZMatrix(         MATRIX   *Out, float  Angle ) ;																	// Ｚ軸を中心とした回?行列を作成する
extern	int			CreateRotationZMatrixD(        MATRIX_D *Out, double Angle ) ;																	// Ｚ軸を中心とした回?行列を作成する
extern	int			CreateTranslationMatrix(       MATRIX   *Out, float  x, float  y, float  z ) ;													// 平行移動行列を作成する
extern	int			CreateTranslationMatrixD(      MATRIX_D *Out, double x, double y, double z ) ;													// 平行移動行列を作成する
extern	int			CreateTransposeMatrix(         MATRIX   *Out, const MATRIX   *In ) ;															// ?置行列を作成する
extern	int			CreateTransposeMatrixD(        MATRIX_D *Out, const MATRIX_D *In ) ;															// ?置行列を作成する
extern	int			CreateInverseMatrix(           MATRIX   *Out, const MATRIX   *In ) ;															// 逆行列を作成する
extern	int			CreateInverseMatrixD(          MATRIX_D *Out, const MATRIX_D *In ) ;															// 逆行列を作成する
extern	int			CreateViewportMatrix(          MATRIX   *Out, float  CenterX, float  CenterY, float  Width, float  Height ) ;					// ビュ???ト行列を作成する
extern	int			CreateViewportMatrixD(         MATRIX_D *Out, double CenterX, double CenterY, double Width, double Height ) ;					// ビュ???ト行列を作成する
extern	int			CreateRotationXYZMatrix(       MATRIX   *Out, float  XRot, float  YRot, float  ZRot ) ;											// Ｘ軸回?→Ｙ軸回?→Ｚ軸回?を合成した行列を作成する
extern	int			CreateRotationXYZMatrixD(      MATRIX_D *Out, double XRot, double YRot, double ZRot ) ;											// Ｘ軸回?→Ｙ軸回?→Ｚ軸回?を合成した行列を作成する
extern	int			CreateRotationXZYMatrix(       MATRIX   *Out, float  XRot, float  YRot, float  ZRot ) ;											// Ｘ軸回?→Ｚ軸回?→Ｙ軸回?を合成した行列を作成する
extern	int			CreateRotationXZYMatrixD(      MATRIX_D *Out, double XRot, double YRot, double ZRot ) ;											// Ｘ軸回?→Ｚ軸回?→Ｙ軸回?を合成した行列を作成する
extern	int			CreateRotationYXZMatrix(       MATRIX   *Out, float  XRot, float  YRot, float  ZRot ) ;											// Ｙ軸回?→Ｘ軸回?→Ｚ軸回?を合成した行列を作成する
extern	int			CreateRotationYXZMatrixD(      MATRIX_D *Out, double XRot, double YRot, double ZRot ) ;											// Ｙ軸回?→Ｘ軸回?→Ｚ軸回?を合成した行列を作成する
extern	int			CreateRotationYZXMatrix(       MATRIX   *Out, float  XRot, float  YRot, float  ZRot ) ;											// Ｙ軸回?→Ｚ軸回?→Ｘ軸回?を合成した行列を作成する
extern	int			CreateRotationYZXMatrixD(      MATRIX_D *Out, double XRot, double YRot, double ZRot ) ;											// Ｙ軸回?→Ｚ軸回?→Ｘ軸回?を合成した行列を作成する
extern	int			CreateRotationZXYMatrix(       MATRIX   *Out, float  XRot, float  YRot, float  ZRot ) ;											// Ｚ軸回?→Ｘ軸回?→Ｙ軸回?を合成した行列を作成する
extern	int			CreateRotationZXYMatrixD(      MATRIX_D *Out, double XRot, double YRot, double ZRot ) ;											// Ｚ軸回?→Ｘ軸回?→Ｙ軸回?を合成した行列を作成する
extern	int			CreateRotationZYXMatrix(       MATRIX   *Out, float  XRot, float  YRot, float  ZRot ) ;											// Ｚ軸回?→Ｙ軸回?→Ｘ軸回?を合成した行列を作成する
extern	int			CreateRotationZYXMatrixD(      MATRIX_D *Out, double XRot, double YRot, double ZRot ) ;											// Ｚ軸回?→Ｙ軸回?→Ｘ軸回?を合成した行列を作成する
extern	int			GetMatrixXYZRotation(          const MATRIX   *In, float  *OutXRot, float  *OutYRot, float  *OutZRot ) ;						// 行列からＸＹＺ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			GetMatrixXYZRotationD(         const MATRIX_D *In, double *OutXRot, double *OutYRot, double *OutZRot ) ;						// 行列からＸＹＺ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			GetMatrixXZYRotation(          const MATRIX   *In, float  *OutXRot, float  *OutYRot, float  *OutZRot ) ;						// 行列からＸＺＹ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			GetMatrixXZYRotationD(         const MATRIX_D *In, double *OutXRot, double *OutYRot, double *OutZRot ) ;						// 行列からＸＺＹ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			GetMatrixYXZRotation(          const MATRIX   *In, float  *OutXRot, float  *OutYRot, float  *OutZRot ) ;						// 行列からＹＸＺ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			GetMatrixYXZRotationD(         const MATRIX_D *In, double *OutXRot, double *OutYRot, double *OutZRot ) ;						// 行列からＹＸＺ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			GetMatrixYZXRotation(          const MATRIX   *In, float  *OutXRot, float  *OutYRot, float  *OutZRot ) ;						// 行列からＹＺＸ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			GetMatrixYZXRotationD(         const MATRIX_D *In, double *OutXRot, double *OutYRot, double *OutZRot ) ;						// 行列からＹＺＸ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			GetMatrixZXYRotation(          const MATRIX   *In, float  *OutXRot, float  *OutYRot, float  *OutZRot ) ;						// 行列からＺＸＹ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			GetMatrixZXYRotationD(         const MATRIX_D *In, double *OutXRot, double *OutYRot, double *OutZRot ) ;						// 行列からＺＸＹ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			GetMatrixZYXRotation(          const MATRIX   *In, float  *OutXRot, float  *OutYRot, float  *OutZRot ) ;						// 行列からＺＹＸ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			GetMatrixZYXRotationD(         const MATRIX_D *In, double *OutXRot, double *OutYRot, double *OutZRot ) ;						// 行列からＺＹＸ軸回?の値を取得する( 戻り値?-1:ジンバルロック発生  0:成功 )
extern	int			VectorConvertFtoD(             VECTOR_D *Out, const VECTOR   *In ) ;															// float?のベクトルをdouble?のベクトルに変換する
extern	int			VectorConvertDtoF(             VECTOR   *Out, const VECTOR_D *In ) ;															// double?のベクトルをfloat?のベクトルに変換する
extern	int			VectorNormalize(               VECTOR   *Out, const VECTOR   *In ) ;															// ベクトルを正規化する
extern	int			VectorNormalizeD(              VECTOR_D *Out, const VECTOR_D *In ) ;															// ベクトルを正規化する
extern	int			VectorScale(                   VECTOR   *Out, const VECTOR   *In, float  Scale ) ;												// ベクトルをスカラ??する
extern	int			VectorScaleD(                  VECTOR_D *Out, const VECTOR_D *In, double Scale ) ;												// ベクトルをスカラ??する
extern	int			VectorMultiply(                VECTOR   *Out, const VECTOR   *In1, const VECTOR   *In2 ) ;										// ベクトルの?け算をする
extern	int			VectorMultiplyD(               VECTOR_D *Out, const VECTOR_D *In1, const VECTOR_D *In2 ) ;										// ベクトルの?け算をする
extern	int			VectorSub(                     VECTOR   *Out, const VECTOR   *In1, const VECTOR   *In2 ) ;										// Out = In1 - In2 のベクトル計算をする 
extern	int			VectorSubD(                    VECTOR_D *Out, const VECTOR_D *In1, const VECTOR_D *In2 ) ;										// Out = In1 - In2 のベクトル計算をする 
extern	int			VectorAdd(                     VECTOR   *Out, const VECTOR   *In1, const VECTOR   *In2 ) ;										// Out = In1 + In2 のベクトル計算をする 
extern	int			VectorAddD(                    VECTOR_D *Out, const VECTOR_D *In1, const VECTOR_D *In2 ) ;										// Out = In1 + In2 のベクトル計算をする 
extern	int			VectorOuterProduct(            VECTOR   *Out, const VECTOR   *In1, const VECTOR   *In2 ) ;										// In1とIn2の外積を計算する
extern	int			VectorOuterProductD(           VECTOR_D *Out, const VECTOR_D *In1, const VECTOR_D *In2 ) ;										// In1とIn2の外積を計算する
extern	float 		VectorInnerProduct(            const VECTOR   *In1, const VECTOR   *In2 ) ;														// In1とIn2の内積を計算する
extern	double		VectorInnerProductD(           const VECTOR_D *In1, const VECTOR_D *In2 ) ;														// In1とIn2の内積を計算する
extern	int			VectorRotationX(               VECTOR   *Out, const VECTOR   *In, double Angle ) ;												// ベクトルのＸ軸を軸にした回?を行う
extern	int			VectorRotationXD(              VECTOR_D *Out, const VECTOR_D *In, double Angle ) ;												// ベクトルのＸ軸を軸にした回?を行う
extern	int			VectorRotationY(               VECTOR   *Out, const VECTOR   *In, double Angle ) ;												// ベクトルのＹ軸を軸にした回?を行う
extern	int			VectorRotationYD(              VECTOR_D *Out, const VECTOR_D *In, double Angle ) ;												// ベクトルのＹ軸を軸にした回?を行う
extern	int			VectorRotationZ(               VECTOR   *Out, const VECTOR   *In, double Angle ) ;												// ベクトルのＺ軸を軸にした回?を行う
extern	int			VectorRotationZD(              VECTOR_D *Out, const VECTOR_D *In, double Angle ) ;												// ベクトルのＺ軸を軸にした回?を行う
extern	int			VectorTransform(               VECTOR   *Out, const VECTOR   *InVec, const MATRIX   *InMatrix ) ;								// ベクトル行列と4x4正方行列を乗算する( w は 1 と仮定 )
extern	int			VectorTransformD(              VECTOR_D *Out, const VECTOR_D *InVec, const MATRIX_D *InMatrix ) ;								// ベクトル行列と4x4正方行列を乗算する( w は 1 と仮定 )
extern	int			VectorTransformSR(             VECTOR   *Out, const VECTOR   *InVec, const MATRIX   *InMatrix ) ;								// ベクトル行列と4x4正方行列の回?部分のみを乗算する
extern	int			VectorTransformSRD(            VECTOR_D *Out, const VECTOR_D *InVec, const MATRIX_D *InMatrix ) ;								// ベクトル行列と4x4正方行列の回?部分のみを乗算する
extern	int			VectorTransform4(              VECTOR   *Out, float  *V4Out, const VECTOR   *InVec, const float  *V4In, const MATRIX   *InMatrix ) ; // ベクトル行列と4x4正方行列を乗算する( w の要素を渡す )
extern	int			VectorTransform4D(             VECTOR_D *Out, double *V4Out, const VECTOR_D *InVec, const double *V4In, const MATRIX_D *InMatrix ) ; // ベクトル行列と4x4正方行列を乗算する( w の要素を渡す )

extern	int			Segment_Segment_Analyse(      const VECTOR   *SegmentAPos1, const VECTOR   *SegmentAPos2, const VECTOR   *SegmentBPos1, const VECTOR   *SegmentBPos2, SEGMENT_SEGMENT_RESULT   *Result ) ;								// 二つの線分の最接近?情報を解析する
extern	int			Segment_Segment_AnalyseD(     const VECTOR_D *SegmentAPos1, const VECTOR_D *SegmentAPos2, const VECTOR_D *SegmentBPos1, const VECTOR_D *SegmentBPos2, SEGMENT_SEGMENT_RESULT_D *Result ) ;								// 二つの線分の最接近?情報を解析する
extern	int			Segment_Point_Analyse(        const VECTOR   *SegmentPos1, const VECTOR   *SegmentPos2, const VECTOR   *PointPos, SEGMENT_POINT_RESULT   *Result ) ;																	// 線分と?の最接近?情報を解析する
extern	int			Segment_Point_AnalyseD(       const VECTOR_D *SegmentPos1, const VECTOR_D *SegmentPos2, const VECTOR_D *PointPos, SEGMENT_POINT_RESULT_D *Result ) ;																	// 線分と?の最接近?情報を解析する
extern	int			Segment_Triangle_Analyse(     const VECTOR   *SegmentPos1, const VECTOR   *SegmentPos2, const VECTOR   *TrianglePos1, const VECTOR   *TrianglePos2, const VECTOR   *TrianglePos3, SEGMENT_TRIANGLE_RESULT   *Result ) ;	// 線分と三角?の最接近?情報を解析する
extern	int			Segment_Triangle_AnalyseD(    const VECTOR_D *SegmentPos1, const VECTOR_D *SegmentPos2, const VECTOR_D *TrianglePos1, const VECTOR_D *TrianglePos2, const VECTOR_D *TrianglePos3, SEGMENT_TRIANGLE_RESULT_D *Result ) ;	// 線分と三角?の最接近?情報を解析する
extern	int			Triangle_Point_Analyse(       const VECTOR   *TrianglePos1, const VECTOR   *TrianglePos2, const VECTOR   *TrianglePos3, const VECTOR   *PointPos, TRIANGLE_POINT_RESULT   *Result ) ;									// 三角?と?の最接近?情報を解析する
extern	int			Triangle_Point_AnalyseD(      const VECTOR_D *TrianglePos1, const VECTOR_D *TrianglePos2, const VECTOR_D *TrianglePos3, const VECTOR_D *PointPos, TRIANGLE_POINT_RESULT_D *Result ) ;									// 三角?と?の最接近?情報を解析する
extern	int			Plane_Point_Analyse(          const VECTOR   *PlanePos, const VECTOR   *PlaneNormal, const VECTOR   *PointPos, PLANE_POINT_RESULT   *Result ) ;																			// 平面と?の最近?情報を解析する
extern	int			Plane_Point_AnalyseD(         const VECTOR_D *PlanePos, const VECTOR_D *PlaneNormal, const VECTOR_D *PointPos, PLANE_POINT_RESULT_D *Result ) ;																			// 平面と?の最近?情報を解析する

extern	void		TriangleBarycenter(           VECTOR   TrianglePos1, VECTOR   TrianglePos2, VECTOR   TrianglePos3, VECTOR   Position, float  *TrianglePos1Weight, float  *TrianglePos2Weight, float  *TrianglePos3Weight ) ;	// 指定の座標から三角?の重心を求める
extern	void		TriangleBarycenterD(          VECTOR_D TrianglePos1, VECTOR_D TrianglePos2, VECTOR_D TrianglePos3, VECTOR_D Position, double *TrianglePos1Weight, double *TrianglePos2Weight, double *TrianglePos3Weight ) ;	// 指定の座標から三角?の重心を求める

extern	float 		Segment_Segment_MinLength(           VECTOR   SegmentAPos1, VECTOR   SegmentAPos2, VECTOR   SegmentBPos1, VECTOR   SegmentBPos2 ) ;														// 二つの線分の最近?間の距離を得る
extern	double		Segment_Segment_MinLengthD(          VECTOR_D SegmentAPos1, VECTOR_D SegmentAPos2, VECTOR_D SegmentBPos1, VECTOR_D SegmentBPos2 ) ;														// 二つの線分の最近?間の距離を得る
extern	float 		Segment_Segment_MinLength_Square(    VECTOR   SegmentAPos1, VECTOR   SegmentAPos2, VECTOR   SegmentBPos1, VECTOR   SegmentBPos2 ) ;														// 二つの線分の最近?間の距離の二乗を得る
extern	double		Segment_Segment_MinLength_SquareD(   VECTOR_D SegmentAPos1, VECTOR_D SegmentAPos2, VECTOR_D SegmentBPos1, VECTOR_D SegmentBPos2 ) ;														// 二つの線分の最近?間の距離の二乗を得る
extern	float 		Segment_Triangle_MinLength(          VECTOR   SegmentPos1, VECTOR   SegmentPos2, VECTOR   TrianglePos1, VECTOR   TrianglePos2, VECTOR   TrianglePos3 ) ;								// 線分と三角?の最近?間の距離を得る 
extern	double		Segment_Triangle_MinLengthD(         VECTOR_D SegmentPos1, VECTOR_D SegmentPos2, VECTOR_D TrianglePos1, VECTOR_D TrianglePos2, VECTOR_D TrianglePos3 ) ;								// 線分と三角?の最近?間の距離を得る 
extern	float 		Segment_Triangle_MinLength_Square(   VECTOR   SegmentPos1, VECTOR   SegmentPos2, VECTOR   TrianglePos1, VECTOR   TrianglePos2, VECTOR   TrianglePos3 ) ;								// 線分と三角?の最近?間の距離の二乗を得る 
extern	double		Segment_Triangle_MinLength_SquareD(  VECTOR_D SegmentPos1, VECTOR_D SegmentPos2, VECTOR_D TrianglePos1, VECTOR_D TrianglePos2, VECTOR_D TrianglePos3 ) ;								// 線分と三角?の最近?間の距離の二乗を得る 
extern	float 		Segment_Point_MinLength(             VECTOR   SegmentPos1, VECTOR   SegmentPos2, VECTOR   PointPos ) ;																					// 線分と?の一番近い距離を得る
extern	double		Segment_Point_MinLengthD(            VECTOR_D SegmentPos1, VECTOR_D SegmentPos2, VECTOR_D PointPos ) ;																					// 線分と?の一番近い距離を得る
extern	float 		Segment_Point_MinLength_Square(      VECTOR   SegmentPos1, VECTOR   SegmentPos2, VECTOR   PointPos ) ;																					// 線分と?の一番近い距離の二乗を得る
extern	double		Segment_Point_MinLength_SquareD(     VECTOR_D SegmentPos1, VECTOR_D SegmentPos2, VECTOR_D PointPos ) ;																					// 線分と?の一番近い距離の二乗を得る
extern	float 		Triangle_Point_MinLength(            VECTOR   TrianglePos1, VECTOR   TrianglePos2, VECTOR   TrianglePos3, VECTOR   PointPos ) ;															// 三角?と?の一番近い距離を得る
extern	double		Triangle_Point_MinLengthD(           VECTOR_D TrianglePos1, VECTOR_D TrianglePos2, VECTOR_D TrianglePos3, VECTOR_D PointPos ) ;															// 三角?と?の一番近い距離を得る
extern	float 		Triangle_Point_MinLength_Square(     VECTOR   TrianglePos1, VECTOR   TrianglePos2, VECTOR   TrianglePos3, VECTOR   PointPos ) ;															// 三角?と?の一番近い距離の二乗を得る
extern	double		Triangle_Point_MinLength_SquareD(    VECTOR_D TrianglePos1, VECTOR_D TrianglePos2, VECTOR_D TrianglePos3, VECTOR_D PointPos ) ;															// 三角?と?の一番近い距離の二乗を得る
extern	float 		Triangle_Triangle_MinLength(         VECTOR   Triangle1Pos1, VECTOR   Triangle1Pos2, VECTOR   Triangle1Pos3, VECTOR   Triangle2Pos1, VECTOR   Triangle2Pos2, VECTOR   Triangle2Pos3 ) ;	// 二つの三角?の最近?間の距離を得る
extern	double		Triangle_Triangle_MinLengthD(        VECTOR_D Triangle1Pos1, VECTOR_D Triangle1Pos2, VECTOR_D Triangle1Pos3, VECTOR_D Triangle2Pos1, VECTOR_D Triangle2Pos2, VECTOR_D Triangle2Pos3 ) ;	// 二つの三角?の最近?間の距離を得る
extern	float 		Triangle_Triangle_MinLength_Square(  VECTOR   Triangle1Pos1, VECTOR   Triangle1Pos2, VECTOR   Triangle1Pos3, VECTOR   Triangle2Pos1, VECTOR   Triangle2Pos2, VECTOR   Triangle2Pos3 ) ;	// 二つの三角?の最近?間の距離の二乗を得る
extern	double		Triangle_Triangle_MinLength_SquareD( VECTOR_D Triangle1Pos1, VECTOR_D Triangle1Pos2, VECTOR_D Triangle1Pos3, VECTOR_D Triangle2Pos1, VECTOR_D Triangle2Pos2, VECTOR_D Triangle2Pos3 ) ;	// 二つの三角?の最近?間の距離の二乗を得る
extern	VECTOR  	Plane_Point_MinLength_Position(      VECTOR   PlanePos, VECTOR   PlaneNormal, VECTOR   PointPos ) ;																						// ?に一番近い平面上の座標を得る
extern	VECTOR_D	Plane_Point_MinLength_PositionD(     VECTOR_D PlanePos, VECTOR_D PlaneNormal, VECTOR_D PointPos ) ;																						// ?に一番近い平面上の座標を得る
extern	float 		Plane_Point_MinLength(               VECTOR   PlanePos, VECTOR   PlaneNormal, VECTOR   PointPos ) ;																						// 平面と?の一番近い距離を得る
extern	double		Plane_Point_MinLengthD(              VECTOR_D PlanePos, VECTOR_D PlaneNormal, VECTOR_D PointPos ) ;																						// 平面と?の一番近い距離を得る

extern	HITRESULT_LINE   HitCheck_Line_Triangle(         VECTOR   LinePos1, VECTOR   LinePos2, VECTOR   TrianglePos1, VECTOR   TrianglePos2, VECTOR   TrianglePos3 ) ;										// 三角?と線の当たり判定
extern	HITRESULT_LINE_D HitCheck_Line_TriangleD(        VECTOR_D LinePos1, VECTOR_D LinePos2, VECTOR_D TrianglePos1, VECTOR_D TrianglePos2, VECTOR_D TrianglePos3 ) ;										// 三角?と線の当たり判定
extern	int			HitCheck_Triangle_Triangle(          VECTOR   Triangle1Pos1, VECTOR   Triangle1Pos2, VECTOR   Triangle1Pos3, VECTOR   Triangle2Pos1, VECTOR   Triangle2Pos2, VECTOR   Triangle2Pos3 ) ;	// 三角?と三角?の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Triangle_TriangleD(         VECTOR_D Triangle1Pos1, VECTOR_D Triangle1Pos2, VECTOR_D Triangle1Pos3, VECTOR_D Triangle2Pos1, VECTOR_D Triangle2Pos2, VECTOR_D Triangle2Pos3 ) ;	// 三角?と三角?の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Triangle_Triangle_2D(       VECTOR   Triangle1Pos1, VECTOR   Triangle1Pos2, VECTOR   Triangle1Pos3, VECTOR   Triangle2Pos1, VECTOR   Triangle2Pos2, VECTOR   Triangle2Pos3 ) ;	// 三角?と三角?の当たり判定( ２Ｄ版 )( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Triangle_TriangleD_2D(      VECTOR_D Triangle1Pos1, VECTOR_D Triangle1Pos2, VECTOR_D Triangle1Pos3, VECTOR_D Triangle2Pos1, VECTOR_D Triangle2Pos2, VECTOR_D Triangle2Pos3 ) ;	// 三角?と三角?の当たり判定( ２Ｄ版 )( TRUE:当たっている  FALSE:当たっていない )
extern	HITRESULT_LINE   HitCheck_Line_Cube(             VECTOR   LinePos1, VECTOR   LinePos2, VECTOR   CubePos1, VECTOR   CubePos2 );																		// 線と箱の当たり判定
extern	HITRESULT_LINE_D HitCheck_Line_CubeD(            VECTOR_D LinePos1, VECTOR_D LinePos2, VECTOR_D CubePos1, VECTOR_D CubePos2 );																		// 線と箱の当たり判定
extern	int			HitCheck_Point_Cone(                 VECTOR   PointPos, VECTOR   ConeTopPos, VECTOR   ConeBottomPos, float  ConeR ) ;																	// ?と?錐の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Point_ConeD(                VECTOR_D PointPos, VECTOR_D ConeTopPos, VECTOR_D ConeBottomPos, double ConeR ) ;																	// ?と?錐の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Line_Sphere(                VECTOR   LinePos1, VECTOR   LinePos2, VECTOR   SphereCenterPos, float  SphereR ) ;																	// 線と球の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Line_SphereD(               VECTOR_D LinePos1, VECTOR_D LinePos2, VECTOR_D SphereCenterPos, double SphereR ) ;																	// 線と球の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Sphere_Sphere(              VECTOR   Sphere1CenterPos, float  Sphere1R, VECTOR   Sphere2CenterPos, float  Sphere2R ) ;															// 球と球の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Sphere_SphereD(             VECTOR_D Sphere1CenterPos, double Sphere1R, VECTOR_D Sphere2CenterPos, double Sphere2R ) ;															// 球と球の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Sphere_Capsule(             VECTOR   SphereCenterPos, float  SphereR, VECTOR   CapPos1, VECTOR   CapPos2, float  CapR ) ;														// 球とカプセルの当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Sphere_CapsuleD(            VECTOR_D SphereCenterPos, double SphereR, VECTOR_D CapPos1, VECTOR_D CapPos2, double CapR ) ;														// 球とカプセルの当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Sphere_Triangle(            VECTOR   SphereCenterPos, float  SphereR, VECTOR   TrianglePos1, VECTOR   TrianglePos2, VECTOR   TrianglePos3 ) ;									// 球と三角?の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Sphere_TriangleD(           VECTOR_D SphereCenterPos, double SphereR, VECTOR_D TrianglePos1, VECTOR_D TrianglePos2, VECTOR_D TrianglePos3 ) ;									// 球と三角?の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Capsule_Capsule(            VECTOR   Cap1Pos1, VECTOR   Cap1Pos2, float  Cap1R, VECTOR   Cap2Pos1, VECTOR   Cap2Pos2, float  Cap2R ) ;											// カプセル同士の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Capsule_CapsuleD(           VECTOR_D Cap1Pos1, VECTOR_D Cap1Pos2, double Cap1R, VECTOR_D Cap2Pos1, VECTOR_D Cap2Pos2, double Cap2R ) ;											// カプセル同士の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Capsule_Triangle(           VECTOR   CapPos1, VECTOR   CapPos2, float  CapR, VECTOR   TrianglePos1, VECTOR   TrianglePos2, VECTOR   TrianglePos3 ) ;							// カプセルと三角?の当たり判定( TRUE:当たっている  FALSE:当たっていない )
extern	int			HitCheck_Capsule_TriangleD(          VECTOR_D CapPos1, VECTOR_D CapPos2, double CapR, VECTOR_D TrianglePos1, VECTOR_D TrianglePos2, VECTOR_D TrianglePos3 ) ;							// カプセルと三角?の当たり判定( TRUE:当たっている  FALSE:当たっていない )

// その他
extern	int			RectClipping( RECT *Rect, const RECT *ClippuRect ) ;									// 矩?のクリッピング
extern	int			RectAdjust(   RECT *Rect ) ;															// 矩?の left が right より値が大きい場合などの誤りを補正する
extern	int			GetRectSize(  const RECT *Rect, int *Width, int *Height ) ;								// 矩?の幅と高さを求める

// ?記簡略版
extern	MATRIX		MGetIdent(          void ) ;															// 単位行列を取得する
extern	MATRIX_D	MGetIdentD(         void ) ;															// 単位行列を取得する
extern	MATRIX		MMult(              MATRIX   In1, MATRIX   In2 ) ;										// 行列の乗算を行う
extern	MATRIX_D	MMultD(             MATRIX_D In1, MATRIX_D In2 ) ;										// 行列の乗算を行う
extern	MATRIX		MScale(             MATRIX   InM, float  Scale ) ;										// 行列のスケ?リングを行う
extern	MATRIX_D	MScaleD(            MATRIX_D InM, double Scale ) ;										// 行列のスケ?リングを行う
extern	MATRIX		MAdd(               MATRIX   In1, MATRIX   In2 ) ;										// 行列の足し算を行う
extern	MATRIX_D	MAddD(              MATRIX_D In1, MATRIX_D In2 ) ;										// 行列の足し算を行う
extern	MATRIX		MGetScale(          VECTOR   Scale ) ;													// 拡大行列を取得する
extern	MATRIX_D	MGetScaleD(         VECTOR_D Scale ) ;													// 拡大行列を取得する
extern	MATRIX		MGetRotX(           float  XAxisRotate ) ;												// Ｘ軸回?行列を取得する
extern	MATRIX_D	MGetRotXD(          double XAxisRotate ) ;												// Ｘ軸回?行列を取得する
extern	MATRIX		MGetRotY(           float  YAxisRotate ) ;												// Ｙ軸回?行列を取得する
extern	MATRIX_D	MGetRotYD(          double YAxisRotate ) ;												// Ｙ軸回?行列を取得する
extern	MATRIX		MGetRotZ(           float  ZAxisRotate ) ;												// Ｚ軸回?行列を取得する
extern	MATRIX_D	MGetRotZD(          double ZAxisRotate ) ;												// Ｚ軸回?行列を取得する
extern	MATRIX		MGetRotAxis(        VECTOR   RotateAxis, float  Rotate ) ;								// 指定軸で指定角度回?する行列を取得する
extern	MATRIX_D	MGetRotAxisD(       VECTOR_D RotateAxis, double Rotate ) ;								// 指定軸で指定角度回?する行列を取得する
extern	MATRIX		MGetRotVec2(        VECTOR   In1, VECTOR   In2 ) ;										// In1 の向きから In2 の向きへ変換する回?行列を取得する( In2 と In1 が?逆のベクトルの場合は回?軸は不定 )
extern	MATRIX_D	MGetRotVec2D(       VECTOR_D In1, VECTOR_D In2 ) ;										// In1 の向きから In2 の向きへ変換する回?行列を取得する( In2 と In1 が?逆のベクトルの場合は回?軸は不定 )
extern	MATRIX		MGetTranslate(      VECTOR   Trans ) ;													// 平行移動行列を取得する
extern	MATRIX_D	MGetTranslateD(     VECTOR_D Trans ) ;													// 平行移動行列を取得する
extern	MATRIX		MGetAxis1(          VECTOR   XAxis, VECTOR   YAxis, VECTOR   ZAxis, VECTOR   Pos ) ;	// 指定の３軸ロ?カルのベクトルを基?軸上のベクトルに変換する行列を取得する( x' = XAxis.x * x + YAixs.x * y + ZAxis.z * z + Pos.x   y' = XAxis.y * x + YAixs.y * y + ZAxis.y * z + Pos.y      z' = XAxis.z * x + YAixs.z * y + ZAxis.z * z + Pos.z )
extern	MATRIX_D	MGetAxis1D(         VECTOR_D XAxis, VECTOR_D YAxis, VECTOR_D ZAxis, VECTOR_D Pos ) ;	// 指定の３軸ロ?カルのベクトルを基?軸上のベクトルに変換する行列を取得する( x' = XAxis.x * x + YAixs.x * y + ZAxis.z * z + Pos.x   y' = XAxis.y * x + YAixs.y * y + ZAxis.y * z + Pos.y      z' = XAxis.z * x + YAixs.z * y + ZAxis.z * z + Pos.z )
extern	MATRIX		MGetAxis2(          VECTOR   XAxis, VECTOR   YAxis, VECTOR   ZAxis, VECTOR   Pos ) ;	// 基?軸上のベクトルを指定の３軸上に投影したベクトルに変換する行列を取得する( x' = XAxis.x * ( x - Pos.x ) + XAxis.y * ( x - Pos.x ) + XAxis.z * ( x - Pos.x )    y' = YAxis.x * ( x - Pos.x ) + YAxis.y * ( x - Pos.x ) + YAxis.z * ( x - Pos.x )    z' = ZAxis.x * ( x - Pos.x ) + ZAxis.y * ( x - Pos.x ) + ZAxis.z * ( x - Pos.x ) )
extern	MATRIX_D	MGetAxis2D(         VECTOR_D XAxis, VECTOR_D YAxis, VECTOR_D ZAxis, VECTOR_D Pos ) ;	// 基?軸上のベクトルを指定の３軸上に投影したベクトルに変換する行列を取得する( x' = XAxis.x * ( x - Pos.x ) + XAxis.y * ( x - Pos.x ) + XAxis.z * ( x - Pos.x )    y' = YAxis.x * ( x - Pos.x ) + YAxis.y * ( x - Pos.x ) + YAxis.z * ( x - Pos.x )    z' = ZAxis.x * ( x - Pos.x ) + ZAxis.y * ( x - Pos.x ) + ZAxis.z * ( x - Pos.x ) )
extern	MATRIX		MTranspose(         MATRIX   InM ) ;													// ?置行列を取得する
extern	MATRIX_D	MTransposeD(        MATRIX_D InM ) ;													// ?置行列を取得する
extern	MATRIX		MInverse(           MATRIX   InM ) ;													// 逆行列を取得する
extern	MATRIX_D	MInverseD(          MATRIX_D InM ) ;													// 逆行列を取得する
extern  VECTOR		MGetSize(           MATRIX   InM ) ;													// 拡大行列のＸ軸、Ｙ軸、Ｚ軸の拡大率を取得する
extern  VECTOR_D	MGetSizeD(          MATRIX_D InM ) ;													// 拡大行列のＸ軸、Ｙ軸、Ｚ軸の拡大率を取得する
extern	MATRIX		MGetRotElem(        MATRIX   InM ) ;													// 行列の回?成分を取得する
extern	MATRIX_D	MGetRotElemD(       MATRIX_D InM ) ;													// 行列の回?成分を取得する

#ifdef __cplusplus

// 行列の平行移動成分を取得する
__inline VECTOR MGetTranslateElem( MATRIX &InM )
{
	VECTOR Result ;

	Result.x = InM.m[ 3 ][ 0 ] ;
	Result.y = InM.m[ 3 ][ 1 ] ;
	Result.z = InM.m[ 3 ][ 2 ] ;

	return Result ;
}

// 行列の平行移動成分を取得する
__inline VECTOR_D MGetTranslateElemD( MATRIX_D &InM )
{
	VECTOR_D Result ;

	Result.x = InM.m[ 3 ][ 0 ] ;
	Result.y = InM.m[ 3 ][ 1 ] ;
	Result.z = InM.m[ 3 ][ 2 ] ;

	return Result ;
}

// ベクトル値の変換
__inline VECTOR_D VConvFtoD( const VECTOR &In )
{
	VECTOR_D Result ;

	Result.x = In.x ;
	Result.y = In.y ;
	Result.z = In.z ;

	return Result ;
}

__inline VECTOR VConvDtoF( const VECTOR_D &In )
{
	VECTOR Result ;

	Result.x = ( float )In.x ;
	Result.y = ( float )In.y ;
	Result.z = ( float )In.z ;

	return Result ;
}

#endif // __cplusplus

// ベクトル値の生成
__inline VECTOR VGet( float x, float y, float z )
{
	VECTOR Result;
	Result.x = x ;
	Result.y = y ;
	Result.z = z ;
	return Result ;
}

__inline VECTOR_D VGetD( double x, double y, double z )
{
	VECTOR_D Result;
	Result.x = x ;
	Result.y = y ;
	Result.z = z ;
	return Result ;
}

__inline FLOAT2 F2Get( float u, float v )
{
	FLOAT2 Result;
	Result.u = u ;
	Result.v = v ;
	return Result ;
}

// ４要素ベクトル値の生成
__inline FLOAT4 F4Get( float x, float y, float z, float w )
{
	FLOAT4 Result;
	Result.x = x ;
	Result.y = y ;
	Result.z = z ;
	Result.w = w ;
	return Result ;
}

__inline DOUBLE4 D4Get( double x, double y, double z, double w )
{
	DOUBLE4 Result;
	Result.x = x ;
	Result.y = y ;
	Result.z = z ;
	Result.w = w ;
	return Result ;
}

#ifdef __cplusplus

// ベクトルの加算
__inline VECTOR		VAdd( const VECTOR &In1, const VECTOR &In2 )
{
	VECTOR Result ;
	Result.x = In1.x + In2.x ;
	Result.y = In1.y + In2.y ;
	Result.z = In1.z + In2.z ;
	return Result ;
}

__inline VECTOR_D	VAddD( const VECTOR_D &In1, const VECTOR_D &In2 )
{
	VECTOR_D Result ;
	Result.x = In1.x + In2.x ;
	Result.y = In1.y + In2.y ;
	Result.z = In1.z + In2.z ;
	return Result ;
}

// ベクトルの減算
__inline VECTOR		VSub( const VECTOR &In1, const VECTOR &In2 )
{
	VECTOR Result ;
	Result.x = In1.x - In2.x ;
	Result.y = In1.y - In2.y ;
	Result.z = In1.z - In2.z ;
	return Result ;
}

__inline VECTOR_D	VSubD( const VECTOR_D &In1, const VECTOR_D &In2 )
{
	VECTOR_D Result ;
	Result.x = In1.x - In2.x ;
	Result.y = In1.y - In2.y ;
	Result.z = In1.z - In2.z ;
	return Result ;
}

__inline FLOAT2		F2Add( const FLOAT2 &In1, const FLOAT2 &In2 )
{
	FLOAT2 Result ;
	Result.u = In1.u + In2.u ;
	Result.v = In1.v + In2.v ;
	return Result ;
}

// ４要素ベクトルの加算
__inline FLOAT4		F4Add( const FLOAT4 &In1, const FLOAT4 &In2 )
{
	FLOAT4 Result ;
	Result.x = In1.x + In2.x ;
	Result.y = In1.y + In2.y ;
	Result.z = In1.z + In2.z ;
	Result.w = In1.w + In2.w ;
	return Result ;
}

__inline DOUBLE4	D4Add( const DOUBLE4 &In1, const DOUBLE4 &In2 )
{
	DOUBLE4 Result ;
	Result.x = In1.x + In2.x ;
	Result.y = In1.y + In2.y ;
	Result.z = In1.z + In2.z ;
	Result.w = In1.w + In2.w ;
	return Result ;
}

__inline FLOAT2		F2Sub( const FLOAT2 &In1, const FLOAT2 &In2 )
{
	FLOAT2 Result ;
	Result.u = In1.u - In2.u ;
	Result.v = In1.v - In2.v ;
	return Result ;
}

// ４要素ベクトルの減算
__inline FLOAT4		F4Sub( const FLOAT4 &In1, const FLOAT4 &In2 )
{
	FLOAT4 Result ;
	Result.x = In1.x - In2.x ;
	Result.y = In1.y - In2.y ;
	Result.z = In1.z - In2.z ;
	Result.w = In1.w - In2.w ;
	return Result ;
}

__inline DOUBLE4	D4Sub( const DOUBLE4 &In1, const DOUBLE4 &In2 )
{
	DOUBLE4 Result ;
	Result.x = In1.x - In2.x ;
	Result.y = In1.y - In2.y ;
	Result.z = In1.z - In2.z ;
	Result.w = In1.w - In2.w ;
	return Result ;
}

// ベクトルの内積
__inline float		VDot( const VECTOR &In1, const VECTOR &In2 )
{
	return In1.x * In2.x + In1.y * In2.y + In1.z * In2.z ;
}

__inline double		VDotD( const VECTOR_D &In1, const VECTOR_D &In2 )
{
	return In1.x * In2.x + In1.y * In2.y + In1.z * In2.z ;
}

// ベクトルの外積
__inline VECTOR		VCross( const VECTOR &In1, const VECTOR &In2 )
{
	VECTOR Result ;
	Result.x = In1.y * In2.z - In1.z * In2.y ;
	Result.y = In1.z * In2.x - In1.x * In2.z ;
	Result.z = In1.x * In2.y - In1.y * In2.x ;
	return Result ;
}

__inline VECTOR_D	VCrossD( const VECTOR_D &In1, const VECTOR_D &In2 )
{
	VECTOR_D Result ;
	Result.x = In1.y * In2.z - In1.z * In2.y ;
	Result.y = In1.z * In2.x - In1.x * In2.z ;
	Result.z = In1.x * In2.y - In1.y * In2.x ;
	return Result ;
}

// ベクトルのスケ?リング
__inline VECTOR		VScale( const VECTOR &In, float Scale )
{
	VECTOR Result ;
	Result.x = In.x * Scale ;
	Result.y = In.y * Scale ;
	Result.z = In.z * Scale ;
	return Result ;
}

__inline VECTOR_D	VScaleD( const VECTOR_D &In, double Scale )
{
	VECTOR_D Result ;
	Result.x = In.x * Scale ;
	Result.y = In.y * Scale ;
	Result.z = In.z * Scale ;
	return Result ;
}

__inline FLOAT2		F2Scale( const FLOAT2 &In, float Scale )
{
	FLOAT2 Result ;
	Result.u = In.u * Scale ;
	Result.v = In.v * Scale ;
	return Result ;
}

// ４要素ベクトルのスケ?リング
__inline FLOAT4		F4Scale( const FLOAT4 &In, float Scale )
{
	FLOAT4 Result ;
	Result.x = In.x * Scale ;
	Result.y = In.y * Scale ;
	Result.z = In.z * Scale ;
	Result.w = In.w * Scale ;
	return Result ;
}

__inline DOUBLE4	D4Scale( const DOUBLE4 &In, double Scale )
{
	DOUBLE4 Result ;
	Result.x = In.x * Scale ;
	Result.y = In.y * Scale ;
	Result.z = In.z * Scale ;
	Result.w = In.w * Scale ;
	return Result ;
}

#endif // __cplusplus

// ベクトルの正規化
extern VECTOR		VNorm(  VECTOR   In ) ;
extern VECTOR_D		VNormD( VECTOR_D In ) ;

// ベクトルのサイズ
extern float		VSize(  VECTOR   In ) ;
extern double		VSizeD( VECTOR_D In ) ;

#ifdef __cplusplus

// ベクトルのサイズの２乗
__inline float		VSquareSize(  const VECTOR   &In )
{
	return In.x * In.x + In.y * In.y + In.z * In.z ;
}

__inline double		VSquareSizeD( const VECTOR_D &In )
{
	return In.x * In.x + In.y * In.y + In.z * In.z ;
}

// 行列を使った座標変換
__inline VECTOR		VTransform( const VECTOR &InV, const MATRIX &InM )
{
	VECTOR Result ;
	Result.x = InV.x * InM.m[0][0] + InV.y * InM.m[1][0] + InV.z * InM.m[2][0] + InM.m[3][0] ;
	Result.y = InV.x * InM.m[0][1] + InV.y * InM.m[1][1] + InV.z * InM.m[2][1] + InM.m[3][1] ;
	Result.z = InV.x * InM.m[0][2] + InV.y * InM.m[1][2] + InV.z * InM.m[2][2] + InM.m[3][2] ;
	return Result ;
}

__inline VECTOR_D	VTransformD( const VECTOR_D &InV, const MATRIX_D &InM )
{
	VECTOR_D Result ;
	Result.x = InV.x * InM.m[0][0] + InV.y * InM.m[1][0] + InV.z * InM.m[2][0] + InM.m[3][0] ;
	Result.y = InV.x * InM.m[0][1] + InV.y * InM.m[1][1] + InV.z * InM.m[2][1] + InM.m[3][1] ;
	Result.z = InV.x * InM.m[0][2] + InV.y * InM.m[1][2] + InV.z * InM.m[2][2] + InM.m[3][2] ;
	return Result ;
}

// 行列を使った座標変換( スケ?リング?回?成分のみ )
__inline VECTOR		VTransformSR( const VECTOR &InV, const MATRIX &InM )
{
	VECTOR Result ;
	Result.x = InV.x * InM.m[0][0] + InV.y * InM.m[1][0] + InV.z * InM.m[2][0] ;
	Result.y = InV.x * InM.m[0][1] + InV.y * InM.m[1][1] + InV.z * InM.m[2][1] ;
	Result.z = InV.x * InM.m[0][2] + InV.y * InM.m[1][2] + InV.z * InM.m[2][2] ;
	return Result ;
}

__inline VECTOR_D	VTransformSRD( const VECTOR_D &InV, const MATRIX_D &InM )
{
	VECTOR_D Result ;
	Result.x = InV.x * InM.m[0][0] + InV.y * InM.m[1][0] + InV.z * InM.m[2][0] ;
	Result.y = InV.x * InM.m[0][1] + InV.y * InM.m[1][1] + InV.z * InM.m[2][1] ;
	Result.z = InV.x * InM.m[0][2] + InV.y * InM.m[1][2] + InV.z * InM.m[2][2] ;
	return Result ;
}

#endif // __cplusplus

// 二つのベクトルが成す角のコサイン値を得る
extern	float		VCos(  VECTOR   In1, VECTOR   In2 ) ;
extern	double		VCosD( VECTOR_D In1, VECTOR_D In2 ) ;

// 二つのベクトルが成す角の角度を得る( 単位：ラジアン )
extern	float		VRad(  VECTOR   In1, VECTOR   In2 ) ;
extern	double		VRadD( VECTOR_D In1, VECTOR_D In2 ) ;


#ifdef __cplusplus

// 2つのクォ??ニオンの乗算の結果を返す
__inline FLOAT4 QTCross( const FLOAT4 &A, const FLOAT4 &B )
{
    FLOAT4 Result ;
    Result.w = A.w * B.w - ( A.x * B.x + A.y * B.y + A.z * B.z ) ;
	Result.x = B.x * A.w + A.x * B.w + ( A.y * B.z - A.z * B.y ) ;
	Result.y = B.y * A.w + A.y * B.w + ( A.z * B.x - A.x * B.z ) ;
	Result.z = B.z * A.w + A.z * B.w + ( A.x * B.y - A.y * B.x ) ;
    return Result ;
}
__inline DOUBLE4 QTCrossD( const DOUBLE4 &A, const DOUBLE4 &B )
{
    DOUBLE4 Result ;
    Result.w = A.w * B.w - ( A.x * B.x + A.y * B.y + A.z * B.z ) ;
	Result.x = B.x * A.w + A.x * B.w + ( A.y * B.z - A.z * B.y ) ;
	Result.y = B.y * A.w + A.y * B.w + ( A.z * B.x - A.x * B.z ) ;
	Result.z = B.z * A.w + A.z * B.w + ( A.x * B.y - A.y * B.x ) ;
    return Result ;
}

// 共役クォ??ニオンを返す
__inline FLOAT4 QTConj( const FLOAT4 &A )
{
	FLOAT4 Result ;
	Result.w =  A.w ;
	Result.x = -A.x ;
	Result.y = -A.y ;
	Result.z = -A.z ;
	return Result ;
}
__inline DOUBLE4 QTConjD( const DOUBLE4 &A )
{
	DOUBLE4 Result ;
	Result.w =  A.w ;
	Result.x = -A.x ;
	Result.y = -A.y ;
	Result.z = -A.z ;
	return Result ;
}

#endif // __cplusplus

// 回?を?すクォ??ニオンを返す
extern FLOAT4  QTRot(  VECTOR   Axis, float  Angle ) ;
extern DOUBLE4 QTRotD( VECTOR_D Axis, double Angle ) ;

// 3次元空間上の?を任意の軸の周りに任意の角度だけ回?させる関数
extern VECTOR   VRotQ(  VECTOR   P, VECTOR   Axis, float  Angle ) ;
extern VECTOR_D VRotQD( VECTOR_D P, VECTOR_D Axis, double Angle ) ;










// DxBaseImage.cpp 関数プロト?イプ宣言

// 基?イメ?ジデ??のロ?ド?ＤＩＢ関係
extern	int			CreateGraphImageOrDIBGraph(            const TCHAR *FileName,                        const void *DataImage, int DataImageSize, int DataImageType /* LOADIMAGE_TYPE_FILE 等 */ , int BmpFlag, int ReverseFlag, BASEIMAGE *BaseImage, BITMAPINFO **BmpInfo, void **GraphData ) ;					// 画像フ?イル若しくはメモリ上に展開された画像フ?イルイメ?ジを読み込み、基?イメ?ジデ??若しくはＢＭＰデ??を?築する
extern	int			CreateGraphImageOrDIBGraphWithStrLen(  const TCHAR *FileName, size_t FileNameLength, const void *DataImage, int DataImageSize, int DataImageType /* LOADIMAGE_TYPE_FILE 等 */ , int BmpFlag, int ReverseFlag, BASEIMAGE *BaseImage, BITMAPINFO **BmpInfo, void **GraphData ) ;					// 画像フ?イル若しくはメモリ上に展開された画像フ?イルイメ?ジを読み込み、基?イメ?ジデ??若しくはＢＭＰデ??を?築する
extern	int			CreateGraphImageType2(                 STREAMDATA *Src, BASEIMAGE *Dest ) ;																																																						// 画像デ??から?築したストリ??読み込み用デ??を使用して基?イメ?ジデ??を?築する
extern	int			CreateBmpInfo(                         BITMAPINFO *BmpInfo, int Width, int Height, int Pitch, const void *SrcGrData, void **DestGrData ) ;																																						// 24bitカラ??式のビット?ップデ??からＢＭＰデ??を?築する
extern	int			GetImageSize_File(                     const TCHAR *FileName,                        int *SizeX, int *SizeY ) ;																																													// 画像フ?イルの横ピクセル数と縦ピクセル数を取得する
extern	int			GetImageSize_FileWithStrLen(           const TCHAR *FileName, size_t FileNameLength, int *SizeX, int *SizeY ) ;																																													// 画像フ?イルの横ピクセル数と縦ピクセル数を取得する
extern	int			GetImageSize_Mem(                      const void *FileImage, int FileImageSize, int *SizeX, int *SizeY ) ;																																														// メモリ上に展開された画像フ?イルの横ピクセル数と縦ピクセル数を取得する
extern	unsigned int GetGraphImageFullColorCode(           const BASEIMAGE *GraphImage, int x, int y ) ;																																																			// 基?イメ?ジデ??の指定座標のフルカラ?コ?ドを取得する
extern	int			CreateGraphImage_plus_Alpha(           const TCHAR *FileName,                        const void *RgbImage, int RgbImageSize, int RgbImageType, const void *AlphaImage, int AlphaImageSize, int AlphaImageType, BASEIMAGE *RgbGraphImage, BASEIMAGE *AlphaGraphImage, int ReverseFlag ) ;		// 画像フ?イル若しくはメモリ上に展開された画像フ?イルイメ?ジを読み込み、基?イメ?ジデ??を?築する
extern	int			CreateGraphImage_plus_AlphaWithStrLen( const TCHAR *FileName, size_t FileNameLength, const void *RgbImage, int RgbImageSize, int RgbImageType, const void *AlphaImage, int AlphaImageSize, int AlphaImageType, BASEIMAGE *RgbGraphImage, BASEIMAGE *AlphaGraphImage, int ReverseFlag ) ;		// 画像フ?イル若しくはメモリ上に展開された画像フ?イルイメ?ジを読み込み、基?イメ?ジデ??を?築する
extern	int			ReverseGraphImage(                     BASEIMAGE *GraphImage ) ;																																																								// 基?イメ?ジデ??を左右反?する

//extern int		AddUserGraphLoadFunction( int ( *UserLoadFunc )( FILE *fp, BITMAPINFO **BmpInfo, void **GraphData ) ) ;																						// ユ?ザ?定?の画像デ??読み込み関数を登?する
//extern int		AddUserGraphLoadFunction2( int ( *UserLoadFunc )( void *Image, int ImageSize, int ImageType, BITMAPINFO **BmpInfo, void **GraphData ) ) ;													// ユ?ザ?定?の画像デ??読み込み関数を登?する
//extern int		AddUserGraphLoadFunction3( int ( *UserLoadFunc )( void *DataImage, int DataImageSize, int DataImageType, int BmpFlag, BASEIMAGE *BaseImage, BITMAPINFO **BmpInfo, void **GraphData ) ) ;	// ユ?ザ?定?の画像デ??読み込み関数Ver3を登?する
extern	int			AddUserGraphLoadFunction4( int (* UserLoadFunc )( STREAMDATA *Src, BASEIMAGE *BaseImage ) ) ; 																								// ユ?ザ?定?の画像デ??読み込み関数Ver4を登?する
//extern int		SubUserGraphLoadFunction( int ( *UserLoadFunc )( FILE *fp, BITMAPINFO **BmpInfo, void **GraphData ) ) ;																						// ユ?ザ?定?の画像デ??読み込み関数を登?から抹消する
//extern int		SubUserGraphLoadFunction2( int ( *UserLoadFunc )( void *Image, int ImageSize, int ImageType, BITMAPINFO **BmpInfo, void **GraphData ) ) ;													// ユ?ザ?定?の画像デ??読み込み関数を登?から抹消する
//extern int		SubUserGraphLoadFunction3( int ( *UserLoadFunc )( void *DataImage, int DataImageSize, int DataImageType, int BmpFlag, BASEIMAGE *BaseImage, BITMAPINFO **BmpInfo, void **GraphData ) ) ;	// ユ?ザ?定?の画像デ??読み込み関数Ver3を登?から抹消する
extern	int			SubUserGraphLoadFunction4( int (* UserLoadFunc )( STREAMDATA *Src, BASEIMAGE *BaseImage ) ) ; 																								// ユ?ザ?定?の画像デ??読み込み関数Ver4を登?から抹消する

extern	int			SetUseFastLoadFlag(              int Flag ) ;														// 高速読み込みル??ンを使用するかどうかを設定する( TRUE:使用する( デフォルト )  FALSE:使用しない )
extern	int			SetGraphDataShavedMode(          int ShavedMode /* DX_SHAVEDMODE_NONE 等 */ ) ;						// 画像減色時の画像劣化緩和処理モ?ドを設定する( デフォルトでは緩和処理無しの DX_SHAVEDMODE_NONE )
extern	int			GetGraphDataShavedMode(          void ) ;															// 画像減色時の画像劣化緩和処理モ?ドを取得する
extern	int			SetUsePremulAlphaConvertLoad(    int UseFlag ) ;													// 画像フ?イル読み込み時に乗算済みアルフ?画像に変換するかどうかを設定する( TRUE:変換処理を行う  FALSE:変換処理を行わない( デフォルト ) )
extern	int			GetUsePremulAlphaConvertLoad(    void ) ;															// 画像フ?イル読み込み時に乗算済みアルフ?画像に変換するかどうかを取得する( TRUE:変換処理を行う  FALSE:変換処理を行わない( デフォルト ) )
extern	int			SetUseConvertNormalFormatLoad(   int UseFlag ) ;													// 画像フ?イル読み込み時に DX_BASEIMAGE_FORMAT_NORMAL 以外の?式のイメ?ジを DX_BASEIMAGE_FORMAT_NORMAL ?式のイメ?ジに変換するかどうかを設定する( TRUE:変換処理を行う  FALSE:変換処理を行なわない( デフォルト ) )
extern	int			GetUseConvertNormalFormatLoad(   void ) ;															// 画像フ?イル読み込み時に DX_BASEIMAGE_FORMAT_NORMAL 以外の?式のイメ?ジを DX_BASEIMAGE_FORMAT_NORMAL ?式のイメ?ジに変換するかどうかを取得する( TRUE:変換処理を行う  FALSE:変換処理を行なわない( デフォルト ) )

// 基?イメ?ジデ???造体関係
extern	int			CreateBaseImage(                 const TCHAR *FileName,                        const void *FileImage, int FileImageSize, int DataType /*=LOADIMAGE_TYPE_FILE*/ , BASEIMAGE *BaseImage,  int ReverseFlag ) ;			// 画像フ?イル若しくはメモリ上に展開された画像フ?イルイメ?ジから基?イメ?ジデ??を?築する
extern	int			CreateBaseImageWithStrLen(       const TCHAR *FileName, size_t FileNameLength, const void *FileImage, int FileImageSize, int DataType /*=LOADIMAGE_TYPE_FILE*/ , BASEIMAGE *BaseImage,  int ReverseFlag ) ;			// 画像フ?イル若しくはメモリ上に展開された画像フ?イルイメ?ジから基?イメ?ジデ??を?築する
extern	int			CreateGraphImage(                const TCHAR *FileName,                        const void *DataImage, int DataImageSize, int DataImageType,                      BASEIMAGE *GraphImage, int ReverseFlag ) ;			// CreateBaseImage の旧名称
extern	int			CreateBaseImageToFile(           const TCHAR *FileName,                                                                                                          BASEIMAGE *BaseImage,  int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像フ?イルから基?イメ?ジデ??を?築する
extern	int			CreateBaseImageToFileWithStrLen( const TCHAR *FileName, size_t FileNameLength,                                                                                   BASEIMAGE *BaseImage,  int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;	// 画像フ?イルから基?イメ?ジデ??を?築する
extern	int			CreateBaseImageToMem(                                                          const void *FileImage, int FileImageSize,                                         BASEIMAGE *BaseImage,  int ReverseFlag DEFAULTPARAM( = FALSE ) ) ;	// メモリ上に展開された画像フ?イルイメ?ジから基?イメ?ジデ??を?築する
extern	int			CreateARGBF32ColorBaseImage(     int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// ?ＲＧＢ各?ャンネル 32bit 浮動小数?? カラ?の基?イメ?ジデ??を作成する
extern	int			CreateARGBF16ColorBaseImage(     int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// ?ＲＧＢ各?ャンネル 16bit 浮動小数?? カラ?の基?イメ?ジデ??を作成する
extern	int			CreateXRGB8ColorBaseImage(       int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// ＸＲＧＢ８カラ?の基?イメ?ジデ??を作成する
extern	int			CreateARGB8ColorBaseImage(       int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// ?ＲＧＢ８カラ?の基?イメ?ジデ??を作成する
extern	int			CreateRGBA8ColorBaseImage(       int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// ＲＧＢ?８カラ?の基?イメ?ジデ??を作成する
extern	int			CreateABGR8ColorBaseImage(       int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// ?ＢＧＲ８カラ?の基?イメ?ジデ??を作成する
extern	int			CreateBGRA8ColorBaseImage(       int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// ＢＧＲ?８カラ?の基?イメ?ジデ??を作成する
extern	int			CreateARGB4ColorBaseImage(       int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// ?ＲＧＢ４カラ?の基?イメ?ジデ??を作成する
extern	int			CreateA1R5G5B5ColorBaseImage(    int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// ?１Ｒ５Ｇ５Ｂ５カラ?の基?イメ?ジデ??を作成する
extern	int			CreateX1R5G5B5ColorBaseImage(    int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// Ｘ１Ｒ５Ｇ５Ｂ５カラ?の基?イメ?ジデ??を作成する
extern	int			CreateR5G5B5A1ColorBaseImage(    int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// Ｒ５Ｇ５Ｂ５?１カラ?の基?イメ?ジデ??を作成する
extern	int			CreateR5G6B5ColorBaseImage(      int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// Ｒ５Ｇ６Ｂ５カラ?の基?イメ?ジデ??を作成する
extern	int			CreateRGB8ColorBaseImage(        int SizeX, int SizeY,                             BASEIMAGE *BaseImage ) ;																							// ＲＧＢ８カラ?の基?イメ?ジデ??を作成する
extern	int			CreatePAL8ColorBaseImage(        int SizeX, int SizeY,                             BASEIMAGE *BaseImage, int UseAlpha DEFAULTPARAM( = FALSE ) ) ;													// パレット８ビットカラ?の基?イメ?ジデ??を作成する
extern	int			CreateColorDataBaseImage(        int SizeX, int SizeY, const COLORDATA *ColorData, BASEIMAGE *BaseImage ) ;																							// 指定のカラ?フォ??ットの基?イメ?ジデ??を作成する
extern	int			GetBaseImageGraphDataSize(       const BASEIMAGE *BaseImage ) ;																																		// 基?イメ?ジデ??のイメ?ジサイズを取得する( 単位：byte )
extern	int			DerivationBaseImage(             const BASEIMAGE *BaseImage, int x1, int y1, int x2, int y2, BASEIMAGE *NewBaseImage ) ;																			// 基?イメ?ジデ??の指定の部分だけを使う基?イメ?ジデ??の情報を作成する( BaseImage にはフォ??ットが DX_BASEIMAGE_FORMAT_NORMAL で?ップ?ップを使用していない画像のみ使用可? )

extern	int			ReleaseBaseImage(                BASEIMAGE *BaseImage ) ;																																			// 基?イメ?ジデ??の後始末を行う
extern	int			ReleaseGraphImage(               BASEIMAGE *GraphImage ) ;																																			// ReleaseBaseImage の旧名称

extern	int			ConvertNormalFormatBaseImage(    BASEIMAGE *BaseImage, int ReleaseOrigGraphData DEFAULTPARAM( = TRUE ) ) ;																							// DX_BASEIMAGE_FORMAT_NORMAL 以外の?式のイメ?ジを DX_BASEIMAGE_FORMAT_NORMAL ?式のイメ?ジに変換する
extern	int			ConvertPremulAlphaBaseImage(     BASEIMAGE *BaseImage ) ;																																			// 通常のα?ャンネル付き画像を乗算済みα?ャンネル付き画像に変換する( ピクセルフォ??ットが ARGB8 以外の場合は ARGB8 に変換されます )
extern	int			ConvertInterpAlphaBaseImage(     BASEIMAGE *BaseImage ) ;																																			// 乗算済みα?ャンネル付き画像を通常のα?ャンネル付き画像に変換する( ピクセルフォ??ットが ARGB8 以外の場合は ARGB8 に変換されます )

extern	int			GetDrawScreenBaseImage(          int x1, int y1, int x2, int y2, BASEIMAGE *BaseImage ) ;																											// ?画対象の画面から指定領域を基?イメ?ジデ??に?送する
extern	int			GetDrawScreenBaseImageDestPos(   int x1, int y1, int x2, int y2, BASEIMAGE *BaseImage, int DestX, int DestY ) ;																						// ?画対象の画面から指定領域を基?イメ?ジデ??に?送する( ?送先座標指定版 )
extern	int			FillBaseImage(                         BASEIMAGE *BaseImage, int r, int g, int b, int a ) ;																											// 基?イメ?ジデ??を指定の色で塗りつぶす
extern	int			FillRectBaseImage(                     BASEIMAGE *BaseImage, int x, int y, int w, int h, int r, int g, int b, int a ) ;																				// 基?イメ?ジデ??の指定の領域を指定の色で塗りつぶす
extern	int			ClearRectBaseImage(                    BASEIMAGE *BaseImage, int x, int y, int w, int h ) ;																											// 基?イメ?ジデ??の指定の領域を０クリアする
extern	int			GetPaletteBaseImage(             const BASEIMAGE *BaseImage, int PaletteNo, int *r, int *g, int *b, int *a ) ;																						// 基?イメ?ジデ??のパレットを取得する
extern	int			SetPaletteBaseImage(                   BASEIMAGE *BaseImage, int PaletteNo, int  r, int  g, int  b, int  a ) ;																						// 基?イメ?ジデ??のパレットをセットする
extern	int			SetPixelPalCodeBaseImage(              BASEIMAGE *BaseImage, int x, int y, int palNo ) ;																											// 基?イメ?ジデ??の指定の座標の色コ?ドを変更する(パレット画像用)
extern	int			GetPixelPalCodeBaseImage(        const BASEIMAGE *BaseImage, int x, int y ) ;																														// 基?イメ?ジデ??の指定の座標の色コ?ドを取得する(パレット画像用)
extern	int			SetPixelBaseImage(                     BASEIMAGE *BaseImage, int x, int y, int    r, int    g, int    b, int    a ) ;																				// 基?イメ?ジデ??の指定の座標の色を変更する(各色要素は０?２５５)
extern	int			SetPixelBaseImageF(                    BASEIMAGE *BaseImage, int x, int y, float  r, float  g, float  b, float  a ) ;																				// 基?イメ?ジデ??の指定の座標の色を変更する(各色要素は浮動小数?数)
extern	int			GetPixelBaseImage(               const BASEIMAGE *BaseImage, int x, int y, int   *r, int   *g, int   *b, int   *a ) ;																				// 基?イメ?ジデ??の指定の座標の色を取得する(各色要素は０?２５５)
extern	int			GetPixelBaseImageF(              const BASEIMAGE *BaseImage, int x, int y, float *r, float *g, float *b, float *a ) ;																				// 基?イメ?ジデ??の指定の座標の色を取得する(各色要素は浮動小数?数)
extern	int			DrawLineBaseImage(                     BASEIMAGE *BaseImage, int x1, int y1, int x2, int y2, int r, int g, int b, int a ) ;																			// 基?イメ?ジデ??の指定の座標に線を?画する(各色要素は０?２５５)
extern	int			DrawCircleBaseImage(                   BASEIMAGE *BaseImage, int x, int y, int radius, int r, int g, int b, int a, int FillFlag DEFAULTPARAM( = TRUE ) ) ;											// 基?イメ?ジデ??の指定の座標に?を?画する(各色要素は０?２５５)
extern	int			BltBaseImage(                          int SrcX, int SrcY, int SrcSizeX, int SrcSizeY, int DestX, int DestY, BASEIMAGE *SrcBaseImage, BASEIMAGE *DestBaseImage ) ;									// 基?イメ?ジデ??を別の基?イメ?ジデ??に?送する
#ifndef DX_COMPILE_TYPE_C_LANGUAGE
extern	int			BltBaseImage(                                                                          int DestX, int DestY, BASEIMAGE *SrcBaseImage, BASEIMAGE *DestBaseImage ) ;									// 基?イメ?ジデ??を別の基?イメ?ジデ??に?送する
#endif // DX_COMPILE_TYPE_C_LANGUAGE
extern	int			BltBaseImage2(                                                                         int DestX, int DestY, BASEIMAGE *SrcBaseImage, BASEIMAGE *DestBaseImage ) ;									// 基?イメ?ジデ??を別の基?イメ?ジデ??に?送する
extern	int			BltBaseImageWithTransColor(            int SrcX, int SrcY, int SrcSizeX, int SrcSizeY, int DestX, int DestY, BASEIMAGE *SrcBaseImage, BASEIMAGE *DestBaseImage, int Tr, int Tg, int Tb, int Ta ) ;	// 基?イメ?ジデ??を別の基?イメ?ジデ??に透過色処理付きで?送する
extern	int			BltBaseImageWithAlphaBlend(            int SrcX, int SrcY, int SrcSizeX, int SrcSizeY, int DestX, int DestY, BASEIMAGE *SrcBaseImage, BASEIMAGE *DestBaseImage, int Opacity DEFAULTPARAM( = 255 ) ) ;	// 基?イメ?ジデ??を別の基?イメ?ジデ??にアルフ?値のブレンドを考慮した上で?送する( Opacity は透明度 : 0( 完全透明 ) ? 255( 完全不透明 ) )( 出力先が ARGB8 ?式以外の場合はエラ?になります )
extern	int			ReverseBaseImageH(                     BASEIMAGE *BaseImage ) ;																																		// 基?イメ?ジデ??を左右反?する
extern	int			ReverseBaseImageV(                     BASEIMAGE *BaseImage ) ;																																		// 基?イメ?ジデ??を上下反?する
extern	int			ReverseBaseImage(                      BASEIMAGE *BaseImage ) ;																																		// 基?イメ?ジデ??を上下左右反?する
extern	int			CheckPixelAlphaBaseImage(        const BASEIMAGE *BaseImage ) ;																																		// 基?イメ?ジデ??に含まれるピクセルのアルフ?値を?ェックする( 戻り値   -1:エラ?  0:画像にアルフ?成分が無い  1:画像にアルフ?成分があり、すべて最大(255)値  2:画像にアルフ?成分があり、存在するアルフ?値は最小(0)と最大(255)もしくは最小(0)のみ?3:画像にアルフ?成分があり、最小と最大以外の中間の値がある )  
extern	int			GetBaseImageUseMaxPaletteNo(     const BASEIMAGE *BaseImage ) ;																																		// 基?イメ?ジデ??で使用されているパレット番号の最大値を取得する( パレット画像では無い場合は -1 が返る )

#ifndef DX_NON_JPEGREAD
extern	int			ReadJpegExif(                    const TCHAR *JpegFilePath,                            BYTE *ExifBuffer_Array, size_t ExifBufferSize ) ;															// JPEGフ?イルの Exif情報を取得する、ExifBuffer を NULL に渡すと、戻り値の情報のサイズのみ取得できます( 戻り値  -1:エラ?  -1以外：Exif情報のサイズ )
extern	int			ReadJpegExifWithStrLen(          const TCHAR *JpegFilePath, size_t JpegFilePathLength, BYTE *ExifBuffer_Array, size_t ExifBufferSize ) ;															// JPEGフ?イルの Exif情報を取得する、ExifBuffer を NULL に渡すと、戻り値の情報のサイズのみ取得できます( 戻り値  -1:エラ?  -1以外：Exif情報のサイズ )
#endif // DX_NON_JPEGREAD

#ifndef DX_NON_SAVEFUNCTION

extern	int			SaveBaseImageToBmp(              const TCHAR *FilePath,                        const BASEIMAGE *BaseImage ) ;																						// 基?イメ?ジデ??をＢＭＰ画像として保存する
extern	int			SaveBaseImageToBmpWithStrLen(    const TCHAR *FilePath, size_t FilePathLength, const BASEIMAGE *BaseImage ) ;																						// 基?イメ?ジデ??をＢＭＰ画像として保存する
extern	int			SaveBaseImageToDds(              const TCHAR *FilePath,                        const BASEIMAGE *BaseImage ) ;																						// 基?イメ?ジデ??をＤＤＳ画像として保存する
extern	int			SaveBaseImageToDdsWithStrLen(    const TCHAR *FilePath, size_t FilePathLength, const BASEIMAGE *BaseImage ) ;																						// 基?イメ?ジデ??をＤＤＳ画像として保存する
#ifndef DX_NON_PNGREAD
extern	int			SaveBaseImageToPng(              const TCHAR *FilePath,                        BASEIMAGE *BaseImage, int CompressionLevel ) ;																		// 基?イメ?ジデ??をＰＮＧ画像として保存する
extern	int			SaveBaseImageToPngWithStrLen(    const TCHAR *FilePath, size_t FilePathLength, BASEIMAGE *BaseImage, int CompressionLevel ) ;																		// 基?イメ?ジデ??をＰＮＧ画像として保存する
#endif // DX_NON_PNGREAD
#ifndef DX_NON_JPEGREAD
extern	int			SaveBaseImageToJpeg(             const TCHAR *FilePath,                        BASEIMAGE *BaseImage, int Quality, int Sample2x1 ) ;																	// 基?イメ?ジデ??をＪＰＥＧ画像として保存する
extern	int			SaveBaseImageToJpegWithStrLen(   const TCHAR *FilePath, size_t FilePathLength, BASEIMAGE *BaseImage, int Quality, int Sample2x1 ) ;																	// 基?イメ?ジデ??をＪＰＥＧ画像として保存する
#endif // DX_NON_JPEGREAD

#endif // DX_NON_SAVEFUNCTION

// 基?イメ?ジ?画
extern	int			DrawBaseImage(					int x, int y, BASEIMAGE *BaseImage ) ;																																// 基?イメ?ジデ??を?画する

// カラ??ッ?ングしながらイメ?ジデ??間?送を行う Ver2
extern int			GraphColorMatchBltVer2(       void *DestGraphData, int DestPitch,  const COLORDATA *DestColorData,
											const void *SrcGraphData,  int SrcPitch,   const COLORDATA *SrcColorData,
											const void *AlphaMask,     int AlphaPitch, const COLORDATA *AlphaColorData,
											POINT DestPoint, const RECT *SrcRect, int ReverseFlag,
											int TransColorAlphaTestFlag, unsigned int TransColor,
											int ImageShavedMode, int AlphaOnlyFlag DEFAULTPARAM( = FALSE ) ,
											int RedIsAlphaFlag DEFAULTPARAM( = FALSE ) , int TransColorNoMoveFlag DEFAULTPARAM( = FALSE ) ,
											int Pal8ColorMatch DEFAULTPARAM( = FALSE ) ) ;


// 色情報取得関係
extern	COLOR_F			GetColorF(               float Red, float Green, float Blue, float Alpha ) ;													// 浮動小数??のカラ?値を作成する
extern	COLOR_U8		GetColorU8(              int Red, int Green, int Blue, int Alpha ) ;															// 符号なし整数８ビットのカラ?値を作成する
extern	unsigned int	GetColor(                int Red, int Green, int Blue ) ;																		// DrawPixel 等の?画関数で使用するカラ?値を取得する
extern	int				GetColor2(               unsigned int Color, int *Red, int *Green, int *Blue ) ;												// カラ?値から赤、緑、青の値を取得する
extern	unsigned int	GetColor3(               const COLORDATA *ColorData, int Red, int Green, int Blue, int Alpha DEFAULTPARAM( = 255 ) ) ;			// 指定のピクセルフォ??ットに対応したカラ?値を得る
extern	unsigned int	GetColor4(               const COLORDATA *DestColorData, const COLORDATA* SrcColorData, unsigned int SrcColor ) ;				// 指定のカラ?フォ??ットのカラ?値を別のカラ?フォ??ットのカラ?値に変換する
extern	int				GetColor5(               const COLORDATA *ColorData, unsigned int Color, int *Red, int *Green, int *Blue, int *Alpha DEFAULTPARAM( = NULL ) ) ;	// 指定のカラ?フォ??ットのカラ?値を赤、緑、青、アルフ?の値を取得する
extern	int				CreatePaletteColorData(  COLORDATA *ColorDataBuf ) ;																			// パレットカラ?のカラ?フォ??ットを?築する
extern	int				CreateARGBF32ColorData(  COLORDATA *ColorDataBuf ) ;																			// ?ＲＧＢ各?ャンネル 32bit 浮動小数??カラ?のカラ?フォ??ットを?築する
extern	int				CreateARGBF16ColorData(  COLORDATA *ColorDataBuf ) ;																			// ?ＲＧＢ各?ャンネル 16bit 浮動小数??カラ?のカラ?フォ??ットを?築する
extern	int				CreateXRGB8ColorData(    COLORDATA *ColorDataBuf ) ;																			// ＸＲＧＢ８カラ?のカラ?フォ??ットを?築する
extern	int				CreateARGB8ColorData(    COLORDATA *ColorDataBuf ) ;																			// ?ＲＧＢ８カラ?のカラ?フォ??ットを?築する
extern	int				CreateRGBA8ColorData(    COLORDATA *ColorDataBuf ) ;																			// ＲＧＢ?８カラ?のカラ?フォ??ットを?築する
extern	int				CreateABGR8ColorData(    COLORDATA *ColorDataBuf ) ;																			// ?ＢＧＲ８カラ?のカラ?フォ??ットを?築する
extern	int				CreateBGRA8ColorData(    COLORDATA *ColorDataBuf ) ;																			// ＢＧＲ?８カラ?のカラ?フォ??ットを?築する
extern	int				CreateBGR8ColorData(     COLORDATA *ColorDataBuf ) ;																			// ＢＧＲ８カラ?のカラ?フォ??ットを?築する
extern	int				CreateARGB4ColorData(    COLORDATA *ColorDataBuf ) ;																			// ?ＲＧＢ４カラ?のカラ?フォ??ットを?築する
extern	int				CreateA1R5G5B5ColorData( COLORDATA *ColorDataBuf ) ;																			// ?１Ｒ５Ｇ５Ｂ５カラ?のカラ?フォ??ットを?築する
extern	int				CreateX1R5G5B5ColorData( COLORDATA *ColorDataBuf ) ;																			// Ｘ１Ｒ５Ｇ５Ｂ５カラ?のカラ?フォ??ットを?築する
extern	int				CreateR5G5B5A1ColorData( COLORDATA *ColorDataBuf ) ;																			// Ｒ５Ｇ５Ｂ５?１カラ?のカラ?フォ??ットを?築する
extern	int				CreateR5G6B5ColorData(   COLORDATA *ColorDataBuf ) ;																			// Ｒ５Ｇ６Ｂ５カラ?のカラ?フォ??ットを?築する
extern	int				CreateFullColorData(     COLORDATA *ColorDataBuf ) ;																			// ２４ビットカラ?のカラ?フォ??ットを?築する
extern	int				CreateGrayColorData(     COLORDATA *ColorDataBuf ) ;																			// グレ?スケ?ルのカラ?フォ??ットを?築する
extern	int				CreatePal8ColorData(     COLORDATA *ColorDataBuf, int UseAlpha DEFAULTPARAM( = FALSE ) ) ;										// パレット２５６色のカラ?フォ??ットを?築する
extern	int				CreateColorData(         COLORDATA *ColorDataBuf, int ColorBitDepth,
										         DWORD RedMask, DWORD GreenMask, DWORD BlueMask, DWORD AlphaMask,
												 int ChannelNum DEFAULTPARAM( = 0 ), int ChannelBitDepth DEFAULTPARAM( = 0 ), int FloatTypeFlag DEFAULTPARAM( = FALSE ) ) ;				// カラ?フォ??ットを作成する
extern	void			SetColorDataNoneMask(    COLORDATA *ColorData ) ;																				// NoneMask 以外の要素を埋めた COLORDATA ?造体の情報を元に NoneMask をセットする
extern	int				CmpColorData(            const COLORDATA *ColorData1, const COLORDATA *ColorData2 ) ;											// 二つのカラ?フォ??ットが等しいかどうか調べる( 戻り値?TRUE:等しい  FALSE:等しくない )













// DxSoftImage.cpp関数プロト?イプ宣言
#ifndef DX_NON_SOFTIMAGE
extern	int			InitSoftImage(                        void ) ;																			// ?フトウエアイメ?ジハンドルを全て削除する
extern	int			LoadSoftImage(                        const TCHAR *FileName                        ) ;									// 画像フ?イルを読み込み?フトウエアイメ?ジハンドルを作成する( -1:エラ?  -1以外:イメ?ジハンドル )
extern	int			LoadSoftImageWithStrLen(              const TCHAR *FileName, size_t FileNameLength ) ;									// 画像フ?イルを読み込み?フトウエアイメ?ジハンドルを作成する( -1:エラ?  -1以外:イメ?ジハンドル )
extern	int			LoadARGB8ColorSoftImage(              const TCHAR *FileName                        ) ;									// 画像フ?イルを読み込み?フトウエアイメ?ジハンドルを作成する( -1:エラ?  -1以外:イメ?ジハンドル )( 読み込んだ画像が RGBA8 以外のフォ??ットだった場合は RGBA8 カラ?に変換 )
extern	int			LoadARGB8ColorSoftImageWithStrLen(    const TCHAR *FileName, size_t FileNameLength ) ;									// 画像フ?イルを読み込み?フトウエアイメ?ジハンドルを作成する( -1:エラ?  -1以外:イメ?ジハンドル )( 読み込んだ画像が RGBA8 以外のフォ??ットだった場合は RGBA8 カラ?に変換 )
extern	int			LoadXRGB8ColorSoftImage(              const TCHAR *FileName                        ) ;									// 画像フ?イルを読み込み?フトウエアイメ?ジハンドルを作成する( -1:エラ?  -1以外:イメ?ジハンドル )( 読み込んだ画像が XGBA8 以外のフォ??ットだった場合は XGBA8 カラ?に変換 )
extern	int			LoadXRGB8ColorSoftImageWithStrLen(    const TCHAR *FileName, size_t FileNameLength ) ;									// 画像フ?イルを読み込み?フトウエアイメ?ジハンドルを作成する( -1:エラ?  -1以外:イメ?ジハンドル )( 読み込んだ画像が XGBA8 以外のフォ??ットだった場合は XGBA8 カラ?に変換 )
extern	int			LoadSoftImageToMem(                   const void *FileImage, int FileImageSize ) ;										// メモリ上に展開された画像フ?イルイメ?ジから?フトウエアイメ?ジハンドルを作成する( -1:エラ?  -1以外:イメ?ジハンドル )
extern	int			LoadARGB8ColorSoftImageToMem(         const void *FileImage, int FileImageSize ) ;										// メモリ上に展開された画像フ?イルイメ?ジから?フトウエアイメ?ジハンドルを作成する( -1:エラ?  -1以外:イメ?ジハンドル )( 読み込んだ画像が RGBA8 以外のフォ??ットだった場合は RGBA8 カラ?に変換 )
extern	int			LoadXRGB8ColorSoftImageToMem(         const void *FileImage, int FileImageSize ) ;										// メモリ上に展開された画像フ?イルイメ?ジから?フトウエアイメ?ジハンドルを作成する( -1:エラ?  -1以外:イメ?ジハンドル )( 読み込んだ画像が XGBA8 以外のフォ??ットだった場合は XGBA8 カラ?に変換 )
extern	int			MakeSoftImage(                        int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( -1:エラ?  -1以外:イメ?ジハンドル )
extern	int			MakeARGBF32ColorSoftImage(            int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( RGBA 各?ャンネル 32bit 浮動小数?? カラ? )
extern	int			MakeARGBF16ColorSoftImage(            int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( RGBA 各?ャンネル 16bit 浮動小数?? カラ? )
extern	int			MakeXRGB8ColorSoftImage(              int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( XRGB8 カラ? )
extern	int			MakeARGB8ColorSoftImage(              int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( RGBA8 カラ? )
extern	int			MakeRGBA8ColorSoftImage(              int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( RGBA8 カラ? )
extern	int			MakeABGR8ColorSoftImage(              int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( ABGR8 カラ? )
extern	int			MakeBGRA8ColorSoftImage(              int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( BGRA8 カラ? )
extern	int			MakeARGB4ColorSoftImage(              int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( ARGB4 カラ? )
extern	int			MakeA1R5G5B5ColorSoftImage(           int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( A1R5G5B5 カラ? )
extern	int			MakeX1R5G5B5ColorSoftImage(           int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( X1R5G5B5 カラ? )
extern	int			MakeR5G5B5A1ColorSoftImage(           int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( R5G5B5A1 カラ? )
extern	int			MakeR5G6B5ColorSoftImage(             int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( R5G6B5 カラ? )
extern	int			MakeRGB8ColorSoftImage(               int SizeX, int SizeY ) ;															// ?フトウエアイメ?ジハンドルの作成( RGB8 カラ? )
extern	int			MakePAL8ColorSoftImage(               int SizeX, int SizeY, int UseAlpha DEFAULTPARAM( = FALSE ) ) ;					// ?フトウエアイメ?ジハンドルの作成( パレット２５６色 カラ? )
extern	int			MakeColorDataSoftImage(               int SizeX, int SizeY, const COLORDATA *ColorData ) ;								// ?フトウエアイメ?ジハンドルの作成( COLORDATA でフォ??ット指定 )

extern	int			DeleteSoftImage(                      int SIHandle ) ;																	// ?フトウエアイメ?ジハンドルの削除する

extern	int			GetSoftImageSize(                     int SIHandle, int *Width, int *Height ) ;											// ?フトウエアイメ?ジハンドルのサイズを取得する
extern	int			CheckPaletteSoftImage(                int SIHandle ) ;																	// ?フトウエアイメ?ジハンドルがパレット画像かどうかを取得する( TRUE:パレット画像  FALSE:パレット画像じゃない )
extern	int			CheckAlphaSoftImage(                  int SIHandle ) ;																	// ?フトウエアイメ?ジハンドルのフォ??ットにα要素があるかどうかを取得する( TRUE:ある  FALSE:ない )
extern	int			CheckPixelAlphaSoftImage(             int SIHandle ) ;																	// ?フトウエアイメ?ジハンドルに含まれるピクセルのα値を?ェックする( 戻り値   -1:エラ?  0:画像にα成分が無い  1:画像にα成分があり、すべて最大(255)値  2:画像にα成分があり、存在するα値は最小(0)と最大(255)もしくは最小(0)のみ?3:画像にα成分があり、最小と最大以外の中間の値がある )  

extern	int			GetDrawScreenSoftImage(               int x1, int y1, int x2, int y2, int SIHandle ) ;									// ?画対象の画面から指定領域を?フトウエアイメ?ジハンドルに?送する
extern	int			GetDrawScreenSoftImageDestPos(        int x1, int y1, int x2, int y2, int SIHandle, int DestX, int DestY ) ;			// ?画対象の画面から指定領域を?フトウエアイメ?ジハンドルに?送する( ?送先座標指定版 )
extern	int			FillSoftImage(                        int SIHandle, int r, int g, int b, int a ) ;										// ?フトウエアイメ?ジハンドルを指定色で塗りつぶす(各色要素は０?２５５)
extern	int			ClearRectSoftImage(                   int SIHandle, int x, int y, int w, int h ) ;										// ?フトウエアイメ?ジハンドルの指定の領域を０クリアする
extern	int			GetPaletteSoftImage(                  int SIHandle, int PaletteNo, int *r, int *g, int *b, int *a ) ;					// ?フトウエアイメ?ジハンドルのパレットを取得する(各色要素は０?２５５)
extern	int			SetPaletteSoftImage(                  int SIHandle, int PaletteNo, int  r, int  g, int  b, int  a ) ;					// ?フトウエアイメ?ジハンドルのパレットを設定する(各色要素は０?２５５)
extern	int			DrawPixelPalCodeSoftImage(            int SIHandle, int x, int y, int palNo ) ;											// ?フトウエアイメ?ジハンドルの指定座標にドットを?画する(パレット画像用、有効値は０?２５５)
extern	int			GetPixelPalCodeSoftImage(             int SIHandle, int x, int y ) ;													// ?フトウエアイメ?ジハンドルの指定座標の色コ?ドを取得する(パレット画像用、戻り値は０?２５５)
extern	void		*GetImageAddressSoftImage(            int SIHandle ) ;																	// ?フトウエアイメ?ジハンドルの画像が格?されているメモリ領域の先頭アドレスを取得する
extern	int			GetPitchSoftImage(                    int SIHandle ) ;																	// ?フトウエアイメ?ジハンドルのメモリに格?されている画像デ??の1ライン辺りのバイト数を取得する
extern	int			DrawPixelSoftImage(                   int SIHandle, int x, int y, int    r, int    g, int    b, int    a ) ;			// ?フトウエアイメ?ジハンドルの指定座標にドットを?画する(各色要素は０?２５５)
extern	int			DrawPixelSoftImageF(                  int SIHandle, int x, int y, float  r, float  g, float  b, float  a ) ;			// ?フトウエアイメ?ジハンドルの指定座標にドットを?画する(各色要素は浮動小数?数)
extern	void		DrawPixelSoftImage_Unsafe_XRGB8(      int SIHandle, int x, int y, int    r, int    g, int    b ) ;						// ?フトウエアイメ?ジハンドルの指定座標にドットを?画する(各色要素は０?２５５)、エラ??ェックをしない代わりに高速ですが、範囲外の座標や ARGB8 以外のフォ??ットの?フトハンドルを渡すと不正なメモリアクセスで強制終了します
extern	void		DrawPixelSoftImage_Unsafe_ARGB8(      int SIHandle, int x, int y, int    r, int    g, int    b, int    a ) ;			// ?フトウエアイメ?ジハンドルの指定座標にドットを?画する(各色要素は０?２５５)、エラ??ェックをしない代わりに高速ですが、範囲外の座標や XRGB8 以外のフォ??ットの?フトハンドルを渡すと不正なメモリアクセスで強制終了します
extern	int			GetPixelSoftImage(                    int SIHandle, int x, int y, int   *r, int   *g, int   *b, int   *a ) ;			// ?フトウエアイメ?ジハンドルの指定座標の色を取得する(各色要素は０?２５５)
extern	int			GetPixelSoftImageF(                   int SIHandle, int x, int y, float *r, float *g, float *b, float *a ) ;			// ?フトウエアイメ?ジハンドルの指定座標の色を取得する(各色要素は浮動小数?数)
extern	void		GetPixelSoftImage_Unsafe_XRGB8(       int SIHandle, int x, int y, int   *r, int   *g, int   *b ) ;						// ?フトウエアイメ?ジハンドルの指定座標の色を取得する(各色要素は０?２５５)、エラ??ェックをしない代わりに高速ですが、範囲外の座標や XRGB8 以外のフォ??ットの?フトハンドルを渡すと不正なメモリアクセスで強制終了します
extern	void		GetPixelSoftImage_Unsafe_ARGB8(       int SIHandle, int x, int y, int   *r, int   *g, int   *b, int   *a ) ;			// ?フトウエアイメ?ジハンドルの指定座標の色を取得する(各色要素は０?２５５)、エラ??ェックをしない代わりに高速ですが、範囲外の座標や ARGB8 以外のフォ??ットの?フトハンドルを渡すと不正なメモリアクセスで強制終了します
extern	int			DrawLineSoftImage(                    int SIHandle, int x1, int y1, int x2, int y2, int r, int g, int b, int a ) ;		// ?フトウエアイメ?ジハンドルの指定座標に線を?画する(各色要素は０?２５５)
extern	int			DrawCircleSoftImage(                  int SIHandle, int x, int y, int radius, int r, int g, int b, int a, int FillFlag DEFAULTPARAM( = TRUE ) ) ;	// ?フトウエアイメ?ジハンドルの指定座標に?を?画する(各色要素は０?２５５)
extern	int			BltSoftImage(                         int SrcX, int SrcY, int SrcSizeX, int SrcSizeY, int SrcSIHandle, int DestX, int DestY, int DestSIHandle ) ;										// ?フトウエアイメ?ジハンドルを別の?フトウエアイメ?ジハンドルに?送する
extern	int			BltSoftImageWithTransColor(           int SrcX, int SrcY, int SrcSizeX, int SrcSizeY, int SrcSIHandle, int DestX, int DestY, int DestSIHandle, int Tr, int Tg, int Tb, int Ta ) ;		// ?フトウエアイメ?ジハンドルを別の?フトウエアイメ?ジハンドルに透過色処理付きで?送する
extern	int			BltSoftImageWithAlphaBlend(           int SrcX, int SrcY, int SrcSizeX, int SrcSizeY, int SrcSIHandle, int DestX, int DestY, int DestSIHandle, int Opacity DEFAULTPARAM( = 255 ) ) ;	// ?フトウエアイメ?ジハンドルを別の?フトウエアイメ?ジハンドルにアルフ?値のブレンドを考慮した上で?送する( Opacity は透明度 : 0( 完全透明 ) ? 255( 完全不透明 ) )( 出力先が ARGB8 ?式以外の場合はエラ?になります )
extern	int			ReverseSoftImageH(                    int SIHandle ) ;																	// ?フトウエアイメ?ジハンドルを左右反?する
extern	int			ReverseSoftImageV(                    int SIHandle ) ;																	// ?フトウエアイメ?ジハンドルを上下反?する
extern	int			ReverseSoftImage(                     int SIHandle ) ;																	// ?フトウエアイメ?ジハンドルを上下左右反?する
extern	int			ConvertPremulAlphaSoftImage(          int SIHandle ) ;																	// 通常のα?ャンネル付き画像を乗算済みα?ャンネル付き画像に変換する( ピクセルフォ??ットが ARGB8 以外の場合は ARGB8 に変換されます )
extern	int			ConvertInterpAlphaSoftImage(          int SIHandle ) ;																	// 乗算済みα?ャンネル付き画像を通常のα?ャンネル付き画像に変換する( ピクセルフォ??ットが ARGB8 以外の場合は ARGB8 に変換されます )

#ifndef DX_NON_FONT
extern	int			BltStringSoftImage(                   int x, int y, const TCHAR *StrData,                       int DestSIHandle, int DestEdgeSIHandle DEFAULTPARAM( = -1 ) ,                        int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;		// ?フトウエアイメ?ジハンドルに文字列を?画する( デフォルトフォントハンドルを使用する )
extern	int			BltStringSoftImageWithStrLen(         int x, int y, const TCHAR *StrData, size_t StrDataLength, int DestSIHandle, int DestEdgeSIHandle DEFAULTPARAM( = -1 ) ,                        int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;		// ?フトウエアイメ?ジハンドルに文字列を?画する( デフォルトフォントハンドルを使用する )
extern	int			BltStringSoftImageToHandle(           int x, int y, const TCHAR *StrData,                       int DestSIHandle, int DestEdgeSIHandle /* 縁が必要ない場合は -1 */ , int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;		// ?フトウエアイメ?ジハンドルに文字列を?画する( フォントハンドル使用版 )
extern	int			BltStringSoftImageToHandleWithStrLen( int x, int y, const TCHAR *StrData, size_t StrDataLength, int DestSIHandle, int DestEdgeSIHandle /* 縁が必要ない場合は -1 */ , int FontHandle, int VerticalFlag DEFAULTPARAM( = FALSE ) ) ;		// ?フトウエアイメ?ジハンドルに文字列を?画する( フォントハンドル使用版 )
#endif // DX_NON_FONT

extern	int			DrawSoftImage(                        int x, int y, int SIHandle ) ;														// ?フトウエアイメ?ジハンドルを画面に?画する

#ifndef DX_NON_SAVEFUNCTION

extern	int			SaveSoftImageToBmp(                   const TCHAR *FilePath,                        int SIHandle ) ;											// ?フトウエアイメ?ジハンドルをＢＭＰ画像フ?イルとして保存する
extern	int			SaveSoftImageToBmpWithStrLen(         const TCHAR *FilePath, size_t FilePathLength, int SIHandle ) ;											// ?フトウエアイメ?ジハンドルをＢＭＰ画像フ?イルとして保存する
extern	int			SaveSoftImageToDds(                   const TCHAR *FilePath,                        int SIHandle ) ;											// ?フトウエアイメ?ジハンドルをＤＤＳ画像フ?イルとして保存する
extern	int			SaveSoftImageToDdsWithStrLen(         const TCHAR *FilePath, size_t FilePathLength, int SIHandle ) ;											// ?フトウエアイメ?ジハンドルをＤＤＳ画像フ?イルとして保存する
#ifndef DX_NON_PNGREAD
extern	int			SaveSoftImageToPng(                   const TCHAR *FilePath,                        int SIHandle, int CompressionLevel ) ;						// ?フトウエアイメ?ジハンドルをＰＮＧ画像フ?イルとして保存する CompressionLevel = 圧縮率、値が大きいほど高圧縮率高負荷、０は無圧縮,0?9
extern	int			SaveSoftImageToPngWithStrLen(         const TCHAR *FilePath, size_t FilePathLength, int SIHandle, int CompressionLevel ) ;						// ?フトウエアイメ?ジハンドルをＰＮＧ画像フ?イルとして保存する CompressionLevel = 圧縮率、値が大きいほど高圧縮率高負荷、０は無圧縮,0?9
#endif // DX_NON_PNGREAD
#ifndef DX_NON_JPEGREAD
extern	int			SaveSoftImageToJpeg(                  const TCHAR *FilePath,                        int SIHandle, int Quality, int Sample2x1 ) ;				// ?フトウエアイメ?ジハンドルをＪＰＥＧ画像フ?イルとして保存する Quality = 画質、値が大きいほど低圧縮高画質,0?100 
extern	int			SaveSoftImageToJpegWithStrLen(        const TCHAR *FilePath, size_t FilePathLength, int SIHandle, int Quality, int Sample2x1 ) ;				// ?フトウエアイメ?ジハンドルをＪＰＥＧ画像フ?イルとして保存する Quality = 画質、値が大きいほど低圧縮高画質,0?100 
#endif // DX_NON_JPEGREAD

#endif // DX_NON_SAVEFUNCTION

#endif // DX_NON_SOFTIMAGE


















#ifndef DX_NON_SOUND

// DxSound.cpp関数プロト?イプ宣言

// サウンドデ??管理系関数
extern	int			InitSoundMem(                        void ) ;																					// 全てのサウンドハンドルを削除する

extern	int			AddSoundData(                        int Handle DEFAULTPARAM( = -1 ) ) ;																											// サウンドハンドルを作成する
extern	int			AddStreamSoundMem(                   STREAMDATA *Stream, int LoopNum,  int SoundHandle, int StreamDataType, int *CanStreamCloseFlag, int UnionHandle DEFAULTPARAM( = -1 ) ) ;		// ストリ??再生?イプのサウンドハンドルにストリ??デ??を再生対象に追加する
extern	int			AddStreamSoundMemToMem(              const void *FileImage, size_t FileImageSize, int LoopNum,  int SoundHandle, int StreamDataType, int UnionHandle DEFAULTPARAM( = -1 ) ) ;		// ストリ??再生?イプのサウンドハンドルにメモリ上に展開したサウンドフ?イルイメ?ジを再生対象に追加する
extern	int			AddStreamSoundMemToFile(             const TCHAR *WaveFile,                            int LoopNum,  int SoundHandle, int StreamDataType, int UnionHandle DEFAULTPARAM( = -1 ) ) ;	// ストリ??再生?イプのサウンドハンドルにサウンドフ?イルを再生対象に追加する
extern	int			AddStreamSoundMemToFileWithStrLen(   const TCHAR *WaveFile, size_t WaveFilePathLength, int LoopNum,  int SoundHandle, int StreamDataType, int UnionHandle DEFAULTPARAM( = -1 ) ) ;	// ストリ??再生?イプのサウンドハンドルにサウンドフ?イルを再生対象に追加する
extern	int			SetupStreamSoundMem(                 int SoundHandle ) ;																		// ストリ??再生?イプのサウンドハンドルの再生?備をする
extern	int			PlayStreamSoundMem(                  int SoundHandle, int PlayType DEFAULTPARAM( = DX_PLAYTYPE_LOOP ) , int TopPositionFlag DEFAULTPARAM( = TRUE ) ) ;	// ストリ??再生?イプのサウンドハンドルの再生を開始する
extern	int			CheckStreamSoundMem(                 int SoundHandle ) ;																		// ストリ??再生?イプのサウンドハンドルの再生状態を取得する
extern	int			StopStreamSoundMem(                  int SoundHandle, int IsNextLoopEnd DEFAULTPARAM( = FALSE ) ) ;								// ストリ??再生?イプのサウンドハンドルの再生を停?する
extern	int			SetStreamSoundCurrentPosition(       LONGLONG Byte, int SoundHandle ) ;															// サウンドハンドルの再生位置をバイト単位で変更する(再生が?まっている時のみ有効)
extern	LONGLONG	GetStreamSoundCurrentPosition(       int SoundHandle ) ;																		// サウンドハンドルの再生位置をバイト単位で取得する
extern	int			SetStreamSoundCurrentTime(           LONGLONG Time, int SoundHandle ) ;															// サウンドハンドルの再生位置を?リ秒単位で設定する(圧縮?式の場合は正しく設定されない場合がある)
extern	LONGLONG	GetStreamSoundCurrentTime(           int SoundHandle ) ;																		// サウンドハンドルの再生位置を?リ秒単位で取得する(圧縮?式の場合は正しい値が返ってこない場合がある)
extern	int			ProcessStreamSoundMem(               int SoundHandle ) ;																		// ストリ??再生?イプのサウンドハンドルの周期的な処理を行う関数( 内部で自動的に呼ばれます )
extern	int			ProcessStreamSoundMemAll(            void ) ;																					// 有効なストリ??再生?イプのサウンドハンドルに対して ProcessStreamSoundMem を実行する( 内部で自動的に呼ばれます )


extern	int			LoadSoundMem2(                       const TCHAR *FileName1,                         const TCHAR *FileName2                         ) ;	// 前奏部とル?プ部に分かれたサウンドフ?イルを読み込みサウンドハンドルを作成する
extern	int			LoadSoundMem2WithStrLen(             const TCHAR *FileName1, size_t FileName1Length, const TCHAR *FileName2, size_t FileName2Length ) ;	// 前奏部とル?プ部に分かれたサウンドフ?イルを読み込みサウンドハンドルを作成する
extern	int			LoadBGM(                             const TCHAR *FileName                        ) ;													// 主にＢＧＭを読み込みサウンドハンドルを作成するのに適した関数
extern	int			LoadBGMWithStrLen(                   const TCHAR *FileName, size_t FileNameLength ) ;													// 主にＢＧＭを読み込みサウンドハンドルを作成するのに適した関数

extern	int			LoadSoundMemBase(                    const TCHAR *FileName,                        int BufferNum,                      int UnionHandle DEFAULTPARAM( = -1 ) ) ;		// サウンドフ?イルからサウンドハンドルを作成する
extern	int			LoadSoundMemBaseWithStrLen(          const TCHAR *FileName, size_t FileNameLength, int BufferNum,                      int UnionHandle DEFAULTPARAM( = -1 ) ) ;		// サウンドフ?イルからサウンドハンドルを作成する
extern	int			LoadSoundMem(                        const TCHAR *FileName,                        int BufferNum DEFAULTPARAM( = 3 ) , int UnionHandle DEFAULTPARAM( = -1 ) ) ;		// LoadSoundMemBase の別名関数
extern	int			LoadSoundMemWithStrLen(              const TCHAR *FileName, size_t FileNameLength, int BufferNum DEFAULTPARAM( = 3 ) , int UnionHandle DEFAULTPARAM( = -1 ) ) ;		// LoadSoundMemBase の別名関数
extern	int			LoadSoundMemToBufNumSitei(           const TCHAR *FileName,                        int BufferNum ) ;									// LoadSoundMem を使用して下さい
extern	int			LoadSoundMemToBufNumSiteiWithStrLen( const TCHAR *FileName, size_t FileNameLength, int BufferNum ) ;									// LoadSoundMem を使用して下さい
extern	int			DuplicateSoundMem(                   int SrcSoundHandle, int BufferNum DEFAULTPARAM( = 3 ) ) ;											// 同じサウンドデ??を使用するサウンドハンドルを作成する( DX_SOUNDDATATYPE_MEMNOPRESS ?イプのサウンドハンドルのみ可? )

extern	int			LoadSoundMemByMemImageBase(          const void *FileImage, size_t FileImageSize, int BufferNum,                      int UnionHandle DEFAULTPARAM( = -1 ) ) ;		// メモリ上に展開されたサウンドフ?イルイメ?ジからサウンドハンドルを作成する
extern	int			LoadSoundMemByMemImage(              const void *FileImage, size_t FileImageSize, int BufferNum DEFAULTPARAM( = 3 ) , int UnionHandle DEFAULTPARAM( = -1 ) ) ;		// LoadSoundMemByMemImageBase の別名関数
extern	int			LoadSoundMemByMemImage2(             const void *WaveImage, size_t WaveImageSize, const WAVEFORMATEX *WaveFormat, size_t WaveHeaderSize ) ;	// メモリ上に展開されたＰＣＭデ??からサウンドハンドルを作成する
extern	int			LoadSoundMemByMemImageToBufNumSitei( const void *FileImage, size_t FileImageSize, int BufferNum ) ;										// LoadSoundMemByMemImageBase を使用して下さい
extern	int			LoadSoundMem2ByMemImage(             const void *FileImage1, size_t FileImageSize1, const void *FileImage2, size_t FileImageSize2 ) ;	// 前奏部とル?プ部に分かれた二つのメモリ上に展開されたサウンドフ?イルイメ?ジからサウンドハンドルを作成する
extern	int			LoadSoundMemFromSoftSound(           int SoftSoundHandle, int BufferNum DEFAULTPARAM( = 3 ) ) ;											// ?フトウエアサウンドハンドルが持つサウンドデ??からサウンドハンドルを作成する

extern	int			DeleteSoundMem(                      int SoundHandle ) ;																		// サウンドハンドルを削除する

extern	int			PlaySoundMem(                        int SoundHandle, int PlayType, int TopPositionFlag DEFAULTPARAM( = TRUE ) ) ;				// サウンドハンドルを再生する
extern	int			StopSoundMem(                                                                        int SoundHandle, int IsNextLoopEnd DEFAULTPARAM( = FALSE ) ) ;	// サウンドハンドルの再生を停?する( IsNextLoopEnd を TRUE にすると次回のル?プ終了の?イ?ングで音を?める )
extern	int			CheckSoundMem(                                                                       int SoundHandle ) ;						// サウンドハンドルが再生中かどうかを取得する
extern	int			SetPanSoundMem(                      int PanPal,                                     int SoundHandle ) ;						// サウンドハンドルのパンを設定する( 100分の1デシベル単位 0 ? 10000 )
extern	int			ChangePanSoundMem(                   int PanPal,                                     int SoundHandle ) ;						// サウンドハンドルのパンを設定する( -255 ? 255 )
extern	int			GetPanSoundMem(                                                                      int SoundHandle ) ;						// サウンドハンドルのパンを取得する
extern	int			SetVolumeSoundMem(                   int VolumePal,                                  int SoundHandle ) ;						// サウンドハンドルの?リュ??を設定する( 100分の1デシベル単位 0 ? 10000 ) 
extern	int			ChangeVolumeSoundMem(                int VolumePal,                                  int SoundHandle ) ;						// サウンドハンドルの?リュ??を設定する( 0 ? 255 )
extern	int			GetVolumeSoundMem(                                                                   int SoundHandle ) ;						// サウンドハンドルの?リュ??を取得する( 100分の1デシベル単位 0 ? 10000 )
extern	int			GetVolumeSoundMem2(                                                                  int SoundHandle ) ;						// サウンドハンドルの?リュ??を取得する( 0 ? 255 )
extern	int			SetChannelVolumeSoundMem(            int Channel, int VolumePal,                     int SoundHandle ) ;						// サウンドハンドルの指定の?ャンネルの?リュ??を設定する( 100分の1デシベル単位 0 ? 10000 )
extern	int			ChangeChannelVolumeSoundMem(         int Channel, int VolumePal,                     int SoundHandle ) ;						// サウンドハンドルの指定の?ャンネルの?リュ??を設定する( 0 ? 255 )
extern	int			GetChannelVolumeSoundMem(            int Channel,                                    int SoundHandle ) ;						// サウンドハンドルの指定の?ャンネルの?リュ??を取得する( 100分の1デシベル単位 0 ? 10000 )
extern	int			GetChannelVolumeSoundMem2(           int Channel,                                    int SoundHandle ) ;						// サウンドハンドルの指定の?ャンネルの?リュ??を取得する( 0 ? 255 )
extern	int			SetFrequencySoundMem(                int FrequencyPal,                               int SoundHandle ) ;						// サウンドハンドルの再生周波数を設定する
extern	int			GetFrequencySoundMem(                                                                int SoundHandle ) ;						// サウンドハンドルの再生周波数を取得する
extern	int			ResetFrequencySoundMem(                                                              int SoundHandle ) ;						// サウンドハンドルの再生周波数を読み込み直後の状態に戻す

extern	int			SetNextPlayPanSoundMem(              int PanPal,                                     int SoundHandle ) ;						// サウンドハンドルの次の再生にのみ使用するパンを設定する( 100分の1デシベル単位 0 ? 10000 )
extern	int			ChangeNextPlayPanSoundMem(           int PanPal,                                     int SoundHandle ) ;						// サウンドハンドルの次の再生にのみ使用するパンを設定する( -255 ? 255 )
extern	int			SetNextPlayVolumeSoundMem(           int VolumePal,                                  int SoundHandle ) ;						// サウンドハンドルの次の再生にのみ使用する?リュ??を設定する( 100分の1デシベル単位 0 ? 10000 )
extern	int			ChangeNextPlayVolumeSoundMem(        int VolumePal,                                  int SoundHandle ) ;						// サウンドハンドルの次の再生にのみ使用する?リュ??を設定する( 0 ? 255 )
extern	int			SetNextPlayChannelVolumeSoundMem(    int Channel, int VolumePal,                     int SoundHandle ) ;						// サウンドハンドルの次の再生にのみ使用する?ャンネルの?リュ??を設定する( 100分の1デシベル単位 0 ? 10000 )
extern	int			ChangeNextPlayChannelVolumeSoundMem( int Channel, int VolumePal,                     int SoundHandle ) ;						// サウンドハンドルの次の再生にのみ使用する?ャンネルの?リュ??を設定する( 0 ? 255 )
extern	int			SetNextPlayFrequencySoundMem(        int FrequencyPal,                               int SoundHandle ) ;						// サウンドハンドルの次の再生にのみ使用する再生周波数を設定する

extern	int			SetCurrentPositionSoundMem(          LONGLONG SamplePosition,                        int SoundHandle ) ;						// サウンドハンドルの再生位置をサンプル単位で設定する(再生が?まっている時のみ有効)
extern	LONGLONG	GetCurrentPositionSoundMem(                                                          int SoundHandle ) ;						// サウンドハンドルの再生位置をサンプル単位で取得する
extern	int			SetSoundCurrentPosition(             LONGLONG Byte,                                  int SoundHandle ) ;						// サウンドハンドルの再生位置をバイト単位で設定する(再生が?まっている時のみ有効)
extern	LONGLONG	GetSoundCurrentPosition(                                                             int SoundHandle ) ;						// サウンドハンドルの再生位置をバイト単位で取得する
extern	int			SetSoundCurrentTime(                 LONGLONG Time,                                  int SoundHandle ) ;						// サウンドハンドルの再生位置を?リ秒単位で設定する(圧縮?式の場合は正しく設定されない場合がある)
extern	LONGLONG	GetSoundCurrentTime(                                                                 int SoundHandle ) ;						// サウンドハンドルの再生位置を?リ秒単位で取得する(圧縮?式の場合は正しい値が返ってこない場合がある)
extern	LONGLONG	GetSoundTotalSample(                                                                 int SoundHandle ) ;						// サウンドハンドルの音の総時間をサンプル単位で取得する
extern	LONGLONG	GetSoundTotalTime(                                                                   int SoundHandle ) ;						// サウンドハンドルの音の総時間を?リ秒単位で取得する

extern	int			SetLoopPosSoundMem(                  LONGLONG LoopTime,                              int SoundHandle ) ;						// SetLoopTimePosSoundMem の別名関数
extern	int			SetLoopTimePosSoundMem(              LONGLONG LoopTime,                              int SoundHandle ) ;						// サウンドハンドルにル?プ位置を設定する(?リ秒単位)
extern	int			SetLoopSamplePosSoundMem(            LONGLONG LoopSamplePosition,                    int SoundHandle ) ;						// サウンドハンドルにル?プ位置を設定する(サンプル単位)

extern	int			SetLoopStartTimePosSoundMem(         LONGLONG LoopStartTime,                         int SoundHandle ) ;						// サウンドハンドルにル?プ開始位置を設定する(?リ秒単位)
extern	int			SetLoopStartSamplePosSoundMem(       LONGLONG LoopStartSamplePosition,               int SoundHandle ) ;						// サウンドハンドルにル?プ開始位置を設定する(サンプル単位)

extern	int			SetLoopAreaTimePosSoundMem(          LONGLONG  LoopStartTime, LONGLONG  LoopEndTime,                     int SoundHandle ) ;	// サウンドハンドルにル?プ範囲を設定する(?リ秒単位)
extern	int			GetLoopAreaTimePosSoundMem(          LONGLONG *LoopStartTime, LONGLONG *LoopEndTime,                     int SoundHandle ) ;	// サウンドハンドルにル?プ範囲を取得する(?リ秒単位)
extern	int			SetLoopAreaSamplePosSoundMem(        LONGLONG  LoopStartSamplePosition, LONGLONG  LoopEndSamplePosition, int SoundHandle ) ;	// サウンドハンドルにル?プ範囲を設定する(サンプル単位)
extern	int			GetLoopAreaSamplePosSoundMem(        LONGLONG *LoopStartSamplePosition, LONGLONG *LoopEndSamplePosition, int SoundHandle ) ;	// サウンドハンドルにル?プ範囲を取得する(サンプル単位)

extern	int			SetPlayFinishDeleteSoundMem(         int DeleteFlag,                                 int SoundHandle ) ;						// サウンドハンドルの再生が終了したら自動的にハンドルを削除するかどうかを設定する

extern	int			Set3DReverbParamSoundMem(            const SOUND3D_REVERB_PARAM *Param,              int SoundHandle ) ;						// サウンドハンドルの３Ｄサウンド用のリバ?ブパラメ??を設定する
extern	int			Set3DPresetReverbParamSoundMem(      int PresetNo /* DX_REVERB_PRESET_DEFAULT 等 */ , int SoundHandle ) ;						// サウンドハンドルの３Ｄサウンド用のリバ?ブパラメ??をプリセットを使用して設定する
extern	int			Set3DReverbParamSoundMemAll(         const SOUND3D_REVERB_PARAM *Param, int PlaySoundOnly DEFAULTPARAM( = FALSE ) ) ;					// 全ての３Ｄサウンドのサウンドハンドルにリバ?ブパラメ??を設定する( PlaySoundOnly TRUE:再生中のサウンドにのみ設定する  FALSE:再生していないサウンドにも設定する )
extern	int			Set3DPresetReverbParamSoundMemAll(   int PresetNo /* DX_REVERB_PRESET_DEFAULT 等 */ , int PlaySoundOnly DEFAULTPARAM( = FALSE )  ) ;	// 全ての３Ｄサウンドのサウンドハンドルにリバ?ブパラメ??をプリセットを使用して設定する( PlaySoundOnly TRUE:再生中のサウンドにのみ設定する  FALSE:再生していないサウンドにも設定する )
extern	int			Get3DReverbParamSoundMem(            SOUND3D_REVERB_PARAM *ParamBuffer,              int SoundHandle ) ;						// サウンドハンドルに設定されている３Ｄサウンド用のリバ?ブパラメ??を取得する
extern	int			Get3DPresetReverbParamSoundMem(      SOUND3D_REVERB_PARAM *ParamBuffer, int PresetNo /* DX_REVERB_PRESET_DEFAULT 等 */ ) ;		// プリセットの３Ｄサウンド用のリバ?ブパラメ??を取得する

extern	int			Set3DPositionSoundMem(               VECTOR Position,                                int SoundHandle ) ;						// サウンドハンドルの３Ｄサウンド用の再生位置を設定する
extern	int			Set3DRadiusSoundMem(                 float Radius,                                   int SoundHandle ) ;						// サウンドハンドルの３Ｄサウンド用の音が聞こえる距離を設定する
extern	int			Set3DVelocitySoundMem(               VECTOR Velocity,                                int SoundHandle ) ;						// サウンドハンドルの３Ｄサウンド用の移動速度を設定する

extern	int			SetNextPlay3DPositionSoundMem(       VECTOR Position,                                int SoundHandle ) ;						// サウンドハンドルの次の再生のみに使用する３Ｄサウンド用の再生位置を設定する
extern	int			SetNextPlay3DRadiusSoundMem(         float Radius,                                   int SoundHandle ) ;						// サウンドハンドルの次の再生のみに使用する３Ｄサウンド用の音が聞こえる距離を設定する
extern	int			SetNextPlay3DVelocitySoundMem(       VECTOR Velocity,                                int SoundHandle ) ;						// サウンドハンドルの次の再生のみに使用する３Ｄサウンド用の移動速度を設定する


// 特殊関数
extern	int			GetMP3TagInfo(           const TCHAR *FileName,                        TCHAR *TitleBuffer, size_t TitleBufferBytes, TCHAR *ArtistBuffer, size_t ArtistBufferBytes, TCHAR *AlbumBuffer, size_t AlbumBufferBytes, TCHAR *YearBuffer, size_t YearBufferBytes, TCHAR *CommentBuffer, size_t CommentBufferBytes, TCHAR *TrackBuffer, size_t TrackBufferBytes, TCHAR *GenreBuffer, size_t GenreBufferBytes, int *PictureGrHandle ) ;		// MP3フ?イルの?グ情報を取得する
extern	int			GetMP3TagInfoWithStrLen( const TCHAR *FileName, size_t FileNameLength, TCHAR *TitleBuffer, size_t TitleBufferBytes, TCHAR *ArtistBuffer, size_t ArtistBufferBytes, TCHAR *AlbumBuffer, size_t AlbumBufferBytes, TCHAR *YearBuffer, size_t YearBufferBytes, TCHAR *CommentBuffer, size_t CommentBufferBytes, TCHAR *TrackBuffer, size_t TrackBufferBytes, TCHAR *GenreBuffer, size_t GenreBufferBytes, int *PictureGrHandle ) ;		// MP3フ?イルの?グ情報を取得する
#ifndef DX_NON_OGGVORBIS
extern	int			GetOggCommentNum(           const TCHAR *FileName                        ) ;																																	// Oggフ?イルのコメント情報の数を取得する
extern	int			GetOggCommentNumWithStrLen( const TCHAR *FileName, size_t FileNameLength ) ;																																	// Oggフ?イルのコメント情報の数を取得する
extern	int			GetOggComment(              const TCHAR *FileName,                        int CommentIndex, TCHAR *CommentNameBuffer, size_t CommentNameBufferBytes, TCHAR *CommentBuffer, size_t CommentBufferBytes ) ;		// Oggフ?イルのコメント情報を取得する
extern	int			GetOggCommentWithStrLen(    const TCHAR *FileName, size_t FileNameLength, int CommentIndex, TCHAR *CommentNameBuffer, size_t CommentNameBufferBytes, TCHAR *CommentBuffer, size_t CommentBufferBytes ) ;		// Oggフ?イルのコメント情報を取得する
#endif // DX_NON_OGGVORBIS
						

// 設定関係関数
extern	int			SetCreateSoundDataType(              int SoundDataType ) ;																		// 作成するサウンドハンドルの再生?イプを設定する( DX_SOUNDDATATYPE_MEMNOPRESS 等 )
extern	int			GetCreateSoundDataType(              void ) ;																					// 作成するサウンドハンドルの再生?イプを取得する( DX_SOUNDDATATYPE_MEMNOPRESS 等 )
extern	int			SetCreateSoundPitchRate(             float Cents ) ;																			// 作成するサウンドハンドルのピッ?( 音の長さを変えずに音程を変更する )レ?トを設定する( 単位はセント( 100.0fで半音、1200.0fで１オク??ヴ )、プラスの値で音程が高く、?イナスの値で音程が低くなります )
extern	float		GetCreateSoundPitchRate(             void ) ;																					// 作成するサウンドハンドルのピッ?( 音の長さを変えずに音程を変更する )レ?トを取得する( 単位はセント( 100.0fで半音、1200.0fで１オク??ヴ )、プラスの値で音程が高く、?イナスの値で音程が低くなります )
extern	int			SetCreateSoundTimeStretchRate(       float Rate ) ;																				// 作成するサウンドハンドルの?イ?ストレッ?( 音程を変えずに音の長さを変更する )レ?トを設定する( 単位は?率、2.0f で音の長さが２?に、0.5f で音の長さが半分になります )
extern	float		GetCreateSoundTimeStretchRate(       void ) ;																					// 作成するサウンドハンドルの?イ?ストレッ?( 音程を変えずに音の長さを変更する )レ?トを取得する( 単位は?率、2.0f で音の長さが２?に、0.5f で音の長さが半分になります )
extern	int			SetCreateSoundLoopAreaTimePos(       LONGLONG  LoopStartTime,           LONGLONG  LoopEndTime ) ;								// 作成するサウンドハンドルのル?プ範囲を設定する( ?リ秒単位 )
extern	int			GetCreateSoundLoopAreaTimePos(       LONGLONG *LoopStartTime,           LONGLONG *LoopEndTime ) ;								// 作成するサウンドハンドルのル?プ範囲を取得する( ?リ秒単位 )
extern	int			SetCreateSoundLoopAreaSamplePos(     LONGLONG  LoopStartSamplePosition, LONGLONG  LoopEndSamplePosition ) ;						// 作成するサウンドハンドルのル?プ範囲を設定する( サンプル単位 )
extern	int			GetCreateSoundLoopAreaSamplePos(     LONGLONG *LoopStartSamplePosition, LONGLONG *LoopEndSamplePosition ) ;						// 作成するサウンドハンドルのル?プ範囲を取得する( サンプル単位 )
extern	int			SetCreateSoundIgnoreLoopAreaInfo(    int IgnoreFlag ) ;																			// LoadSoundMem などで読み込むサウンドデ??にル?プ範囲情報があっても無視するかどうかを設定する( TRUE:無視する  FALSE:無視しない( デフォルト ) )
extern	int			GetCreateSoundIgnoreLoopAreaInfo(    void ) ;																					// LoadSoundMem などで読み込むサウンドデ??にル?プ範囲情報があっても無視するかどうかを取得する( TRUE:無視する  FALSE:無視しない( デフォルト ) )
extern	int			SetDisableReadSoundFunctionMask(     int Mask ) ;																				// 使用しないサウンドデ??読み込み処理の?スクを設定する( DX_READSOUNDFUNCTION_PCM 等 )
extern	int			GetDisableReadSoundFunctionMask(     void ) ;																					// 使用しないサウンドデ??読み込み処理の?スクを取得する( DX_READSOUNDFUNCTION_PCM 等 )
extern	int			SetEnableSoundCaptureFlag(           int Flag ) ;																				// サウンドキャプ?ャを前提とした動作をするかどうかを設定する
extern	int			SetUseOldVolumeCalcFlag(             int Flag ) ;																				// ChangeVolumeSoundMem, ChangeNextPlayVolumeSoundMem, ChangeMovieVolumeToGraph の音量計算式を Ver3.10c以前のものを使用するかどうかを設定する( TRUE:Ver3.10c以前の計算式を使用  FALSE:3.10d以?の計算式を使用( デフォルト ) )
extern	int			SetSoundCurrentTimeType(             int Type /* DX_SOUNDCURRENTTIME_TYPE_LOW_LEVEL など */ ) ;									// GetSoundCurrentTime などを使用した場合に取得できる再生時間の?イプを設定する
extern	int			GetSoundCurrentTimeType(             void ) ;																					// GetSoundCurrentTime などを使用した場合に取得できる再生時間の?イプを取得する

extern	int			SetCreate3DSoundFlag(                     int Flag ) ;																			// 次に作成するサウンドハンドルを３Ｄサウンド用にするかどうかを設定する( TRUE:３Ｄサウンド用にする  FALSE:３Ｄサウンド用にしない( デフォルト ) )
extern	int			Set3DSoundOneMetre(                       float Distance ) ;																	// ３Ｄ空間の１メ?トルに相当する距離を設定する、DxLib_Init を呼び出す前でのみ呼び出し可?( デフォルト:1.0f )
extern	int			Set3DSoundListenerPosAndFrontPos_UpVecY(  VECTOR Position, VECTOR FrontPosition ) ;												// ３Ｄサウンドのリスナ?の位置とリスナ?の前方位置を設定する( リスナ?の上方向はＹ軸固定 )
extern	int			Set3DSoundListenerPosAndFrontPosAndUpVec( VECTOR Position, VECTOR FrontPosition, VECTOR UpVector ) ;							// ３Ｄサウンドのリスナ?の位置とリスナ?の前方位置とリスナ?の上方向を設定する
extern	int			Set3DSoundListenerVelocity(               VECTOR Velocity ) ;																	// ３Ｄサウンドのリスナ?の移動速度を設定する
extern	int			Set3DSoundListenerConeAngle(              float InnerAngle, float OuterAngle ) ;												// ３Ｄサウンドのリスナ?の可聴角度範囲を設定する
extern	int			Set3DSoundListenerConeVolume(             float InnerAngleVolume, float OuterAngleVolume ) ;									// ３Ｄサウンドのリスナ?の可聴角度範囲の音量?率を設定する

#ifndef DX_NON_BEEP
// BEEP音再生用命令
extern	int			SetBeepFrequency(					int Freq ) ;																				// ビ?プ音周波数設定関数
extern	int			PlayBeep(							void ) ;																					// ビ?プ音を再生する
extern	int			StopBeep(							void ) ;																					// ビ?プ音を?める
#endif // DX_NON_BEEP

// ラッパ?関数
extern	int			PlaySoundFile(						const TCHAR *FileName,                        int PlayType ) ;								// サウンドフ?イルを再生する
extern	int			PlaySoundFileWithStrLen(			const TCHAR *FileName, size_t FileNameLength, int PlayType ) ;								// サウンドフ?イルを再生する
#ifndef DX_COMPILE_TYPE_C_LANGUAGE
extern	int			PlaySound(							const TCHAR *FileName,                        int PlayType ) ;								// PlaySoundFile の旧名称
extern	int			PlaySoundWithStrLen(				const TCHAR *FileName, size_t FileNameLength, int PlayType ) ;								// PlaySoundFile の旧名称
#endif // DX_COMPILE_TYPE_C_LANGUAGE
extern	int			PlaySoundDX(						const TCHAR *FileName,                        int PlayType ) ;								// PlaySoundFile の旧名称
extern	int			PlaySoundDXWithStrLen(				const TCHAR *FileName, size_t FileNameLength, int PlayType ) ;								// PlaySoundFile の旧名称
extern	int			CheckSoundFile(						void ) ;																					// サウンドフ?イルの再生中かどうかを取得する
extern	int			CheckSound(							void ) ;																					// CheckSoundFile の旧名称
extern	int			StopSoundFile(						void ) ;																					// サウンドフ?イルの再生を停?する
extern	int			StopSound(							void ) ;																					// StopSoundFile の旧名称
extern	int			SetVolumeSoundFile(					int VolumePal ) ;																			// サウンドフ?イルの音量を設定する
extern	int			SetVolumeSound(						int VolumePal ) ;																			// SetVolumeSound の旧名称

// ?フトウエア制御サウンド系関数
extern	int			InitSoftSound(						void ) ;																					// ?フトウエアで扱う波?デ??ハンドルをすべて削除する
extern	int			LoadSoftSound(						const TCHAR *FileName                        ) ;											// ?フトウエアで扱う波?デ??ハンドルをサウンドフ?イルから作成する
extern	int			LoadSoftSoundWithStrLen(			const TCHAR *FileName, size_t FileNameLength ) ;											// ?フトウエアで扱う波?デ??ハンドルをサウンドフ?イルから作成する
extern	int			LoadSoftSoundFromMemImage(			const void *FileImage, size_t FileImageSize ) ;												// ?フトウエアで扱う波?デ??ハンドルをメモリ上に展開されたサウンドフ?イルイメ?ジから作成する
extern	int			MakeSoftSound(						int UseFormat_SoftSoundHandle, LONGLONG SampleNum ) ;										// ?フトウエアで扱う空の波?デ??ハンドルを作成する( フォ??ットは引数の?フトウエアサウンドハンドルと同じものにする )
extern	int			MakeSoftSound2Ch16Bit44KHz(			LONGLONG SampleNum ) ;																		// ?フトウエアで扱う空の波?デ??ハンドルを作成する( ?ャンネル数:2 量子化ビット数:16bit サンプリング周波数:44.1KHz )
extern	int			MakeSoftSound2Ch16Bit22KHz(			LONGLONG SampleNum ) ;																		// ?フトウエアで扱う空の波?デ??ハンドルを作成する( ?ャンネル数:2 量子化ビット数:16bit サンプリング周波数:22KHz )
extern	int			MakeSoftSound2Ch8Bit44KHz(			LONGLONG SampleNum ) ;																		// ?フトウエアで扱う空の波?デ??ハンドルを作成する( ?ャンネル数:2 量子化ビット数: 8bit サンプリング周波数:44.1KHz )
extern	int			MakeSoftSound2Ch8Bit22KHz(			LONGLONG SampleNum ) ;																		// ?フトウエアで扱う空の波?デ??ハンドルを作成する( ?ャンネル数:2 量子化ビット数: 8bit サンプリング周波数:22KHz )
extern	int			MakeSoftSound1Ch16Bit44KHz(			LONGLONG SampleNum ) ;																		// ?フトウエアで扱う空の波?デ??ハンドルを作成する( ?ャンネル数:1 量子化ビット数:16bit サンプリング周波数:44.1KHz )
extern	int			MakeSoftSound1Ch16Bit22KHz(			LONGLONG SampleNum ) ;																		// ?フトウエアで扱う空の波?デ??ハンドルを作成する( ?ャンネル数:1 量子化ビット数:16bit サンプリング周波数:22KHz )
extern	int			MakeSoftSound1Ch8Bit44KHz(			LONGLONG SampleNum ) ;																		// ?フトウエアで扱う空の波?デ??ハンドルを作成する( ?ャンネル数:1 量子化ビット数: 8bit サンプリング周波数:44.1KHz )
extern	int			MakeSoftSound1Ch8Bit22KHz(			LONGLONG SampleNum ) ;																		// ?フトウエアで扱う空の波?デ??ハンドルを作成する( ?ャンネル数:1 量子化ビット数: 8bit サンプリング周波数:22KHz )
extern	int			MakeSoftSoundCustom(				int ChannelNum, int BitsPerSample, int SamplesPerSec, LONGLONG SampleNum, int IsFloatType DEFAULTPARAM( = 0 ) ) ;	// ?フトウエアで扱う空の波?デ??ハンドルを作成する
extern	int			DeleteSoftSound(					int SoftSoundHandle ) ;																		// ?フトウエアで扱う波?デ??ハンドルを削除する
#ifndef DX_NON_SAVEFUNCTION
extern	int			SaveSoftSound(						int SoftSoundHandle, const TCHAR *FileName                        ) ;						// ?フトウエアで扱う波?デ??ハンドルをWAVEフ?イル(PCM)?式で保存する
extern	int			SaveSoftSoundWithStrLen(			int SoftSoundHandle, const TCHAR *FileName, size_t FileNameLength ) ;						// ?フトウエアで扱う波?デ??ハンドルをWAVEフ?イル(PCM)?式で保存する
#endif // DX_NON_SAVEFUNCTION
extern	LONGLONG	GetSoftSoundSampleNum(				int SoftSoundHandle ) ;																		// ?フトウエアで扱う波?デ??ハンドルのサンプル数を取得する
extern	int			GetSoftSoundFormat(					int SoftSoundHandle, int *Channels, int *BitsPerSample, int *SamplesPerSec, int *IsFloatType DEFAULTPARAM( = NULL ) ) ;				// ?フトウエアで扱う波?デ??ハンドルのフォ??ットを取得する
extern	int			ReadSoftSoundData(					int SoftSoundHandle, LONGLONG SamplePosition, int   *Channel1, int   *Channel2 ) ;			// ?フトウエアで扱う波?デ??ハンドルのサンプルを読み取る
extern	int			ReadSoftSoundDataF(					int SoftSoundHandle, LONGLONG SamplePosition, float *Channel1, float *Channel2 ) ;			// ?フトウエアで扱う波?デ??ハンドルのサンプルを読み取る( float?版 )
extern	int			WriteSoftSoundData(					int SoftSoundHandle, LONGLONG SamplePosition, int    Channel1, int    Channel2 ) ;			// ?フトウエアで扱う波?デ??ハンドルのサンプルを書き込む
extern	int			WriteSoftSoundDataF(				int SoftSoundHandle, LONGLONG SamplePosition, float  Channel1, float  Channel2 ) ;			// ?フトウエアで扱う波?デ??ハンドルのサンプルを書き込む( float?版 )
extern	int			WriteTimeStretchSoftSoundData(		int SrcSoftSoundHandle, int DestSoftSoundHandle ) ;											// ?フトウエアで扱う波?デ??ハンドルの波?デ??を音程を変えずにデ??の長さを変更する
extern	int			WritePitchShiftSoftSoundData(		int SrcSoftSoundHandle, int DestSoftSoundHandle ) ;											// ?フトウエアで扱う波?デ??ハンドルの波?デ??の長さを変更する
extern	void*		GetSoftSoundDataImage(				int SoftSoundHandle ) ;																		// ?フトウエアで扱う波?デ??ハンドルの波?イメ?ジが格?されているメモリアドレスを取得する
extern	int			GetFFTVibrationSoftSound(			int SoftSoundHandle, int Channel, LONGLONG SamplePosition, int SampleNum, float *Buffer_Array, int BufferLength ) ;									// ?フトウエアで扱う波?デ??ハンドルの指定の範囲を高速フ?リエ変換を行い、各周波数域の振幅を取得する( SampleNum は 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 の何れかである必要があります、Channel を -1 にすると二つの?ャンネルを合成した結果になります )
extern	int			GetFFTVibrationSoftSoundBase(		int SoftSoundHandle, int Channel, LONGLONG SamplePosition, int SampleNum, float *RealBuffer_Array, float *ImagBuffer_Array, int BufferLength ) ;	// ?フトウエアで扱う波?デ??ハンドルの指定の範囲を高速フ?リエ変換を行い、各周波数域の振幅を取得する、結果の実数と虚数を別々に取得することができるバ?ジョン( SampleNum は 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 の何れかである必要があります、Channel を -1 にすると二つの?ャンネルを合成した結果になります )

extern	int			InitSoftSoundPlayer(				void ) ;																					// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルをすべて解放する
extern	int			MakeSoftSoundPlayer(				int UseFormat_SoftSoundHandle ) ;															// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルを作成する( フォ??ットは引数の?フトウエアサウンドハンドルと同じものにする )
extern	int			MakeSoftSoundPlayer2Ch16Bit44KHz(	void ) ;																					// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルを作成する( ?ャンネル数:2 量子化ビット数:16bit サンプリング周波数:44.1KHz )
extern	int			MakeSoftSoundPlayer2Ch16Bit22KHz(	void ) ;																					// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルを作成する( ?ャンネル数:2 量子化ビット数:16bit サンプリング周波数:22KHz )
extern	int			MakeSoftSoundPlayer2Ch8Bit44KHz(	void ) ;																					// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルを作成する( ?ャンネル数:2 量子化ビット数: 8bit サンプリング周波数:44.1KHz )
extern	int			MakeSoftSoundPlayer2Ch8Bit22KHz(	void ) ;																					// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルを作成する( ?ャンネル数:2 量子化ビット数: 8bit サンプリング周波数:22KHz )
extern	int			MakeSoftSoundPlayer1Ch16Bit44KHz(	void ) ;																					// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルを作成する( ?ャンネル数:1 量子化ビット数:16bit サンプリング周波数:44.1KHz )
extern	int			MakeSoftSoundPlayer1Ch16Bit22KHz(	void ) ;																					// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルを作成する( ?ャンネル数:1 量子化ビット数:16bit サンプリング周波数:22KHz )
extern	int			MakeSoftSoundPlayer1Ch8Bit44KHz(	void ) ;																					// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルを作成する( ?ャンネル数:1 量子化ビット数: 8bit サンプリング周波数:44.1KHz )
extern	int			MakeSoftSoundPlayer1Ch8Bit22KHz(	void ) ;																					// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルを作成する( ?ャンネル数:1 量子化ビット数: 8bit サンプリング周波数:22KHz )
extern	int			MakeSoftSoundPlayerCustom(			int ChannelNum, int BitsPerSample, int SamplesPerSec ) ;									// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルを作成する
extern	int			DeleteSoftSoundPlayer(				int SSoundPlayerHandle ) ;																	// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルを削除する
extern	int			AddDataSoftSoundPlayer(				int SSoundPlayerHandle, int SoftSoundHandle, LONGLONG AddSamplePosition, int AddSampleNum ) ;	// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルに波?デ??を追加する( フォ??ットが同じではない場合はエラ? )
extern	int			AddDirectDataSoftSoundPlayer(		int SSoundPlayerHandle, const void *SoundData, int AddSampleNum ) ;							// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルにプレイヤ?が対応したフォ??ットの生波?デ??を追加する
extern	int			AddOneDataSoftSoundPlayer(			int SSoundPlayerHandle, int Channel1, int Channel2 ) ;										// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルに波?デ??を一つ追加する
extern	int			GetSoftSoundPlayerFormat(			int SSoundPlayerHandle, int *Channels, int *BitsPerSample, int *SamplesPerSec ) ;			// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルが扱うデ??フォ??ットを取得する
extern	int			StartSoftSoundPlayer(				int SSoundPlayerHandle ) ;																	// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルの再生処理を開始する
extern	int			CheckStartSoftSoundPlayer(			int SSoundPlayerHandle ) ;																	// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルの再生処理が開始されているか取得する( TRUE:開始している  FALSE:停?している )
extern	int			StopSoftSoundPlayer(				int SSoundPlayerHandle ) ;																	// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルの再生処理を停?する
extern	int			ResetSoftSoundPlayer(				int SSoundPlayerHandle ) ;																	// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルの状態を初期状態に戻す( 追加された波?デ??は削除され、再生状態だった場合は停?する )
extern	int			GetStockDataLengthSoftSoundPlayer(	int SSoundPlayerHandle, int *SoundBufferStockSamples DEFAULTPARAM( = NULL ) ) ;					// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルに追加した波?デ??でまだ再生用サウンドバッフ?に?送されていない波?デ??のサンプル数を取得する、SoundBufferStockSamples を指定すると再生用サウンドバッフ?で未再生のサウンドデ??のサンプル数が代入されます
extern	int			CheckSoftSoundPlayerNoneData(		int SSoundPlayerHandle ) ;																	// ?フトウエアで扱う波?デ??のプレイヤ?ハンドルに再生用サウンドバッフ?に?送していない波?デ??が無く、再生用サウンドバッフ?にも無音デ??以外無いかどうかを取得する( TRUE:無音デ??以外無い  FALSE:有効デ??がある )




// ＭＩＤＩ制御関数
extern	int			DeleteMusicMem(						int MusicHandle ) ;																			// ＭＩＤＩハンドルを削除する
extern	int			LoadMusicMem(						const TCHAR *FileName                        ) ;											// ＭＩＤＩフ?イルを読み込みＭＩＤＩハンドルを作成する
extern	int			LoadMusicMemWithStrLen(				const TCHAR *FileName, size_t FileNameLength ) ;											// ＭＩＤＩフ?イルを読み込みＭＩＤＩハンドルを作成する
extern	int			LoadMusicMemByMemImage(				const void *FileImage, size_t FileImageSize ) ;												// メモリ上に展開されたＭＩＤＩフ?イルイメ?ジからＭＩＤＩハンドルを作成する
extern	int			PlayMusicMem(						int MusicHandle, int PlayType ) ;															// ＭＩＤＩハンドルの演奏を開始する
extern	int			StopMusicMem(						int MusicHandle ) ;																			// ＭＩＤＩハンドルの演奏を停?する
extern	int			CheckMusicMem(						int MusicHandle ) ;																			// ＭＩＤＩハンドルが演奏中かどうかを取得する( TRUE:演奏中  FALSE:停?中 )
extern	int			SetVolumeMusicMem(					int Volume, int MusicHandle ) ;																// ＭＩＤＩハンドルの再生音量をセットする
extern	int			GetMusicMemPosition(				int MusicHandle ) ;																			// ＭＩＤＩハンドルの現在の再生位置を取得する
extern	int			InitMusicMem(						void ) ;																					// ＭＩＤＩハンドルをすべて削除する
extern	int			ProcessMusicMem(					void ) ;																					// ＭＩＤＩハンドルの周期的処理( 内部で呼ばれます )

extern	int			PlayMusic(							const TCHAR *FileName,                        int PlayType ) ;								// ＭＩＤＩフ?イルを演奏する
extern	int			PlayMusicWithStrLen(				const TCHAR *FileName, size_t FileNameLength, int PlayType ) ;								// ＭＩＤＩフ?イルを演奏する
extern	int			PlayMusicByMemImage(				const void *FileImage, size_t FileImageSize,  int PlayType ) ;								// メモリ上に展開されているＭＩＤＩフ?イルを演奏する
extern	int			SetVolumeMusic(						int Volume ) ;																				// ＭＩＤＩの再生音量をセットする
extern	int			StopMusic(							void ) ;																					// ＭＩＤＩフ?イルの演奏停?
extern	int			CheckMusic(							void ) ;																					// ＭＩＤＩフ?イルが演奏中か否か情報を取得する
extern	int			GetMusicPosition(					void ) ;																					// ＭＩＤＩの現在の再生位置を取得する

extern	int			SelectMidiMode(						int Mode ) ;																				// ＭＩＤＩの再生?式を設定する

#endif // DX_NON_SOUND










// DxArchive_.cpp 関数 プロト?イプ宣言
extern	int			SetUseDXArchiveFlag(				int Flag ) ;															// ＤＸア?カイブフ?イルの読み込み??を使うかどうかを設定する( FALSE:使用しない  TRUE:使用する )
extern	int			SetDXArchivePriority(				int Priority DEFAULTPARAM( = 0 ) ) ;													// 同名のＤＸア?カイブフ?イルとフォル?が存在した場合、どちらを優先させるかを設定する( 1:フォル?を優先? 0:ＤＸア?カイブフ?イルを優先( デフォルト ) )
extern	int			SetDXArchiveExtension(				const TCHAR *Extension DEFAULTPARAM( = NULL ) ) ;										// 検索するＤＸア?カイブフ?イルの拡張子を設定する( Extension:拡張子名文字列 )
extern	int			SetDXArchiveExtensionWithStrLen(	const TCHAR *Extension DEFAULTPARAM( = NULL ) , size_t ExtensionLength DEFAULTPARAM( = 0 ) ) ;			// 検索するＤＸア?カイブフ?イルの拡張子を設定する( Extension:拡張子名文字列 )
extern	int			SetDXArchiveKeyString(				const TCHAR *KeyString DEFAULTPARAM( = NULL ) ) ;										// ＤＸア?カイブフ?イルの鍵文字列を設定する( KeyString:鍵文字列 )
extern	int			SetDXArchiveKeyStringWithStrLen(	const TCHAR *KeyString DEFAULTPARAM( = NULL ) , size_t KeyStringLength DEFAULTPARAM( = 0 ) ) ;			// ＤＸア?カイブフ?イルの鍵文字列を設定する( KeyString:鍵文字列 )

extern	int			DXArchivePreLoad(					const TCHAR *FilePath,                        int ASync DEFAULTPARAM( = FALSE ) ) ;		// 指定のＤＸ?フ?イルを丸ごとメモリに読み込む( 戻り値  -1:エラ?  0:成功 )
extern	int			DXArchivePreLoadWithStrLen(			const TCHAR *FilePath, size_t FilePathLength, int ASync DEFAULTPARAM( = FALSE ) ) ;		// 指定のＤＸ?フ?イルを丸ごとメモリに読み込む( 戻り値  -1:エラ?  0:成功 )
extern	int			DXArchiveCheckIdle(					const TCHAR *FilePath                        ) ;						// 指定のＤＸ?フ?イルの事前読み込みが完了したかどうかを取得する( 戻り値  TRUE:完了した FALSE:まだ )
extern	int			DXArchiveCheckIdleWithStrLen(		const TCHAR *FilePath, size_t FilePathLength ) ;						// 指定のＤＸ?フ?イルの事前読み込みが完了したかどうかを取得する( 戻り値  TRUE:完了した FALSE:まだ )
extern	int			DXArchiveRelease(					const TCHAR *FilePath                        ) ;						// 指定のＤＸ?フ?イルをメモリから解放する
extern	int			DXArchiveReleaseWithStrLen(			const TCHAR *FilePath, size_t FilePathLength ) ;						// 指定のＤＸ?フ?イルをメモリから解放する
extern	int			DXArchiveCheckFile(					const TCHAR *FilePath,                        const TCHAR *TargetFilePath                              ) ;	// ＤＸ?フ?イルの中に指定のフ?イルが存在するかどうかを調べる、TargetFilePath はＤＸ?フ?イルをカレントフォル?とした場合のパス( 戻り値:  -1=エラ?  0:無い  1:ある )
extern	int			DXArchiveCheckFileWithStrLen(		const TCHAR *FilePath, size_t FilePathLength, const TCHAR *TargetFilePath, size_t TargetFilePathLength ) ;	// ＤＸ?フ?イルの中に指定のフ?イルが存在するかどうかを調べる、TargetFilePath はＤＸ?フ?イルをカレントフォル?とした場合のパス( 戻り値:  -1=エラ?  0:無い  1:ある )
extern	int			DXArchiveSetMemImage(				void *ArchiveImage, int ArchiveImageSize, const TCHAR *EmulateFilePath,                               int ArchiveImageCopyFlag DEFAULTPARAM( = FALSE ) , int ArchiveImageReadOnly DEFAULTPARAM( = TRUE ) ) ;	// メモリ上に展開されたＤＸ?フ?イルを指定のフ?イルパスにあることにする( EmulateFilePath は見立てる dxa フ?イルのパス、例えばＤＸ?フ?イルイメ?ジを Image.dxa というフ?イル名で c:\Temp にあることにしたい場合は EmulateFilePath に "c:\\Temp\\Image.dxa" を渡す、SetDXArchiveExtension で拡張子を変更している場合は EmulateFilePath に渡すフ?イルパスの拡張子もそれに合わせる必要あり )
extern	int			DXArchiveSetMemImageWithStrLen(		void *ArchiveImage, int ArchiveImageSize, const TCHAR *EmulateFilePath, size_t EmulateFilePathLength, int ArchiveImageCopyFlag DEFAULTPARAM( = FALSE ) , int ArchiveImageReadOnly DEFAULTPARAM( = TRUE ) ) ;	// メモリ上に展開されたＤＸ?フ?イルを指定のフ?イルパスにあることにする( EmulateFilePath は見立てる dxa フ?イルのパス、例えばＤＸ?フ?イルイメ?ジを Image.dxa というフ?イル名で c:\Temp にあることにしたい場合は EmulateFilePath に "c:\\Temp\\Image.dxa" を渡す、SetDXArchiveExtension で拡張子を変更している場合は EmulateFilePath に渡すフ?イルパスの拡張子もそれに合わせる必要あり )
extern	int			DXArchiveReleaseMemImage(			void *ArchiveImage ) ;													// DXArchiveSetMemImage の設定を解除する

extern	DWORD		HashCRC32(							const void *SrcData, size_t SrcDataSize ) ;								// バイナリデ??を元に CRC32 のハッシュ値を計算する





















// DxModel.cpp 関数 プロト?イプ宣言

#ifndef DX_NON_MODEL

// モデルの読み込み・複製関係
extern	int			MV1LoadModel(						const TCHAR *FileName ) ;											// モデルの読み込み( -1:エラ?  0以上:モデルハンドル )
extern	int			MV1LoadModelWithStrLen(				const TCHAR *FileName, size_t FileNameLength ) ;					// モデルの読み込み( -1:エラ?  0以上:モデルハンドル )
extern	int			MV1LoadModelFromMem(				const void *FileImage, int FileSize, int (* FileReadFunc )( const TCHAR *FilePath, void **FileImageAddr, int *FileSize, void *FileReadFuncData ), int (* FileReleaseFunc )( void *MemoryAddr, void *FileReadFuncData ), void *FileReadFuncData DEFAULTPARAM( = NULL ) ) ;	// メモリ上のモデルフ?イルイメ?ジと独自の読み込みル??ンを使用してモデルを読み込む
extern	int			MV1DuplicateModel(					int SrcMHandle ) ;													// 指定のモデルと同じモデル基?デ??を使用してモデルを作成する( -1:エラ?  0以上:モデルハンドル )
extern	int			MV1CreateCloneModel(				int SrcMHandle ) ;													// 指定のモデルをモデル基?デ??も含め複製する( MV1DuplicateModel はモデル基?デ??は共有しますが、こちらは複製元のモデルとは一切共有デ??の無いモデルハンドルを作成します )( -1:エラ?  0以上:モデルハンドル )
extern	int			MV1CreateSimpleModel(				VERTEX3D *Vertex, int VertexNum, unsigned int *Index, int IndexNum, MATERIALPARAM *Material, int GrHandle ) ;		// 指定の頂?デ??と?テリアル情報、テクス?ャを使用したシンプルな３Ｄモデルのハンドルを作成する

extern	int			MV1DeleteModel(						int MHandle ) ;														// モデルを削除する
extern	int			MV1InitModel(						void ) ;															// すべてのモデルを削除する

extern	int			MV1SetLoadModelReMakeNormal(						int Flag ) ;														// モデルを読み込む際に?線の再計算を行うかどうかを設定する( TRUE:行う  FALSE:行わない( デフォルト ) )
extern	int			MV1SetLoadModelReMakeNormalSmoothingAngle(			float SmoothingAngle DEFAULTPARAM( = 1.562069f ) ) ;				// モデルを読み込む際に行う?泉の再計算で使用するス??ジング角度を設定する( 単位はラジアン )
extern	int			MV1SetLoadModelIgnoreScaling(						int Flag ) ;														// モデルを読み込む際にスケ?リングデ??を無視するかどうかを設定する( TRUE:無視する  FALSE:無視しない( デフォルト ) )
extern	int			MV1SetLoadModelPositionOptimize(					int Flag ) ;														// モデルを読み込む際に座標デ??の最適化を行うかどうかを設定する( TRUE:行う  FALSE:行わない( デフォルト ) )
extern	int			MV1SetLoadModelNotEqNormalSide_AddZeroAreaPolygon(	int Flag ) ;														// モデルを読み込む際に?リゴンの辺が接していて、且つ?線の方向が異なる辺に面積０の?リゴンを埋め込むかどうかを設定する( TRUE:埋め込む?FALSE:埋め込まない( デフォルト ) )、( MV1フ?イルの読み込みではこの関数の設定は無視され、?リゴンの埋め込みは実行されません )
extern	int			MV1SetLoadModelUsePhysicsMode(						int PhysicsMode /* DX_LOADMODEL_PHYSICS_LOADCALC 等 */ ) ;			// 読み込むモデルの物理演算モ?ドを設定する
extern	int			MV1SetLoadModelPhysicsWorldGravity(					float Gravity ) ;													// 読み込むモデルの物理演算に適用する重力パラメ??を設定する
extern	float		MV1GetLoadModelPhysicsWorldGravity(					void ) ;															// 読み込むモデルの物理演算に適用する重力パラメ??を取得する
extern	int			MV1SetLoadCalcPhysicsWorldGravity(					int GravityNo, VECTOR Gravity ) ;									// 読み込むモデルの物理演算モ?ドが事前計算( DX_LOADMODEL_PHYSICS_LOADCALC )だった場合に適用される重力の設定をする
extern	VECTOR		MV1GetLoadCalcPhysicsWorldGravity(					int GravityNo ) ;													// 読み込むモデルの物理演算モ?ドが事前計算( DX_LOADMODEL_PHYSICS_LOADCALC )だった場合に適用される重力を取得する
extern	int			MV1SetLoadModelPhysicsCalcPrecision(				int Precision ) ;													// 読み込むモデルの物理演算モ?ドが事前計算( DX_LOADMODEL_PHYSICS_LOADCALC )だった場合に適用される物理演算の時間進行の精度を設定する( 0:60FPS  1:120FPS  2:240FPS  3:480FPS  4:960FPS  5:1920FPS )
extern	int			MV1SetLoadModel_PMD_PMX_AnimationFPSMode(			int FPSMode /* DX_LOADMODEL_PMD_PMX_ANIMATION_FPSMODE_30 等 */ ) ;	// PMD, PMX フ?イルを読み込んだ際のアニメ?ションの FPS モ?ドを設定する
extern	int			MV1AddLoadModelDisablePhysicsNameWord(				const TCHAR *NameWord ) ;											// 読み込むモデルの物理演算を特定の剛体のみ無効にするための名前のワ?ドを追加する、追加できるワ?ド文字列の最大長は 63 文字、追加できるワ?ドの数は最大 256 個
extern	int			MV1AddLoadModelDisablePhysicsNameWordWithStrLen(	const TCHAR *NameWord, size_t NameWordLength ) ;					// 読み込むモデルの物理演算を特定の剛体のみ無効にするための名前のワ?ドを追加する、追加できるワ?ド文字列の最大長は 63 文字、追加できるワ?ドの数は最大 256 個
extern	int			MV1ResetLoadModelDisablePhysicsNameWord(			void ) ;															// MV1AddLoadModelDisablePhysicsNameWord で追加した剛体の無効ワ?ドをリセットして無効ワ?ド無しの初期状態に戻す
extern	int			MV1SetLoadModelDisablePhysicsNameWordMode(			int DisableNameWordMode /* DX_LOADMODEL_PHYSICS_DISABLENAMEWORD_ALWAYS 等 */ ) ;	// MV1AddLoadModelDisablePhysicsNameWord で追加した剛体の無効ワ?ドの適用ル?ルを変更する
extern	int			MV1SetLoadModelAnimFilePath(						const TCHAR *FileName ) ;											// 読み込むモデルに適用するアニメ?ションフ?イルのパスを設定する、NULLを渡すと設定リセット( 現在は PMD,PMX のみに効果あり )
extern	int			MV1SetLoadModelAnimFilePathWithStrLen(				const TCHAR *FileName, size_t FileNameLength ) ;					// 読み込むモデルに適用するアニメ?ションフ?イルのパスを設定する、NULLを渡すと設定リセット( 現在は PMD,PMX のみに効果あり )
extern	int			MV1SetLoadModelUsePackDraw(							int Flag ) ;														// 読み込むモデルを同時複数?画に対応させるかどうかを設定する( TRUE:対応させる  FALSE:対応させない( デフォルト ) )、( 「対応させる」にすると?画が高速になる可?性がある代わりに消費VRAMが増えます )
extern	int			MV1SetLoadModelTriangleListUseMaxBoneNum(			int UseMaxBoneNum ) ;												// 読み込むモデルのひとつのトライアングルリストで使用できる最大??ン数を設定する( UseMaxBoneNum で指定できる値の範囲は 8 ? 54、 0 を指定するとデフォルト動作に戻る )
extern	int			MV1SetLoadModelTextureLoad(							int Flag ) ;														// 読み込むモデルで使用するテクス?ャフ?イルを読み込むかどうかを設定する( TRUE:読み込む(デフォルト) FALSE:読み込まない )
extern	int			MV1SetLoadModelIgnoreIK(							int IgnoreFlag ) ;													// 読み込むモデルのIK情報を無視するかどうかを設定する( TRUE:無視する  FALSE:無視しない(デフォルト) )

// モデル保存関係
extern	int			MV1SaveModelToMV1File(				int MHandle, const TCHAR *FileName,                        int SaveType DEFAULTPARAM( = MV1_SAVETYPE_NORMAL ) , int AnimMHandle DEFAULTPARAM( = -1 ) , int AnimNameCheck DEFAULTPARAM( = TRUE ) , int Normal8BitFlag DEFAULTPARAM( = 1 ) , int Position16BitFlag DEFAULTPARAM( = 1 ) , int Weight8BitFlag DEFAULTPARAM( = 0 ) , int Anim16BitFlag DEFAULTPARAM( = 1 ) ) ;		// 指定のパスにモデルを保存する( 戻り値  0:成功  -1:メモリ不足  -2:使われていないアニメ?ションがあった )
extern	int			MV1SaveModelToMV1FileWithStrLen(	int MHandle, const TCHAR *FileName, size_t FileNameLength, int SaveType DEFAULTPARAM( = MV1_SAVETYPE_NORMAL ) , int AnimMHandle DEFAULTPARAM( = -1 ) , int AnimNameCheck DEFAULTPARAM( = TRUE ) , int Normal8BitFlag DEFAULTPARAM( = 1 ) , int Position16BitFlag DEFAULTPARAM( = 1 ) , int Weight8BitFlag DEFAULTPARAM( = 0 ) , int Anim16BitFlag DEFAULTPARAM( = 1 ) ) ;		// 指定のパスにモデルを保存する( 戻り値  0:成功  -1:メモリ不足  -2:使われていないアニメ?ションがあった )
#ifndef DX_NON_SAVEFUNCTION
extern	int			MV1SaveModelToXFile(				int MHandle, const TCHAR *FileName,                        int SaveType DEFAULTPARAM( = MV1_SAVETYPE_NORMAL ) , int AnimMHandle DEFAULTPARAM( = -1 ) , int AnimNameCheck DEFAULTPARAM( = TRUE ) ) ;	// 指定のパスにモデルをＸフ?イル?式で保存する( 戻り値  0:成功  -1:メモリ不足  -2:使われていないアニメ?ションがあった )
extern	int			MV1SaveModelToXFileWithStrLen(		int MHandle, const TCHAR *FileName, size_t FileNameLength, int SaveType DEFAULTPARAM( = MV1_SAVETYPE_NORMAL ) , int AnimMHandle DEFAULTPARAM( = -1 ) , int AnimNameCheck DEFAULTPARAM( = TRUE ) ) ;	// 指定のパスにモデルをＸフ?イル?式で保存する( 戻り値  0:成功  -1:メモリ不足  -2:使われていないアニメ?ションがあった )
#endif // DX_NON_SAVEFUNCTION

// モデル?画関係
extern	int			MV1DrawModel(						int MHandle ) ;														// モデルを?画する
extern	int			MV1DrawFrame(						int MHandle, int FrameIndex ) ;										// モデルの指定のフレ??を?画する
extern	int			MV1DrawMesh(						int MHandle, int MeshIndex ) ;										// モデルの指定のメッシュを?画する
extern	int			MV1DrawTriangleList(				int MHandle, int TriangleListIndex ) ;								// モデルの指定のトライアングルリストを?画する
extern	int			MV1DrawModelDebug(					int MHandle, unsigned int Color, int IsNormalLine, float NormalLineLength, int IsPolyLine, int IsCollisionBox ) ;	// モデルのデバッグ?画

// ?画設定関係
extern	int			MV1SetUseOrigShader(				int UseFlag ) ;														// モデルの?画に SetUseVertexShader, SetUsePixelShader で指定したシェ???を使用するかどうかを設定する( TRUE:使用する  FALSE:使用しない( デフォルト ) )
extern	int			MV1SetDrawMode(						int DrawMode /* DX_MV1_DRAWMODE_NORMAL 等 */ ) ;					// モデルの?画モ?ドの設定
extern	int			MV1SetSemiTransDrawMode(			int DrawMode /* DX_SEMITRANSDRAWMODE_ALWAYS 等 */ ) ;				// モデルの半透明要素がある部分についての?画モ?ドを設定する

// モデル基?制御関係
extern	MATRIX		MV1GetLocalWorldMatrix(				int MHandle ) ;														// モデルのロ?カル座標からワ?ルド座標に変換する行列を得る
extern	MATRIX_D	MV1GetLocalWorldMatrixD(			int MHandle ) ;														// モデルのロ?カル座標からワ?ルド座標に変換する行列を得る
extern	int			MV1SetPosition(						int MHandle, VECTOR   Position ) ;									// モデルの座標をセット
extern	int			MV1SetPositionD(					int MHandle, VECTOR_D Position ) ;									// モデルの座標をセット
extern	VECTOR		MV1GetPosition(						int MHandle ) ;														// モデルの座標を取得
extern	VECTOR_D	MV1GetPositionD(					int MHandle ) ;														// モデルの座標を取得
extern	int			MV1SetScale(						int MHandle, VECTOR Scale ) ;										// モデルの拡大値をセット
extern	VECTOR		MV1GetScale(						int MHandle ) ;														// モデルの拡大値を取得
extern	int			MV1SetRotationXYZ(					int MHandle, VECTOR Rotate ) ;										// モデルの回?値をセット( X軸回?→Y軸回?→Z軸回?方式 )
extern	VECTOR		MV1GetRotationXYZ(					int MHandle ) ;														// モデルの回?値を取得( X軸回?→Y軸回?→Z軸回?方式 )
extern	int			MV1SetRotationZYAxis(				int MHandle, VECTOR ZAxisDirection, VECTOR YAxisDirection, float ZAxisTwistRotate ) ;	// モデルのＺ軸とＹ軸の向きをセットする
extern	int			MV1SetRotationYUseDir(				int MHandle, VECTOR Direction, float OffsetYAngle ) ;				// モデルのＹ軸の回?値を指定のベクトルの向きを元に設定する、モデルはZ軸の?イナス方向を向いていることを想定するので、そうではない場合は OffsetYAngle で補正する、Ｘ軸回?、Ｚ軸回?は０で固定
extern	int			MV1SetRotationMatrix(				int MHandle, MATRIX Matrix ) ;										// モデルの回?用行列をセットする
extern	MATRIX		MV1GetRotationMatrix(				int MHandle ) ;														// モデルの回?用行列を取得する
extern	int			MV1SetMatrix(						int MHandle, MATRIX   Matrix ) ;									// モデルの変?用行列をセットする
extern	int			MV1SetMatrixD(						int MHandle, MATRIX_D Matrix ) ;									// モデルの変?用行列をセットする
extern	MATRIX		MV1GetMatrix(						int MHandle ) ;														// モデルの変?用行列を取得する
extern	MATRIX_D	MV1GetMatrixD(						int MHandle ) ;														// モデルの変?用行列を取得する
extern	int			MV1SetVisible(						int MHandle, int VisibleFlag ) ;									// モデルの?示、非?示状態を変更する( TRUE:?示  FALSE:非?示 )
extern	int			MV1GetVisible(						int MHandle ) ;														// モデルの?示、非?示状態を取得する( TRUE:?示  FALSE:非?示 )
extern	int			MV1SetMeshCategoryVisible(			int MHandle, int MeshCategory, int VisibleFlag ) ;					// モデルのメッシュの種類( DX_MV1_MESHCATEGORY_NORMAL など )毎の?示、非?示を設定する( TRUE:?示  FALSE:非?示 )
extern	int			MV1GetMeshCategoryVisible(			int MHandle, int MeshCategory ) ;									// モデルのメッシュの種類( DX_MV1_MESHCATEGORY_NORMAL など )毎の?示、非?示を取得する( TRUE:?示  FALSE:非?示 )
extern	int			MV1SetDifColorScale(				int MHandle, COLOR_F Scale ) ;										// モデルのディフュ?ズカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetDifColorScale(				int MHandle ) ;														// モデルのディフュ?ズカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	int			MV1SetSpcColorScale(				int MHandle, COLOR_F Scale ) ;										// モデルのスペキュラカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetSpcColorScale(				int MHandle ) ;														// モデルのスペキュラカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	int			MV1SetEmiColorScale(				int MHandle, COLOR_F Scale ) ;										// モデルのエ?ッシブカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetEmiColorScale(				int MHandle ) ;														// モデルのエ?ッシブカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	int			MV1SetAmbColorScale(				int MHandle, COLOR_F Scale ) ;										// モデルのアンビエントカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetAmbColorScale(				int MHandle ) ;														// モデルのアンビエントカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	int			MV1GetSemiTransState(				int MHandle ) ;														// モデルに半透明要素があるかどうかを取得する( 戻り値 TRUE:ある  FALSE:ない )
extern	int			MV1SetOpacityRate(					int MHandle, float Rate ) ;											// モデルの不透明度を設定する( 不透明 1.0f ? 透明 0.0f )
extern	float		MV1GetOpacityRate(					int MHandle ) ;														// モデルの不透明度を取得する( 不透明 1.0f ? 透明 0.0f )
extern	int			MV1SetUseDrawMulAlphaColor(			int MHandle, int Flag ) ;											// モデルを?画する際にRGB値に対してA値を乗算するかどうかを設定する( ?画結果が乗算済みアルフ?画像になります )( Flag   TRUE:RGB値に対してA値を乗算する  FALSE:乗算しない(デフォルト) )
extern	int			MV1GetUseDrawMulAlphaColor(			int MHandle ) ;														// モデルを?画する際にRGB値に対してA値を乗算するかどうかを取得する( ?画結果が乗算済みアルフ?画像になります )( 戻り値 TRUE:RGB値に対してA値を乗算する  FALSE:乗算しない(デフォルト) )
extern	int			MV1SetUseZBuffer(					int MHandle, int Flag ) ;											// モデルを?画する際にＺバッフ?を使用するかどうかを設定する
extern	int			MV1SetWriteZBuffer(					int MHandle, int Flag ) ;											// モデルを?画する際にＺバッフ?に書き込みを行うかどうかを設定する
extern	int			MV1SetZBufferCmpType(				int MHandle, int CmpType /* DX_CMP_NEVER 等 */ ) ;					// モデルの?画時のＺ値の比較モ?ドを設定する
extern	int			MV1SetZBias(						int MHandle, int Bias ) ;											// モデルの?画時の書き込むＺ値のバイアスを設定する
extern	int			MV1SetUseVertDifColor(				int MHandle, int UseFlag ) ;										// モデルの含まれるメッシュの頂?ディフュ?ズカラ?を?テリアルのディフュ?ズカラ?の代わりに使用するかどうかを設定する( TRUE:?テリアルカラ?の代わりに使用する  FALSE:?テリアルカラ?を使用する )
extern	int			MV1SetUseVertSpcColor(				int MHandle, int UseFlag ) ;										// モデルに含まれるメッシュの頂?スペキュラカラ?を?テリアルのスペキュラカラ?の代わりに使用するかどうかを設定する( TRUE:?テリアルカラ?の代わりに使用する  FALSE:?テリアルカラ?を使用する )
extern	int			MV1SetSampleFilterMode(				int MHandle, int FilterMode ) ;										// モデルのテクス?ャのサンプルフィル??モ?ドを変更する( FilterMode は DX_DRAWMODE_NEAREST 等 )
extern	int			MV1SetMaxAnisotropy(				int MHandle, int MaxAnisotropy ) ;									// モデルの異方性フィル?リングの最大次数を設定する
extern	int			MV1SetWireFrameDrawFlag(			int MHandle, int Flag ) ;											// モデルをワイヤ?フレ??で?画するかどうかを設定する
extern	int			MV1RefreshVertColorFromMaterial(	int MHandle ) ;														// モデルの頂?カラ?を現在設定されている?テリアルのカラ?にする
extern	int			MV1SetPhysicsWorldGravity(			int MHandle, VECTOR Gravity ) ;										// モデルの物理演算の重力を設定する
extern	int			MV1PhysicsCalculation(				int MHandle, float MillisecondTime ) ;								// モデルの物理演算を指定時間分経過したと仮定して計算する( MillisecondTime で指定する時間の単位は?リ秒 )
extern	int			MV1PhysicsResetState(				int MHandle ) ;														// モデルの物理演算の状態をリセットする( 位置がワ?プしたとき用 )
extern	int			MV1SetPrioritizePhysicsOverAnimFlag( int MHandle, int Flag ) ;											// モデルの物理演算をアニメ?ションより優先するかどうかを設定する( TRUE:物理演算を優先する  FALSE:アニメ?ションを優先する( デフォルト ) )
extern	int			MV1SetUseShapeFlag(					int MHandle, int UseFlag ) ;										// モデルのシェイプ??を使用するかどうかを設定する( UseFlag  TRUE:使用する( デフォルト )  FALSE:使用しない )
extern	int			MV1GetMaterialNumberOrderFlag(		int MHandle ) ;														// モデルの?テリアル番号順にメッシュを?画するかどうかのフラグを取得する( TRUE:?テリアル番号順に?画  FALSE:不透明メッシュの後半透明メッシュ )

// アニメ?ション関係
extern	int			MV1AttachAnim(						int MHandle, int AnimIndex, int AnimSrcMHandle DEFAULTPARAM( = -1 ) , int NameCheck DEFAULTPARAM( = TRUE ) ) ;		// アニメ?ションをア?ッ?する( 戻り値  -1:エラ?  0以上:ア?ッ?インデックス )
extern	int			MV1DetachAnim(						int MHandle, int AttachIndex ) ;													// アニメ?ションをデ?ッ?する
extern	int			MV1SetAttachAnimTime(				int MHandle, int AttachIndex, float Time ) ;										// ア?ッ?しているアニメ?ションの再生時間を設定する
extern	float		MV1GetAttachAnimTime(				int MHandle, int AttachIndex ) ;													// ア?ッ?しているアニメ?ションの再生時間を取得する
extern	float		MV1GetAttachAnimTotalTime(			int MHandle, int AttachIndex ) ;													// ア?ッ?しているアニメ?ションの総時間を得る
extern	int			MV1SetAttachAnimBlendRate(			int MHandle, int AttachIndex, float Rate DEFAULTPARAM( = 1.0f ) ) ;					// ア?ッ?しているアニメ?ションのブレンド率を設定する
extern	float		MV1GetAttachAnimBlendRate(			int MHandle, int AttachIndex ) ;													// ア?ッ?しているアニメ?ションのブレンド率を取得する
extern	int			MV1SetAttachAnimBlendRateToFrame(	int MHandle, int AttachIndex, int FrameIndex, float Rate, int SetChild DEFAULTPARAM( = TRUE ) ) ;	// ア?ッ?しているアニメ?ションのブレンド率を設定する( フレ??単位 )
extern	float		MV1GetAttachAnimBlendRateToFrame(	int MHandle, int AttachIndex, int FrameIndex ) ;									// ア?ッ?しているアニメ?ションのブレンド率を取得する( フレ??単位 )
extern	int			MV1SetAttachAnimTimeToFrame(		int MHandle, int AttachIndex, int FrameIndex, float Time, int SetChild DEFAULTPARAM( = TRUE ) ) ;	// ア?ッ?しているアニメ?ションの再生時間を設定する( フレ??単位 )( Time に?イナスの値を渡すと設定を解除 )
extern	float		MV1GetAttachAnimTimeToFrame(		int MHandle, int AttachIndex, int FrameIndex ) ;									// ア?ッ?しているアニメ?ションの再生時間を取得する( フレ??単位 )
extern	int			MV1GetAttachAnim(					int MHandle, int AttachIndex ) ;													// ア?ッ?しているアニメ?ションのアニメ?ションインデックスを取得する
extern	int			MV1SetAttachAnimUseShapeFlag(		int MHandle, int AttachIndex, int UseFlag ) ;										// ア?ッ?しているアニメ?ションのシェイプを使用するかどうかを設定する( UseFlag  TRUE:使用する( デフォルト )  FALSE:使用しない )
extern	int			MV1GetAttachAnimUseShapeFlag(		int MHandle, int AttachIndex ) ;													// ア?ッ?しているアニメ?ションのシェイプを使用するかどうかを取得する
extern	VECTOR		MV1GetAttachAnimFrameLocalPosition(	int MHandle, int AttachIndex, int FrameIndex ) ;									// ア?ッ?しているアニメ?ションの指定のフレ??の現在のロ?カル座標を取得する
extern	MATRIX		MV1GetAttachAnimFrameLocalMatrix(	int MHandle, int AttachIndex, int FrameIndex ) ;									// ア?ッ?しているアニメ?ションの指定のフレ??の現在のロ?カル変換行列を取得する

extern	int			MV1GetAnimNum(						int MHandle ) ;																		// アニメ?ションの数を取得する
extern	const TCHAR *MV1GetAnimName(					int MHandle, int AnimIndex ) ;														// 指定番号のアニメ?ション名を取得する( NULL:エラ? )
extern	int			MV1SetAnimName(						int MHandle, int AnimIndex, const TCHAR *AnimName                        ) ;		// 指定番号のアニメ?ション名を変更する
extern	int			MV1SetAnimNameWithStrLen(			int MHandle, int AnimIndex, const TCHAR *AnimName, size_t AnimNameLength ) ;		// 指定番号のアニメ?ション名を変更する
extern	int			MV1GetAnimIndex(					int MHandle, const TCHAR *AnimName                        ) ;						// 指定名のアニメ?ション番号を取得する( -1:エラ? )
extern	int			MV1GetAnimIndexWithStrLen(			int MHandle, const TCHAR *AnimName, size_t AnimNameLength ) ;						// 指定名のアニメ?ション番号を取得する( -1:エラ? )
extern	float		MV1GetAnimTotalTime(				int MHandle, int AnimIndex ) ;														// 指定番号のアニメ?ションの総時間を得る
extern	int			MV1GetAnimTargetFrameNum(			int MHandle, int AnimIndex ) ;														// 指定のアニメ?ションが??ゲットとするフレ??の数を取得する
extern	const TCHAR *MV1GetAnimTargetFrameName(			int MHandle, int AnimIndex, int AnimFrameIndex ) ;									// 指定のアニメ?ションが??ゲットとするフレ??の名前を取得する
extern	int			MV1GetAnimTargetFrame(				int MHandle, int AnimIndex, int AnimFrameIndex ) ;									// 指定のアニメ?ションが??ゲットとするフレ??の番号を取得する
extern	int			MV1GetAnimTargetFrameKeySetNum(		int MHandle, int AnimIndex, int AnimFrameIndex ) ;									// 指定のアニメ?ションが??ゲットとするフレ??用のアニメ?ションキ?セットの数を取得する
extern	int			MV1GetAnimTargetFrameKeySet(		int MHandle, int AnimIndex, int AnimFrameIndex, int Index ) ;						// 指定のアニメ?ションが??ゲットとするフレ??用のアニメ?ションキ?セットキ?セットインデックスを取得する

extern	int			MV1GetAnimKeySetNum(				int MHandle ) ;																		// モデルに含まれるアニメ?ションキ?セットの総数を得る 
extern	int			MV1GetAnimKeySetType(				int MHandle, int AnimKeySetIndex ) ;												// 指定のアニメ?ションキ?セットの?イプを取得する( MV1_ANIMKEY_TYPE_QUATERNION 等 )
extern	int			MV1GetAnimKeySetDataType(			int MHandle, int AnimKeySetIndex ) ;												// 指定のアニメ?ションキ?セットのデ???イプを取得する( MV1_ANIMKEY_DATATYPE_ROTATE 等 )
extern	int			MV1GetAnimKeySetTimeType(			int MHandle, int AnimKeySetIndex ) ;												// 指定のアニメ?ションキ?セットのキ?の時間デ???イプを取得する( MV1_ANIMKEY_TIME_TYPE_ONE 等 )
extern	int			MV1GetAnimKeySetDataNum(			int MHandle, int AnimKeySetIndex ) ;												// 指定のアニメ?ションキ?セットのキ?の数を取得する
extern	float		MV1GetAnimKeyDataTime(				int MHandle, int AnimKeySetIndex, int Index ) ;										// 指定のアニメ?ションキ?セットのキ?の時間を取得する
extern	int			MV1GetAnimKeyDataIndexFromTime(		int MHandle, int AnimKeySetIndex, float Time ) ;									// 指定のアニメ?ションキ?セットの指定の時間でのキ?の番号を取得する
extern	FLOAT4		MV1GetAnimKeyDataToQuaternion(		int MHandle, int AnimKeySetIndex, int Index ) ;										// 指定のアニメ?ションキ?セットのキ?を取得する、キ??イプが MV1_ANIMKEY_TYPE_QUATERNION では無かった場合は失敗する
extern	FLOAT4		MV1GetAnimKeyDataToQuaternionFromTime( int MHandle, int AnimKeySetIndex, float Time ) ;									// 指定のアニメ?ションキ?セットのキ?を取得する、キ??イプが MV1_ANIMKEY_TYPE_QUATERNION では無かった場合は失敗する( 時間指定版 )
extern	VECTOR		MV1GetAnimKeyDataToVector(			int MHandle, int AnimKeySetIndex, int Index ) ;										// 指定のアニメ?ションキ?セットのキ?を取得する、キ??イプが MV1_ANIMKEY_TYPE_VECTOR では無かった場合は失敗する
extern	VECTOR		MV1GetAnimKeyDataToVectorFromTime(	int MHandle, int AnimKeySetIndex, float Time ) ;									// 指定のアニメ?ションキ?セットのキ?を取得する、キ??イプが MV1_ANIMKEY_TYPE_VECTOR では無かった場合は失敗する( 時間指定版 )
extern	MATRIX		MV1GetAnimKeyDataToMatrix(			int MHandle, int AnimKeySetIndex, int Index ) ;										// 指定のアニメ?ションキ?セットのキ?を取得する、キ??イプが MV1_ANIMKEY_TYPE_MATRIX4X4C か MV1_ANIMKEY_TYPE_MATRIX3X3 では無かった場合は失敗する
extern	MATRIX		MV1GetAnimKeyDataToMatrixFromTime(	int MHandle, int AnimKeySetIndex, float Time ) ;									// 指定のアニメ?ションキ?セットのキ?を取得する、キ??イプが MV1_ANIMKEY_TYPE_MATRIX4X4C か MV1_ANIMKEY_TYPE_MATRIX3X3 では無かった場合は失敗する( 時間指定版 )
extern	float		MV1GetAnimKeyDataToFlat(			int MHandle, int AnimKeySetIndex, int Index ) ;										// 指定のアニメ?ションキ?セットのキ?を取得する、キ??イプが MV1_ANIMKEY_TYPE_FLAT では無かった場合は失敗する
extern	float		MV1GetAnimKeyDataToFlatFromTime(	int MHandle, int AnimKeySetIndex, float Time ) ;									// 指定のアニメ?ションキ?セットのキ?を取得する、キ??イプが MV1_ANIMKEY_TYPE_FLAT では無かった場合は失敗する( 時間指定版 )
extern	float		MV1GetAnimKeyDataToLinear(			int MHandle, int AnimKeySetIndex, int Index ) ;										// 指定のアニメ?ションキ?セットのキ?を取得する、キ??イプが MV1_ANIMKEY_TYPE_LINEAR では無かった場合は失敗する
extern	float		MV1GetAnimKeyDataToLinearFromTime(	int MHandle, int AnimKeySetIndex, float Time ) ;									// 指定のアニメ?ションキ?セットのキ?を取得する、キ??イプが MV1_ANIMKEY_TYPE_LINEAR では無かった場合は失敗する( 時間指定版 )

// ?テリアル関係
extern	int			MV1GetMaterialNum(					int MHandle ) ;															// モデルで使用している?テリアルの数を取得する
extern	const TCHAR *MV1GetMaterialName(				int MHandle, int MaterialIndex ) ;										// 指定の?テリアルの名前を取得する
extern	int			MV1SetMaterialTypeAll(				int MHandle,                    int Type ) ;							// 全ての?テリアルの?イプを変更する( Type : DX_MATERIAL_TYPE_NORMAL など )
extern	int			MV1SetMaterialType(					int MHandle, int MaterialIndex, int Type ) ;							// 指定の?テリアルの?イプを変更する( Type : DX_MATERIAL_TYPE_NORMAL など )
extern	int			MV1GetMaterialType(					int MHandle, int MaterialIndex ) ;										// 指定の?テリアルの?イプを取得する( 戻り値 : DX_MATERIAL_TYPE_NORMAL など )
extern	int			MV1SetMaterialTypeParamAll(			int MHandle,                    ... ) ;									// 全ての?テリアルの?イプ別パラメ??を変更する( ?テリアル?イプ DX_MATERIAL_TYPE_MAT_SPEC_LUMINANCE_TWO_COLOR などで使用 )
extern	int			MV1SetMaterialTypeParam(			int MHandle, int MaterialIndex, ... ) ;									// 指定の?テリアルの?イプ別パラメ??を変更する( ?テリアル?イプ DX_MATERIAL_TYPE_MAT_SPEC_LUMINANCE_TWO_COLOR などで使用 )
//		int			MV1SetMaterialTypeParam(			int MHandle, int MaterialIndex, /* DX_MATERIAL_TYPE_MAT_SPEC_LUMINANCE_UNORM            の場合 */ float MinParam = 正規化の下限値( この値以下が 0.0f になる ), float MaxParam = 正規化の上限値( この値以上が 1.0f になる ) ) ;
//		int			MV1SetMaterialTypeParam(			int MHandle, int MaterialIndex, /* DX_MATERIAL_TYPE_MAT_SPEC_LUMINANCE_CLIP_UNORM       の場合 */ float MinParam = 正規化の下限値( この値以下が 0.0f になる ), float MaxParam = 正規化の上限値( この値以上が 1.0f になる ), float ClipParam = ?値( この値未満が 0.0f になる ) ) ;
//		int			MV1SetMaterialTypeParam(			int MHandle, int MaterialIndex, /* DX_MATERIAL_TYPE_MAT_SPEC_LUMINANCE_CMP_GREATEREQUAL の場合 */ float CmpParam = 比較値( この値以上の場合は 1.0f が、未満の場合は 0.0f が書き込まれる ) ) ;
//		int			MV1SetMaterialTypeParam(			int MHandle, int MaterialIndex, /* DX_MATERIAL_TYPE_MAT_SPEC_POWER_UNORM                の場合 */ float MinParam = 正規化の下限値( この値以下が 0.0f になる ), float MaxParam = 正規化の上限値( この値以上が 1.0f になる ) ) ;
//		int			MV1SetMaterialTypeParam(			int MHandle, int MaterialIndex, /* DX_MATERIAL_TYPE_MAT_SPEC_POWER_CLIP_UNORM           の場合 */ float MinParam = 正規化の下限値( この値以下が 0.0f になる ), float MaxParam = 正規化の上限値( この値以上が 1.0f になる ), float ClipParam = ?値( この値未満が 0.0f になる ) ) ;
//		int			MV1SetMaterialTypeParam(			int MHandle, int MaterialIndex, /* DX_MATERIAL_TYPE_MAT_SPEC_POWER_CMP_GREATEREQUAL     の場合 */ float CmpParam = 比較値( この値以上の場合は 1.0f が、未満の場合は 0.0f が書き込まれる ) ) ;
extern	int			MV1SetMaterialDifColor(				int MHandle, int MaterialIndex, COLOR_F Color ) ;						// 指定の?テリアルのディフュ?ズカラ?を設定する
extern	COLOR_F		MV1GetMaterialDifColor( 			int MHandle, int MaterialIndex ) ;										// 指定の?テリアルのディフュ?ズカラ?を取得する
extern	int			MV1SetMaterialSpcColor( 			int MHandle, int MaterialIndex, COLOR_F Color ) ;						// 指定の?テリアルのスペキュラカラ?を設定する
extern	COLOR_F		MV1GetMaterialSpcColor( 			int MHandle, int MaterialIndex ) ;										// 指定の?テリアルのスペキュラカラ?を取得する
extern	int			MV1SetMaterialEmiColor( 			int MHandle, int MaterialIndex, COLOR_F Color ) ;						// 指定の?テリアルのエ?ッシブカラ?を設定する
extern	COLOR_F		MV1GetMaterialEmiColor( 			int MHandle, int MaterialIndex ) ;										// 指定の?テリアルのエ?ッシブカラ?を取得する
extern	int			MV1SetMaterialAmbColor( 			int MHandle, int MaterialIndex, COLOR_F Color ) ;						// 指定の?テリアルのアンビエントカラ?を設定する
extern	COLOR_F		MV1GetMaterialAmbColor( 			int MHandle, int MaterialIndex ) ;										// 指定の?テリアルのアンビエントカラ?を取得する
extern	int			MV1SetMaterialSpcPower( 			int MHandle, int MaterialIndex, float Power ) ;							// 指定の?テリアルのスペキュラの強さを設定する
extern	float		MV1GetMaterialSpcPower( 			int MHandle, int MaterialIndex ) ;										// 指定の?テリアルのスペキュラの強さを取得する
extern	int			MV1SetMaterialDifMapTexture(		int MHandle, int MaterialIndex, int TexIndex ) ;						// 指定の?テリアルでディフュ?ズ?ップとして使用するテクス?ャを指定する
extern	int			MV1GetMaterialDifMapTexture(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルでディフュ?ズ?ップとして使用されているテクス?ャのインデックスを取得する
extern	int			MV1SetMaterialSubDifMapTexture(		int MHandle, int MaterialIndex, int TexIndex ) ;						// 指定の?テリアルでサブディフュ?ズ?ップとして使用するテクス?ャを指定する
extern	int			MV1GetMaterialSubDifMapTexture(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルでサブディフュ?ズ?ップとして使用されているテクス?ャのインデックスを取得する
extern	int			MV1SetMaterialSpcMapTexture(		int MHandle, int MaterialIndex, int TexIndex ) ;						// 指定の?テリアルでスペキュラ?ップとして使用するテクス?ャを指定する
extern	int			MV1GetMaterialSpcMapTexture(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルでスペキュラ?ップとして使用されているテクス?ャのインデックスを取得する
extern	int			MV1SetMaterialNormalMapTexture(		int MHandle, int MaterialIndex, int TexIndex ) ;						// 指定の?テリアルで?線?ップとして使用するテクス?ャを指定する
extern	int			MV1GetMaterialNormalMapTexture(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルで?線?ップとして使用されているテクス?ャのインデックスを取得する
extern	int			MV1SetMaterialEmissiveMapTexture(	int MHandle, int MaterialIndex, int TexIndex ) ;						// 指定の?テリアルで自己発光?ップとして使用するテクス?ャを指定する
extern	int			MV1GetMaterialEmissiveMapTexture(	int MHandle, int MaterialIndex ) ;										// 指定の?テリアルで自己発光?ップとして使用されているテクス?ャのインデックスを取得する
extern	int			MV1SetMaterialShininessMapTexture(	int MHandle, int MaterialIndex, int TexIndex ) ;						// 指定の?テリアルでラフネス?ップとして使用するテクス?ャを指定する
extern	int			MV1GetMaterialShininessMapTexture(	int MHandle, int MaterialIndex ) ;										// 指定の?テリアルでラフネス?ップとして使用されているテクス?ャのインデックスを取得する
extern	int			MV1SetMaterialReflectionFactorMapTexture( int MHandle, int MaterialIndex, int TexIndex ) ;					// 指定の?テリアルでメ?リック?ップとして使用するテクス?ャを指定する
extern	int			MV1GetMaterialReflectionFactorMapTexture( int MHandle, int MaterialIndex ) ;								// 指定の?テリアルでメ?リック?ップとして使用されているテクス?ャのインデックスを取得する
extern	int			MV1SetMaterialDifGradTexture(		int MHandle, int MaterialIndex, int TexIndex ) ;						// 指定の?テリアルでトゥ?ンレン?リングのディフュ?ズグラデ?ション?ップとして使用するテクス?ャを設定する
extern	int			MV1GetMaterialDifGradTexture(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルでトゥ?ンレン?リングのディフュ?ズグラデ?ション?ップとして使用するテクス?ャを取得する
extern	int			MV1SetMaterialSpcGradTexture(		int MHandle, int MaterialIndex, int TexIndex ) ;						// 指定の?テリアルでトゥ?ンレン?リングのスペキュラグラデ?ション?ップとして使用するテクス?ャを設定する
extern	int			MV1GetMaterialSpcGradTexture(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルでトゥ?ンレン?リングのスペキュラグラデ?ション?ップとして使用するテクス?ャを取得する
extern	int			MV1SetMaterialSphereMapTexture(		int MHandle, int MaterialIndex, int TexIndex ) ;						// 指定の?テリアルでトゥ?ンレン?リングのスフィア?ップとして使用するテクス?ャを設定する
extern	int			MV1GetMaterialSphereMapTexture(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルでトゥ?ンレン?リングのスフィア?ップとして使用するテクス?ャを取得する
extern	int			MV1SetMaterialDifGradBlendTypeAll(	int MHandle,                    int BlendType ) ;						// 全ての?テリアルのトゥ?ンレン?リングで使用するディフュ?ズグラデ?ション?ップとディフュ?ズカラ?の合成方?を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern	int			MV1SetMaterialDifGradBlendType(		int MHandle, int MaterialIndex, int BlendType ) ;						// 指定の?テリアルのトゥ?ンレン?リングで使用するディフュ?ズグラデ?ション?ップとディフュ?ズカラ?の合成方?を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern	int			MV1GetMaterialDifGradBlendType(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルのトゥ?ンレン?リングで使用するディフュ?ズグラデ?ション?ップとディフュ?ズカラ?の合成方?を取得する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern	int			MV1SetMaterialSpcGradBlendTypeAll(	int MHandle,                    int BlendType ) ;						// 全ての?テリアルのトゥ?ンレン?リングで使用するスペキュラグラデ?ション?ップとスペキュラカラ?の合成方?を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern	int			MV1SetMaterialSpcGradBlendType(		int MHandle, int MaterialIndex, int BlendType ) ;						// 指定の?テリアルのトゥ?ンレン?リングで使用するスペキュラグラデ?ション?ップとスペキュラカラ?の合成方?を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern	int			MV1GetMaterialSpcGradBlendType(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルのトゥ?ンレン?リングで使用するスペキュラグラデ?ション?ップとスペキュラカラ?の合成方?を取得する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern	int			MV1SetMaterialSphereMapBlendTypeAll( int MHandle,                   int BlendType ) ;						// 全ての?テリアルのトゥ?ンレン?リングで使用するスフィア?ップの合成方?を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern	int			MV1SetMaterialSphereMapBlendType(	int MHandle, int MaterialIndex, int BlendType ) ;						// 指定の?テリアルのトゥ?ンレン?リングで使用するスフィア?ップの合成方?を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern	int			MV1GetMaterialSphereMapBlendType(	int MHandle, int MaterialIndex ) ;										// 指定の?テリアルのトゥ?ンレン?リングで使用するスフィア?ップの合成方?を取得する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern	int			MV1SetMaterialOutLineWidthAll(		int MHandle,                    float Width ) ;							// 全ての?テリアルのトゥ?ンレン?リングで使用する輪郭線の太さを設定する
extern	int			MV1SetMaterialOutLineWidth(			int MHandle, int MaterialIndex, float Width ) ;							// 指定の?テリアルのトゥ?ンレン?リングで使用する輪郭線の太さを設定する
extern	float		MV1GetMaterialOutLineWidth(			int MHandle, int MaterialIndex ) ;										// 指定の?テリアルのトゥ?ンレン?リングで使用する輪郭線の太さを取得する
extern	int			MV1SetMaterialOutLineDotWidthAll(	int MHandle,                    float Width ) ;							// 全ての?テリアルのトゥ?ンレン?リングで使用する輪郭線のドット単位の太さを設定する
extern	int			MV1SetMaterialOutLineDotWidth(		int MHandle, int MaterialIndex, float Width ) ;							// 指定の?テリアルのトゥ?ンレン?リングで使用する輪郭線のドット単位の太さを設定する
extern	float		MV1GetMaterialOutLineDotWidth(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルのトゥ?ンレン?リングで使用する輪郭線のドット単位の太さを取得する
extern	int			MV1SetMaterialOutLineColorAll(		int MHandle,                    COLOR_F Color ) ;						// 全ての?テリアルのトゥ?ンレン?リングで使用する輪郭線の色を設定する
extern	int			MV1SetMaterialOutLineColor(			int MHandle, int MaterialIndex, COLOR_F Color ) ;						// 指定の?テリアルのトゥ?ンレン?リングで使用する輪郭線の色を設定する
extern	COLOR_F		MV1GetMaterialOutLineColor(			int MHandle, int MaterialIndex ) ;										// 指定の?テリアルのトゥ?ンレン?リングで使用する輪郭線の色を取得する
extern	int			MV1SetMaterialDrawBlendModeAll(		int MHandle,                    int BlendMode ) ;						// 全ての?テリアルの?画ブレンドモ?ドを設定する( DX_BLENDMODE_ALPHA 等 )
extern	int			MV1SetMaterialDrawBlendMode(		int MHandle, int MaterialIndex, int BlendMode ) ;						// 指定の?テリアルの?画ブレンドモ?ドを設定する( DX_BLENDMODE_ALPHA 等 )
extern	int			MV1GetMaterialDrawBlendMode(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルの?画ブレンドモ?ドを取得する( DX_BLENDMODE_ALPHA 等 )
extern	int			MV1SetMaterialDrawBlendParamAll(	int MHandle,                    int BlendParam ) ;						// 全ての?テリアルの?画ブレンドパラメ??を設定する
extern	int			MV1SetMaterialDrawBlendParam(		int MHandle, int MaterialIndex, int BlendParam ) ;						// 指定の?テリアルの?画ブレンドパラメ??を設定する
extern	int			MV1GetMaterialDrawBlendParam(		int MHandle, int MaterialIndex ) ;										// 指定の?テリアルの?画ブレンドパラメ??を設定する
extern	int			MV1SetMaterialDrawAlphaTestAll(		int MHandle,                    int Enable, int Mode, int Param ) ;		// 全ての?テリアルの?画時のアルフ?テストの設定を行う( Enable:αテストを行うかどうか( TRUE:行う  FALSE:行わない( デフォルト ) ) Mode:テストモ?ド( DX_CMP_GREATER等 )  Param:?画アルフ?値との比較に使用する値( 0?255 ) )
extern	int			MV1SetMaterialDrawAlphaTest(		int MHandle, int MaterialIndex,	int Enable, int Mode, int Param ) ;		// 指定の?テリアルの?画時のアルフ?テストの設定を行う( Enable:αテストを行うかどうか( TRUE:行う  FALSE:行わない( デフォルト ) ) Mode:テストモ?ド( DX_CMP_GREATER等 )  Param:?画アルフ?値との比較に使用する値( 0?255 ) )
extern	int			MV1GetMaterialDrawAlphaTestEnable(	int MHandle, int MaterialIndex ) ;										// 指定の?テリアルの?画時のアルフ?テストを行うかどうかを取得する( 戻り値  TRUE:アルフ?テストを行う  FALSE:アルフ?テストを行わない )
extern	int			MV1GetMaterialDrawAlphaTestMode(	int MHandle, int MaterialIndex ) ;										// 指定の?テリアルの?画時のアルフ?テストのテストモ?ドを取得する( 戻り値  テストモ?ド( DX_CMP_GREATER等 ) )
extern	int			MV1GetMaterialDrawAlphaTestParam(	int MHandle, int MaterialIndex ) ;										// 指定の?テリアルの?画時のアルフ?テストの?画アルフ?地との比較に使用する値( 0?255 )を取得する
extern	int			MV1SetMaterialDrawAddColorAll(		int MHandle,                    int Red, int Green, int Blue ) ;		// 全ての?テリアルの?画時の加算カラ?を設定する
extern	int			MV1SetMaterialDrawAddColor(			int MHandle, int MaterialIndex, int Red, int Green, int Blue ) ;		// 指定の?テリアルの?画時の加算カラ?を設定する
extern	int			MV1GetMaterialDrawAddColor(			int MHandle, int MaterialIndex, int *Red, int *Green, int *Blue ) ;		// 指定の?テリアルの?画時の加算カラ?を取得する

// テクス?ャ関係
extern	int			MV1GetTextureNum(						int MHandle ) ;														// テクス?ャの数を取得
extern	const TCHAR *MV1GetTextureName(						int MHandle, int TexIndex ) ;										// テクス?ャの名前を取得
extern	int			MV1SetTextureColorFilePath(				int MHandle, int TexIndex, const TCHAR *FilePath                        ) ;	// カラ?テクス?ャのフ?イルパスを変更する
extern	int			MV1SetTextureColorFilePathWithStrLen(	int MHandle, int TexIndex, const TCHAR *FilePath, size_t FilePathLength ) ;	// カラ?テクス?ャのフ?イルパスを変更する
extern	const TCHAR *MV1GetTextureColorFilePath(			int MHandle, int TexIndex ) ;										// カラ?テクス?ャのフ?イルパスを取得
extern	int			MV1SetTextureAlphaFilePath(				int MHandle, int TexIndex, const TCHAR *FilePath                        ) ;	// アルフ?テクス?ャのフ?イルパスを変更する
extern	int			MV1SetTextureAlphaFilePathWithStrLen(	int MHandle, int TexIndex, const TCHAR *FilePath, size_t FilePathLength ) ;	// アルフ?テクス?ャのフ?イルパスを変更する
extern	const TCHAR *MV1GetTextureAlphaFilePath(			int MHandle, int TexIndex ) ;										// アルフ?テクス?ャのフ?イルパスを取得
extern	int			MV1SetTextureGraphHandle(				int MHandle, int TexIndex, int GrHandle, int SemiTransFlag ) ;		// テクス?ャで使用するグラフィックハンドルを変更する( GrHandle を -1 にすると解除 )
extern	int			MV1GetTextureGraphHandle(				int MHandle, int TexIndex ) ;										// テクス?ャのグラフィックハンドルを取得する
extern	int			MV1SetTextureAddressMode(				int MHandle, int TexIndex, int AddrUMode, int AddrVMode ) ;			// テクス?ャのアドレスモ?ドを設定する( AddUMode の値は DX_TEXADDRESS_WRAP 等 )
extern	int			MV1GetTextureAddressModeU(				int MHandle, int TexIndex ) ;										// テクス?ャのＵ値のアドレスモ?ドを取得する( 戻り値:DX_TEXADDRESS_WRAP 等 )
extern	int			MV1GetTextureAddressModeV(				int MHandle, int TexIndex ) ;										// テクス?ャのＶ値のアドレスモ?ドを取得する( 戻り値:DX_TEXADDRESS_WRAP 等 )
extern	int			MV1GetTextureWidth(						int MHandle, int TexIndex ) ;										// テクス?ャの幅を取得する
extern	int			MV1GetTextureHeight(					int MHandle, int TexIndex ) ;										// テクス?ャの高さを取得する
extern	int			MV1GetTextureSemiTransState(			int MHandle, int TexIndex ) ;										// テクス?ャに半透明要素があるかどうかを取得する( 戻り値  TRUE:ある  FALSE:ない )
extern	int			MV1SetTextureBumpImageFlag(				int MHandle, int TexIndex, int Flag ) ;								// テクス?ャで使用している画像がバンプ?ップかどうかを設定する
extern	int			MV1GetTextureBumpImageFlag(				int MHandle, int TexIndex ) ;										// テクス?ャがバンプ?ップかどうかを取得する( 戻り値  TRUE:バンプ?ップ  FALSE:違う )
extern	int			MV1SetTextureBumpImageNextPixelLength(	int MHandle, int TexIndex, float Length ) ;							// バンプ?ップ画像の場合の隣のピクセルとの距離を設定する
extern	float		MV1GetTextureBumpImageNextPixelLength(	int MHandle, int TexIndex ) ;										// バンプ?ップ画像の場合の隣のピクセルとの距離を取得する
extern	int			MV1SetTextureSampleFilterMode(			int MHandle, int TexIndex, int FilterMode ) ;						// テクス?ャのフィル?リングモ?ドを設定する
extern	int			MV1GetTextureSampleFilterMode(			int MHandle, int TexIndex ) ;										// テクス?ャのフィル?リングモ?ドを取得する( 戻り値  DX_DRAWMODE_BILINEAR等 )
extern	int			MV1AddTexture(							int MHandle, const TCHAR *Name,                    const TCHAR *ColorFilePath,                             const TCHAR *AlphaFilePath DEFAULTPARAM( = NULL ) ,                                                  void *ColorFileImage DEFAULTPARAM( = NULL ) , void *AlphaFileImage DEFAULTPARAM( = NULL ) , int AddressModeU DEFAULTPARAM( = DX_TEXADDRESS_WRAP ) , int AddressModeV DEFAULTPARAM( = DX_TEXADDRESS_WRAP ) , int FilterMode DEFAULTPARAM( = DX_DRAWMODE_ANISOTROPIC ) , int BumpImageFlag DEFAULTPARAM( = FALSE ) , float BumpImageNextPixelLength DEFAULTPARAM( = 0.1f ) , int ReverseFlag DEFAULTPARAM( = FALSE ) , int Bmp32AllZeroAlphaToXRGB8Flag DEFAULTPARAM( = FALSE ) ) ;	// モデルで使用するテクス?ャを追加する
extern	int			MV1AddTextureWithStrLen(				int MHandle, const TCHAR *Name, size_t NameLength, const TCHAR *ColorFilePath, size_t ColorFilePathLength, const TCHAR *AlphaFilePath DEFAULTPARAM( = NULL ) , size_t AlphaFilePathLength DEFAULTPARAM( = 0 ) , void *ColorFileImage DEFAULTPARAM( = NULL ) , void *AlphaFileImage DEFAULTPARAM( = NULL ) , int AddressModeU DEFAULTPARAM( = DX_TEXADDRESS_WRAP ) , int AddressModeV DEFAULTPARAM( = DX_TEXADDRESS_WRAP ) , int FilterMode DEFAULTPARAM( = DX_DRAWMODE_ANISOTROPIC ) , int BumpImageFlag DEFAULTPARAM( = FALSE ) , float BumpImageNextPixelLength DEFAULTPARAM( = 0.1f ) , int ReverseFlag DEFAULTPARAM( = FALSE ) , int Bmp32AllZeroAlphaToXRGB8Flag DEFAULTPARAM( = FALSE ) ) ;	// モデルで使用するテクス?ャを追加する
extern	int			MV1AddTextureGraphHandle(				int MHandle, const TCHAR *Name,                    int GrHandle, int SemiTransFlag,                                                                                                                                                                                                                         int AddressModeU DEFAULTPARAM( = DX_TEXADDRESS_WRAP ) , int AddressModeV DEFAULTPARAM( = DX_TEXADDRESS_WRAP ) , int FilterMode DEFAULTPARAM( = DX_DRAWMODE_ANISOTROPIC ) ) ;																																																			// モデルで使用するテクス?ャを追加する( グラフィックハンドルをテクス?ャとして追加 )

extern	int			MV1LoadTexture(							const TCHAR *FilePath                        ) ;					// ３Ｄモデルに?り付けるのに向いた画像の読み込み方式で画像を読み込む( 戻り値  -1:エラ?  0以上:グラフィックハンドル )
extern	int			MV1LoadTextureWithStrLen(				const TCHAR *FilePath, size_t FilePathLength ) ;					// ３Ｄモデルに?り付けるのに向いた画像の読み込み方式で画像を読み込む( 戻り値  -1:エラ?  0以上:グラフィックハンドル )

// フレ??関係
extern	int			MV1GetFrameNum(						int MHandle ) ;															// フレ??の数を取得する
extern	int			MV1SearchFrame(						int MHandle, const TCHAR *FrameName                         ) ;			// フレ??の名前からモデル中のフレ??のフレ??インデックスを取得する( 無かった場合は戻り値が-1 )
extern	int			MV1SearchFrameWithStrLen(			int MHandle, const TCHAR *FrameName, size_t FrameNameLength ) ;			// フレ??の名前からモデル中のフレ??のフレ??インデックスを取得する( 無かった場合は戻り値が-1 )
extern	int			MV1SearchFrameChild(				int MHandle, int FrameIndex DEFAULTPARAM( = -1 ) , const TCHAR *ChildName DEFAULTPARAM( = NULL )                              ) ;	// フレ??の名前から指定のフレ??の子フレ??のフレ??インデックスを取得する( 名前指定版 )( FrameIndex を -1 にすると親を持たないフレ??を ChildIndex で指定する )( 無かった場合は戻り値が-1 )
extern	int			MV1SearchFrameChildWithStrLen(		int MHandle, int FrameIndex DEFAULTPARAM( = -1 ) , const TCHAR *ChildName DEFAULTPARAM( = NULL ) , size_t ChildNameLength DEFAULTPARAM( = 0 ) ) ;	// フレ??の名前から指定のフレ??の子フレ??のフレ??インデックスを取得する( 名前指定版 )( FrameIndex を -1 にすると親を持たないフレ??を ChildIndex で指定する )( 無かった場合は戻り値が-1 )
extern	const TCHAR *MV1GetFrameName(					int MHandle, int FrameIndex ) ;											// 指定のフレ??の名前を取得する( エラ?の場合は戻り値が NULL )
extern	int			MV1GetFrameName2(					int MHandle, int FrameIndex, TCHAR *StrBuffer ) ;						// 指定のフレ??の名前を取得する( 戻り値   -1:エラ?  -1以外:文字列のサイズ )
extern	int			MV1GetFrameParent(					int MHandle, int FrameIndex ) ;											// 指定のフレ??の親フレ??のインデックスを得る( 親がいない場合は -2 が返る )
extern	int			MV1GetFrameChildNum(				int MHandle, int FrameIndex DEFAULTPARAM( = -1 ) ) ;											// 指定のフレ??の子フレ??の数を取得する( FrameIndex を -1 にすると親を持たないフレ??の数が返ってくる )
extern	int			MV1GetFrameChild(					int MHandle, int FrameIndex DEFAULTPARAM( = -1 ) , int ChildIndex DEFAULTPARAM( = 0 ) ) ;		// 指定のフレ??の子フレ??のフレ??インデックスを取得する( 番号指定版 )( FrameIndex を -1 にすると親を持たないフレ??を ChildIndex で指定する )( エラ?の場合は戻り値が-1 )
extern	VECTOR		MV1GetFramePosition(				int MHandle, int FrameIndex ) ;											// 指定のフレ??の座標を取得する
extern	VECTOR_D	MV1GetFramePositionD(				int MHandle, int FrameIndex ) ;											// 指定のフレ??の座標を取得する
extern	MATRIX		MV1GetFrameBaseLocalMatrix(			int MHandle, int FrameIndex ) ;											// 指定のフレ??の初期状態での座標変換行列を取得する
extern	MATRIX_D	MV1GetFrameBaseLocalMatrixD(		int MHandle, int FrameIndex ) ;											// 指定のフレ??の初期状態での座標変換行列を取得する
extern	MATRIX		MV1GetFrameLocalMatrix(				int MHandle, int FrameIndex ) ;											// 指定のフレ??の座標変換行列を取得する
extern	MATRIX_D	MV1GetFrameLocalMatrixD(			int MHandle, int FrameIndex ) ;											// 指定のフレ??の座標変換行列を取得する
extern	MATRIX		MV1GetFrameLocalWorldMatrix(		int MHandle, int FrameIndex ) ;											// 指定のフレ??のロ?カル座標からワ?ルド座標に変換する行列を得る
extern	MATRIX_D	MV1GetFrameLocalWorldMatrixD(		int MHandle, int FrameIndex ) ;											// 指定のフレ??のロ?カル座標からワ?ルド座標に変換する行列を得る
extern	int			MV1SetFrameUserLocalMatrix(			int MHandle, int FrameIndex, MATRIX   Matrix ) ;						// 指定のフレ??の座標変換行列( ロ?カル行列 )を設定する
extern	int			MV1SetFrameUserLocalMatrixD(		int MHandle, int FrameIndex, MATRIX_D Matrix ) ;						// 指定のフレ??の座標変換行列( ロ?カル行列 )を設定する
extern	int			MV1ResetFrameUserLocalMatrix(		int MHandle, int FrameIndex ) ;											// 指定のフレ??の座標変換行列( ロ?カル行列 )をデフォルトに戻す
extern	int			MV1SetFrameUserLocalWorldMatrix(	int MHandle, int FrameIndex, MATRIX   Matrix ) ;						// 指定のフレ??の座標変換行列( ロ?カル座標からワ?ルド座標に変換する行列 )を設定する
extern	int			MV1SetFrameUserLocalWorldMatrixD(	int MHandle, int FrameIndex, MATRIX_D Matrix ) ;						// 指定のフレ??の座標変換行列( ロ?カル座標からワ?ルド座標に変換する行列 )を設定する
extern	int			MV1ResetFrameUserLocalWorldMatrix(	int MHandle, int FrameIndex ) ;											// 指定のフレ??の座標変換行列( ロ?カル座標からワ?ルド座標に変換する行列 )をデフォルトに戻す
extern	VECTOR		MV1GetFrameMaxVertexLocalPosition(	int MHandle, int FrameIndex ) ;											// 指定のフレ??が持つメッシュ頂?のロ?カル座標での最大値を得る
extern	VECTOR_D	MV1GetFrameMaxVertexLocalPositionD(	int MHandle, int FrameIndex ) ;											// 指定のフレ??が持つメッシュ頂?のロ?カル座標での最大値を得る
extern	VECTOR		MV1GetFrameMinVertexLocalPosition(	int MHandle, int FrameIndex ) ;											// 指定のフレ??が持つメッシュ頂?のロ?カル座標での最小値を得る
extern	VECTOR_D	MV1GetFrameMinVertexLocalPositionD(	int MHandle, int FrameIndex ) ;											// 指定のフレ??が持つメッシュ頂?のロ?カル座標での最小値を得る
extern	VECTOR		MV1GetFrameAvgVertexLocalPosition(	int MHandle, int FrameIndex ) ;											// 指定のフレ??が持つメッシュ頂?のロ?カル座標での平均値を得る
extern	VECTOR_D	MV1GetFrameAvgVertexLocalPositionD(	int MHandle, int FrameIndex ) ;											// 指定のフレ??が持つメッシュ頂?のロ?カル座標での平均値を得る
extern	int			MV1GetFrameVertexNum(				int MHandle, int FrameIndex ) ;											// 指定のフレ??に含まれる頂?の数を取得する
extern	int			MV1GetFrameTriangleNum(				int MHandle, int FrameIndex ) ;											// 指定のフレ??に含まれる?リゴンの数を取得する
extern	int			MV1GetFrameMeshNum(					int MHandle, int FrameIndex ) ;											// 指定のフレ??が持つメッシュの数を取得する
extern	int			MV1GetFrameMesh(					int MHandle, int FrameIndex, int Index ) ;								// 指定のフレ??が持つメッシュのメッシュインデックスを取得する
extern	int			MV1SetFrameVisible(					int MHandle, int FrameIndex, int VisibleFlag ) ;						// 指定のフレ??の?示、非?示状態を変更する( TRUE:?示  FALSE:非?示 )
extern	int			MV1GetFrameVisible(					int MHandle, int FrameIndex ) ;											// 指定のフレ??の?示、非?示状態を取得する( TRUE:?示  FALSE:非?示 )
extern	int			MV1SetFrameDifColorScale(			int MHandle, int FrameIndex, COLOR_F Scale ) ;							// 指定のフレ??のディフュ?ズカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	int			MV1SetFrameSpcColorScale(			int MHandle, int FrameIndex, COLOR_F Scale ) ;							// 指定のフレ??のスペキュラカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	int			MV1SetFrameEmiColorScale(			int MHandle, int FrameIndex, COLOR_F Scale ) ;							// 指定のフレ??のエ?ッシブカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	int			MV1SetFrameAmbColorScale(			int MHandle, int FrameIndex, COLOR_F Scale ) ;							// 指定のフレ??のアンビエントカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetFrameDifColorScale(			int MHandle, int FrameIndex ) ;											// 指定のフレ??のディフュ?ズカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetFrameSpcColorScale(			int MHandle, int FrameIndex ) ;											// 指定のフレ??のスペキュラカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetFrameEmiColorScale(			int MHandle, int FrameIndex ) ;											// 指定のフレ??のエ?ッシブカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetFrameAmbColorScale(			int MHandle, int FrameIndex ) ;											// 指定のフレ??のアンビエントカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	int			MV1GetFrameSemiTransState(			int MHandle, int FrameIndex ) ;											// 指定のフレ??に半透明要素があるかどうかを取得する( 戻り値 TRUE:ある  FALSE:ない )
extern	int			MV1SetFrameOpacityRate(				int MHandle, int FrameIndex, float Rate ) ;								// 指定のフレ??の不透明度を設定する( 不透明 1.0f ? 透明 0.0f )
extern	float		MV1GetFrameOpacityRate(				int MHandle, int FrameIndex ) ;											// 指定のフレ??の不透明度を取得する( 不透明 1.0f ? 透明 0.0f )
extern	int			MV1SetFrameBaseVisible(				int MHandle, int FrameIndex, int VisibleFlag ) ;						// 指定のフレ??の初期?示状態を設定する( TRUE:?示  FALSE:非?示 )
extern	int			MV1GetFrameBaseVisible(				int MHandle, int FrameIndex ) ;											// 指定のフレ??の初期?示状態を取得する( TRUE:?示  FALSE:非?示 )
extern	int			MV1SetFrameTextureAddressTransform( int MHandle, int FrameIndex, float TransU, float TransV, float ScaleU, float ScaleV, float RotCenterU, float RotCenterV, float Rotate ) ;	// 指定のフレ??のテクス?ャ座標変換パラメ??を設定する
extern	int			MV1SetFrameTextureAddressTransformMatrix( int MHandle, int FrameIndex, MATRIX Matrix ) ;					// 指定のフレ??のテクス?ャ座標変換行列をセットする
extern	int			MV1ResetFrameTextureAddressTransform( int MHandle, int FrameIndex ) ;										// 指定のフレ??のテクス?ャ座標変換パラメ??をリセットする

// メッシュ関係
extern	int			MV1GetMeshNum(						int MHandle ) ;															// モデルに含まれるメッシュの数を取得する
extern	int			MV1GetMeshMaterial(					int MHandle, int MeshIndex ) ;											// 指定メッシュが使用している?テリアルのインデックスを取得する
extern	int			MV1GetMeshVertexNum(				int MHandle, int MeshIndex ) ;											// 指定メッシュに含まれる頂?の数を取得する
extern	int			MV1GetMeshTriangleNum(				int MHandle, int MeshIndex ) ;											// 指定メッシュに含まれる三角??リゴンの数を取得する
extern	int			MV1SetMeshVisible(					int MHandle, int MeshIndex, int VisibleFlag ) ;							// 指定メッシュの?示、非?示状態を変更する( TRUE:?示  FALSE:非?示 )
extern	int			MV1GetMeshVisible(					int MHandle, int MeshIndex ) ;											// 指定メッシュの?示、非?示状態を取得する( TRUE:?示  FALSE:非?示 )
extern	int			MV1SetMeshDifColorScale(			int MHandle, int MeshIndex, COLOR_F Scale ) ;							// 指定のメッシュのディフュ?ズカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	int			MV1SetMeshSpcColorScale( 			int MHandle, int MeshIndex, COLOR_F Scale ) ;							// 指定のメッシュのスペキュラカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	int			MV1SetMeshEmiColorScale( 			int MHandle, int MeshIndex, COLOR_F Scale ) ;							// 指定のメッシュのエ?ッシブカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	int			MV1SetMeshAmbColorScale( 			int MHandle, int MeshIndex, COLOR_F Scale ) ;							// 指定のメッシュのアンビエントカラ?のスケ?ル値を設定する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetMeshDifColorScale( 			int MHandle, int MeshIndex ) ;											// 指定のメッシュのディフュ?ズカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetMeshSpcColorScale( 			int MHandle, int MeshIndex ) ;											// 指定のメッシュのスペキュラカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetMeshEmiColorScale( 			int MHandle, int MeshIndex ) ;											// 指定のメッシュのエ?ッシブカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		MV1GetMeshAmbColorScale( 			int MHandle, int MeshIndex ) ;											// 指定のメッシュのアンビエントカラ?のスケ?ル値を取得する( デフォルト値は 1.0f )
extern	int			MV1SetMeshOpacityRate( 				int MHandle, int MeshIndex, float Rate ) ;								// 指定のメッシュの不透明度を設定する( 不透明 1.0f ? 透明 0.0f )
extern	float		MV1GetMeshOpacityRate( 				int MHandle, int MeshIndex ) ;											// 指定のメッシュの不透明度を取得する( 不透明 1.0f ? 透明 0.0f )
extern	int			MV1SetMeshDrawBlendMode( 			int MHandle, int MeshIndex, int BlendMode ) ;							// 指定のメッシュの?画ブレンドモ?ドを設定する( DX_BLENDMODE_ALPHA 等 )
extern	int			MV1SetMeshDrawBlendParam( 			int MHandle, int MeshIndex, int BlendParam ) ;							// 指定のメッシュの?画ブレンドパラメ??を設定する
extern	int			MV1GetMeshDrawBlendMode( 			int MHandle, int MeshIndex ) ;											// 指定のメッシュの?画ブレンドモ?ドを取得する( DX_BLENDMODE_ALPHA 等 )
extern	int			MV1GetMeshDrawBlendParam( 			int MHandle, int MeshIndex ) ;											// 指定のメッシュの?画ブレンドパラメ??を設定する
extern	int			MV1SetMeshBaseVisible( 				int MHandle, int MeshIndex, int VisibleFlag ) ;							// 指定のメッシュの初期?示状態を設定する( TRUE:?示  FALSE:非?示 )
extern	int			MV1GetMeshBaseVisible( 				int MHandle, int MeshIndex ) ;											// 指定のメッシュの初期?示状態を取得する( TRUE:?示  FALSE:非?示 )
extern	int			MV1SetMeshBackCulling( 				int MHandle, int MeshIndex, int CullingFlag ) ;							// 指定のメッシュのバックカリングを行うかどうかを設定する( DX_CULLING_LEFT 等 )
extern	int			MV1GetMeshBackCulling( 				int MHandle, int MeshIndex ) ;											// 指定のメッシュのバックカリングを行うかどうかを取得する( DX_CULLING_LEFT 等 )
extern	VECTOR		MV1GetMeshMaxPosition( 				int MHandle, int MeshIndex ) ;											// 指定のメッシュに含まれる?リゴンの最大ロ?カル座標を取得する
extern	VECTOR		MV1GetMeshMinPosition( 				int MHandle, int MeshIndex ) ;											// 指定のメッシュに含まれる?リゴンの最小ロ?カル座標を取得する
extern	int			MV1GetMeshTListNum( 				int MHandle, int MeshIndex ) ;											// 指定のメッシュに含まれるトライアングルリストの数を取得する
extern	int			MV1GetMeshTList( 					int MHandle, int MeshIndex, int Index ) ;								// 指定のメッシュに含まれるトライアングルリストのインデックスを取得する
extern	int			MV1GetMeshSemiTransState( 			int MHandle, int MeshIndex ) ;											// 指定のメッシュに半透明要素があるかどうかを取得する( 戻り値 TRUE:ある  FALSE:ない )
extern	int			MV1SetMeshUseVertDifColor( 			int MHandle, int MeshIndex, int UseFlag ) ;								// 指定のメッシュの頂?ディフュ?ズカラ?を?テリアルのディフュ?ズカラ?の代わりに使用するかどうかを設定する( TRUE:?テリアルカラ?の代わりに使用する  FALSE:?テリアルカラ?を使用する )
extern	int			MV1SetMeshUseVertSpcColor( 			int MHandle, int MeshIndex, int UseFlag ) ;								// 指定のメッシュの頂?スペキュラカラ?を?テリアルのスペキュラカラ?の代わりに使用するかどうかを設定する( TRUE:?テリアルカラ?の代わりに使用する  FALSE:?テリアルカラ?を使用する )
extern	int			MV1GetMeshUseVertDifColor( 			int MHandle, int MeshIndex ) ;											// 指定のメッシュの頂?ディフュ?ズカラ?を?テリアルのディフュ?ズカラ?の代わりに使用するかどうかの設定を取得する( 戻り値  TRUE:?テリアルカラ?の代わりに使用する  FALSE:?テリアルカラ?を使用する )
extern	int			MV1GetMeshUseVertSpcColor( 			int MHandle, int MeshIndex ) ;											// 指定のメッシュの頂?スペキュラカラ?を?テリアルのスペキュラカラ?の代わりに使用するかどうかの設定を取得する( 戻り値  TRUE:?テリアルカラ?の代わりに使用する  FALSE:?テリアルカラ?を使用する )
extern	int			MV1GetMeshShapeFlag(				int MHandle, int MeshIndex ) ;											// 指定のメッシュがシェイプメッシュかどうかを取得する( 戻り値 TRUE:シェイプメッシュ  FALSE:通常メッシュ )

// シェイプ関係
extern	int			MV1GetShapeNum(						int MHandle ) ;															// モデルに含まれるシェイプの数を取得する
extern	int			MV1SearchShape(						int MHandle, const TCHAR *ShapeName                         ) ;			// シェイプの名前からモデル中のシェイプのシェイプインデックスを取得する( 無かった場合は戻り値が-1 )
extern	int			MV1SearchShapeWithStrLen(			int MHandle, const TCHAR *ShapeName, size_t ShapeNameLength ) ;			// シェイプの名前からモデル中のシェイプのシェイプインデックスを取得する( 無かった場合は戻り値が-1 )
extern	const TCHAR *MV1GetShapeName(					int MHandle, int ShapeIndex ) ;											// 指定シェイプの名前を取得する
extern	int			MV1GetShapeTargetMeshNum(			int MHandle, int ShapeIndex ) ;											// 指定シェイプが対象としているメッシュの数を取得する
extern	int			MV1GetShapeTargetMesh(				int MHandle, int ShapeIndex, int Index ) ;								// 指定シェイプが対象としているメッシュのメッシュインデックスを取得する
extern	int			MV1SetShapeRate(					int MHandle, int ShapeIndex, float Rate, int Type DEFAULTPARAM( = DX_MV1_SHAPERATE_ADD ) ) ;	// 指定シェイプの有効率を設定する( Rate  0.0f:0% ? 1.0f:100% )
extern	float		MV1GetShapeRate(					int MHandle, int ShapeIndex ) ;											// 指定シェイプの有効率を取得する( 戻り値  0.0f:0% ? 1.0f:100% )
extern	float		MV1GetShapeApplyRate(				int MHandle, int ShapeIndex ) ;											// 指定シェイプの有効率を取得する( 戻り値  0.0f:0% ? 1.0f:100% )( MV1SetShapeRate で指定した値がそのまま戻り値となる MV1GetShapeRate と異なりアニメ?ションのシェイプ情報なども加味した値が戻り値となります )

// トライアングルリスト関係
extern	int			MV1GetTriangleListNum(						int MHandle ) ;																		// モデルに含まれるトライアングルリストの数を取得する
extern	int			MV1GetTriangleListVertexType(				int MHandle, int TListIndex ) ;														// 指定のトライアングルリストの頂?デ???イプを取得する( DX_MV1_VERTEX_TYPE_1FRAME 等 )
extern	int			MV1GetTriangleListPolygonNum(				int MHandle, int TListIndex ) ;														// 指定のトライアングルリストに含まれる?リゴンの数を取得する
extern	int			MV1GetTriangleListVertexNum(				int MHandle, int TListIndex ) ;														// 指定のトライアングルリストに含まれる頂?デ??の数を取得する
extern	int			MV1GetTriangleListLocalWorldMatrixNum(		int MHandle, int TListIndex ) ;														// 指定のトライアングルリストが使用する座標変換行列の数を取得する
extern	MATRIX		MV1GetTriangleListLocalWorldMatrix(			int MHandle, int TListIndex, int LWMatrixIndex ) ;									// 指定のトライアングルリストが使用する座標変換行列( ロ?カル→ワ?ルド )を取得する
extern	int			MV1GetTriangleListPolygonVertexPosition(	int MHandle, int TListIndex, int PolygonIndex, VECTOR *VertexPositionArray DEFAULTPARAM( = NULL ) , float *MatrixWeightArray DEFAULTPARAM( = NULL ) ) ;	// 指定のトライアングルリストの指定の?リゴンが使用している頂?の座標を取得する( 戻り値  エラ?：-1  0以上：?リゴンが使用している頂?の数 )
extern	int			MV1GetTriangleListUseMaterial(				int MHandle, int TListIndex ) ;														// 指定のトライアングルリストが使用している?テリアルのインデックスを取得する

// コリジョン関係
extern	int							MV1SetupCollInfo(				int MHandle, int FrameIndex DEFAULTPARAM( = -1 ) , int XDivNum DEFAULTPARAM( = 32 ) , int YDivNum DEFAULTPARAM( = 8 ) , int ZDivNum DEFAULTPARAM( = 32 ) , int MeshIndex DEFAULTPARAM( = -1 ) ) ;		// コリジョン情報を?築する
extern	int							MV1TerminateCollInfo(			int MHandle, int FrameIndex DEFAULTPARAM( = -1 ) , int MeshIndex DEFAULTPARAM( = -1 ) ) ;																// コリジョン情報の後始末
extern	int							MV1RefreshCollInfo(				int MHandle, int FrameIndex DEFAULTPARAM( = -1 ) , int MeshIndex DEFAULTPARAM( = -1 ) ) ;																// コリジョン情報を更新する
extern	MV1_COLL_RESULT_POLY		MV1CollCheck_Line(				int MHandle, int FrameIndex, VECTOR PosStart, VECTOR PosEnd , int MeshIndex DEFAULTPARAM( = -1 ) ) ;					// 線とモデルの当たり判定
extern	MV1_COLL_RESULT_POLY_DIM	MV1CollCheck_LineDim(			int MHandle, int FrameIndex, VECTOR PosStart, VECTOR PosEnd , int MeshIndex DEFAULTPARAM( = -1 ) ) ;					// 線とモデルの当たり判定( 戻り値が MV1_COLL_RESULT_POLY_DIM )
extern	MV1_COLL_RESULT_POLY_DIM	MV1CollCheck_Sphere(			int MHandle, int FrameIndex, VECTOR CenterPos, float r , int MeshIndex DEFAULTPARAM( = -1 ) ) ;							// 球とモデルの当たり判定
extern	MV1_COLL_RESULT_POLY_DIM	MV1CollCheck_Capsule(			int MHandle, int FrameIndex, VECTOR Pos1, VECTOR Pos2, float r , int MeshIndex DEFAULTPARAM( = -1 ) ) ;					// カプセルとモデルの当たり判定
extern	MV1_COLL_RESULT_POLY_DIM	MV1CollCheck_Triangle(			int MHandle, int FrameIndex, VECTOR Pos1, VECTOR Pos2, VECTOR Pos3 , int MeshIndex DEFAULTPARAM( = -1 ) ) ;				// 三角?とモデルの当たり判定
extern	MV1_COLL_RESULT_POLY		MV1CollCheck_GetResultPoly(		MV1_COLL_RESULT_POLY_DIM ResultPolyDim, int PolyNo ) ;																	// コリジョン結果?リゴン配列から指定番号の?リゴン情報を取得する
extern	int							MV1CollResultPolyDimTerminate(	MV1_COLL_RESULT_POLY_DIM ResultPolyDim ) ;																				// コリジョン結果?リゴン配列の後始末をする

// 参照用メッシュ関係
extern	int					MV1SetupReferenceMesh(		int MHandle, int FrameIndex, int IsTransform, int IsPositionOnly DEFAULTPARAM( = FALSE ) , int MeshIndex DEFAULTPARAM( = -1 ) ) ;					// 参照用メッシュのセットアップ
extern	int					MV1TerminateReferenceMesh(	int MHandle, int FrameIndex, int IsTransform, int IsPositionOnly DEFAULTPARAM( = FALSE ) , int MeshIndex DEFAULTPARAM( = -1 ) ) ;					// 参照用メッシュの後始末
extern	int					MV1RefreshReferenceMesh(	int MHandle, int FrameIndex, int IsTransform, int IsPositionOnly DEFAULTPARAM( = FALSE ) , int MeshIndex DEFAULTPARAM( = -1 ) ) ;					// 参照用メッシュの更新
extern	MV1_REF_POLYGONLIST	MV1GetReferenceMesh(		int MHandle, int FrameIndex, int IsTransform, int IsPositionOnly DEFAULTPARAM( = FALSE ) , int MeshIndex DEFAULTPARAM( = -1 ) ) ;					// 参照用メッシュを取得する

#endif // DX_NON_MODEL
























// DxLive2DCubism4.cpp 関数 プロト?イプ宣言

#ifndef DX_NON_LIVE2D_CUBISM4

extern	int			Live2D_SetCubism4CoreDLLPath(           const TCHAR *CoreDLLFilePath ) ;								// Live2DCubismCore.dll のフ?イルパスを設定する
extern	int			Live2D_SetCubism4CoreDLLPathWithStrLen( const TCHAR *CoreDLLFilePath, size_t CoreDLLFilePathLength ) ;	// Live2DCubismCore.dll のフ?イルパスを設定する
extern	int			Live2D_SetCubism3CoreDLLPath(           const TCHAR *CoreDLLFilePath ) ;								// Live2DCubismCore.dll のフ?イルパスを設定する
extern	int			Live2D_SetCubism3CoreDLLPathWithStrLen( const TCHAR *CoreDLLFilePath, size_t CoreDLLFilePathLength ) ;	// Live2DCubismCore.dll のフ?イルパスを設定する

extern	int			Live2D_RenderBegin( void ) ;																			// Live2D の?画処理を開始する
extern	int			Live2D_RenderEnd( void ) ;																				// Live2D の?画処理を終了する

extern	int			Live2D_LoadModel(			const TCHAR *FilePath ) ;													// Live2D のモデルフ?イルを読み込む( 0以上:Live2Dモデルハンドル  ?イナス値:エラ?発生 )
extern	int			Live2D_LoadModelWithStrLen(	const TCHAR *FilePath, size_t FilePathLength ) ;							// Live2D のモデルフ?イルを読み込む( 0以上:Live2Dモデルハンドル  ?イナス値:エラ?発生 )
extern	int			Live2D_DeleteModel(			int Live2DModelHandle ) ;													// Live2D のモデルを削除する
extern	int			Live2D_InitModel(			void ) ;																	// すべての Live2D のモデルを削除する

extern	int			Live2D_SetUserShader( int TargetShader /* DX_LIVE2D_SHADER_NORMAL_PIXEL 等 */ , int ShaderHandle DEFAULTPARAM( = -1 ) ) ;		// Live2D のモデル?画で使用するシェ???を設定する( ShaderHandle に -1 を渡すと設定を解除 )
extern	int			Live2D_DrawCallback( void ( *Callback )( int Live2DModelHandle, int TextureIndex, void *UserData ), void *UserData ) ;			// Live2D のモデル?画の前に呼ばれるコ?ルバック関数を設定する Callback に NULL を渡すと設定を解除 )
extern	int			Live2D_SetUseAutoScaling( int UseFlag ) ;																// Live2D のモデル?画をする際に、画面サイズに応じたスケ?リングを行うかを設定する( UseFlag  TRUE:スケ?リングを行う( デフォルト )  FALSE:スケ?リングを行わない )
extern	int			Live2D_SetUseAutoCentering( int UseFlag ) ;																// Live2D のモデルを画面の中心に?画するかを設定する( UseFlag   TRUE:画面の中心に?画する( デフォルト )   FALSE:画面の中心に?画しない )
extern	int			Live2D_SetUseReverseYAxis( int UseFlag ) ;																// Live2D_Model_SetTranslate で指定する平行移動値の y の向きを反?するかを設定する( UseFlag   TRUE:反?する( デフォルト )   FALSE:反?しない )

extern	int			Live2D_Model_Update(			int Live2DModelHandle, float DeltaTimeSeconds ) ;						// Live2D のモデルの状態を更新する
extern	int			Live2D_Model_SetTranslate(		int Live2DModelHandle, float x, float y ) ;								// Live2D のモデルの位置を設定する
extern	int			Live2D_Model_SetExtendRate(		int Live2DModelHandle, float ExRateX, float ExRateY ) ;					// Live2D のモデルの拡大率を設定する
extern	int			Live2D_Model_SetRotate(			int Live2DModelHandle, float RotAngle ) ;								// Live2D のモデルの回?を設定する
extern	int			Live2D_Model_Draw(				int Live2DModelHandle ) ;												// Live2D のモデルを?画する

extern	int			Live2D_Model_StartMotion(				int Live2DModelHandle, const TCHAR *group,						int no, float fadeInSeconds DEFAULTPARAM( = -1.0f ) , float fadeOutSeconds DEFAULTPARAM( = -1.0f ) , int isLoopFadeIn DEFAULTPARAM( = TRUE ) , int isLoop DEFAULTPARAM( = FALSE ) ) ;	// Live2D のモデルの指定のモ?ションを再生する
extern	int			Live2D_Model_StartMotionWithStrLen(		int Live2DModelHandle, const TCHAR *group, size_t groupLength,	int no, float fadeInSeconds DEFAULTPARAM( = -1.0f ) , float fadeOutSeconds DEFAULTPARAM( = -1.0f ) , int isLoopFadeIn DEFAULTPARAM( = TRUE ) , int isLoop DEFAULTPARAM( = FALSE ) ) ;	// Live2D のモデルの指定のモ?ションを再生する
extern	int			Live2D_Model_GetLastPlayMotionNo(		int Live2DModelHandle ) ;																			// Live2D のモデルで最後に再生したモ?ションのグル?プ内の番号を取得する
extern	int			Live2D_Model_IsMotionFinished(			int Live2DModelHandle ) ;																			// Live2D のモデルのモ?ション再生が終了しているかを取得する( 戻り値  TRUE:再生が終了している  FALSE:再生中 )
extern	float		Live2D_Model_GetMotionPlayTime(			int Live2DModelHandle ) ;																			// Live2D のモデルのモ?ション再生時間を取得する
extern	int			Live2D_Model_SetExpression(				int Live2DModelHandle, const TCHAR *expressionID ) ;												// Live2D のモデルの指定の?情モ?ションを設定する
extern	int			Live2D_Model_SetExpressionWithStrLen(	int Live2DModelHandle, const TCHAR *expressionID, size_t expressionIDLength ) ;						// Live2D のモデルの指定の?情モ?ションを設定する
extern	int			Live2D_Model_HitTest(					int Live2DModelHandle, const TCHAR *hitAreaName,							float x, float y ) ;	// 指定の座標が Live2D のモデルの指定の当たり判定の矩?範囲内か判定する( TRUE:矩?範囲内  FALSE:矩?範囲外 )
extern	int			Live2D_Model_HitTestWithStrLen(			int Live2DModelHandle, const TCHAR *hitAreaName, size_t hitAreaNameLength,	float x, float y ) ;	// 指定の座標が Live2D のモデルの指定の当たり判定の矩?範囲内か判定する( TRUE:矩?範囲内  FALSE:矩?範囲外 )

extern	int			Live2D_Model_GetParameterCount(						int Live2DModelHandle ) ;																// Live2D のモデルに設定されているパラメ??の数を取得する
extern	const TCHAR *Live2D_Model_GetParameterId(						int Live2DModelHandle, int index ) ;													// Live2D のモデルに設定されているパラメ??のIDを取得する
extern	float		Live2D_Model_GetParameterValue(						int Live2DModelHandle, const TCHAR *parameterId ) ;										// Live2D のモデルに設定されているパラメ??を取得する
extern	float		Live2D_Model_GetParameterValueWithStrLen(			int Live2DModelHandle, const TCHAR *parameterId, size_t parameterIdLength ) ;			// Live2D のモデルに設定されているパラメ??を取得する
extern	int			Live2D_Model_SetParameterValue(						int Live2DModelHandle, const TCHAR *parameterId,                           float value ) ;	// Live2D のモデルに設定されているパラメ??を設定する
extern	int			Live2D_Model_SetParameterValueWithStrLen(			int Live2DModelHandle, const TCHAR *parameterId, size_t parameterIdLength, float value ) ;	// Live2D のモデルに設定されているパラメ??を設定する

extern	int			Live2D_Model_GetHitAreasCount(						int Live2DModelHandle ) ;																// Live2D のモデルに設定された当たり判定の数を取得する
extern	const TCHAR *Live2D_Model_GetHitAreaName(						int Live2DModelHandle, int index ) ;													// Live2D のモデルの当たり判定に設定された名前を取得する
extern	const TCHAR *Live2D_Model_GetPhysicsFileName(					int Live2DModelHandle ) ;																// Live2D のモデルの物理演算設定フ?イルの名前を取得する
extern	const TCHAR *Live2D_Model_GetPoseFileName(						int Live2DModelHandle ) ;																// Live2D のモデルのパ?ツ切り替え設定フ?イルの名前を取得する
extern	int			Live2D_Model_GetExpressionCount(					int Live2DModelHandle ) ;																// Live2D のモデルの?情設定フ?イルの数を取得する
extern	const TCHAR *Live2D_Model_GetExpressionName(					int Live2DModelHandle, int index ) ;													// Live2D のモデルの?情設定フ?イルを識別するIDを取得する
extern	const TCHAR *Live2D_Model_GetExpressionFileName(				int Live2DModelHandle, int index ) ;													// Live2D のモデルの?情設定フ?イルの名前を取得する
extern	int			Live2D_Model_GetMotionGroupCount(					int Live2DModelHandle ) ;																// Live2D のモデルのモ?ショングル?プの数を取得する
extern	const TCHAR *Live2D_Model_GetMotionGroupName(					int Live2DModelHandle, int index ) ;													// Live2D のモデルのモ?ショングル?プの名前を取得する
extern	int			Live2D_Model_GetMotionCount(						int Live2DModelHandle, const TCHAR *groupName ) ;										// Live2D のモデルのモ?ショングル?プに含まれるモ?ションの数を取得する
extern	int			Live2D_Model_GetMotionCountWithStrLen(				int Live2DModelHandle, const TCHAR *groupName, size_t groupNameLength ) ;				// Live2D のモデルのモ?ショングル?プに含まれるモ?ションの数を取得する
extern	const TCHAR *Live2D_Model_GetMotionFileName(					int Live2DModelHandle, const TCHAR *groupName,							int index ) ;	// Live2D のモデルのグル?プ名とインデックス値からモ?ションフ?イルの名前を取得する
extern	const TCHAR *Live2D_Model_GetMotionFileNameWithStrLen(			int Live2DModelHandle, const TCHAR *groupName, size_t groupNameLength,	int index ) ;	// Live2D のモデルのグル?プ名とインデックス値からモ?ションフ?イルの名前を取得する
extern	const TCHAR *Live2D_Model_GetMotionSoundFileName(				int Live2DModelHandle, const TCHAR *groupName,							int index ) ;	// Live2D のモデルのモ?ションに対応するサウンドフ?イルの名前を取得する
extern	const TCHAR *Live2D_Model_GetMotionSoundFileNameWithStrLen(		int Live2DModelHandle, const TCHAR *groupName, size_t groupNameLength,	int index ) ;	// Live2D のモデルのモ?ションに対応するサウンドフ?イルの名前を取得する
extern	float		Live2D_Model_GetMotionFadeInTimeValue(				int Live2DModelHandle, const TCHAR *groupName,							int index ) ;	// Live2D のモデルのモ?ション開始時のフェ?ドイン処理時間を取得する
extern	float		Live2D_Model_GetMotionFadeInTimeValueWithStrLen(	int Live2DModelHandle, const TCHAR *groupName, size_t groupNameLength,	int index ) ;	// Live2D のモデルのモ?ション開始時のフェ?ドイン処理時間を取得する
extern	float		Live2D_Model_GetMotionFadeOutTimeValue(				int Live2DModelHandle, const TCHAR *groupName,							int index ) ;	// Live2D のモデルのモ?ション終了時のフェ?ドアウト処理時間を取得する
extern	float		Live2D_Model_GetMotionFadeOutTimeValueWithStrLen(	int Live2DModelHandle, const TCHAR *groupName, size_t groupNameLength,	int index ) ;	// Live2D のモデルのモ?ション終了時のフェ?ドアウト処理時間を取得する
extern	const TCHAR *Live2D_Model_GetUserDataFile(						int Live2DModelHandle ) ;																// Live2D のモデルのユ?ザデ??のフ?イル名を取得する
extern	int			Live2D_Model_GetEyeBlinkParameterCount(				int Live2DModelHandle ) ;																// Live2D のモデルの目パ?に関連付けられたパラメ??の数を取得する
extern	const TCHAR *Live2D_Model_GetEyeBlinkParameterId(				int Live2DModelHandle, int index ) ;													// Live2D のモデルの目パ?に関連付けられたパラメ??のIDを取得する
extern	int			Live2D_Model_GetLipSyncParameterCount(				int Live2DModelHandle ) ;																// Live2D のモデルのリップシンクに関連付けられたパラメ??の数を取得する
extern	const TCHAR *Live2D_Model_GetLipSyncParameterId(				int Live2DModelHandle, int index ) ;													// Live2D のモデルのリップシンクに関連付けられたパラメ??のIDを取得する
extern	float		Live2D_Model_GetCanvasWidth(						int Live2DModelHandle ) ;																// Live2D のモデルのキャンバスの横幅を取得する
extern	float		Live2D_Model_GetCanvasHeight(						int Live2DModelHandle ) ;																// Live2D のモデルのキャンバスの縦幅を取得する

#endif // DX_NON_LIVE2D_CUBISM4








#undef DX_FUNCTION_START
#define DX_FUNCTION_END
#undef DX_FUNCTION_END

#if defined( __cplusplus ) && defined( DX_COMPILE_TYPE_C_LANGUAGE )
}
#endif

#ifndef DX_NON_NAMESPACE

}

#endif // DX_NON_NAMESPACE

// ネ??スペ?ス DxLib を使用する ------------------------------------------------------
#ifndef DX_NON_NAMESPACE
#ifndef DX_NON_USING_NAMESPACE_DXLIB

using namespace DxLib ;

#endif // DX_NON_USING_NAMESPACE_DXLIB
#endif // DX_NON_NAMESPACE

// ＤＸライブラリ内部でのみ使用するヘッ?フ?イルのインクル?ド -------------------------

#ifdef DX_MAKE
//	#include "DxStatic.h"
#endif

#endif // DX_LIB_H


