
#include "Game.h"
#include "Projectile.h"
#include "Enemy.h"


Projectile::Projectile(int index, Enemy* owner, Vector2 speed, Vector2 location, EType type)
    : index(index)
    , Location(location.X, location.Y)
    , Speed(speed.X, speed.Y)
    , Type(type)
    , Owner(owner)
{
    // 親の弾画像を参照
    switch (type)
    {
        case Normal:
        {
            Image = Owner->ProjectileImage;
            Size = Vector2(5.f, 5.f);
            Collision = AABB(Location, Vector2(Location.X + Size.X, Location.Y + Size.Y));
            bCanMove = true;
        }
        break;

        case Large:
        {
            Image = Owner->ProjectileImage2;
            Size = Vector2(15.f, 15.f);
            Collision = AABB(Location, Vector2(Location.X + Size.X, Location.Y + Size.Y));
            bCanMove = true;
        }
        break;

        case Spear:
        {
            Image = Owner->SpearProjectileImage;
            Size = Vector2(4.f, 32.f);
            Collision = AABB(Location, Vector2(Location.X + Size.X, Location.Y + Size.Y));
            bCanMove = false;
        }
        break;
    }
}

Projectile::~Projectile()
{
}

void Projectile::Tick(float DeltaTime, float dilation)
{
    static float waitTime = 0.f;
    if (Type == EType::Spear && !bCanMove) waitTime += 1.f * DeltaTime * dilation;

    if (waitTime >= 1.f)
    {
        bCanMove = true;
        waitTime = 0.f;
    }

    switch (Type)
    {
        case EType::Large:
        {
            if (Location.X < 5.0f
                || Location.X > static_cast<float>(conioex2::Width()) - Size.X - 5.f
                || Location.Y < 20.0f
                || Location.Y > static_cast<float>(conioex2::Height()) - Size.Y - 5.f)
            {
                Destroy();
            }
        }
        break;

        case EType::Spear:
        {
            if (Location.Y > static_cast<float>(conioex2::Height()) - Size.Y - 5.0f)
            {
                Destroy();
            }
        }
    break;

        case EType::Normal:
        {
            if (Location.X < 5.0f
                || Location.X > static_cast<float>(conioex2::Width()) - Size.X - 5.f
                || Location.Y < 5.0f
                || Location.Y > static_cast<float>(conioex2::Height()) - Size.Y - 5.f)
            {
                Destroy();
            }
        }
        break;
    }

    // プレイヤーとの交差判定をとる
    if (conioex2::IsHit(Collision, Game::GetGameInstance()->m_Player->CapsuleCollision))
    {
        Game::GetGameInstance()->m_Player->NotifyHit(this);
    }

    if (bCanMove) Move(DeltaTime * dilation);

    // ボックスコリジョンの座標更新
    Collision.Min.X = Location.X;
    Collision.Min.Y = Location.Y;
    Collision.Max.X = Location.X + Size.X;
    Collision.Max.Y = Location.Y + Size.Y;

    Draw();
}

void Projectile::Draw()
{
    conioex2::DrawImageGS(Image, static_cast<int>(Location.X), static_cast<int>(Location.Y));
}

void Projectile::Move(float DeltaTime)
{
    Location.X += Speed.X * DeltaTime;
    Location.Y += Speed.Y * DeltaTime;
}

void Projectile::Destroy()
{
    Owner->DestroyProjectile(index);
}
