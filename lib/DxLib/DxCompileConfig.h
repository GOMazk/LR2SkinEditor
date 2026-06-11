// ----------------------------------------------------------------------------
//
//		ＤＸライブラリ?コンパイルコンフィグヘッ?フ?イル
//
//				Ver 3.25 
//
// ----------------------------------------------------------------------------

/*
<<注意>>
??このヘッ?フ?イルの #define のコメントアウト状態を変更しても、
  ＤＸライブラリ自体の再コンパイルを行わないと効果はありません。
*/

// 多重インクル?ド防?用?クロ
#ifndef DXCOMPILECONFIG_H
#define DXCOMPILECONFIG_H

// 設定 -----------------------------------------------------------------------

// ＤＸライブラリに必要な lib フ?イルを、プロジェクトのカレントフォル?や
// コンパイラのデフォルト LIB パスに設定せずに使用される場合は以下の
// コメントを外してください
//#define DX_LIB_NOT_DEFAULTPATH

#ifndef DX_MAKE

// ?画関連の関数を一切使用されない場合は以下のコメントを外して下さい
//#define DX_NOTUSE_DRAWFUNCTION

#endif // DX_MAKE

// ス?ティックライブラリ生成時ライブラリ??制限用定? -----------------------

// C言語用としてコンパイルする場合は次のコメントを外してください
//#define DX_COMPILE_TYPE_C_LANGUAGE

// namespace DxLib を使用しない場合は次のコメントを外してください
//#define DX_NON_NAMESPACE

// DxLib.h に書かれている using namespace DxLib ; を使用しない場合は次のコメントを外してください
//#define DX_NON_USING_NAMESPACE_DXLIB

// インラインアセンブラを使用しない??スコ?ドでコンパイルする場合は以下のコメントアウトを外してください
#define DX_NON_INLINE_ASM

// ＤＸライブラリのプログラ?から文字列を一切排除する場合はコメントを外してください
// (バイナリエディ?で実行フ?イルを開いてもＤＸライブラリを使用していることが分かり難くなります(よく調べれば当然分かりますが)
//  因みに、エラ?出力が一切されなくなりますので管理人の対応時に支障をきたすかもしれません)
//#define DX_NON_LITERAL_STRING

// すべての関数をスレッドセ?フにする場合はコメントを外してください
//#define DX_THREAD_SAFE

// ネットワ?ク関連の関数だけスレッドセ?フにする場合はコメントを外して下さい
#define DX_THREAD_SAFE_NETWORK_ONLY

// ＤＸア?カイブがいらない方は次のコメントを外してください
// ( ＤＸア?カイブを無効にすると、ＤＸア?カイブを内部で使っている関係上 DX_NON_MODEL と DX_NON_FILTER と DX_NON_MOVIE と DX_NON_NORMAL_DRAW_SHADER も有効になります )
//#define DX_NON_DXA

// ??ビ???がいらない方は次のコメントを外してください
//#define DX_NON_MOVIE

// Switch の MP4 再生??がいらない方は次のコメントを外してください
//#define DX_NON_NSW_MP4

// ＴＧ?画像の読み込み??がいらない方は次のコメントを外してください
//#define DX_NON_TGA

// ＪＰＥＧ画像の読みこみ??がいらない方は次のコメントを外してください
// ※DxUseCLib.lib も再コンパイルする必要があります
//#define DX_NON_JPEGREAD

// ＰＮＧ画像の読みこみ??がいらない方は次のコメントを外してください
// ※DxUseCLib.lib も再コンパイルする必要があります
//#define DX_NON_PNGREAD

// ＴＩＦＦ画像の読みこみ??がいらない方は次のコメントを外してください
// ※DxUseCLib.lib も再コンパイルする必要があります
//#define DX_NON_TIFFREAD

// GraphFilter や GraphBlend を使用しない方は次のコメントを外して下さい
//#define DX_NON_FILTER

// 通常?画にプログラ?ブルシェ???を使用しない方は次のコメントを外してください
//#define DX_NON_NORMAL_DRAW_SHADER

// グラフィックス処理がいらない方は次のコメントを外してください
//#define DX_NON_GRAPHICS

// ?フトウエア２Ｄ?画がいらない方は次のコメントを外してください
//#define DX_NON_2DDRAW

// 標? WAVE フ?イル以外の音声フ?イルを使用しない方は次のコメントを外してください
//#define DX_NON_ACM

