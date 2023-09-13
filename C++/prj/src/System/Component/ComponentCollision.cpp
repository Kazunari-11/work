//---------------------------------------------------------------------------
//! @file   ComponentCollision.h
//! @brief  コリジョンコンポーネント(ベースクラス)
//---------------------------------------------------------------------------
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentTransform.h>

#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionLine.h>
#include <System/Component/ComponentModel.h>

#include <System/Utils/HelperLib.h>

#include <System/Object.h>

namespace
{
float3 merge(float3 v1, float3 v2)
{
    float3 vh{v2.x, v2.y, v2.z};

    if(fabs(v1.x) > fabs(v2.x))
        vh.x = v1.x;

    if(fabs(v1.y) > fabs(v2.y))
        vh.y = v1.y;

    if(fabs(v1.z) > fabs(v2.z))
        vh.z = v1.z;

    return vh;
}
}   // namespace

ComponentCollision::ComponentCollision()
{
    // 複数設定可能とする
    SetStatus(Component::StatusBit::SameType, true);
}

void ComponentCollision::Construct(ObjectPtr owner)
{
    // 識別子設定
    assert(owner);
    owner_    = owner;
    auto cmps = owner->GetComponents<ComponentCollision>();
    int  max  = -1;
    for(ComponentCollisionPtr cmp : cmps) {
        if((int)cmp->collision_id_ > max)
            max = (int)cmp->collision_id_;
    }
    collision_id_ = (u32)(max + 1);
}

//! @brief 当たった情報はコールバックで送られてくる
//! @param hitInfo 当たった情報
//! @details 当たった回数分ここに来ます
void ComponentCollision::OnHit(const HitInfo& hitInfo)
{
    // オーバーラップする場合は当たりをすり抜ける
    if(is_overlap_)
        return;

    auto obj = GetOwner();

    // Staticな物質にぶつかった場合、gravity_を下げる
    if(hitInfo.hit_collision_->GetMass() < 0) {
        auto   vec = obj->GetTranslate() - obj->GetOldWorldMatrix().translate();
        float3 nvc = {0.0f, -1.0f, 0.0f};
        if(length(vec).x <= 0 || length(now_gravity_).x <= 0) {
            now_gravity_ = 0.0f;
        }
        else {
            nvc     = normalize(vec);
            float d = dot(normalize(now_gravity_), nvc);
            now_gravity_ *= (1 - d * d * 0.1f);
        }
    }

    obj->OnHit(hitInfo);
}

#if 0
void ComponentCollision::SetName(std::string_view name)
{
    name_ = name;
}
#endif

std::string_view ComponentCollision::GetName()
{
    return name_;
}

void ComponentCollision::GUICollisionData(bool use_attach)
{
    auto str = u8"コリジョンタイプ : " + CollisionTypeName[(u32)collision_type_];
    ImGui::Text(str.c_str());
    ImGui::Separator();
    // @todo ListBox化する
    str = u8"コリジョングループ : " + GetCollisionGroupName();
    ImGui::Text(str.c_str());
    ImGui::Separator();
    //  ImGui::CheckboxFlags(u8"初期化済み", &collision_status_.get(), 1 << (u32)CollisionBit::Initialized);
    ImGui::CheckboxFlags(u8"ヒットしない", &collision_status_.get(), 1 << (u32)CollisionBit::DisableHit);
    ImGui::CheckboxFlags(u8"ゲーム中表示", &collision_status_.get(), 1 << (u32)CollisionBit::ShowInGame);
    ImGui::Separator();
    ImGui::Text(u8"Hitするグループ");
    ImGui::CheckboxFlags("WALL", (u32*)&collision_hit_, (u32)CollisionGroup::WALL);
    ImGui::CheckboxFlags("GROUND", (u32*)&collision_hit_, (u32)CollisionGroup::GROUND);
    ImGui::CheckboxFlags("PLAYER", (u32*)&collision_hit_, (u32)CollisionGroup::PLAYER);
    ImGui::CheckboxFlags("ENEMY", (u32*)&collision_hit_, (u32)CollisionGroup::ENEMY);
    ImGui::CheckboxFlags("WEAPON", (u32*)&collision_hit_, (u32)CollisionGroup::WEAPON);
    ImGui::CheckboxFlags("ITEM", (u32*)&collision_hit_, (u32)CollisionGroup::ITEM);
    ImGui::CheckboxFlags("CAMERA", (u32*)&collision_hit_, (u32)CollisionGroup::CAMERA);
    ImGui::CheckboxFlags("ETC", (u32*)&collision_hit_, (u32)CollisionGroup::ETC);
    ImGui::Checkbox(u8"オーバーラップする", &is_overlap_);
    ImGui::Separator();
    ImGui::Checkbox(u8"重力を使用する", &use_gravity_);
    ImGui::DragFloat3(u8"重力加速度", (float*)&gravity_);

    // アタッチが存在しないときはここで終了
    if(!use_attach)
        return;

    if(auto cmp = GetOwner()->GetComponent<ComponentModel>()) {
        bool attach = false;
        if(attach_node_ >= 0) {
            attach = true;
            if(ImGui::Checkbox("AttachNode", &attach)) {
                if(!attach)
                    attach_node_ = -1;
            }
        }
        else {
            attach = false;
            if(ImGui::Checkbox("AttachNode", &attach)) {
                if(attach)
                    attach_node_ = 0;
            }
        }

        auto items = cmp->GetNodesNamePChar();
        if(ImGui::Combo("Node", &attach_node_, items.data(), (int)items.size())) {
            // 切り替えたとき
            collision_transform_ = matrix::identity();
        }
    }
}

