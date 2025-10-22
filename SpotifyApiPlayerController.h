// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "updateDebugDisplay.h" //finds the debug widget
#include "gameInstanceforretrievingdata.h" //finds the game instance code
#include "SpotifyApiPlayerController.generated.h"

class UInputMappingContext;

/**
 *
 */
UCLASS()
class SPOTIFYAPI_API ASpotifyApiPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	void setDebugText();

	UgameInstanceforretrievingdata* gi;
	UupdateDebugDisplay* widget;

	// Begin Actor interface
protected:

	virtual void BeginPlay() override;

	// End Actor interface
};
