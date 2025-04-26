
#include "Player.h"
#include "Game.h"
#include "Projectile.h"


Player::Player()
    : Location(120.0f, 60.0f)
    , Size(16.0f, 24.0f)
    , KnockBack(200.f, 200.0f)
    , AttackCollision(Vector2(0.0f, 0.f), Vector2(0.0f, 0.0f))
    , CapsuleCollision(Vector2(0.0f, 0.f), Vector2(0.0f, 0.0f))
    , Speed(100.0f)
    , JumpSpeed(500.0f)
    , GravityScale(150.0f)
    , bCanJump(true)
    , bIsJumping(false)
    , bIsAttacking(false)
    , bIsWalking(false)
    , HP(5.0f)
    , WalkAnimationTransitionTime(0.1f)
    , AirAnimationTransitionTime(0.1f)
    , AttackAnimationTransitionTime(0.05f)
    , AnimationIndex(0)
    , AttackAnimIndex(1)
    , JumpCount(0)
    , CurrentAnimation(EPlayerAnimation::R_Idle)
    , bMoveRight(true)
    , bRecieveDamage(true)
    , bIsDash(false)
    , bIsinAir(true)
    , bIsWitchTime(false)
    , bCanAttack(true)
    , bCanMove(true)
    , bHitProjectile(false)
    , bIsVibrating(false)
    , bKnockback(false)
    , bOnFloor(false)
    , bIsWitchtimeCoolTime(false)
    , bIsInvincible(false)
{
}

Player::~Player()
{
}

void Player::Init()
{
#if 0
    Location = Vector2(120.0f, 60.0f);
    Speed = 100.0f;
    JumpSpeed = 500.0f;
    GravityScale = 150.0f;
    bCanJump = true;
    bIsJumping = false;
    bIsAttacking = false;
    bIsWalking = false;
    HP = 5.0f;
    AnimationIndex = 0;
    AttackAnimIndex = 1;
    JumpCount = 0;
    CurrentAnimation = EPlayerAnimation::R_Idle;
    bMoveRight = true;
    bRecieveDamage = true;
    bIsDash = false;
    bIsinAir = true;
    bIsWitchTime = false;
    bCanAttack = true;
    bCanMove = true;
    bHitProjectile = false;
    bIsVibrating = false;
    bKnockback = false;
    bOnFloor = false;
    bIsWitchtimeCoolTime = false;
    bIsInvincible = false;
#endif
}

void Player::Tick(float DeltaTime, float dilation)
{
    UpdateInput(DeltaTime);       // 入力の更新
    UpdatePhysics(DeltaTime);     // 判定の更新
    UpdateState(DeltaTime);       // ステートの更新
    UpdateAnimimation(DeltaTime); // アニメーションの更新
    Draw();                       // 描画の更新
}

void Player::MoveUp(float DeltaTime)
{
    if (Location.Y >= 2.0f)
    {
        Location.Y += -JumpSpeed * DeltaTime;
    }
}

void Player::Move(float DeltaTime, Vector2 value)
{
    if (bCanMove)
    {
        Location.X += value.X * Speed * DeltaTime;
    }
}

// 最後に解放するからdeleteしないこと
void Player::Dead()
{
    AudioManager::StopStresm(Game::GetGameInstance()->BGM_Main);

    conioex2::StopVibrate();
    Game::GetGameInstance()->OpenLevel(ELevel::GameOver);
}

void Player::Jump()
{
    if ((bCanJump) && (JumpCount < 1))
    {
        bIsJumping = true;
        JumpCount++;
    }
}

void Player::Attack()
{
    // 当たり判定と攻撃エフェクトを生成
    if (!bIsAttacking && bCanAttack)
    {
        bIsAttacking = true;
        AudioManager::SetSEVolume(SE_Attack, 0.1f);
        AudioManager::PlaySE(SE_Attack);
    }
}

