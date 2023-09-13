//---------------------------------------------------------------------------
//! @file   Camera.cpp
//! @brief  カメラ
//---------------------------------------------------------------------------
#include <Game/Objects/Camera.h>
#include <System/Scene.h>
#include <System/Component/ComponentSpringArm.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentModel.h>
#include <Game/Objects/Player.h>

// カメラ作成
CameraPtr Camera::Create(ObjectPtr obj)
{
    auto camobj = Scene::CreateObject<Camera>();
    auto camera = camobj->AddComponent<ComponentCamera>();
    camera->SetPositionAndTarget({0, 0, -1}, {0, 0, 0});

    auto col = camobj->AddComponent<ComponentCollisionSphere>();
    col->SetRadius(2.0f);
    col->SetMass(0.0f);

    auto spring_arm = camobj->AddComponent<ComponentSpringArm>();

    spring_arm->SetSpringArmObject(obj);

    spring_arm->SetSpringArmVector({0, 30, 30});
    spring_arm->SetSpringArmOffset({0, 10, -5});
    spring_arm->SetSpringArmLength(30.0f);

    spring_arm->SetSpringArmRotate({0, 90, 0});

    return camobj;
}

void Camera::Update([[maybe_unused]] float delta)
{
    auto player = Scene::GetObjectPtr<Player>("Player");

    // マウス右クリックしていないならカメラ移動
    if(player->GetIsPushMouseR() == false) {
        // カメラをエイム状態に
        SetCameraNormal();
    }
    else {
        //! カメラを通常状態に
        SetCameraAim();
    }
}

// カメラをエイム状態に
void Camera::SetCameraAim()
{
    auto spring_arm = GetComponent<ComponentSpringArm>();
    auto player     = Scene::GetObjectPtr<Player>("Player");

    // プレイヤーにカメラを近づける
    spring_arm->SetSpringArmLength(20.0f);
    spring_arm->SetSpringArmOffset({10, 15, -5});

    auto mdl = player->GetComponent<ComponentModel>();
    mdl->SetRotationAxisXYZ(
        {player->GetRotationAxisXYZ().x, GetRotationAxisXYZ().y + 180, player->GetRotationAxisXYZ().z});

    rot_.y += GetMouseMoveX() * 0.4f;
    rot_.x = -45.0f;

    spring_arm->SetSpringArmRotate(rot_);   // 回転
}

//! カメラを通常状態に
void Camera::SetCameraNormal()
{
    auto spring_arm = GetComponent<ComponentSpringArm>();
    auto player     = Scene::GetObjectPtr<Player>("Player");

    rot_.x += GetMouseMoveY() * 0.4f;
    rot_.y += GetMouseMoveX() * 0.4f;

    // 上下の回転角度制限
    rot_.x = std::clamp(float(rot_.x), -60.0f, +0.0f);

    spring_arm->SetSpringArmLength(30.0f);
    spring_arm->SetSpringArmOffset({0, 10, -5});

    spring_arm->SetSpringArmVector(vec_);   // カメラの距離
    spring_arm->SetSpringArmRotate(rot_);   // 回転
}
