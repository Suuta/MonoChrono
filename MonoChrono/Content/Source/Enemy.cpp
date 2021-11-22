
#include "Enemy.h"
#include "Game.h"
#include "Projectile.h"
#include "Player.h"


Enemy::Enemy()
	: Location(200.0f, 160.0f)
	, Size(60.0f, 60.0f)
	, HP(100.f)
	, Idle(nullptr)
	, ProjectileCount(0)
	, RapidCount(0)
	, Collision(Vector2(Location.X, Location.Y), Vector2(Location.X + Size.X, Location.Y + Size.Y))
	, bRecieveDamage(true)
	, bRapidFire(false)
	, tp1( 50.f, 80.f)
	, tp2(125.f, 50.f)
	, tp3(200.f, 80.f)
{
}

Enemy::~Enemy()
{
}

void Enemy::Init()
{
	Location = Vector2(200.0f, 160.0f);
	HP = 100.f;
	ProjectileCount = 0;
	RapidCount = 0;
	Collision = AABB(Vector2(Location.X, Location.Y), Vector2(Location.X + Size.X, Location.Y + Size.Y));
	bRecieveDamage = true;
	bRapidFire = false;
	tp1 = Vector2(50.f, 80.f);
	tp2 = Vector2(125.f, 50.f);
	tp3 = Vector2(200.f, 80.f);

	// 弾を全部空にする
	ProjectiletList.clear();
}

void Enemy::Tick(float DeltaTime, float dilation)
{
	UpdatePhysics(DeltaTime, dilation);
	UpdateState(DeltaTime, dilation);

	Draw();
	UpdateProjectile(DeltaTime, dilation);
}

void Enemy::UpdatePhysics(float DeltaTime, float dilation)
{
	// ボックスコリジョンの更新
	Collision.Min.X = Location.X;
	Collision.Min.Y = Location.Y;
	Collision.Max.X = Location.X + Size.X;
	Collision.Max.Y = Location.Y + Size.Y;
}

void Enemy::UpdateState(float deltaTime, float dilation)
{
	static float teleporTime = 0.f;
	static float attack1Span = 0.f;
	static float attack2Span = 0.f;
	static float attack3Span = 0.f;

	teleporTime += 1.f * deltaTime * dilation;
	attack1Span += 1.f * deltaTime * dilation;

	if (HP <= 50.f)
	{
		attack2Span += 1.f * deltaTime * dilation;
	}

	if (HP <= 20.f)
	{
		if(bRapidFire)
		{
			Attack3(deltaTime, dilation);
		}
		else
		{
			attack3Span += 1.f * deltaTime * dilation;
		}
	}


	// プレイヤーが攻撃中でなければダメージを受け付けフラグにする
	if (!Game::GetGameInstance()->m_Player->bIsAttacking)
	{
		bRecieveDamage = true;
	}

	if (HP <= 0.f)
	{
		Dead();
	}

	if (teleporTime >= 4.f)
	{
		Teleport();
		teleporTime = 0.f;
	}

	if (attack1Span >= 3.f)
	{
		Attack1();
		attack1Span = 0.f;
	}
	if (attack2Span >= 6.f)
	{
		Attack2();
		attack2Span = 0.f;
	}
	if (attack3Span >= 4.f)
	{
		bRapidFire = true;
		attack3Span = 0.f;
	}
}

void Enemy::Draw()
{
	bRecieveDamage ? conioex2::DrawImageGS(Idle, Location.X, Location.Y) : conioex2::DrawImageGS(Damaged, Location.X, Location.Y);

	// HPの描画
	FOR(50)
	{
		conioex2::Draw(Location.X + i + 5, Location.Y - 10, " ", 3 << 4, 0);
		conioex2::Draw(Location.X + i + 5, Location.Y -  9, " ", 3 << 4, 0);
		conioex2::Draw(Location.X + i + 5, Location.Y -  8, " ", 3 << 4, 0);
	}									
	FOR(HP / 2)				
	{									
		conioex2::Draw(Location.X + i + 5, Location.Y - 10, " ", 13 << 4, 0);
		conioex2::Draw(Location.X + i + 5, Location.Y -  9, " ", 13 << 4, 0);
		conioex2::Draw(Location.X + i + 5, Location.Y -  8, " ", 13 << 4, 0);
	}
}

