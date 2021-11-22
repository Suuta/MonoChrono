
#pragma once
#include "conioex2.h"

struct Enemy;

enum EType
{
	Normal,
	Large,
	Spear,
};

struct Projectile
{
	Projectile(int index, Enemy* owner, Vector2 speed, Vector2 location, EType type);
	~Projectile();

	void Tick(float DeltaTime, float dilation);
	void Draw();
	void Move(float DeltaTime);
	void Destroy();

	bool bCanMove;

	int index;
	Vector2 Location;
	Vector2 Size;
	Vector2 Speed;
	AABB Collision;
	EType Type;

	conioex2::Engine::Image* Image;

	Enemy* Owner;
};

