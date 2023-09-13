//---------------------------------------------------------------------------
//! @file   Player.cpp
//! @brief  プレイヤー
//---------------------------------------------------------------------------
#include <Game/Objects/Player.h>
#include <Game/Objects/Bullet.h>
#include <Game/Objects/Camera.h>
#include <Game/Objects/PlayerKick.h>
#include <Game/Objects/Gun.h>
#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentTargetTracking.h>
#include <System/Component/ComponentAttachModel.h>

// コンストラクタ
Player::Player()
{
}

// デストラクタ
Player::~Player()
{
}

// 初期化
bool Player::Init()
{
    __super::Init();

    // モデルコンポーネント(0.08倍)
    auto model = AddComponent<ComponentModel>("data/Game/Player/model.mv1")
                     ->SetScaleAxisXYZ({
                         0.08f
    })
                     ->SetAnimation({
                         // アニメーション
                         {"idle", "data/Game/Player/Anim/Idle.mv1", 0, 2.0f},
                         {"run", "data/Game/Player/Anim/Run Forward.mv1", 0, 1.0f},
                         {"shot", "data/Game/Player/Anim/Gunplay.mv1", 0, 0.5f},
                         {"reload", "data/Game/Player/Anim/Reload.mv1", 0, 1.5f},
                         {"reload_move", "data/Game/Player/Anim/Reload_move.mv1", 0, 1.0f},
                         {"death", "data/Game/Player/Anim/Death.mv1", 0, 0.5f},
                         {"kick", "data/Game/Player/Anim/Kick.mv1", 0, 2.0f},
                     });

    // コリジョン(カプセル)
    auto col = AddComponent<ComponentCollisionCapsule>();
    col->SetTranslate({0, 0, 0});
    col->SetRadius(4);
    col->SetHeight(15);
    col->SetCollisionGroup(ComponentCollision::CollisionGroup::PLAYER);
    col->UseGravity();

    skill_timer_ = SKILL_TIMER_MAX_;
    hp_          = HP_MAX_;

    // チュートリアルなら
    if(is_tutorial_ == true) {
        guns_mag_num_ = 30;

        have_bullet_num_ = 10000000;
    }
    else {
        guns_mag_num_    = 30;
        have_bullet_num_ = 60;
    }

    shot_se_         = LoadSoundMem("data/Game/SE/Player/shot.mp3");
    no_ammo_se_      = LoadSoundMem("data/Game/SE/Player/noammo.mp3");
    death_se_        = LoadSoundMem("data/Game/SE/Player/death.mp3");
    skill_activ_se_  = LoadSoundMem("data/Game/SE/Player/skillactiv.mp3");
    skill_release_se = LoadSoundMem("data/Game/SE/Player/skillrelease.mp3");
    skill_empty_se_  = LoadSoundMem("data/Game/SE/Player/skillenpty.mp3");
    skill_notuse_se_ = LoadSoundMem("data/Game/SE/Player/notuseskil.mp3");

    gun_image_ = LoadGraph("data/Game/Image/gun.png");

    font_handle_ = LoadFontDataToHandle("data/Game/Font/InfoFont.dft");

    AddComponent<ComponentAttachModel>();

    //==========================================================
    // 定数バッファを作成
    //==========================================================
    cb_fade_info_ = CreateShaderConstantBuffer(sizeof(FadeInfo));

    return true;
}

// 更新処理
void Player::Update([[maybe_unused]] float delta)
{
    if(is_clear_ == true)
        return;

    // 定数バッファワークメモリをGPU側へ転送
    UpdateShaderConstantBuffer(cb_fade_info_);

    damage_flag_ = false;

    auto mdl = GetComponent<ComponentModel>();

    auto bullet  = Scene::GetObjectPtr<Bullet>("Bullet");
    auto bullets = Scene::GetObjectsPtr<Bullet>();

    float3 move = (0.0f, 0.0f, 0.0f);

    // カメラの向いている方向に移動
    auto mat = GetMatrix();
    MoveCameraFront(mat);
    // WASDで移動
    MoveWASD(mat, move);

    // 白黒、色反転の半径を設定
    UseGrayNegaR();

    // マウスの右ボタンを押したら
    PushMouseRight();

    // 体力が0になったとき
    if(hp_ < 1) {
        is_dead_ = true;
        hp_      = 0;
    }

    // 死んでいたら
    if(is_dead_ == true) {
        Dead();
        return;
    }

    // キックしていなかったら
    if(is_kicking_ == false) {
        // 移動処理
        Move(move, delta);

        // プレイヤーのキック
        Kick();

        // Shot
        Shot();

        // リロード
        Reload(move);
    }

    // スキル使用可能かどうか　できるなら
    Skill();

    // 定数バッファを設定
    SetShaderConstantBuffer(cb_fade_info_, DX_SHADERTYPE_PIXEL, 5);
}

