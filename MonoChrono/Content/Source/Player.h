
#pragma once
#include "conioex2.h"

struct Projectile;

enum EPlayerAnimation
{
    R_Walk,
    L_Walk,
    R_Falling,
    L_Falling,
    R_Idle,
    L_Idle,
    R_Dash,
    L_Dash,
    R_Air,
    L_Air,
    R_Attack,
    L_Attack,
};

struct Player {

    Player();
    ~Player();

    // パラメーター
    Vector2 Location;
    Vector2 Size;
    Vector2 KnockBack;
    AABB    AttackCollision;  // 攻撃判定
    AABB    CapsuleCollision; // キャラクターの判定

    // ステータス
    float Speed;
    float HP;
    float JumpSpeed;
    float GravityScale;
    int   JumpCount;

    // アクションが可能かどうか
    bool bCanJump;
    bool bCanAttack;
    bool bCanMove;

    // プレイヤーの行動状態
    bool bIsAttacking;
    bool bIsWalking;
    bool bIsJumping;
    bool bIsDash;
    bool bIsinAir;
    bool bMoveRight;
    bool bOnFloor;

    // ダメージ系
    bool bRecieveDamage;
    bool bHitProjectile;
    bool bKnockback;
    bool bIsVibrating;
    bool bIsInvincible;

    // ウィッチタイム
    bool bIsWitchTime;
    bool bIsWitchtimeCoolTime;

    // アニメーションパラメーター
    float WalkAnimationTransitionTime;
    float AirAnimationTransitionTime;
    float AttackAnimationTransitionTime;

    // サウンドハンドル
    SoundHandle SE_Damage;
    SoundHandle SE_Attack;

    void Init();

    // 更新関数
    void Tick(float DeltaTime, float dilation);
    void UpdatePhysics(float DeltaTime);
    void UpdateAnimimation(float DeltaTime);
    void UpdateState(float DeltaTime);
    void UpdateInput(float DeltaTime);
    void Draw();
    void HUD();

    // ステータス関数
    void Damage(float amount);
    void NotifyHit(Projectile* projectile);
    void Dead();

    // アクション関数
    void MoveRight(float DeltaTime);
    void MoveLeft(float DeltaTime);
    void MoveUp(float DeltaTime);

    void Move(float DeltaTime, Vector2 value);	// コントローラー移動

    void Jump();
    void Attack();


    // アニメーション遷移管理
    void AnimSequence(EPlayerAnimation animation, float transitionTime, int index, float DeltaTime);

    // 現在のアニメーション
    EPlayerAnimation CurrentAnimation;

    // 現在のアニメーションインデックス
    int AnimationIndex;
    int AttackAnimIndex;

    // アイドルアニメーション
    conioex2::Engine::Image* R_IdleAnimation;
    conioex2::Engine::Image* L_IdleAnimation;
    // 空中アニメーション
    conioex2::Engine::Image* R_AirAnimation[2];
    conioex2::Engine::Image* L_AirAnimation[2];

    // ダッシュアニメーション
    conioex2::Engine::Image* R_DashAnimation;
    conioex2::Engine::Image* L_DashAnimation;
    // 移動アニメーション
    conioex2::Engine::Image* R_WalkAnimation[7];
    conioex2::Engine::Image* L_WalkAnimation[7];

    // 攻撃アニメーション
    conioex2::Engine::Image* R_AttackAnimation[3];
    conioex2::Engine::Image* L_AttackAnimation[3];

    // 攻撃トレイル
    conioex2::Engine::Image* L_attackTrail;
    conioex2::Engine::Image* R_attackTrail;
};
