// Fill out your copyright notice in the Description page of Project Settings.


#include "npcScript.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "ImageUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "GameFramework/Actor.h"
#include "../SpotifyApiCharacter.h" 

#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UnpcScript::UnpcScript()
{
	PrimaryComponentTick.bCanEverTick = false;

	//static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
}


// Called when the game starts
void UnpcScript::BeginPlay()
{
	Super::BeginPlay();

	//finding all the components
	TArray<UStaticMeshComponent*> meshes;
	GetOwner()->GetComponents<UStaticMeshComponent>(meshes);
	
	for (UStaticMeshComponent* mesh : meshes)
	{
		if (mesh->GetName().Contains("head"))
		{
			head = mesh;
		}
	}

	TArray<UTextRenderComponent*> texts;
	GetOwner()->GetComponents<UTextRenderComponent>(texts);

	for (UTextRenderComponent* t : texts)
	{
		if (t->GetName().Contains("speechText"))
		{
			speech = t;
		}
	}

	if (UWorld* world = GetWorld()) //checks if the world exists
	{
		gi = Cast<UgameInstanceforretrievingdata>(UGameplayStatics::GetGameInstance(world)); //game instance class (where all the data is)
		if(gi)
		{
			gi->onFinishedRetrieve.AddDynamic(this, &UnpcScript::onDataRecieved); //binding the event
			gi->gottenCurrentPlayback.AddDynamic(this, &UnpcScript::playNextSong); //binding the event
		}
	}

	detectionSphere = GetOwner()->FindComponentByClass<USphereComponent>(); //gets the collision sphere
	if (detectionSphere)
	{
		UE_LOG(LogTemp, Warning, TEXT("FIND ME! Found detection sphere!"));
		detectionSphere->OnComponentBeginOverlap.AddDynamic(this, &UnpcScript::OnPlayerEnterSphere);
	}

	indexTracker = -1; //for the dialogue
}

void UnpcScript::onDataRecieved()
{
	UE_LOG(LogTemp, Warning, TEXT("FIND ME! recieved event dispatcher, %d"), assignedNumber);
	UMaterialInstanceDynamic* dyMat = UMaterialInstanceDynamic::Create(BaseMaterial, this); //creating a dynamic material
	if (dyMat && gi->topArtists[assignedNumber-1].image)
	{
		
		//UE_LOG(LogTemp, Warning, TEXT("FIND ME! found gi and the material, %d"), assignedNumber);

		//UE_LOG(LogTemp, Warning, TEXT("FIND ME! artist texture pointer 2: %p"), gi->topArtists[assignedNumber-1].image);

		UTexture2D* tex = gi->topArtists[assignedNumber-1].image; //getting the Texture2D reference

		//texture debug stuff
		if (!IsValid(tex))
		{
			UE_LOG(LogTemp, Error, TEXT("FIND ME! Texture is invalid"));
			return;
		}
		if (!tex->GetPlatformData() || tex->GetPlatformData()->Mips.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("FIND ME! Texture is missing mip data!"));
			return;
		}
		

		UE_LOG(LogTemp, Warning, TEXT("Assigning texture: %s"), *tex->GetName());
		dyMat->SetTextureParameterValue("textureParam", gi->topArtists[assignedNumber-1].image); //setting the Texture2D to the dynamic material
		/*
		FString Path = FString::Printf(TEXT("/Game/image%d.image%d"), assignedNumber, assignedNumber);
		UTexture2D* EditorTexture = Cast<UTexture2D>(
			StaticLoadObject(UTexture2D::StaticClass(), nullptr, *Path)
		);
		if (EditorTexture)
		{
			dyMat->SetTextureParameterValue("textureParam", EditorTexture);
			UE_LOG(LogTemp, Warning, TEXT("FIND ME! %d found Texture: %s"), assignedNumber, *EditorTexture->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("FIND ME! no texture found, %d"), assignedNumber);
		}
			*/

		if (head) //meant to set the head to a texture
		{
			UE_LOG(LogTemp, Warning, TEXT("FIND ME! found and set head, %d"), assignedNumber);
			head->SetMaterial(0, dyMat);
		}

		if (speech) //sets the speech text
		{
			UE_LOG(LogTemp, Warning, TEXT("FIND ME! found and set text, %d"), assignedNumber);
			speech->SetText(FText::FromString(gi->topArtists[assignedNumber-1].name));
		}
	}
	
}

//triggers the dialogue and song playback when the player gets close
void UnpcScript::OnPlayerEnterSphere(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	if (otherActor && otherActor != this->GetOwner() && otherActor->IsA(ASpotifyApiCharacter::StaticClass()) && speech && gi->topArtists[assignedNumber-1].image)
	{
		if (indexTracker == -1)
		{
			indexTracker = 0;
			gi->getCurrentSong(assignedNumber);
			dialogueChain();
		}
		
	}
}

void UnpcScript::playNextSong(int32 num)
{
	if (num == assignedNumber)
	{
		if (!(gi->topArtists[assignedNumber-1].topSong.id).IsEmpty())
		{
			if (gi->currentsongid != gi->topArtists[assignedNumber-1].topSong.id)
			{
				UE_LOG(LogTemp, Warning, TEXT("FIND ME! npc %d, song id: %s"), assignedNumber, *gi->topArtists[assignedNumber-1].topSong.id);
				FString trackURI = "spotify:track:" + gi->topArtists[assignedNumber-1].topSong.id; //creates the track uri from the id

				gi->queueSpotifyTrack(trackURI); //queues a track

				//small delay to let the track be queued
				FTimerHandle delayHandler;
				FTimerDelegate timerDelegate;

				timerDelegate.BindLambda([this]()
				{
					if (gi)
					{
						//skips to and plays the queued track
						gi->skipTrack();
						gi->play();
					}
				});
				GetWorld()->GetTimerManager().SetTimer(delayHandler, timerDelegate, 0.5f, false);
			}
		}
	}
}

//a recursive function that sets the text to pre-written dialogue
void UnpcScript::dialogueChain()
{
	UE_LOG(LogTemp, Warning, TEXT("FIND ME! started dialogue chain - %s"), *gi->topArtists[assignedNumber-1].name);
	FString message = gi->topArtists[assignedNumber-1].name + ":\n" + dialogue[indexTracker]; //gets the dialogue and formats it
	speech->SetText(FText::FromString(message)); //sets the text UI component to the given text
	indexTracker++;

	if (indexTracker < dialogue.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("FIND ME! if is true. index: %d"), indexTracker);
		GetWorld()->GetTimerManager().SetTimer( //delays the next sentence
			loopingTimer,
			this,
			&UnpcScript::dialogueChain,
			2.0f,
			true
		);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(loopingTimer); //ends the loop
		indexTracker = -1;
	}
}

// Called every frame
void UnpcScript::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