void ComponentCollision::Update(float delta)
{
    __super::Update(delta);
    if(attach_node_ >= 0) {
        attach_node_matrix_ = matrix::identity();
        if(auto mdl = GetOwner()->GetComponent<ComponentModel>()) {
            attach_node_matrix_ = MV1GetFrameLocalWorldMatrix(mdl->GetModel(), attach_node_);
        }
    }
#ifdef USE_JOLT_PHYSICS
#else
    // 重力加速度
    if(use_gravity_) {
        GetOwner()->SetGravity(now_gravity_);
        now_gravity_ += gravity_ * delta;
    }
#endif
}

void ComponentCollision::PostUpdate()
{
    //! 古いマトリクスを更新します
    old_transform_ = collision_transform_;
}

//! @brief GUI処理
void ComponentCollision::GUI()
{
}

void ComponentCollision::AttachToModel(int node)
{
    if(auto mdl = GetOwner()->GetComponent<ComponentModel>()) {
        attach_node_ = node;
#ifdef USE_JOLT_PHYSICS
        if(GetRigidBody())
            GetRigidBody()->setGravityFactor(0.0f);
#endif USE_JOLT_PHYSICS
    }
}

//! @brief Capsule VS Sphere
//! @param col1 Capsuleコリジョン
//! @param col2 Sphere コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionCapsulePtr col1,
                                                      ComponentCollisionSpherePtr  col2)
{
    ComponentCollision::HitInfo info{};

    // 自分のコリジョン
    float3 cpos1 = col1->GetTranslate();
    float3 cpos2 = col1->GetVectorAxisY() * col1->GetHeight() + cpos1;
    float  cs    = 1.0f;   //< スケール

    // モデルアタッチ
    if(col1->attach_node_ >= 0) {
        if(auto mdl = col1->GetOwner()->GetComponent<ComponentModel>()) {
            cpos1 = mul(float4(cpos1, 1), col1->attach_node_matrix_).xyz;
            cpos2 = mul(float4(cpos2, 1), col1->attach_node_matrix_).xyz;
            cpos2 = normalize(cpos2 - cpos1) * col1->GetHeight() + cpos1;
        }
    }
    else {
        // ComponentTransform(オブジェクト姿勢)
        if(auto cmp = col1->GetOwner()->GetComponent<ComponentTransform>()) {
            // 高さに回転とスケールを掛け合わせる
            cpos1 = mul(float4(cpos1, 1), cmp->GetMatrix()).xyz;
            cpos2 = mul(float4(cpos2, 1), cmp->GetMatrix()).xyz;
            // 半径はXZで平均としておく
            cs = (length(cmp->GetMatrix().axisX()) + length(cmp->GetMatrix().axisZ())) / 2;
        }
    }

    float3 epos1;
    float  es = 1.0f;

    // モデルアタッチ
    if(col2->attach_node_ >= 0) {
        if(auto mdl = col2->GetOwner()->GetComponent<ComponentModel>()) {
            epos1 = col2->GetTranslate();
            epos1 = mul(float4(epos1, 1), col2->attach_node_matrix_).xyz;
        }
    }
    else {
        if(auto cmp = col2->GetOwner()->GetComponent<ComponentTransform>()) {
            epos1 = mul(col2->GetMatrix(), cmp->GetMatrix())._41_42_43;
            //pos1 = mul( float4( pos1, 0 ) , cmp->GetMatrix() ).xyz;
            //pos1 += cmp->GetTranslate().xyz;
            float sx = length(cmp->GetVectorAxisX());
            float sy = length(cmp->GetVectorAxisY());
            float sz = length(cmp->GetVectorAxisZ());
            es       = (sx + sy + sz) / 3.0f;
        }
    }

    float  cr = col1->GetRadius() * cs;
    float3 cv = normalize(cpos1 - cpos2);
    VECTOR c1 = cast(cpos1 - cv * cr);
    VECTOR c2 = cast(cpos2 + cv * cr);

    float  er = col2->GetRadius() * es;
    VECTOR e1 = cast(epos1);

    // 跳ね返り点が欲しいため、HitCheck_Capsule_Capsuleは使わない
    SEGMENT_POINT_RESULT result;
    Segment_Point_Analyse(&c1, &c2, &e1, &result);

    if(result.Seg_Point_MinDist_Square < (cr + er) * (cr + er)) {
        // 線と線で一番近くなる点を求め、ベクトル化する
        // 最も近い点
        float3 c0  = cast(result.Seg_MinDist_Pos);
        float3 e0  = cast(e1);
        float3 vec = e0 - c0;   // 調べたほうの跳ね返りの方向(100%)
        float  len = length(vec);
        if(abs(len) <= abs(len) * FLT_EPSILON) {
            // 全く同じ位置にいる場合はz移動する形にしておく
            vec = {0, 0, 1};
        }

        float3 vs = normalize(vec) * (cr + er);
        vec -= vs;

        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_         = vec;
        info.hit_          = true;
        info.hit_position_ = (e0 + c0) * 0.5f;
    }

    return info;
}

