#include "WinMain.h"
#include "Game/GameMain.h"
#include <System/SystemMain.h>
#include <System/Utils/IniFileLib.h>

s32 WINDOW_W = 1280;
s32 WINDOW_H = 720;

//---------------------------------------------------------------------------
//! アプリケーションエントリーポイント
//---------------------------------------------------------------------------
int WINAPI WinMain(_In_ [[maybe_unused]] HINSTANCE     hInstance,
                   _In_opt_ [[maybe_unused]] HINSTANCE hPrevInstance,
                   _In_ [[maybe_unused]] LPSTR         lpCmdLine,
                   _In_ [[maybe_unused]] int           nShowCmd)
{
    // 高DPI対応
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    SetOutApplicationLogValidFlag(FALSE);

    // Game.iniから読み込む
    IniFileLib   ini("Game.ini");
    const bool   is_fullscreen = ini.GetBool("System", "FullScreen");
    const float2 screen_size   = ini.GetFloat2("System", "ScreenSize", {WINDOW_W, WINDOW_H});
    const auto   title_name    = ini.GetString("System", "Title", "BaseProject2022");

    WINDOW_W = static_cast<int>(screen_size.x);
    WINDOW_H = static_cast<int>(screen_size.y);

    SetGraphMode(WINDOW_W, WINDOW_H, 32);

    // ウィンドウモード / フルスクリーンモード 切り替え
    ChangeWindowMode(!is_fullscreen);

    if(!is_fullscreen) {
        // 高DPI対応のための自動リサイズを抑制するために手動でウィンドウサイズを指定。
        // 但し、フルスクリーンモードでこれを実行すると描画範囲が壊れるためウィンドウモード時のみ実行
        SetWindowSize(WINDOW_W, WINDOW_H);
    }

    SetBackgroundColor(0, 0, 0);
    SetMainWindowText(title_name.c_str());
    SetAlwaysRunFlag(true);   // ウィンドウメッセージを常に実行
    HideMouse();

    // DirectX11を使用するようにする(DxLib_Init関数前) - Effekseer対応
    SetUseDirect3DVersion(DX_DIRECT3D_11);

    // デプスバッファの精度向上
    SetZBufferBitDepth(32);

    // 非同期読み込み処理を行うスレッドの数を設定
    SetASyncLoadThreadNum(4);

    if(DxLib_Init() == -1) {
        return -1;
    }

    // Effekseerの初期化
    if(Effekseer_Init(8000) == -1) {
        DxLib_End();
        return -1;
    }

    // Effekseer対応
    SetChangeScreenModeGraphicsSystemResetFlag(FALSE);
    Effekseer_SetGraphicsDeviceLostCallbackFunctions();

    SetDrawScreen(DX_SCREEN_BACK);
    SetTransColor(255, 0, 255);
    srand(GetNowCount() % RAND_MAX);
    SRand(GetNowCount() % RAND_MAX);

    SetCameraNearFar(1.0f, 150.0f);
    SetupCamera_Perspective(D2R(45.0f));

    //----------------------------------------------------------
    // 初期化処理
    //----------------------------------------------------------
    InputKeyInit();
    InputPadInit();
    InputMouseInit();
    RenderInit();   // Render初期化
    SystemInit();
    GameInit();
    ImGuiInit();
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);

    //----------------------------------------------------------
    // メインループ
    //----------------------------------------------------------
    while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0 && !IsProcEnd()) {
        // 1フレームの開始
        SystemBeginFrame();

        // 描画先の設定と画面クリア
        SetRenderTarget(GetBackBuffer(), GetDepthStencil());
        ClearColor(GetBackBuffer(), float4(0.5, 0.5f, 0.5f, 0.0f));
        ClearDepth(GetDepthStencil(), 1.0f);

        clsDx();

        InputKeyUpdate();
        InputPadUpdate();
        InputMouseUpdate();
        ImGuiUpdate();

        ShaderBase::updateFileWatcher();   // ファイル監視を更新

        // ---------------
        // 更新処理
        // ---------------

        Effekseer_Sync3DSetting();
        GameUpdate();
        SystemUpdate();
        UpdateEffekseer3D();

        // ---------------
        // 描画処理
        // ---------------
        GameDraw();
        SystemDraw();
        DrawEffekseer3D();
        ImGuiDraw();

        // 1フレームの終了
        SystemEndFrame();

        // ---------------
        // 画面更新
        // ---------------
        ScreenFlip();
    }

    //----------------------------------------------------------
    // 終了処理
    //----------------------------------------------------------
    ImGuiExit();
    InputKeyExit();
    InputPadExit();
    InputMouseExit();
    GameExit();
    SystemExit();
    RenderExit();   // Render終了

    Effkseer_End();
    WaitHandleASyncLoadAll();   // 非同期ロード中のハンドルを全て待つ
    DxLib_End();

    return 0;
}
