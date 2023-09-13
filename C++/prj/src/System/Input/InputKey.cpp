//---------------------------------------------------------------------------
//! @file   InputKey.cpp
//! @brief  キー入力管理
//---------------------------------------------------------------------------
#include "InputKey.h"

namespace Input
{

namespace
{
constexpr int MAX_KEY_NUM = 256;

enum face
{
    Primary,
    Secondary,

    Num,
};

std::array<std::array<char, MAX_KEY_NUM>, face::Num> keys;

int primary = face::Primary;

int key_now()
{
    return primary;
}

int key_old()
{
    return primary == face::Primary ? face::Secondary : face::Primary;
}

std::array<unsigned char, MAX_KEY_NUM> count;

//-----------------------------------------------------------------------
//! keys_配列検証用
//! @param  [in]    keyID   キー種別
//! @return キー種別が範囲外であればtrueが返ります。
//-----------------------------------------------------------------------
bool IsOverKeyNum(u32 keyID)
{
    return (keyID >= MAX_KEY_NUM);
}
};   // namespace

//---------------------------------------------------------------------------
// 初期化
//---------------------------------------------------------------------------
void InputKeyInit()
{
    std::fill(count.begin(), count.end(), 0);

    // 以下同じ意味です
    //std::fill_n( count, MAX_KEY_NUM, 0 );

    /*
		for( int i = 0; i < MAX_KEY_NUM; ++i )
		{
			keys[ i ] = 0;
		}
		*/
}

//---------------------------------------------------------------------------
// 更新
//---------------------------------------------------------------------------
int InputKeyUpdate()
{
    primary++;
    primary %= face::Num;

    GetHitKeyStateAll(keys[key_now()].data());

    for(int i = 0; i < MAX_KEY_NUM; ++i) {
        if(keys[key_now()][i] != 0) {
            count[i]++;
            if(count[i] >= CHAR_MAX)
                count[i] = CHAR_MAX;
            continue;
        }

        count[i] = 0;
    }

    return 0;
}

//---------------------------------------------------------------------------
// 終了
//---------------------------------------------------------------------------
void InputKeyExit()
{
}

//---------------------------------------------------------------------------
// 指定キーの1回だけ押下検証
//---------------------------------------------------------------------------
bool IsKeyOn(int keyID)
{
    if(ImGui::IsAnyItemActive())
        return false;

    if(IsOverKeyNum(keyID))
        return false;
    return (count[keyID] == 1);
}

//---------------------------------------------------------------------------
// 指定キーが押されていない検証
//---------------------------------------------------------------------------
bool IsKeyRelease(int keyID)
{
    if(ImGui::IsAnyItemActive())
        return false;

    if(IsOverKeyNum(keyID))
        return false;
    return (count[keyID] == 0);
}

//---------------------------------------------------------------------------
// 指定キーの長押し検証
//---------------------------------------------------------------------------
bool IsKeyRepeat(int keyID, u32 frame)
{
    if(ImGui::IsAnyItemActive())
        return false;

    if(IsOverKeyNum(keyID))
        return false;
    return (count[keyID] >= frame);
}

//---------------------------------------------------------------------------
// 指定キーの検証
//---------------------------------------------------------------------------
bool IsKey(int keyID)
{
    if(ImGui::IsAnyItemActive())
        return false;

    return keys[key_now()][keyID];
}

//---------------------------------------------------------------------------
// Windowsポンプに合わせた命名
//---------------------------------------------------------------------------
bool IsKeyDown(int keyID)
{
    if(ImGui::IsAnyItemActive())
        return false;

    return keys[key_now()][keyID] && !keys[key_old()][keyID];
}

bool IsKeyUp(int keyID)
{
    if(ImGui::IsAnyItemActive())
        return false;

    return !keys[key_now()][keyID] && keys[key_old()][keyID];
}

}   // namespace Input
