// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "gremlinAI.generated.h"

/**
 * 
 */
UCLASS()
class SPOTIFYAPI_API AgremlinAI : public AAIController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	//for when it has attached to a gremlin
	virtual void OnPossess(APawn* InPawn) override;

	//finds and follows the player using a NavMesh
	void chasePlayer();

};
