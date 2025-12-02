// Fill out your copyright notice in the Description page of Project Settings.

#include "gremlin.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SpotifyApiCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
Agremlin::Agremlin()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void Agremlin::BeginPlay()
{
	Super::BeginPlay();

	UCapsuleComponent* capsuleCol = GetCapsuleComponent();

	if(capsuleCol) //finding the gremlin's collider
	{
		UE_LOG(LogTemp, Warning, TEXT("FIND ME! found capsule col"));
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &Agremlin::onOverlapWithPlayer);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FIND ME! no capsule col :("));
	}

	//finding it's AI controller and verifying it
	AIControllerClass = AgremlinAI::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	UE_LOG(LogTemp, Warning, TEXT("FIND ME! Gremlin AIControllerClass is: %s"), *AIControllerClass->GetName());
}

void Agremlin::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    UE_LOG(LogTemp, Warning, TEXT("FIND ME! Gremlin possessed by: %s"), *NewController->GetName());
}


// Called every frame
void Agremlin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//ensure it looks at the player (for extra creep)
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	FRotator LookAtRotation = (Player->GetActorLocation() - this->GetActorLocation()).Rotation();
	LookAtRotation.Pitch = 0.f; //keep it upright
	LookAtRotation.Roll = 0.f;
	this->SetActorRotation(LookAtRotation);

}

void Agremlin::onOverlapWithPlayer(UPrimitiveComponent* OverlappedComponent, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (otherActor->IsA(ASpotifyApiCharacter::StaticClass())) //checking for the player
	{
		UE_LOG(LogTemp, Warning, TEXT("FIND ME! found player"));
		ASpotifyApiCharacter* player = Cast<ASpotifyApiCharacter>(otherActor);
		//hurts the player and destroys itself
		player->takeDamage(10.f);
		Destroy();
	}
}

void Agremlin::onHitByProjectile()
{
	//reduces health
	health -= 10.f;
	UE_LOG(LogTemp, Warning, TEXT("FIND ME! hit gremlin: %f"), health);

	if (health <= 0) //handles death
	{
		Destroy();
	}
}

/* Called to bind functionality to input <- Defauly Function not needed (i think)
void Agremlin::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
*/