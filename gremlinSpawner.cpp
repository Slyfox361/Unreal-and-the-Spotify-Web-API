// Fill out your copyright notice in the Description page of Project Settings.


#include "gremlinSpawner.h"
#include "gremlin.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"



// Sets default values
AgremlinSpawner::AgremlinSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AgremlinSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (gremlinClass) //starts the spawner in intervals
	{
		GetWorld()->GetTimerManager().SetTimer(
			timerHandler,
			this,
			&AgremlinSpawner::spawnGremlins,
			spawnInterval,
			true
		);
	}

	spawnArea = Cast<UBoxComponent>(GetDefaultSubobjectByName(TEXT("spawnBox"))); //getting the collison box to know where to spawn the gremlins
	
}

//spawns gremlins after the interval
void AgremlinSpawner::spawnGremlins()
{
	int32 numToSpawn = FMath::RandRange(1,3); //random amount of gremlins to summon

	for (int32 i = 0; i < numToSpawn; i++)
	{
		//instatiating variables required
		FVector spawnPos = getSpawnPoint();
		FRotator spawnRot = FRotator::ZeroRotator;

		//ensuring they don't spawn in each other
		FActorSpawnParameters actorSpawningParams;
		actorSpawningParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		//summoning the gremlin and ensuring it's AI controller is assigned
		Agremlin* spawnedgremlin = GetWorld()->SpawnActor<Agremlin>(gremlinClass, spawnPos, spawnRot, actorSpawningParams);
		if(spawnedgremlin)
		{
			spawnedgremlin->SpawnDefaultController();
		}
	}
}

FVector AgremlinSpawner::getSpawnPoint()
{
	//finds a random point in the spawn area
	FVector center = spawnArea->Bounds.Origin;
	FVector size = spawnArea->Bounds.BoxExtent;

	return UKismetMathLibrary::RandomPointInBoundingBox(center, size);
}

// Called every frame
void AgremlinSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