// DirectShow を使用した MP3 フ?イルのデコ?ドを行わない場合は次のコメントをはずしてください
//#define DX_NON_DSHOW_MP3

// DirectShow を使用した動画フ?イルのデコ?ドを行わない場合は次のコメントをはずしてください
//#define DX_NON_DSHOW_MOVIE

// Media Foundation を使用した動画フ?イルのデコ?ドを行わない場合は次のコメントをはずしてください
//#define DX_NON_MEDIA_FOUNDATION

// キ???ドの基?的な入力??以外はいらない方は次のコメントを外してください
//#define DX_NON_KEYEX

// 文字列入力??はいらない方は次のコメントを外して下さい
//#define DX_NON_INPUTSTRING

// 通信??がいらない方は次のコメントを外してください
//#define DX_NON_NETWORK

// ?スク??がいらない方は次のコメントを外してください
//#define DX_NON_MASK

// Ｏｇｇ Ｖｏｒｂｉｓ デ??を使用しない方は次のコメントをはずしてください
// ※DxUseCLib.lib も再コンパイルする必要があります
//#define DX_NON_OGGVORBIS

// Ｏｇｇ Ｔｈｅｏｒａ デ??を使用しない方は次のコメントをはずしてください
// ※DxUseCLib.lib も再コンパイルする必要があります
//#define DX_NON_OGGTHEORA

// Opus デ??を使用しない方は次のコメントをはずしてください
// ※DxUseCLib.lib も再コンパイルする必要があります
//#define DX_NON_OPUS

// ASIO を使用しない方は次のコメントをはずしてください
//#define DX_NON_ASIO

// 乱数発生器に Mersenne Twister を使用しない場合は以下のコメントを外して下さい
// ※DxUseCLib.lib も再コンパイルする必要があります
//#define DX_NON_MERSENNE_TWISTER

// ＤＸライブラリ内の確保メモリの?ンプ??を有効にする場合は次のコメントを外してください
#define DX_USE_DXLIB_MEM_DUMP

// モデルデ??を扱う??を使用しない方は次のコメントを外してください
//#define DX_NON_MODEL

// Bullet Physics をリンクしない場合は次のコメントを外してください
// ※DxUseCLib.lib も再コンパイルする必要があります
//#define DX_NON_BULLET_PHYSICS

// ＦＢＸフ?イルを読み込む??を使用する方は次のコメントを外してください( 使用には FBX SDK のセットアップが必要です )
//#define DX_LOAD_FBX_MODEL

// ビ?プ音??がいらない方は次のコメントを外してください
//#define DX_NON_BEEP

// ?スクスイッ?をＯＦＦにする??がいらない方は次のコメントを外してください
// ( ?スクスイッ?ＯＦＦ??は使用不可です )
//#define DX_NON_STOPTASKSWITCH

// ログ出力を行わない場合は次のコメントを外して下さい
//#define DX_NON_LOG

// printfDx を無効にする場合は次のコメントを外して下さい
//#define DX_NON_PRINTF_DX

// 非同期読み込みを無効にする場合は次のコメントを外して下さい
//#define DX_NON_ASYNCLOAD

// フ?イル保存??を無効にする場合は次のコメントを外して下さい
//#define DX_NON_SAVEFUNCTION

// ?フトウェアで扱うイメ?ジを無効にする場合は次のコメントを外して下さい
//#define DX_NON_SOFTIMAGE

// フォント?画??を無効にする場合は次のコメントを外して下さい
//#define DX_NON_FONT

// サウンド再生??( ?フトウエアサウンド、MIDI含む )を無効にする場合は次のコメントを外して下さい
//#define DX_NON_SOUND

// 入力??を無効にする場合は次のコメントを外して下さい
//#define DX_NON_INPUT

// ?ル?スレッドを使用しない場合は次のコメントを外してください
//#define DX_NON_MULTITHREAD

// 各ハンドルのエラ??ェックを無効にする場合は次のコメントを外してください( 若干高速化される代わりに無効なハンドルを関数に渡すと即不正なメモリアクセスエラ?が発生するようになります )
//#define DX_NON_HANDLE_ERROR_CHECK

// Direct3D11 を使用しない場合は以下のコメントを外してください( 現在開発中なので、必ずコメントを外した状態にしてください )
//#define DX_NON_DIRECT3D11

// Direct3D9 を使用しない場合は以下のコメントを外してください
//#define DX_NON_DIRECT3D9

