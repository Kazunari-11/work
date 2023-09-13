//----------------------------------------------------------------------------
// Doxygen 総合概要
//----------------------------------------------------------------------------
//! @mainpage TimeShooter
//! @li @b ゲーム概要 \n
//! 　　TPSシューティングゲームです。\n
//!     敵を倒しながら目標地点を目指してください。\n
//!
//! @image html "..\prj\data\TimeShooter.png" width=640px height=400px
//! \n
//! @ref section_component
//!

//! @page section_control ゲームの操作
//!
//! WASD: 移動\n
//! マウス: 視点移動\n
//! 右クリック: 射撃\n
//! 左クリック: ターゲッティング\n
//! ホイールクリック: 近接攻撃\n
//! スペース: スキル(時を止める)\n
//! Rボタン: リロード\n
//!

//---------------------------------------------------------------------------
//! @file   Title.cpp
//! @brief  タイトル
//---------------------------------------------------------------------------
#include <Game/Scenes/Title.h>
#include <Game/Scenes/Tutorial.h>
#include <Game/Objects/TitleCharacter.h>
#include <System/Component/ComponentFilterFade.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCamera.h>

BP_CLASS_IMPL(Title, u8"タイトル");

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool Title::Init()
{
    auto character = Scene::CreateObject<TitleCharacter>();
    character->SetName("Titlecharacter")->SetTranslate({0, 48, 20});

    font_handle_ = LoadFontDataToHandle("data/Game/Font/TypewriterFont.dft");

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void Title::Update([[maybe_unused]] float delta)
{
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void Title::Draw()
{
}

//---------------------------------------------------------------------------
//!< 描画
//---------------------------------------------------------------------------
void Title::PostDraw()
{
    DrawStringToHandle(int(WINDOW_W / 3.5), int(WINDOW_H / 3.5), "TimeShooter", WHITE, font_handle_);

    DrawStringToHandle(120, WINDOW_H - 200, "Push mouse left button", WHITE, font_handle_);
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void Title::Exit()
{
    DeleteFontToHandle(font_handle_);
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void Title::GUI()
{
}