//! @brief Sphere VS Capsule
//! @param col1 Sphereコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionSpherePtr  col1,
                                                      ComponentCollisionCapsulePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;   // push方向を反対にする
    return hit;
}

//! @brief Capsule VS Capsule
//! @param col1 Capsuleコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionCapsulePtr col1,
                                                      ComponentCollisionCapsulePtr col2)
{
    ComponentCollision::HitInfo info{};

    // 自分のコリジョン
    float3 cpos1 = col1->GetTranslate();
    float3 cpos2 = normalize(col1->GetVectorAxisY()) * col1->GetHeight() + cpos1;
    float  cs    = 1.0f;   //< スケール

    // モデルアタッチ
    if(col1->attach_node_ >= 0) {
        if(auto mdl = col1->GetOwner()->GetComponent<ComponentModel>()) {
            cpos1 = mul(float4(cpos1, 1), col1->attach_node_matrix_).xyz;
            cpos2 = mul(float4(cpos2, 1), col1->attach_node_matrix_).xyz;
            cpos2 = normalize(cpos2 - cpos1) * col1->GetHeight() + cpos1;
        }
    }
    else {
        // ComponentTransform(オブジェクト姿勢)
        if(auto cmp = col1->GetOwner()->GetComponent<ComponentTransform>()) {
            auto& mtx = cmp->GetMatrix();
            // 高さに回転とスケールを掛け合わせる
            cpos1 = mul(float4(cpos1, 1), mtx).xyz;
            cpos2 = mul(float4(cpos2, 1), mtx).xyz;
            // 半径はXZで平均としておく
            cs = (length(mtx.axisX()) + length(mtx.axisZ())) / 2;
        }
    }

    // 相手のコリジョン
    float3 epos1 = col2->GetTranslate();
    float3 epos2 = normalize(col2->GetVectorAxisY()) * col2->GetHeight() + epos1;
    float  es    = 1.0f;   //< スケール

    // モデルアタッチ
    if(col2->attach_node_ >= 0) {
        if(auto mdl = col2->GetOwner()->GetComponent<ComponentModel>()) {
            epos1 = mul(float4(epos1, 1), col2->attach_node_matrix_).xyz;
            epos2 = mul(float4(epos2, 1), col2->attach_node_matrix_).xyz;
            epos2 = normalize(epos2 - epos1) * col2->GetHeight() + epos1;
        }
    }
    else {
        // ComponentTransform(オブジェクト姿勢)
        if(auto cmp = col2->GetOwner()->GetComponent<ComponentTransform>()) {
            auto& mtx = cmp->GetMatrix();
            // 高さに回転とスケールを掛け合わせる
            epos1 = mul(float4(epos1, 1), mtx).xyz;
            epos2 = mul(float4(epos2, 1), mtx).xyz;
            // 半径はXZで平均としておく
            es = (length(mtx.axisX()) + length(mtx.axisZ())) / 2;
        }
    }

    float  cr = col1->GetRadius() * cs;
    float3 cv = normalize(cpos1 - cpos2);
    VECTOR c1 = cast(cpos1 - (cv * cr));
    VECTOR c2 = cast(cpos2 + (cv * cr));
    float  er = col2->GetRadius() * es;
    float3 ev = normalize(epos1 - epos2);
    VECTOR e1 = cast(epos1 - (ev * er));
    VECTOR e2 = cast(epos2 + (ev * er));

    // 跳ね返り点が欲しいため、HitCheck_Capsule_Capsuleは使わない
    SEGMENT_SEGMENT_RESULT result;
    Segment_Segment_Analyse(&c1, &c2, &e1, &e2, &result);

#if 0   // 当たり不具合チェック用
	if ( col1->attach_node_ )
		DrawCapsule3D( c1, c2, cr, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );

	if ( col2->attach_node_ )
		DrawCapsule3D( e1, e2, er, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );
#endif

    if(result.SegA_SegB_MinDist_Square < (cr + er) * (cr + er)) {
        // 線と線で一番近くなる点を求め、ベクトル化する
        // 最も近い点
        float3 c0  = cast(result.SegA_MinDist_Pos);
        float3 e0  = cast(result.SegB_MinDist_Pos);
        float3 vec = e0 - c0;   // 調べたほうの跳ね返りの方向(100%)

        float len = length(vec);
        if(abs(len) <= abs(len) * FLT_EPSILON) {
            // 全く同じ位置にいる場合はz移動する形にしておく
            vec = {0, 0, 1};
        }
        float3 vs = normalize(vec) * (cr + er);
        vec -= vs;

        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_         = vec;
        info.hit_          = true;
        info.hit_position_ = (e0 + c0) * 0.5f;
    }

    return info;
}

