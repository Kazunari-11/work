//---------------------------------------------------------------------------
//! @file   LightManager.h
//! @brief  光源管理
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! 光源管理
//===========================================================================
class LightManager
{
public:
    //! デフォルトコンストラクタ
    LightManager() = default;

    //! 初期化
    bool initialize();

    //! 更新
    void update();

    //! 解放
    void finalize();

private:
    //----------------------------------------------------------
    //! @name   copy/move禁止
    //----------------------------------------------------------
    //@{

    LightManager(const LightManager&)   = delete;
    LightManager(LightManager&&)        = delete;
    void operator=(const LightManager&) = delete;
    void operator=(LightManager&&)      = delete;

    //@}

private:
    // 定数バッファのデーターとして共用
    // 注意:16バイトの境界にサイズを統一

    // 平行光源 Directional Light
    struct LightDirectional
    {
        float3 color_     = float3(1.0f, 1.0f, 1.0f);   //!< カラー
        float3 light_dir_ = float3(0.0f, 1.0f, 0.0f);   //!< 方向 (Lベクトル)
    };

    // 点光源 Point Light
    struct LightPoint
    {
        float3 color_    = float3(1.0f, 1.0f, 1.0f);   //!< カラー
        float3 position_ = float3(0.0f, 5.0f, 0.0f);   //!< 位置
        float  radius_   = 10.0f;                      //!< 影響半径
        int    unused1_  = 0;
        int    unused2_  = 0;
        int    unused3_  = 0;
    };

    // スポット光源 Spot Light
    struct LightSpot
    {
        float3 color_     = float3(1.0f, 1.0f, 1.0f);    //!< カラー
        float3 position_  = float3(0.0f, 5.0f, 0.0f);    //!< 位置
        float3 dir_       = float3(0.0f, -1.0f, 0.0f);   //!< 方向
        float  radius_    = 10.0f;                       //!< 影響半径
        float  angle_in_  = 30.0f * DegToRad;            //!< 減衰が始まる角度
        float  angle_out_ = 45.0f * DegToRad;            //!< 減衰が終わる角度
        int    unused0_   = 0;
    };

    struct LightInfo
    {
        LightDirectional light_directional_[4];
        LightPoint       light_point_[8];
        LightSpot        light_spot_;
        int              light_count_directional_ = 0;   //!< 平行光源の個数
        int              light_count_point_       = 0;   //!< 点光源の個数
        int              light_count_spot_        = 0;   //!< スポット光源の個数
    };

    LightInfo info_;             //!< 定数バッファ用データー
    int       cb_handle_ = -1;   //!< 定数バッファハンドル
};