void Player::UpdatePhysics(float DeltaTime)
{
    // ボックスコリジョンの更新
    CapsuleCollision.Min.X = Location.X;
    CapsuleCollision.Min.Y = Location.Y;
    CapsuleCollision.Max.X = Location.X + Size.X;
    CapsuleCollision.Max.Y = Location.Y + Size.Y;

    // 攻撃判定の更新
    if (bMoveRight)
    {
        AttackCollision.Min.X = Location.X + Size.X;
        AttackCollision.Max.X = Location.X + Size.X + 32.f;
        AttackCollision.Min.Y = Location.Y;
        AttackCollision.Max.Y = Location.Y + Size.Y;
    }
    else
    {
        AttackCollision.Min.X = Location.X - 32.f;
        AttackCollision.Max.X = Location.X;
        AttackCollision.Min.Y = Location.Y;
        AttackCollision.Max.Y = Location.Y + 24.f;
    }

    FOR(3)
    {
        if (Game::GetGameInstance()->Floors[i]->IsOnFloor(this))
        {
            GravityScale = 0.f;
            JumpCount = 0;
            bIsinAir = false;
            bOnFloor = true;
            break;
        }
        else
        {
            if (!bIsDash) GravityScale = 150.0f;
            bOnFloor = false;
        }
    }

    // 攻撃のヒット判定を敵に通知
    if (bIsAttacking)
    {
        if (conioex2::IsHit(AttackCollision, Game::GetGameInstance()->m_Enemy->Collision))
        {
            Game::GetGameInstance()->m_Enemy->NotifyHit(this, 1.f);
        }
    }

    // X画面内強制
    if (Location.X <= 5.0f)
    {
        Location.X = 5.0f;
    }
    if (Location.X >= conioex2::Width() - Size.X - 5.f)
    {
        Location.X = conioex2::Width() - Size.X - 5.f;
    }

    // Y画面内強制
    if (Location.Y <= 5.0f)
    {
        Location.Y = 5.0f;
    }
    // 擬似重力を適応
    if (Location.Y <= conioex2::Height() - Size.Y - 5.0f)
    {
        if (!bOnFloor && !bIsAttacking)
        {
            bIsinAir = true;
            Location.Y += GravityScale * DeltaTime;
        }
    }
    else
    {
        bIsinAir = false;
        JumpCount = 0;
    }
}

void Player::AnimSequence(EPlayerAnimation animation, float transitionTime, int index, float DeltaTime)
{
    static float currentTime = 0.f;
    currentTime += 1.f * DeltaTime;
    CurrentAnimation = animation;

    if (currentTime >= transitionTime)
    {
        AnimationIndex < index ? AnimationIndex++ : AnimationIndex = 0;
        currentTime = 0.f;
    }
}