//! @brief Sphere VS Sphere
//! @param col1 Sphereコリジョン
//! @param col2 Sphere コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionSpherePtr col1,
                                                      ComponentCollisionSpherePtr col2)
{
    ComponentCollision::HitInfo info{};

    float3 pos1;
    float  scale1 = 1.0f;

    // モデルアタッチ
    if(col1->attach_node_ >= 0) {
        if(auto mdl = col1->GetOwner()->GetComponent<ComponentModel>()) {
            pos1 = col1->GetTranslate();
            pos1 = mul(float4(pos1, 1), col1->attach_node_matrix_).xyz;
        }
    }
    else {
        if(auto cmp = col1->GetOwner()->GetComponent<ComponentTransform>()) {
            pos1 = mul(col1->GetMatrix(), cmp->GetMatrix())._41_42_43;
            //pos1 = mul( float4( pos1, 0 ) , cmp->GetMatrix() ).xyz;
            //pos1 += cmp->GetTranslate().xyz;
            float sx = length(cmp->GetVectorAxisX());
            float sy = length(cmp->GetVectorAxisY());
            float sz = length(cmp->GetVectorAxisZ());
            scale1   = (sx + sy + sz) / 3.0f;
        }
    }

    float3 pos2;
    float  scale2 = 1.0f;

    // モデルアタッチ
    if(col2->attach_node_ >= 0) {
        if(auto mdl = col2->GetOwner()->GetComponent<ComponentModel>()) {
            pos2 = col2->GetTranslate();
            pos2 = mul(float4(pos2, 1), col2->attach_node_matrix_).xyz;
        }
    }
    else {
        if(auto cmp = col2->GetOwner()->GetComponent<ComponentTransform>()) {
            pos2 = mul(col2->GetMatrix(), cmp->GetMatrix())._41_42_43;
            //pos2 = mul( float4( pos2, 0 ), cmp->GetMatrix() ).xyz;
            //pos2 += cmp->GetTranslate().xyz;
            float sx = length(cmp->GetVectorAxisX());
            float sy = length(cmp->GetVectorAxisY());
            float sz = length(cmp->GetVectorAxisZ());
            scale2   = (sx + sy + sz) / 3.0f;
        }
    }

    if(HitCheck_Sphere_Sphere(cast(pos1), col1->GetRadius() * scale1, cast(pos2), col2->GetRadius() * scale2)) {
        // 中間地点を当たった場所にする
        info.hit_          = true;
        info.hit_position_ = (pos1 - pos2) * 0.5f + pos2;

        // 押し出し方向
        float3 distance = pos1 - pos2;
        float  len      = length(distance);
        if(abs(len) <= abs(len) * FLT_EPSILON) {
            // 全く同じ位置にいる場合はz移動する形にしておく
            distance = {0, 0, 1};
        }

        float3 vec     = normalize(distance);
        float  reallen = col1->GetRadius() * scale1 + col2->GetRadius() * scale2;
        vec            = vec * (reallen - len);

        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_ = vec;
    }

    return info;
}