void Enemy::Attack1()
{
	SpawnBullet(Vector2(Collision.Max.X - Size.X / 2, Collision.Max.Y - Size.Y / 2), Vector2(Vector2::UnitVector(  0.f) * 50.f), EType::Normal);
	SpawnBullet(Vector2(Collision.Max.X - Size.X / 2, Collision.Max.Y - Size.Y / 2), Vector2(Vector2::UnitVector( 45.f) * 50.f), EType::Normal);
	SpawnBullet(Vector2(Collision.Max.X - Size.X / 2, Collision.Max.Y - Size.Y / 2), Vector2(Vector2::UnitVector( 90.f) * 50.f), EType::Normal);
	SpawnBullet(Vector2(Collision.Max.X - Size.X / 2, Collision.Max.Y - Size.Y / 2), Vector2(Vector2::UnitVector(135.f) * 50.f), EType::Normal);
	SpawnBullet(Vector2(Collision.Max.X - Size.X / 2, Collision.Max.Y - Size.Y / 2), Vector2(Vector2::UnitVector(180.f) * 50.f), EType::Normal);
	SpawnBullet(Vector2(Collision.Max.X - Size.X / 2, Collision.Max.Y - Size.Y / 2), Vector2(Vector2::UnitVector(225.f) * 50.f), EType::Normal);
	SpawnBullet(Vector2(Collision.Max.X - Size.X / 2, Collision.Max.Y - Size.Y / 2), Vector2(Vector2::UnitVector(270.f) * 50.f), EType::Normal);
	SpawnBullet(Vector2(Collision.Max.X - Size.X / 2, Collision.Max.Y - Size.Y / 2), Vector2(Vector2::UnitVector(315.f) * 50.f), EType::Normal);
}

void Enemy::Attack2()
{
	SpawnBullet(Vector2( 50.f, 5.f), Vector2(0.f, 100.f), EType::Spear);
	SpawnBullet(Vector2(100.f, 5.f), Vector2(0.f, 100.f), EType::Spear);
	SpawnBullet(Vector2(150.f, 5.f), Vector2(0.f, 100.f), EType::Spear);
	SpawnBullet(Vector2(200.f, 5.f), Vector2(0.f, 100.f), EType::Spear);
}

void Enemy::Attack3(float DeltaTime, float dilation)
{
	static float rapidRate = 0.f;
	rapidRate += 1.f * DeltaTime * dilation;

	if (rapidRate >= 0.5f)
	{
		Vector2 v = Vector2::Normalize(Vector2::Between(Location, Game::GetGameInstance()->m_Player->Location));
		SpawnBullet(Vector2(Collision.Max.X - Size.X / 2, Collision.Max.Y - Size.Y / 2), Vector2(v.X * 140.f, v.Y * 140.f), EType::Large);
		rapidRate = 0.f;

		if (RapidCount < 6)
		{
			RapidCount++;
		}
		else
		{
			RapidCount = 0;
			bRapidFire = false;
		}
	}
}

void Enemy::Teleport()
{
	switch (conioex2::RangeRand(1, 3))
	{
		case 1: Location = tp1; break;
		case 2: Location = tp2; break;
		case 3: Location = tp3; break;
	}
}

void Enemy::UpdateProjectile(float DeltaTime, float dilation)
{
	for (int i = 0; i < ProjectiletList.size(); i++)
	{
		ProjectiletList[i]->Tick(DeltaTime, dilation);
	}
}

void Enemy::SpawnBullet(Vector2 location, Vector2 speed, EType type)
{
	ProjectileCount++;
	ProjectiletList.push_back(new Projectile(ProjectileCount, this, speed, location, type));
}

void Enemy::DestroyProjectile(int index)
{
	ProjectiletList.erase(ProjectiletList.begin() + (index - 1));
	ProjectileCount = (int)ProjectiletList.size();

	for (int i = index - 1; i < ProjectiletList.size(); i++)
	{
		ProjectiletList[i]->index--;
	}
}

void Enemy::NotifyHit(Player* projectile, float amount)
{
	if (bRecieveDamage)
	{
		Damage(amount);
		//PlaySound(SE_Damage, false);
		AudioManager::PlaySE(SE_Damage);
		bRecieveDamage = false;
	}
}

void Enemy::Damage(float amount)
{
	if (HP > 0.0f)
	{
		HP -= amount;
	}
}

// 最後に解放するからdeleteしないこと
void Enemy::Dead()
{
	//conioex2::StopSound(0);
	AudioManager::StopStresm(Game::GetGameInstance()->BGM_Main);
	conioex2::StopVibrate();
	Game::GetGameInstance()->OpenLevel(ELevel::Clear);
}