// 基本描画の後に処理します
void Player::PostDraw()
{
    // クリアか死亡で何も表示しない
    bool show_ui = is_clear_ == true || is_dead_ == true;
    if(show_ui) {
        return;
    }

    // スキルゲージ
    DrawSkillGauge();

    // HPゲージ
    DrawHPGauge();

    // 残弾
    DrawBulletInfo();

    // ダメージ中
    if(damage_flag_ == true) {
        // 画面を少し赤く
        int alpha = 50;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawFillBox(0, 0, WINDOW_W, WINDOW_H, RED);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
    }

    // リロード中
    if(is_reloading_ == true) {
        constexpr int STR_POS_X = 100;
        DrawFillBox(0, WINDOW_H - 80, 360, WINDOW_H - 60, GetColor(75, 75, 75));
        DrawStringToHandle(STR_POS_X, WINDOW_H - 80, "リロード中", WHITE, font_handle_);
    }
}

void Player::GUI()
{
    __super::GUI();

    // GUI描画
    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();

        ImGui::DragFloat(u8"速度", &sp_, 0.01f, 0.01f, 10.0f, "%2.2f");
        ImGui::DragFloat(u8"HitPoint", &hp_, 1, 0, 100, "%2.2f");
        ImGui::DragFloat(u8"skilltimer", &skill_timer_, 1, 0, 100);
        ImGui::DragInt(u8"skillcooltimer", &skill_cooltime_, 1, 0, 100);
        ImGui::DragFloat(u8"HP", &hp_, 1, 0, 100, "%d");
    }
    ImGui::End();
}

// 接触処理
void Player::OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)
{
    auto owner = hitInfo.hit_collision_->GetOwnerPtr();

    // Enemyと当たったら
    if(owner->GetNameDefault() == "Enemy") {
        bool normal_alive = is_use_skill_ == false && is_dead_ == false;
        if(normal_alive) {
            constexpr float DAMAGE = 1.0f;
            Damage(DAMAGE);
        }
    }

    // 当たりで移動させる
    __super::OnHit(hitInfo);
}

void Player::Exit()
{
    // 定数バッファを解放
    DeleteShaderConstantBuffer(cb_fade_info_);

    DeleteFontToHandle(font_handle_);
}

// マウスの右ボタンを押している間is_push_mouse_right_をtrueに 押していないと falseに
void Player::PushMouseRight()
{
    if(IsMouseRepeat(MOUSE_INPUT_RIGHT)) {
        is_push_mouse_right_ = true;
    }
    else {
        is_push_mouse_right_ = false;
    }
}

// 弾数を追加
void Player::GetAmmo()
{
    // 追加する弾数
    constexpr int PLUS_NUM = 3;
    have_bullet_num_ += PLUS_NUM;
}

// 銃のリロード
void Player::MagReload()
{
    // 銃のマガジン容量
    constexpr int GUNS_MAG_MAX = 30;

    // マガジン容量がMAXならもしくは予備弾を一発も持っていなかったら何も行わない
    if(GUNS_MAG_MAX == guns_mag_num_ || have_bullet_num_ <= 0) {
        return;
    }
    // 持っている弾薬が30発以上なら
    else if(GUNS_MAG_MAX <= have_bullet_num_) {
        // 弾薬減少(マガジン内の弾薬は破棄されない)
        have_bullet_num_ += guns_mag_num_;
        have_bullet_num_ -= GUNS_MAG_MAX;
        // マガジン容量をリロード
        guns_mag_num_ = GUNS_MAG_MAX;
    }
    // 持っている弾薬が30発以下なら
    else if(have_bullet_num_ < GUNS_MAG_MAX) {
        // 今持っている弾薬をリロード
        guns_mag_num_ += have_bullet_num_;
        // 弾薬減少(手持ちのすべての弾をリロード所持数0に)
        have_bullet_num_ -= have_bullet_num_;
        // マガジン内が30以上になったら
        if(GUNS_MAG_MAX < guns_mag_num_) {
            // あふれた弾を手持ちに追加
            have_bullet_num_ += guns_mag_num_ % GUNS_MAG_MAX;
            // マガジンを30発に
            guns_mag_num_ = GUNS_MAG_MAX;
        }
    }
}