// 軽量バ?ジョンのＤＸライブラリを生成する場合は次のコメントを外してください
//#define DX_LIB_LITEVER

// コンパイル済みのシェ???バイナリを使用せず、シェ???コ?ドの実行時コンパイルを利用する場合は次のコメントを外してください
//#define DX_NON_SHADERCODE_BINARY

// Live2D Cubism 4 関連の??を使用しない場合は次のコメントを外してください
//#define DX_NON_LIVE2D_CUBISM4

// ウィンドウを作成しない場合は次のコメントを外してください
//#define DX_NON_WINDOW

#ifndef __cplusplus
	#ifndef DX_COMPILE_TYPE_C_LANGUAGE
		#define DX_COMPILE_TYPE_C_LANGUAGE
	#endif // DX_COMPILE_TYPE_C_LANGUAGE
#endif // __cplusplus

#ifdef DX_COMPILE_TYPE_C_LANGUAGE
	#ifndef DX_NON_NAMESPACE
		#define DX_NON_NAMESPACE
	#endif // DX_NON_NAMESPACE
#endif // DX_COMPILE_TYPE_C_LANGUAGE

#ifndef __APPLE__
#ifndef __ANDROID__
	#define WINDOWS_DESKTOP_OS
#endif // __ANDROID__
#endif // __APPLE__

#if defined( DX_GCC_COMPILE ) || defined( __ANDROID__ ) || defined( __APPLE__ ) || defined( BC2_COMPILER )
	#define USE_ULL
#endif



#ifndef WINDOWS_DESKTOP_OS
	#ifndef DX_NON_BEEP
		#define DX_NON_BEEP
	#endif // DX_NON_BEEP
	#ifndef DX_NON_ACM
		#define DX_NON_ACM
	#endif // DX_NON_ACM
	#ifndef DX_NON_DSHOW_MP3
		#define DX_NON_DSHOW_MP3
	#endif // DX_NON_DSHOW_MP3
	#ifndef DX_NON_DSHOW_MOVIE
		#define DX_NON_DSHOW_MOVIE
	#endif // DX_NON_DSHOW_MOVIE
	#ifndef DX_NON_MEDIA_FOUNDATION
		#define DX_NON_MEDIA_FOUNDATION
	#endif // DX_NON_MEDIA_FOUNDATION
#endif // WINDOWS_DESKTOP_OS

#if defined( __ANDROID__ ) || defined( __APPLE__ )
//#define DX_NON_2DDRAW
#define DX_NON_ACM
#define DX_NON_DSHOW_MP3
#define DX_NON_DSHOW_MOVIE
#define DX_NON_MEDIA_FOUNDATION
#define DX_NON_KEYEX
#define DX_NON_INPUTSTRING
#define DX_NON_NETWORK
#define DX_NON_STOPTASKSWITCH
#define DX_NON_DIRECT3D11
#define DX_NON_DIRECT3D9
#endif





#ifdef DX_LIB_LITEVER
#define DX_NON_ACM
#define DX_NON_2DDRAW
#define DX_NON_MOVIE
#define DX_NON_KEYEX
#define DX_NON_NETWORK
#define DX_NON_MASK
#define DX_NON_JPEGREAD
#define DX_NON_PNGREAD
#define DX_NON_TIFFREAD
#define DX_NON_BEEP
#define DX_NON_OGGVORBIS
#define DX_NON_OGGTHEORA
#define DX_NON_OPUS
#define DX_NON_MODEL
#define DX_NON_SHADERCODE_BINARY
#define DX_NON_INPUTSTRING
#endif

#ifdef DX_NON_WINDOW
	#ifndef DX_NON_GRAPHICS
		#define DX_NON_GRAPHICS
	#endif
	#ifndef DX_NON_SOUND
		#define DX_NON_SOUND
	#endif
	#ifndef DX_NON_INPUT
		#define DX_NON_INPUT
	#endif
	#ifndef DX_NON_NETWORK
		#define DX_NON_NETWORK
	#endif
#endif // DX_NON_WINDOW