//! @brief Model VS Sphere
//! @param col1 Model コリジョン
//! @param col2 Sphereコリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionSpherePtr col1, ComponentCollisionModelPtr col2)
{
    ComponentCollision::HitInfo info{};

    // モデルが存在していない
    auto mdl = col2->GetOwner()->GetComponent<ComponentModel>();
    if(mdl == nullptr)
        return info;

    float3 opos{};
    float3 cpos{};
    if(col1->attach_node_ >= 0) {
        cpos = mul(float4(col1->GetTranslate(), 1), col1->attach_node_matrix_).xyz;
        opos = cpos;
    }
    else {
        // オブジェクト位置に対するコリジョン(1フレーム前)
        opos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetOldWorldMatrix()).xyz;

        // オブジェクト位置に対するコリジョン
        cpos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetMatrix()).xyz;

        // 実際の移動できる量にする
        auto  move  = cpos - opos;
        float movey = move.y;
        move.y      = 0;

        if(col1->IsUseGravity()) {
            move   = col2->checkMovement(opos, move, difficult_to_climb_);
            move.y = movey;
            cpos   = opos + move;

            // 当たりからキャラの位置を求める
            auto rot                      = col1->GetOwner()->GetMatrix();
            rot._41_42_43                 = {0, 0, 0};
            auto col1r                    = mul(float4(col1->GetTranslate(), 1), rot).xyz;
            col1->GetOwner()->Translate() = cpos - col1r;
        }
    }

    float scale = 1.0f;
    if(col1->attach_node_ < 0) {
        auto  mat = col1->GetMatrix();
        float sx  = length(mat.axisVectorX());
        float sy  = length(mat.axisVectorY());
        float sz  = length(mat.axisVectorZ());
        scale     = (sx + sy + sz) / 3.0f;
    }

    float radius = col1->GetRadius() * scale;
#if 0
	MV1_COLL_RESULT_POLY hit_poly{};
	float3				 bottom = cpos - float3{ 0, col1->GetRadius() * scale, 0 };
	float3				 top	= opos + float3{ 0, col1->GetRadius() * scale, 0 };
	hit_poly					= MV1CollCheck_Line( mdl->GetModel(), -1, cast( top ), cast( bottom ) );
	DrawLine3D( cast( top ), cast( bottom ), 0xff00ffff );

	if( hit_poly.HitFlag != 0 )
	{
		float3 pos = cast( hit_poly.HitPosition );

		// 半径分押し戻し
		float3 vec = pos - bottom;

		// 一旦1つ目の当たりだけで返してみる
		// このpush_は、調べたほうの押し戻し方向100%で作成する
		info.push_		   = vec;
		info.hit_		   = true;
		info.hit_position_ = pos;
	}
#endif

#if 0
	// 球当たりの確認
	{
		// 戻し量
		float3 vh = 0;

		MV1_COLL_RESULT_POLY_DIM hit_poly_dim{};

		float3 oc = opos;
		float3 cc = cpos - now_gravity_;

		//DrawCapsule3D( cast( oc ), cast( cc ), radius, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );

		hit_poly_dim = MV1CollCheck_Capsule( mdl->GetModel(), -1, cast( oc ), cast( cc ), radius );
		for( int i = 0; i < hit_poly_dim.HitNum; i++ )
		{
			SEGMENT_TRIANGLE_RESULT result{};

			VECTOR v1 = cast( oc );
			VECTOR v2 = cast( cc );
			DxLib::Segment_Triangle_Analyse( &v1,
											 &v2,
											 &hit_poly_dim.Dim[ i ].Position[ 0 ],
											 &hit_poly_dim.Dim[ i ].Position[ 1 ],
											 &hit_poly_dim.Dim[ i ].Position[ 2 ],
											 &result );

			float3 line_pos = cast( result.Seg_MinDist_Pos );
			float3 tri_pos	= cast( result.Tri_MinDist_Pos );

			// カプセルへの戻し方向
			if( HelperLib::Math::NearlyEqual( length( line_pos - tri_pos ), 0 ) )
			{
				float3 v = ( tri_pos - cpos );
				vh		 = merge( vh, v );
			}
			else
			{
				float3 vec = normalize( line_pos - tri_pos );
				// 戻し量
				float  len = radius - length( line_pos - tri_pos );
				float3 v   = vec * len;
				vh		   = merge( vh, v );
			}
		}

		if( hit_poly_dim.HitNum > 0 )
		{
			info.push_		   = vh;
			info.hit_		   = true;
			info.hit_position_ = cpos - vh;
		}
	}
