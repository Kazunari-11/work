//---------------------------------------------------------------------------
//! @file   Arrow.h
//! @brief  目標地点を示す矢印
//---------------------------------------------------------------------------
#pragma once
#include <System/Object.h>

//! @class Arrow
//! @brief 目標地点を示す矢印
class Arrow : public Object
{
public:
    //! @brief コンストラクタ
    Arrow();

    //! @brief デストラクタ
    ~Arrow();

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新処理
    void Update([[maybe_unused]] float delta) override;

    //! @brief GUI
    void GUI() override;

private:
    //! @brief ターゲットの方向へ向ける
    void RotationToTarget();
};
