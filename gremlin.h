// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "gremlinAI.h"
#include "gremlin.generated.h"

UCLASS()
class SPOTIFYAPI_API Agremlin : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	Agremlin();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//health (just a one shot but can be changed if needed)
	UPROPERTY(EditAnywhere)
	float health = 10.f;

	//overlap function to detect the player
	UFUNCTION()
	void onOverlapWithPlayer(UPrimitiveComponent* OverlappedComponent, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//to detect when it has an AI controller
	virtual void PossessedBy(AController* NewController) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//detects when it overlaps with a projectile
	UFUNCTION()
	void onHitByProjectile();

};