#endif
#if 1
    // 当たりの確認(通常)
    {
        // 戻し量
        float3 vh = 0;

        MV1_COLL_RESULT_POLY_DIM hit_poly_dim{};

        float3 ocenter = opos + info.push_;
        float3 ncenter = cpos + info.push_;

        //DrawCapsule3D( cast( ocenter ), cast( ncenter ), radius, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );

        // スピードは一旦無視。面倒なのでカプセルをそのまま利用
        hit_poly_dim = MV1CollCheck_Capsule(mdl->GetModel(), -1, cast(ocenter), cast(ncenter), radius);
        for(int i = 0; i < hit_poly_dim.HitNum; i++) {
            SEGMENT_TRIANGLE_RESULT result{};

            VECTOR v1 = cast(ocenter);
            VECTOR v2 = cast(ncenter);
            DxLib::Segment_Triangle_Analyse(&v1,
                                            &v2,
                                            &hit_poly_dim.Dim[i].Position[0],
                                            &hit_poly_dim.Dim[i].Position[1],
                                            &hit_poly_dim.Dim[i].Position[2],
                                            &result);

            float3 line_pos = cast(result.Seg_MinDist_Pos);
            float3 tri_pos  = cast(result.Tri_MinDist_Pos);
            float3 tri_nml  = cast(hit_poly_dim.Dim[i].Normal);

            // カプセルへの戻し方向
            if(HelperLib::Math::NearlyEqual(length(line_pos - tri_pos), 0)) {
                float3 v = (tri_pos - ncenter);
                v        = v + (tri_nml * radius);

                vh = merge(vh, v);
            }
            else {
                float3 vec = normalize(line_pos - tri_pos);
                // 戻し量
                float  len = radius - length(line_pos - tri_pos);
                float3 v   = vec * len;
                vh         = merge(vh, v);
            }
        }
        if(hit_poly_dim.HitNum > 0) {
            info.push_ += vh;
            info.hit_ = true;
            info.hit_position_ += (cpos - vh);
        }
    }
#endif
    return info;
}

//! @brief Sphere VS Capsule
//! @param col1 Sphereコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionModelPtr col1, ComponentCollisionSpherePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;   // push方向を反対にする
    return hit;
}

//! @brief Model VS Capsule
//! @param col1 Modelコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionCapsulePtr col1,
                                                      ComponentCollisionModelPtr   col2)
{
    ComponentCollision::HitInfo info{};

    // モデルが存在していない
    auto mdl = col2->GetOwner()->GetComponent<ComponentModel>();
    if(mdl == nullptr)
        return info;

    float3 opos{};
    float3 cpos{};
    float3 opos1{};
    float3 cpos1{};
    if(col1->attach_node_ >= 0) {
        cpos = mul(float4(col1->GetTranslate(), 1), col1->attach_node_matrix_).xyz;
        opos = cpos;

        cpos1 = mul(float4(col1->GetTranslate() + float3{0, col1->GetHeight(), 0}, 1), col1->attach_node_matrix_).xyz;
        opos1 = cpos1;
    }
    else {
        // オブジェクト位置に対するコリジョン(1フレーム前)
        opos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetOldWorldMatrix()).xyz;

        // オブジェクト位置に対するコリジョン
        cpos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetMatrix()).xyz;

        // 実際の移動できる量にする
        auto  move  = cpos - opos;
        float movey = move.y;
        move.y      = 0;

        if(col1->IsUseGravity()) {
            move   = col2->checkMovement(opos, move, difficult_to_climb_);
            move.y = movey;
            cpos   = opos + move;

            // 当たりからキャラの位置を求める
            auto rot                      = col1->GetOwner()->GetMatrix();
            rot._41_42_43                 = {0, 0, 0};
            auto col1r                    = mul(float4(col1->GetTranslate(), 1), rot).xyz;
            col1->GetOwner()->Translate() = cpos - col1r;
        }

        // その頭の位置
        opos1 = mul(float4(col1->GetTranslate() + float3(0, col1->GetHeight(), 0), 1),
                    col1->GetOwner()->GetOldWorldMatrix())
                    .xyz;
        cpos1 =
            mul(float4(col1->GetTranslate() + float3(0, col1->GetHeight(), 0), 1), col1->GetOwner()->GetMatrix()).xyz;
    }

    float scale = 1.0f;
    if(col1->attach_node_ < 0) {
        auto  mat = col1->GetMatrix();
        float sx  = length(mat.axisVectorX());
        float sy  = length(mat.axisVectorY());
        float sz  = length(mat.axisVectorZ());
        scale     = (sx + sy + sz) / 3.0f;
    }

    float radius = col1->GetRadius() * scale;

    MV1_COLL_RESULT_POLY hit_poly{};
    float3               bottom = cpos;    //-float3{ 0, col1->GetRadius() * scale, 0 };
    float3               top    = cpos1;   //bottom + float3{ 0, col1->GetRadius() * scale * 2, 0 };
    hit_poly                    = MV1CollCheck_Line(mdl->GetModel(), -1, cast(top), cast(bottom));

    if(hit_poly.HitFlag != 0) {
        float3 pos = cast(hit_poly.HitPosition);

        // 半径分押し戻し
        float3 vec = pos - cpos;

        // 一旦1つ目の当たりだけで返してみる
        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_         = vec;
        info.hit_          = true;
        info.hit_position_ = pos;
    }