void Player::UpdateAnimimation(float DeltaTime)
{
    // 移動中
    if (bIsWalking)
    {
        // 右移動中
        if (bMoveRight)
        {
            // 攻撃中
            if (bIsAttacking)
            {
                static float transitionTime = 0.f;
                transitionTime += 1.f * DeltaTime;
                CurrentAnimation = EPlayerAnimation::R_Attack;

                // 0.1秒経過したら次のアニメーションフレームへ
                if (transitionTime >= AttackAnimationTransitionTime)
                {
                    AttackAnimIndex < 2 ? AttackAnimIndex++ : AttackAnimIndex = 0;
                    transitionTime = 0.f;
                }
            }
            else
            {
                // ダッシュ
                if (bIsDash)
                {
                    CurrentAnimation = EPlayerAnimation::R_Dash;
                }
                else
                {
                    // 空中かどうか
                    if (bIsinAir)
                    {
                        AnimSequence(EPlayerAnimation::R_Air, AirAnimationTransitionTime, 1, DeltaTime);
                    }
                    else
                    {
                        AnimSequence(EPlayerAnimation::R_Walk, WalkAnimationTransitionTime, 6, DeltaTime);
                    }
                }
            }
        }
        // 左移動中
        else
        {
            // 攻撃中
            if (bIsAttacking)
            {
                static float transitionTime = 0.f;
                transitionTime += 1.f * DeltaTime;
                CurrentAnimation = EPlayerAnimation::L_Attack;

                // 0.1秒経過したら次のアニメーションフレームへ
                if (transitionTime >= AttackAnimationTransitionTime)
                {
                    AttackAnimIndex < 2 ? AttackAnimIndex++ : AttackAnimIndex = 0;
                    transitionTime = 0.f;
                }
            }
            else
            {
                // ダッシュ
                if (bIsDash)
                {
                    CurrentAnimation = EPlayerAnimation::L_Dash;
                }
                else
                {
                    // 空中かどうか
                    if (bIsinAir)
                    {
                        AnimSequence(EPlayerAnimation::L_Air, AirAnimationTransitionTime, 1, DeltaTime);
                    }
                    else
                    {
                        AnimSequence(EPlayerAnimation::L_Walk, WalkAnimationTransitionTime, 6, DeltaTime);
                    }
                }
            }
        }
    }
    // ニュートラル
    else
    {
        if (bMoveRight)
        {
            // 攻撃中
            if (bIsAttacking)
            {
                static float transitionTime = 0.f;
                transitionTime += 1.f * DeltaTime;
                CurrentAnimation = EPlayerAnimation::R_Attack;

                // 0.1秒経過したら次のアニメーションフレームへ
                if (transitionTime >= AttackAnimationTransitionTime)
                {
                    AttackAnimIndex < 2 ? AttackAnimIndex++ : AttackAnimIndex = 0;
                    transitionTime = 0.f;
                }
            }
            else
            {
                if (bIsinAir)
                {
                    AnimSequence(EPlayerAnimation::R_Air, AirAnimationTransitionTime, 1, DeltaTime);
                }
                else
                {
                    CurrentAnimation = EPlayerAnimation::R_Idle;
                }
            }
        }
        else
        {
            // 攻撃中
            if (bIsAttacking)
            {
                static float transitionTime = 0.f;
                transitionTime += 1.f * DeltaTime;
                CurrentAnimation = EPlayerAnimation::L_Attack;

                // 0.1秒経過したら次のアニメーションフレームへ
                if (transitionTime >= AttackAnimationTransitionTime)
                {
                    AttackAnimIndex < 2 ? AttackAnimIndex++ : AttackAnimIndex = 0;
                    transitionTime = 0.f;
                }
            }
            else
            {
                if (bIsinAir)
                {
                    AnimSequence(EPlayerAnimation::L_Air, AirAnimationTransitionTime, 1, DeltaTime);
                }
                else
                {
                    CurrentAnimation = EPlayerAnimation::L_Idle;
                }
            }
        }
    }
}

void Player::NotifyHit(Projectile* projectile)
{
    if (!bRecieveDamage || bIsInvincible)
    {
        bHitProjectile = true;
    }
    else
    {
        if (!bIsInvincible)
        {
            Damage(1.f);
        }

        bRecieveDamage = false;
        bKnockback = true;
        conioex2::PlayVibrate();

        AudioManager::SetSEVolume(SE_Damage, 0.1f);
        AudioManager::PlaySE(SE_Damage);
        projectile->Destroy();
        bIsVibrating = true;
        bHitProjectile = false;
    }
}