#ifdef DX_NON_GRAPHICS
	#ifndef DX_NON_FONT
		#define DX_NON_FONT
	#endif
	#ifndef DX_NON_MOVIE
		#define DX_NON_MOVIE
	#endif
	#ifndef DX_NON_FILTER
		#define DX_NON_FILTER
	#endif
	#ifndef DX_NON_2DDRAW
		#define DX_NON_2DDRAW
	#endif
	#ifndef DX_NON_MASK
		#define DX_NON_MASK
	#endif
	#ifndef DX_NON_MODEL
		#define DX_NON_MODEL
	#endif
	#ifndef DX_NON_BULLET_PHYSICS
		#define DX_NON_BULLET_PHYSICS
	#endif
	#ifndef DX_NON_PRINTF_DX
		#define DX_NON_PRINTF_DX
	#endif
	#ifndef DX_NON_KEYEX
		#define DX_NON_KEYEX
	#endif
	#ifndef DX_NON_DIRECT3D11
		#define DX_NON_DIRECT3D11
	#endif
	#ifndef DX_NON_DIRECT3D9
		#define DX_NON_DIRECT3D9
	#endif
	#ifndef DX_NOTUSE_DRAWFUNCTION
		#define DX_NOTUSE_DRAWFUNCTION
	#endif
	#ifndef DX_NON_LIVE2D_CUBISM4
		#define DX_NON_LIVE2D_CUBISM4
	#endif
#endif // DX_NON_GRAPHICS

#ifdef DX_NON_SOUND
	#ifndef DX_NON_OPUS
		#define DX_NON_OPUS
	#endif
	#ifndef DX_NON_OGGVORBIS
		#define DX_NON_OGGVORBIS
	#endif
	#ifndef DX_NON_OGGTHEORA
		#define DX_NON_OGGTHEORA
	#endif
	#ifndef DX_NON_ACM
		#define DX_NON_ACM
	#endif
	#ifndef DX_NON_DSHOW_MP3
		#define DX_NON_DSHOW_MP3
	#endif
	#ifndef DX_NON_DSHOW_MOVIE
		#define DX_NON_DSHOW_MOVIE
	#endif
	#ifndef DX_NON_MEDIA_FOUNDATION
		#define DX_NON_MEDIA_FOUNDATION
	#endif
	#ifndef DX_NON_MOVIE
		#define DX_NON_MOVIE
	#endif
#endif // DX_NON_SOUND

#ifdef DX_NON_MULTITHREAD
	#ifndef DX_NON_ASYNCLOAD
		#define DX_NON_ASYNCLOAD
	#endif
	#ifdef DX_THREAD_SAFE
		#undef DX_THREAD_SAFE
	#endif
#endif

#ifdef DX_NON_INPUTSTRING
	#ifndef DX_NON_KEYEX
		#define DX_NON_KEYEX
	#endif
#endif

#ifdef DX_NON_DXA
	#ifndef DX_NON_NORMAL_DRAW_SHADER
		#define DX_NON_NORMAL_DRAW_SHADER
	#endif
	#ifndef DX_NON_MODEL
		#define DX_NON_MODEL
	#endif
	#ifndef DX_NON_FILTER
		#define DX_NON_FILTER
	#endif
	#ifndef DX_NON_MOVIE
		#define DX_NON_MOVIE
	#endif
#endif

#ifdef DX_NON_MOVIE
	#ifndef DX_NON_OGGTHEORA
		#define DX_NON_OGGTHEORA
	#endif
	#ifndef DX_NON_MEDIA_FOUNDATION
		#define DX_NON_MEDIA_FOUNDATION
	#endif // DX_NON_MEDIA_FOUNDATION
#endif

#ifdef DX_NON_INPUT
	#ifndef DX_NON_KEYEX
		#define DX_NON_KEYEX
	#endif
#endif

#ifdef DX_NON_FONT
	#ifndef DX_NON_PRINTF_DX
		#define DX_NON_PRINTF_DX
	#endif
	#ifndef DX_NON_KEYEX
		#define DX_NON_KEYEX
	#endif
#endif

#ifdef DX_NON_OGGVORBIS
	#ifndef DX_NON_OGGTHEORA
		#define DX_NON_OGGTHEORA
	#endif
	#ifndef DX_NON_OPUS
		#define DX_NON_OPUS
	#endif
#endif


#if defined( _WIN64 ) || defined( __LP64__ )
	#ifndef PLATFORM_64BIT
		#define PLATFORM_64BIT
	#endif
#endif




#if defined( _WIN64 ) || defined( __ANDROID__ ) || defined( __APPLE__ ) || defined( DX_GCC_COMPILE )
	#ifndef DX_NON_INLINE_ASM
		#define DX_NON_INLINE_ASM
	#endif
#endif





#include "DxDataType.h"

#endif // DXCOMPILECONFIG_H