#if 0
	// 下の球当たりの確認
	{
		// 戻し量
		float3 vh = 0;

		MV1_COLL_RESULT_POLY_DIM hit_poly_dim{};

		float3 oc = opos + float3( 0, radius, 0 );
		float3 cc = cpos + float3( 0, radius, 0 ) - now_gravity_;

		//DrawCapsule3D( cast( oc ), cast( cc ), radius, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );

		hit_poly_dim = MV1CollCheck_Capsule( mdl->GetModel(), -1, cast( oc ), cast( cc ), radius );
		for( int i = 0; i < hit_poly_dim.HitNum; i++ )
		{
			SEGMENT_TRIANGLE_RESULT result{};

			VECTOR v1 = cast( oc );
			VECTOR v2 = cast( cc );
			DxLib::Segment_Triangle_Analyse( &v1,
											 &v2,
											 &hit_poly_dim.Dim[ i ].Position[ 0 ],
											 &hit_poly_dim.Dim[ i ].Position[ 1 ],
											 &hit_poly_dim.Dim[ i ].Position[ 2 ],
											 &result );

			float3 line_pos = cast( result.Seg_MinDist_Pos );
			float3 tri_pos	= cast( result.Tri_MinDist_Pos );

			// カプセルへの戻し方向
			if( HelperLib::Math::NearlyEqual( length( line_pos - tri_pos ), 0 ) )
			{
				float3 v = ( tri_pos - cpos );
				vh		 = merge( vh, v );
			}
			else
			{
				float3 vec = normalize( line_pos - tri_pos );
				// 戻し量
				float  len = radius - length( line_pos - tri_pos );
				float3 v   = vec * len;
				vh		   = merge( vh, v );
			}
		}

		if( hit_poly_dim.HitNum > 0 )
		{
			info.push_		   += vh;
			info.hit_		   = true;
			info.hit_position_ += cpos - vh;
		}
	}
