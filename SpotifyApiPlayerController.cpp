// Copyright Epic Games, Inc. All Rights Reserved.


#include "SpotifyApiPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

#include "Kismet/GameplayStatics.h" //allows this to find the player

void ASpotifyApiPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}

	if (UWorld* world = GetWorld()) //checks if the world exists
	{
		UE_LOG(LogTemp, Warning, TEXT("FIND ME! found world"));
		APlayerController* pc = UGameplayStatics::GetPlayerController(world, 0); //gets the player controller

		if (pc)
		{
			UE_LOG(LogTemp, Warning, TEXT("FIND ME! found pc"));
			TSubclassOf<UupdateDebugDisplay> widgetclass = LoadClass<UupdateDebugDisplay>(nullptr, TEXT("/Game/DebugDisplay.DebugDisplay_C")); //finds the actual widget AND MAKES IT WORK
			widget = CreateWidget<UupdateDebugDisplay>(pc, widgetclass); //creates the widget
			
			if (widget) //if it was created properly
			{
				UE_LOG(LogTemp, Warning, TEXT("FIND ME! made widget"));
				widget->AddToViewport(); //add it to the gui

				UE_LOG(LogTemp, Warning, TEXT("FIND ME! looking for gi"));
				gi = Cast<UgameInstanceforretrievingdata>(UGameplayStatics::GetGameInstance(world)); //finds the game instance
				
				if (gi)
				{
					UE_LOG(LogTemp, Warning, TEXT("FIND ME! found gi!"));
					//widget->setCodeText(gi->accessToken); //set the text to the code
					
				}
				
			}
		}
		else if (pc == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("FIND ME! null pc"));
		}
	}
}

void ASpotifyApiPlayerController::setDebugText()
{
	FString debugText = "Top Artists: ";

	for (FspotifyArtist artist : gi->topArtists)
	{
		debugText += artist.name;
	}
	widget->setCodeText(debugText);
}