void Player::UpdateState(float DeltaTime)
{
    static float jumpTime = 0.f;
    static float invincibleTime = 0.f;
    static float dashTime = 0.f;
    static float attackTime = 0.f;
    static float witchTime = 0.f;
    static float vibrateTime = 0.f;
    static float knockbackTime = 0.f;
    static float witchtimeCoolTime = 0.f;

    // 死亡判定
    if (HP <= 0.f)
    {
        Dead();
    }

    // ジャンプ
    if (bIsJumping)
    {
        jumpTime += 1.0f * DeltaTime;
        MoveUp(DeltaTime);
    }
    if (jumpTime >= 0.2f)
    {
        bIsJumping = false;
        jumpTime = 0.0f;
    }

    // ダッシュ
    if (bIsDash)
    {
        bIsInvincible = true;
        bCanAttack = false;
        Speed = 300.0f;
        GravityScale = 0.0f;

        dashTime += 1.0f * DeltaTime;

        // ダッシュ中に弾を食らっていたらウィッチタイムに移行
        if (bHitProjectile && !bIsWitchTime && !bIsWitchtimeCoolTime)
        {
            bIsWitchTime = true;
            bHitProjectile = false;
            Game::GetGameInstance()->SetTimeDilation(0.1f);
            Game::GetGameInstance()->InverseGrayScale();
            AudioManager::SetPitch(Game::GetGameInstance()->BGM_Main, -0.5f);
        }
    }
    if (dashTime >= 0.15f)
    {
        bIsDash = false;
        bIsInvincible = false;
        bCanAttack = true;
        Speed = 100.0f;
        GravityScale = 150.0f;
        dashTime = 0.0f;
    }

    // ダメージ後無敵時間
    if (!bRecieveDamage)
    {
        invincibleTime += 1.0f * DeltaTime;
    }
    if (invincibleTime >= 3.0f)
    {
        invincibleTime = 0.0f;
        bRecieveDamage = true;
    }

    // ノックバック中
    if (bKnockback)
    {
        bCanAttack = false;
        bIsDash = false;
        bIsJumping = false;
        bCanMove = false;
        knockbackTime += 1.0f * DeltaTime;

        if (bMoveRight)
        {
            Location.X += -KnockBack.X * DeltaTime;
            Location.Y += -KnockBack.Y * DeltaTime;
        }
        else
        {
            Location.X += KnockBack.X * DeltaTime;
            Location.Y += -KnockBack.Y * DeltaTime;
        }
    }
    if (knockbackTime >= 0.2f)
    {
        knockbackTime = 0.f;
        bKnockback = false;
        bCanAttack = true;
        bCanMove = true;
    }


    // ダメージ時のバイブ時間
    if (bIsVibrating)
    {
        vibrateTime += 1.0f * DeltaTime;
    }
    if (vibrateTime >= 0.5f)
    {
        vibrateTime = 0.f;
        bIsVibrating = false;
        conioex2::StopVibrate();
    }

    // ウィッチタイム時間
    if (bIsWitchTime)
    {
        bHitProjectile = false; // 再度判定された時対応
        bCanAttack = true;
        bRecieveDamage = false;

        witchTime += 1.0f * DeltaTime;
    }
    if (witchTime >= 2.5f)
    {
        Game::GetGameInstance()->SetTimeDilation(1.0f);
        Game::GetGameInstance()->SetGrayScale();
        //conioex2::SetPitch(0, 0.0f);
        AudioManager::SetPitch(Game::GetGameInstance()->BGM_Main, 0.0f);

        bIsWitchTime = false;
        bRecieveDamage = true;
        invincibleTime = 0.f;
        bIsWitchtimeCoolTime = true;
        witchTime = 0.0f;
    }

    // ウィッチタイムのクールタイム
    if (bIsWitchtimeCoolTime)
    {
        witchtimeCoolTime += 1.f * DeltaTime;
    }
    if (witchtimeCoolTime >= 5.f)
    {
        bIsWitchtimeCoolTime = false;
        witchtimeCoolTime = 0.f;
    }

    // 攻撃
    if (bIsAttacking)
    {
        attackTime += 1.0f * DeltaTime;
        GravityScale = 0.0f;
        bCanMove = false;
        bMoveRight ?
            conioex2::DrawImageGS(R_attackTrail, (int)Location.X + 24, (int)Location.Y) :
            conioex2::DrawImageGS(L_attackTrail, (int)Location.X - 42, (int)Location.Y);
    }
    if (attackTime >= 0.15f)
    {
        attackTime = 0.0f;
        GravityScale = 150.0f;
        bIsAttacking = false;
        bCanMove = true;
        AttackAnimIndex = 0;
    }

    bHitProjectile = false;
}

void Player::UpdateInput(float DeltaTime)
{
    // Lスティック入力
    Vector2 L = conioex2::GetInputXBoxAxis(Stick::Stick_L);

    // デッドゾーンとして絶対値0.15以下を0に Clamp() する
    if (abs(L.X) <= 0.15f) L.X = 0.0f;
    if (abs(L.Y) <= 0.15f) L.Y = 0.0f;

    if (conioex2::GetInputKey(Key::A, InputState::Hold)) L.X = -1.0f;
    if (conioex2::GetInputKey(Key::D, InputState::Hold)) L.X =  1.0f;


    // クランプ後に入力判定があるなら
    if (L.X != 0.0f)
    {
        Move(DeltaTime, L);
        bIsWalking = true;

        L.X > 0.0f ? bMoveRight = true : bMoveRight = false;
    }
    else
    {
        bIsWalking = false;
    }

    // ジャンプ
    if (conioex2::GetInputXBoxButton(Button::XBox_Button_A, XBoxInputState::XBox_Down) ||
        conioex2::GetInputKey(Key::SPACE, InputState::Pressed))
    {
        Jump();
    }

    // 攻撃
    if (conioex2::GetInputXBoxButton(Button::XBox_Button_R_Shoulder, XBoxInputState::XBox_Down) ||
        conioex2::GetInputKey(Key::K, InputState::Pressed))
    {
        Attack();
    }

    // ダッシュ
    if (conioex2::GetInputXBoxButton(Button::XBox_Button_L_Shoulder, XBoxInputState::XBox_Down) || 
        conioex2::GetInputKey(Key::L, InputState::Pressed))
    {
        if (!bKnockback)
        {
            bIsDash = true;
        }
    }
}