#endif
    // 上の球当たりの確認
    {
        // 戻し量
        float3 vh = 0;

        MV1_COLL_RESULT_POLY_DIM hit_poly_dim{};

        float3 oc = opos1 - float3(0, radius, 0) + info.push_;
        float3 cc = cpos1 - float3(0, radius, 0) + info.push_;
        //DrawCapsule3D( cast( oc ), cast( cc ), radius, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );

        hit_poly_dim = MV1CollCheck_Capsule(mdl->GetModel(), -1, cast(oc), cast(cc), radius);
        for(int i = 0; i < hit_poly_dim.HitNum; i++) {
            SEGMENT_TRIANGLE_RESULT result{};

            VECTOR v1 = cast(oc);
            VECTOR v2 = cast(cc);
            DxLib::Segment_Triangle_Analyse(&v1,
                                            &v2,
                                            &hit_poly_dim.Dim[i].Position[0],
                                            &hit_poly_dim.Dim[i].Position[1],
                                            &hit_poly_dim.Dim[i].Position[2],
                                            &result);

            float3 line_pos = cast(result.Seg_MinDist_Pos);
            float3 tri_pos  = cast(result.Tri_MinDist_Pos);

            // カプセルへの戻し方向
            if(HelperLib::Math::NearlyEqual(length(line_pos - tri_pos), 0)) {
                float3 v = (tri_pos - cpos);
                vh       = merge(vh, v);
            }
            else {
                float3 vec = normalize(line_pos - tri_pos);
                // 戻し量
                float  len = radius - length(line_pos - tri_pos);
                float3 v   = vec * len;
                vh         = merge(vh, v);
            }
        }
        if(hit_poly_dim.HitNum > 0) {
            info.push_ += vh;
            info.hit_ = true;
            info.hit_position_ += (cpos - vh);
        }
    }

    // 当たりの確認(通常カプセル)
    float3 nyopos = opos;
    float3 nycpos = cpos;
    nyopos.y      = 0;
    nycpos.y      = 0;
    if(!HelperLib::Math::NearlyEqual(length(nyopos - nycpos), 0)) {
        // 戻し量
        float3 vh = 0;

        MV1_COLL_RESULT_POLY_DIM hit_poly_dim{};

        float3 bottomx = cpos + info.push_ + float3(0, radius, 0);
        float3 topx    = cpos1 - float3(0, radius, 0);

        //DrawCapsule3D( cast( topx ), cast( bottomx ), radius, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );

        hit_poly_dim = MV1CollCheck_Capsule(mdl->GetModel(), -1, cast(topx), cast(bottomx), radius);
        for(int i = 0; i < hit_poly_dim.HitNum; i++) {
            SEGMENT_TRIANGLE_RESULT result{};

            VECTOR v1 = cast(topx);
            VECTOR v2 = cast(bottomx);
            DxLib::Segment_Triangle_Analyse(&v1,
                                            &v2,
                                            &hit_poly_dim.Dim[i].Position[0],
                                            &hit_poly_dim.Dim[i].Position[1],
                                            &hit_poly_dim.Dim[i].Position[2],
                                            &result);

            float3 line_pos = cast(result.Seg_MinDist_Pos);
            float3 tri_pos  = cast(result.Tri_MinDist_Pos);

            // カプセルへの戻し方向
            if(HelperLib::Math::NearlyEqual(length(line_pos - tri_pos), 0)) {
                float3 v = (tri_pos - cpos);
                vh       = merge(vh, v);
            }
            else {
                float3 vec = normalize(line_pos - tri_pos);
                // 戻し量
                float  len = radius - length(line_pos - tri_pos);
                float3 v   = vec * len;
                vh         = merge(vh, v);
            }
        }
        if(hit_poly_dim.HitNum > 0) {
            info.push_ += vh;
            info.hit_ = true;
            info.hit_position_ += (cpos - vh);
        }
    }

    return info;
}

ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionLinePtr col1, ComponentCollisionSpherePtr col2)
{
    ComponentCollision::HitInfo info{};

    auto line_owner = col1->GetOwner();
    auto line_comp  = line_owner->GetComponent<ComponentCollisionLine>();
    auto line       = line_comp->GetWorldLine();

    auto sphere_owner = col2->GetOwner();
    if(sphere_owner == nullptr)
        return info;

    auto sphere = sphere_owner->GetComponent<ComponentCollisionSphere>();
    if(sphere == nullptr)
        return info;

    auto  pos    = sphere->GetWorldMatrix().translate();
    float radius = sphere->GetRadius();

    SEGMENT_POINT_RESULT result;

    VECTOR start  = cast(line[0]);
    VECTOR end    = cast(line[1]);
    VECTOR center = cast(pos);

    Segment_Point_Analyse(&start, &end, &center, &result);

    auto  point = cast(result.Seg_MinDist_Pos);
    float len   = length(point - pos);

    if(len <= radius) {
        auto vec           = normalize(line[0] - line[1]);
        info.hit_          = true;
        info.hit_position_ = point + vec * acos(len / radius) / 0.5f * DX_PI_F;
    }

    return info;
}

ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionLinePtr col1, ComponentCollisionCapsulePtr col2)
{
    ComponentCollision::HitInfo info{};

    auto line_owner = col1->GetOwner();
    auto line_comp  = line_owner->GetComponent<ComponentCollisionLine>();
    auto line       = line_comp->GetWorldLine();

    auto capsule_owner = col2->GetOwner();
    if(capsule_owner == nullptr)
        return info;

    auto capsule = capsule_owner->GetComponent<ComponentCollisionCapsule>();
    if(capsule == nullptr)
        return info;

    auto  mat    = capsule->GetWorldMatrix();
    auto  pos    = mat.translate();
    float height = capsule->GetHeight();
    float radius = capsule->GetRadius();
    auto  pos1   = pos + height * mat.axisY();

    SEGMENT_SEGMENT_RESULT result;

    VECTOR l1_start = cast(line[0]);
    VECTOR l1_end   = cast(line[1]);
    VECTOR l2_start = cast(pos);
    VECTOR l2_end   = cast(pos1);

    Segment_Segment_Analyse(&l1_start, &l1_end, &l2_start, &l2_end, &result);

    auto  point     = cast(result.SegA_MinDist_Pos);
    auto  cap_point = cast(result.SegB_MinDist_Pos);
    float len       = length(point - cap_point);

    if(len <= radius) {
        auto vec           = normalize(line[0] - line[1]);
        info.hit_          = true;
        info.hit_position_ = point + vec * acos(len / radius) / 0.5f * DX_PI_F;
    }

    return info;
}

ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionLinePtr col1, ComponentCollisionModelPtr col2)
{
    ComponentCollision::HitInfo info{};

    auto line_owner = col1->GetOwner();
    auto line_comp  = line_owner->GetComponent<ComponentCollisionLine>();
    auto line       = line_comp->GetWorldLine();

    auto model_owner = col2->GetOwner();
    if(model_owner == nullptr)
        return info;

    auto model = model_owner->GetComponent<ComponentModel>();
    if(model == nullptr)
        return info;

    MV1_COLL_RESULT_POLY hit_poly{};

    hit_poly = MV1CollCheck_Line(model->GetModel(), -1, cast(line[0]), cast(line[1]));

    info.hit_          = hit_poly.HitFlag;
    info.hit_position_ = float3(hit_poly.HitPosition.x, hit_poly.HitPosition.y, hit_poly.HitPosition.z);

    return info;
}

//! @brief Capsule VS Model
//! @param col1 Capsuleコリジョン
//! @param col2 Model コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionModelPtr   col1,
                                                      ComponentCollisionCapsulePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;   // push方向を反対にする
    return hit;
}
