// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include "Templates/SharedPointer.h"

#include "gameInstanceforretrievingdata.generated.h"

USTRUCT(BlueprintType)
struct FspotifyArtist
{
	GENERATED_BODY()

	FString name;

	FString imageURL;
};

UCLASS()
class SPOTIFYAPI_API UgameInstanceforretrievingdata : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override; //runs when the game is launched

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Spotify")
	FString accessToken; //the authernticator code

	void onTokenResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful); //recieves the response and decodes it

	void onTopArtistsResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful); //recieves the response for top artists and decodes it

	TArray<FspotifyArtist> topArtists;

	int32 expires;

private:
	void parseSpotifyCode(); //the decoding function

	void getTopArtists();

};
