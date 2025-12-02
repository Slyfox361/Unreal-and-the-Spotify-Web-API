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
#include "PixelFormat.h"

#include "gameInstanceforretrievingdata.generated.h"

class UTexture2D;

//song structure
USTRUCT(BlueprintType)
struct FspotifySong
{
	GENERATED_BODY()

	FString name; //for debug

	FString id; //spotify ID to queue and play it
};

//artist structure
USTRUCT(BlueprintType)
struct FspotifyArtist
{
	GENERATED_BODY()

	FString name; //artist (user)name

	FString imageURL; //profile picture image

	UPROPERTY(BlueprintReadWrite)
	UTexture2D* image; //Texture2D to allow image to be used in the scene

	FString id; //spotify ID to find the top song

	FspotifySong topSong; //associated top song
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FonFinishedRetrieve);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FgottenCurrentPlayback, int32, num);

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

	void onTopSongResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful, FspotifyArtist& artist); //recieves the response for top songs and decodes it

	void onArtistImageResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful, FspotifyArtist& artist); //recieves the response for the artist's downlaoded image and tries to convert it to a Texture2D

	void onCurrentSongResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful, int32 num); //recieves the response for the current playback

	//controlling playback
	void getCurrentSong(int32 num); //gets the song currently playing
	FString currentsongid;
	void queueSpotifyTrack(FString uri);
	void skipTrack();
	void play();

	UPROPERTY(BlueprintAssignable)
	FonFinishedRetrieve onFinishedRetrieve; //event dispatcher for when all data is collected

	UPROPERTY(BlueprintAssignable)
	FgottenCurrentPlayback gottenCurrentPlayback; //event dispatcher for when the current song playing has been retrieved

	TArray<FspotifyArtist> topArtists; //array of the 5 top artists

	int32 expires; //expiration of the accessToken

	int32 completionCount; //for the event dispatcher

private:
	void parseSpotifyCode(); //the inital decoding function to get access

	void getTopArtists(); //gets the user's top 5 artists

	void getTopSong(FspotifyArtist& artist); //gets the user's top 5 artists' top song

	void getArtistImage(FspotifyArtist& artist); //IT DOES download the image from the url

	

};