// ダメージ
void Player::Damage(float damage)
{
    // プレイヤーの体力を減らす
    hp_ -= damage;

    damage_flag_ = true;
}

// is_reloading_の取得
bool Player::GetIsReloading() const
{
    return is_reloading_;
}

// is_kicking_の取得
bool Player::GetIsKicking() const
{
    return is_kicking_;
}

// is_kicking_をセット
void Player::SetIsKicking(bool flag)
{
    is_kicking_ = flag;
}

// is_use_skill_をセット
void Player::SetIsUseSkill(bool flag)
{
    is_use_skill_ = flag;
}

// is_clearをセット
void Player::SetIsClear(bool flag)
{
    is_clear_ = flag;
}

// is_push_mouse_right_の取得
bool Player::GetIsPushMouseR() const
{
    return is_push_mouse_right_;
}

// is_clearを取得
bool Player::GetIsClear() const
{
    return is_clear_;
}

// HPゲージの描画
void Player::DrawHPGauge() const
{
    if(is_tutorial_ == true)
        return;
    int color = 0;
    if(hp_ <= 600 && 300 < hp_) {
        color = GREEN;
    }
    else if(hp_ <= 300 && 150 < hp_) {
        color = YELLOW;
    }
    else if(hp_ <= 150) {
        color = RED;
    }

    constexpr int HP_BOX_POS_X = 950;
    constexpr int HP_BOX_POS_Y = 670;
    constexpr int SPACE        = 1;

    // 下地
    DrawFillBox(HP_BOX_POS_X - 80,
                HP_BOX_POS_Y,
                HP_BOX_POS_X + (int)HP_MAX_ / 2 + SPACE,
                HP_BOX_POS_Y + 30,
                GetColor(75, 75, 75));
    // 内
    DrawFillBox(HP_BOX_POS_X, HP_BOX_POS_Y, HP_BOX_POS_X + (int)hp_ / 2, HP_BOX_POS_Y + 30, color);
    // 外枠
    DrawBox(HP_BOX_POS_X - SPACE,
            HP_BOX_POS_Y - SPACE,
            HP_BOX_POS_X + (int)HP_MAX_ / 2 + SPACE,
            HP_BOX_POS_Y + 30 + SPACE,
            WHITE,
            false);

    DrawStringToHandle(HP_BOX_POS_X - 50, HP_BOX_POS_Y, "HP", WHITE, font_handle_);
}

// スキルゲージの描画
void Player::DrawSkillGauge() const
{
    constexpr int SKILL_BOX_POS_X = 950;
    constexpr int SKILL_BOX_POS_Y = 630;
    constexpr int SPACE           = 1;

    // 下地
    DrawFillBox(SKILL_BOX_POS_X - 80,
                SKILL_BOX_POS_Y,
                SKILL_BOX_POS_X + (int)SKILL_TIMER_MAX_,
                SKILL_BOX_POS_Y + 30,
                GetColor(75, 75, 75));
    // 内
    DrawFillBox(SKILL_BOX_POS_X, SKILL_BOX_POS_Y, SKILL_BOX_POS_X + (int)skill_timer_, SKILL_BOX_POS_Y + 30, BLUE);
    if(1 < skill_cooltime_) {
        DrawFillBox(SKILL_BOX_POS_X, SKILL_BOX_POS_Y, SKILL_BOX_POS_X + (int)skill_timer_, SKILL_BOX_POS_Y + 30, RED);
        DrawStringToHandle(SKILL_BOX_POS_X + 15, SKILL_BOX_POS_Y, "スキル使用不可", WHITE, font_handle_);
    }
    // 外枠
    DrawBox(SKILL_BOX_POS_X - SPACE,
            SKILL_BOX_POS_Y - SPACE,
            SKILL_BOX_POS_X + (int)SKILL_TIMER_MAX_ + SPACE,
            SKILL_BOX_POS_Y + 30 + SPACE,
            WHITE,
            false);

    DrawStringToHandle(SKILL_BOX_POS_X - 80, SKILL_BOX_POS_Y, "スキル", WHITE, font_handle_);
}

