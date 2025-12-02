// Fill out your copyright notice in the Description page of Project Settings.

#include "gremlinAI.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void AgremlinAI::BeginPlay() //doesn't activate for some reason??
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("FIND ME! Gremlin AI activated"));

}

void AgremlinAI::OnPossess(APawn* InPawn) //when it has activated on an object
{
	Super::OnPossess(InPawn);

	//getting the navmesh
	APawn* player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	if (NavSys && GetPawn())
	{
		//getting start and end positions
		FVector start = GetPawn()->GetActorLocation();
		FVector end = player->GetActorLocation();

		//finds a path to the player
		UNavigationPath* Path = NavSys->FindPathToActorSynchronously(GetWorld(), start, player);

		//checking if the path is valid
		if (Path && Path->IsValid() && Path->PathPoints.Num() > 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("Path valid: YES, points: %d"), Path->PathPoints.Num());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Path valid: NO"));
		}

		chasePlayer();
	}
}

void AgremlinAI::chasePlayer()
{
	APawn* player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (player)
	{
		UE_LOG(LogTemp, Warning, TEXT("FIND ME! gremlin found player"));

		MoveToActor(player, 5.f);

		EPathFollowingRequestResult::Type Result = MoveToActor(player); //trying to move to the player
		UE_LOG(LogTemp, Warning, TEXT("FIND ME! MoveToActor result: %d"), (int32)Result);

		if (Result == 0) //if they can't
		{
			FNavLocation ProjectedLocation;
			UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
			if (NavSys && NavSys->ProjectPointToNavigation(player->GetActorLocation(), ProjectedLocation)) //find the player's nearest position to the NavMesh
			{
				MoveToLocation(ProjectedLocation.Location); //move to that point
			}
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AgremlinAI::chasePlayer); //try again if unable to
		}
	}
}
