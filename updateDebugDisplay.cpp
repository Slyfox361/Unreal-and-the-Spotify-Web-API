// Fill out your copyright notice in the Description page of Project Settings.


#include "updateDebugDisplay.h"
#include "Components/TextBlock.h"

void UupdateDebugDisplay::setCodeText(const FString& texttoadd)
{
    if (!texttoadd.IsEmpty() && debugText) //checks it actually has some text
    {
        debugText->SetText(FText::FromString(texttoadd)); //adds the text to the widget
    }
    else if (!debugText)
    {
        UE_LOG(LogTemp, Warning, TEXT("FIND ME! FUCK no debugText TT"));
    }
    else if (texttoadd.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("FIND ME! FUCK no texttoadd TT"));
    }
}