void Player::Draw()
{
    HUD();

    switch (CurrentAnimation)
    {
        case EPlayerAnimation::R_Idle:
        {
            conioex2::DrawImageGS(R_IdleAnimation, (int)Location.X, (int)Location.Y);
            break;
        }
        case EPlayerAnimation::L_Idle:
        {
            conioex2::DrawImageGS(L_IdleAnimation, (int)Location.X, (int)Location.Y);
            break;
        }
        case EPlayerAnimation::R_Walk:
        {
            conioex2::DrawImageGS(R_WalkAnimation[AnimationIndex], (int)Location.X, (int)Location.Y);
            break;
        }
        case EPlayerAnimation::L_Walk:
        {
            conioex2::DrawImageGS(L_WalkAnimation[AnimationIndex], (int)Location.X, (int)Location.Y);
            break;
        }
        case EPlayerAnimation::R_Dash:
        {
            conioex2::DrawImageGS(R_DashAnimation, (int)Location.X, (int)Location.Y);
            break;
        }
        case EPlayerAnimation::L_Dash:
        {
            conioex2::DrawImageGS(L_DashAnimation, (int)Location.X, (int)Location.Y);
            break;
        }
        case EPlayerAnimation::R_Air:
        {
            conioex2::DrawImageGS(R_AirAnimation[AnimationIndex], (int)(Location.X - 8.f), (int)Location.Y);
            break;
        }
        case EPlayerAnimation::L_Air:
        {
            conioex2::DrawImageGS(L_AirAnimation[AnimationIndex], (int)(Location.X - 8.f), (int)Location.Y);
            break;
        }
        case EPlayerAnimation::R_Attack:
        {
            conioex2::DrawImageGS(R_AttackAnimation[AttackAnimIndex], (int)(Location.X - 8.0f), (int)Location.Y);
            break;
        }
        case EPlayerAnimation::L_Attack:
        {
            conioex2::DrawImageGS(L_AttackAnimation[AttackAnimIndex], (int)(Location.X - 8.0f), (int)Location.Y);
            break;
        }
    }
}

void Player::HUD()
{
    if (HP == 1.0f)
    {
        conioex2::DrawImageGS(Game::GetGameInstance()->m_hp_1, 5, 5);
    }
    else if (HP == 2.0f)
    {
        conioex2::DrawImageGS(Game::GetGameInstance()->m_hp_2, 5, 5);
    }
    else if (HP == 3.0f)
    {
        conioex2::DrawImageGS(Game::GetGameInstance()->m_hp_3, 5, 5);
    }
    else if (HP == 4.0f)
    {
        conioex2::DrawImageGS(Game::GetGameInstance()->m_hp_4, 5, 5);
    }
    else if (HP == 5.0f)
    {
        conioex2::DrawImageGS(Game::GetGameInstance()->m_hp_5, 5, 5);
    }
}

void Player::Damage(float amount)
{
    if (HP > 0.0f)
    {
        HP -= amount;
    }
}

void Player::MoveRight(float DeltaTime)
{
    if (Location.X <= conioex2::Width() - Size.X)
    {
        Location.X += Speed * DeltaTime;
        bIsWalking = true;
        bMoveRight = true;
    }
}

void Player::MoveLeft(float DeltaTime)
{
    if (Location.X >= 1.0f)
    {
        Location.X += -Speed * DeltaTime;
        bIsWalking = true;
        bMoveRight = false;
    }
}