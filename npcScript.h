// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SphereComponent.h"
#include "Components/PrimitiveComponent.h"

#include "gameInstanceforretrievingdata.h"
#include "Components/TextRenderComponent.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "npcScript.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPOTIFYAPI_API UnpcScript : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UnpcScript();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//it's number, which is the position in the artist array it looks to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 assignedNumber;

	//the game instance with all the required data
	UgameInstanceforretrievingdata* gi;

	//it's collision sphere, used to detect the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* detectionSphere;

	//activates when a player enters the sphere and activates it's functions
	UFUNCTION()
	void OnPlayerEnterSphere(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);

	//displays the dialogue with a small delay between sentences
	UFUNCTION()
	void dialogueChain();

	UFUNCTION()
	void playNextSong(int32 num);

	//handles the delay mentioned above
	FTimerHandle loopingTimer;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//calls when it revieves the event dispatcher from the game instance (when all the data has been collected and assigned)
	UFUNCTION()
	void onDataRecieved();

	//components
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	UStaticMeshComponent* head;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* BaseMaterial;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	UTextRenderComponent* speech;

	//other variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> dialogue; //the current npcs dialogue chain (set in the editor)

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	int32 indexTracker; //tracks where we are in the array above

};
