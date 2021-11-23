
#pragma once
#include "conioex2.h"
#include <vector>

enum   EType;
struct Projectile;
struct Player;

struct Enemy
{
    Enemy();
    ~Enemy();

    // 座標系パラメーター
    Vector2 Location;
    Vector2 Size;

    AABB Collision;
    float HP;
    bool bRecieveDamage;
    bool bRapidFire;
    SoundHandle  SE_Damage;
    int  RapidCount;

    void Init();
    void Tick(float DeltaTime, float dilation);
    void Draw();
    void UpdateState(float DeltaTime, float dilation);
    void UpdatePhysics(float DeltaTime, float dilation);

    void Attack1();
    void Attack2();
    void Attack3(float DeltaTime, float dilation);

    void Teleport();
    void SpawnBullet(Vector2 location, Vector2 speed, EType type);
    void UpdateProjectile(float DeltaTime, float dilation);
    void DestroyProjectile(int index);

    void NotifyHit(Player* projectile, float amount);
    void Damage(float amount);
    void Dead();

    // 画像
    conioex2::Engine::Image* Idle;
    conioex2::Engine::Image* Damaged;
    conioex2::Engine::Image* ProjectileImage;
    conioex2::Engine::Image* ProjectileImage2;
    conioex2::Engine::Image* SpearProjectileImage;

    // 弾
    std::vector<Projectile*> ProjectiletList;

    int ProjectileCount;

    Vector2 tp1;
    Vector2 tp2;
    Vector2 tp3;

};