// 所持、マガジン弾数表示
void Player::DrawBulletInfo() const
{
    DrawFillBox(0, WINDOW_H - 60, 360, WINDOW_H, GetColor(75, 75, 75));
    DrawExtendGraph(0, WINDOW_H - 80, 140, WINDOW_H + 30, gun_image_, TRUE);

    SetFontSize(50);
    // チュートリアルなら弾の表示を無限に
    constexpr s32 STR_POS_X = 145;
    if(is_tutorial_ == false) {
        DrawFormatString(STR_POS_X, WINDOW_H - 50, BLACK, "%2d / %3d", guns_mag_num_, have_bullet_num_);
    }
    else {
        DrawFormatString(STR_POS_X, WINDOW_H - 50, BLACK, "%2d / ∞", guns_mag_num_);
    }
}

// カメラの向いている方向に移動
void Player::MoveCameraFront(matrix& mat)
{
    if(auto cam = Scene::GetObjectPtr<Camera>("PlayerCamera")) {
        float3 vec = cam->GetWorldMatrix().axisZ();
        vec.y      = 0;
        vec        = normalize(vec);
        mat        = HelperLib::Math::CreateMatrixByFrontVector(-vec);
    }
}

// WASDで移動
void Player::MoveWASD(matrix& mat, float3& move)
{
    if(IsKeyRepeat(KEY_INPUT_W)) {
        float3 vec = mat.axisZ();
        move += -vec;
    }
    if(IsKeyRepeat(KEY_INPUT_D)) {
        float3 vec = mat.axisX();
        move += -vec;
    }
    if(IsKeyRepeat(KEY_INPUT_S)) {
        float3 vec = mat.axisZ();
        move += vec;
    }
    if(IsKeyRepeat(KEY_INPUT_A)) {
        float3 vec = mat.axisX();
        move += vec;
    }
}

// 白黒、色反転の半径を設定
void Player::UseGrayNegaR()
{
    // 定数バッファを更新
    // 更新に必要なワークメモリの場所を取得
    void* p = GetBufferShaderConstantBuffer(cb_fade_info_);
    {
        auto* info = reinterpret_cast<FadeInfo*>(p);

        // プレイヤーを中心に設定
        info->center_position_ = float4(GetTranslate(), 1.0f);
        // スキル発動中なら
        if(IsUseSkill() == true) {
            // 色反転をする
            info->nega_radius += 5.0f;
            // 一定値まで増えたら
            if(150.0f <= info->nega_radius) {
                // グレーに
                info->gray_radius += 5.0f;
            }

            // 最大半径
            constexpr float MAX_RADIUS = 1000.0f;
            // 到達するとストップ
            if(MAX_RADIUS <= info->nega_radius) {
                info->nega_radius = MAX_RADIUS;
            }
            if(MAX_RADIUS <= info->gray_radius) {
                info->gray_radius = MAX_RADIUS;
            }
        }
        else {
            info->gray_radius -= 20.0f;

            // 一定値まで減ったら
            if(info->gray_radius <= 800.0f) {
                // 色反転を小さく
                info->nega_radius -= 20.0f;
            }

            if(info->nega_radius <= 0) {
                info->nega_radius = 0;
            }
            if(info->gray_radius <= 0) {
                info->gray_radius = 0;
            }
        }
    }
}

//モデルを進行方向に向ける
void Player::RotatoMoveDir(float3& move)
{
    move        = normalize(move);
    float x     = -move.x;
    float z     = -move.z;
    float theta = atan2(x, z) * RadToDeg;
    auto  mdl   = GetComponent<ComponentModel>();
    mdl->SetRotationAxisXYZ({0, theta, 0});
}

// ボリュームを設定しSEを再生
void Player::PlaySE(int se_handle, int volume) const
{
    ChangeVolumeSoundMem(volume, se_handle);
    PlaySoundMem(se_handle, DX_PLAYTYPE_BACK);
}

// リロード 弾を装填、アニメーションを変更
void Player::Reload(float3& move)
{
    auto mdl = GetComponent<ComponentModel>();

    if(is_reloading_ == true) {
        // リロードフラグがtrue中にアニメーションが終わるとフラグをfalseに
        if(mdl->IsPlaying() == false) {
            MagReload();
            is_reloading_ = false;
            return;
        }

        // 動くとリロードキャンセル
        if(dot(move, move).x > 0) {
            is_reloading_ = false;
        }
        return;
    }

    // マガジン内が30発以下で持っている弾が1発でもあるなら
    bool can_reload = guns_mag_num_ < 30 && have_bullet_num_ != 0;
    if(can_reload) {
        if(IsKeyOn(KEY_INPUT_R)) {
            // リロード中のフラグをtrueに
            is_reloading_ = true;

            if(mdl->GetPlayAnimationName() != "reload") {
                mdl->PlayAnimation("reload");
            }
        }
    }
}

