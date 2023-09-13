//---------------------------------------------------------------------------
//! @file   Camera.h
//! @brief  カメラ
//---------------------------------------------------------------------------
#pragma once
#include <System/Object.h>
#include <System/Component/ComponentCamera.h>

USING_PTR(Camera);

//! @class Camera
//! @brief カメラ
class Camera : public Object
{
private:
    float3 vec_{0.0f, 2.0f, 1.0f};   //!< カメラの距離
    float3 rot_{0.0f, 0.0f, 0.0f};   //!< カメラの回転

public:
    //! @brief カメラ作成
    //! @param obj カメラを付けるオブジェクト
    //! @return カメラオブジェクト
    static CameraPtr Create(ObjectPtr obj);

    //! @brief 更新処理
    void Update([[maybe_unused]] float delta);

    //! @brief カメラをエイム状態に
    void SetCameraAim();

    //! @brief カメラを通常状態に
    void SetCameraNormal();
};
