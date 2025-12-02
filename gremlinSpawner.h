// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "gremlinSpawner.generated.h"

UCLASS()
class SPOTIFYAPI_API AgremlinSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AgremlinSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//the class it summons
	UPROPERTY(EditAnywhere)
	TSubclassOf<class Agremlin> gremlinClass;

	//how frequently it summons a gremlin
	UPROPERTY(EditAnywhere)
	float spawnInterval = 5.f;

	//how many can exist at once (don't think this is in use though...)
	UPROPERTY(EditAnywhere)
	int32 maxGremlins = 10;

	//the collision component denoting the spawn area
	UPROPERTY(EditAnywhere)
	UBoxComponent* spawnArea;

	//gets a random point in the spawn area
	FVector getSpawnPoint();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//handles the spawning delay
	FTimerHandle timerHandler;
	int32 spawnedCount = 0;

	//instantiates a random amount of gremlins
	void spawnGremlins();

};