// ショット
void Player::Shot()
{
    // リロード中なら何もしない
    if(is_reloading_ == true)
        return;

    // 弾切れ時
    if(guns_mag_num_ <= 0) {
        if(IsMouseOn(MOUSE_INPUT_LEFT)) {
            PlaySE(no_ammo_se_);
        }
        return;
    }

    // 弾作成
    if(IsMouseOn(MOUSE_INPUT_LEFT))
        bullet_timer_ = 0;
    if(IsMouseRepeat(MOUSE_INPUT_LEFT)) {
        if(bullet_timer_ % 15 == 0 || bullet_timer_ == 0) {
            guns_mag_num_--;
            auto mdl = GetComponent<ComponentModel>();
            auto vec = mul(-float4(mdl->GetMatrix().axisZ(), 0), GetMatrix());

            Scene::CreateObject<Bullet>()->SetPositionAndDirection(GetTranslate() + float3(0, 11.0f, 0), vec.xyz);

            PlaySE(shot_se_, 150);
        }
        bullet_timer_++;
    }
}

// 移動処理
void Player::Move(float3& move, float delta)
{
    auto mdl = GetComponent<ComponentModel>();

    if(dot(move, move).x <= 0) {
        if(is_reloading_ == false) {
            // 止まってる
            if(mdl->GetPlayAnimationName() != "idle") {
                mdl->PlayAnimation("idle", true);
            }
        }
        return;
    }

    if(GetIsPushMouseR() == false) {
        // モデルを進行方向に向ける
        RotatoMoveDir(move);
    }
    if(is_reloading_ == false) {
        // 移動アニメーション
        if(mdl->GetPlayAnimationName() != "run") {
            mdl->PlayAnimation("run", true);
        }
    }

    move *= sp_ * (delta * 60.0f);
    // 地面移動スピードを決定する
    AddTranslate(move);
}

// キック
void Player::Kick()
{
    // マウスホイールクリック
    if(IsMouseOn(3)) {
        if(is_reloading_ == false) {
            Scene::CreateObject<PlayerKick>();
        }
    }
}

// スキル
void Player::Skill()
{
    // スキルクールタイム中は使用不可
    if(0 < skill_cooltime_) {
        // クールタイム中にスペースを押したら
        if(IsKeyOn(KEY_INPUT_SPACE)) {
            PlaySE(skill_notuse_se_, 150);
        }

        // スキルクールタイム減少
        skill_cooltime_ -= 1;
        return;
    }

    // スキル使用中なら
    if(is_use_skill_ == true) {
        skill_timer_ -= 0.4f;
        // 0秒以下になるとスキル解除
        if(skill_timer_ <= 0) {
            is_use_skill_   = false;
            skill_cooltime_ = SKILL_COOLTIME_MAX_;

            PlaySE(skill_empty_se_, 150);
        }

        // 時止め使用中に解除
        if(IsKeyOn(KEY_INPUT_SPACE)) {
            is_use_skill_ = false;
            // スキルクールタイム
            skill_cooltime_ = SKILL_COOLTIME_MAX_;

            PlaySE(skill_release_se, 150);
        }
        return;
    }

    // スキル発動
    if(IsKeyOn(KEY_INPUT_SPACE)) {
        PlaySE(skill_activ_se_, 150);

        is_use_skill_ = true;
    }

    // スキルゲージ回復
    skill_timer_ += 0.3f;

    // スキルタイマー上限
    if(SKILL_TIMER_MAX_ < skill_timer_) {
        skill_timer_ = SKILL_TIMER_MAX_;
    }
}

// 死亡
void Player::Dead()
{
    auto mdl = GetComponent<ComponentModel>();
    if(mdl->IsPlaying() == false) {
        mdl->PlayPause();
        return;
    }

    if(mdl->GetPlayAnimationName() != "death") {
        mdl->PlayAnimation("death");

        PlaySE(death_se_, 100);
    }
}
