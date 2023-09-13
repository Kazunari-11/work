//---------------------------------------------------------------------------
//!	@file	InputMouse.cpp
//! @brief	マウス入力管理
//---------------------------------------------------------------------------
#include "InputMouse.h"
#include "WinMain.h"

namespace
{
enum face
{
    primary,
    secondary,

    num,
};

constexpr int MAX_MOUSE_BUTTON                = 8;
constexpr int MOUSE_BUTTONS[MAX_MOUSE_BUTTON] = {
    MOUSE_INPUT_LEFT,
    MOUSE_INPUT_RIGHT,
    MOUSE_INPUT_MIDDLE,
    MOUSE_INPUT_1,
    MOUSE_INPUT_2,
    MOUSE_INPUT_3,
    MOUSE_INPUT_4,
    MOUSE_INPUT_5,
    // これ以降のマウスのボタンの押下状態を取得する場合は、
    // 事前にSetUseDirectInputFlagを実行する必要がある
    //MOUSE_INPUT_6,    MOUSE_INPUT_7,     MOUSE_INPUT_8
};

int mouseX    = 0;
int mouseY    = 0;
int mouseOldX = 0;
int mouseOldY = 0;

bool hide = false;

std::array<u32, MAX_MOUSE_BUTTON> mouseButtons;
std::array<int, face::num>        mouseStatus;
int                               status_index = face::primary;

int now_index()
{
    return status_index;
}

int old_index()
{
    return status_index == face::primary ? face::secondary : face::primary;
}

// マウスの配列検証用
bool IsOverMouseNum(u32 mouseID)
{
    return (mouseID >= MAX_MOUSE_BUTTON);
}
};   // namespace

//---------------------------------------------------------------------------
// 初期化
//---------------------------------------------------------------------------
void InputMouseInit()
{
    mouseX = 0;
    mouseY = 0;

    std::fill(mouseButtons.begin(), mouseButtons.end(), 0);

    // 以下同じ意味です
    //std::fill_n( mouseButtons, MAX_MOUSE_BUTTON, 0 );

    /*
	for( int i = 0; i < MAX_MOUSE_BUTTON; ++i )
	{
		mouseButtons[ i ] = 0;
	}
	*/
}

//---------------------------------------------------------------------------
// 更新
//---------------------------------------------------------------------------
void InputMouseUpdate()
{
    mouseOldX = mouseX;
    mouseOldY = mouseY;

    GetMousePoint(&mouseX, &mouseY);

    for(int i = 0; i < MAX_MOUSE_BUTTON; ++i) {
        // 各マウスボタンとの押下状態を取得する
        if(GetMouseInput() & MOUSE_BUTTONS[i]) {
            ++mouseButtons[i];
            if(mouseButtons[i] >= INT_MAX)
                mouseButtons[i] = INT_MAX;
            continue;
        }

        mouseButtons[i] = 0;
    }

    status_index++;
    status_index %= face::num;
    mouseStatus[status_index] = GetMouseInput();

    if(hide) {
        const int pos_x = (int)(WINDOW_W / 2.0f);
        const int pos_y = (int)(WINDOW_H / 2.0f);
        SetMousePoint(pos_x, pos_y);
        mouseOldX = pos_x;
        mouseOldY = pos_y;
    }
}

//---------------------------------------------------------------------------
// 終了
//---------------------------------------------------------------------------
void InputMouseExit()
{
    ;
}

//---------------------------------------------------------------------------
// 指定マウスボタンの1回だけ押下検証
//---------------------------------------------------------------------------
bool IsMouseOn(int mouseID)
{
    int tmp_mouseID = mouseID - 1;
    if(IsOverMouseNum(tmp_mouseID))
        return false;
    return (mouseButtons[tmp_mouseID] == 1);
}

//---------------------------------------------------------------------------
// 指定キーが押されていない検証
//---------------------------------------------------------------------------
bool IsMouseRelease(int mouseID)
{
    int tmp_mouseID = mouseID - 1;
    if(IsOverMouseNum(tmp_mouseID))
        return false;
    return (mouseButtons[tmp_mouseID] == 0);
}

//---------------------------------------------------------------------------
// 指定キーの長押し検証
//---------------------------------------------------------------------------
bool IsMouseRepeat(int mouseID, u32 frame)
{
    int tmp_mouseID = mouseID - 1;
    if(IsOverMouseNum(tmp_mouseID))
        return false;
    return (mouseButtons[tmp_mouseID] >= frame);
}

//---------------------------------------------------------------------------
// 指定マウスボタンの1回だけ押下検証
//---------------------------------------------------------------------------
bool IsMouseDown(int mouseID)
{
    auto now = mouseStatus[now_index()] & mouseID;
    auto old = mouseStatus[old_index()] & mouseID;

    return now && !old;
}

//---------------------------------------------------------------------------
// 指定キーが押されていない検証
//---------------------------------------------------------------------------
bool IsMouseUp(int mouseID)
{
    auto now = mouseStatus[now_index()] & mouseID;
    auto old = mouseStatus[old_index()] & mouseID;

    return !now && old;
}

//---------------------------------------------------------------------------
// 指定キーの検証
//---------------------------------------------------------------------------
bool IsMouse(int mouseID)
{
    auto now = mouseStatus[now_index()] & mouseID;
    return now;
}

//---------------------------------------------------------------------------
// マウスのX座標 取得
//---------------------------------------------------------------------------
int GetMouseX()
{
    return mouseX;
}

//---------------------------------------------------------------------------
// マウスのY座標 取得
//---------------------------------------------------------------------------
int GetMouseY()
{
    return mouseY;
}

//---------------------------------------------------------------------------
// マウスのX移動量 取得
//---------------------------------------------------------------------------
int GetMouseMoveX()
{
    return mouseX - mouseOldX;
}

//---------------------------------------------------------------------------
// マウスのY移動量 取得
//---------------------------------------------------------------------------
int GetMouseMoveY()
{
    return mouseY - mouseOldY;
}

//---------------------------------------------------------------------------
// マウスを消去する ( 真ん中に置いておく )
// hide 消すかどうか
//---------------------------------------------------------------------------
void HideMouse(bool _hide)
{
    hide = _hide;
    SetMouseDispFlag(!hide);
}
