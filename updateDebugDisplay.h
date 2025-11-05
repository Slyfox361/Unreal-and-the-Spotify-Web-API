// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "updateDebugDisplay.generated.h"

/**
 * 
 */
UCLASS()
class SPOTIFYAPI_API UupdateDebugDisplay : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void setCodeText(const FString& texttoadd); //function to set the code in the widget

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* debugText; //gets access to the text block to change it
	
};
