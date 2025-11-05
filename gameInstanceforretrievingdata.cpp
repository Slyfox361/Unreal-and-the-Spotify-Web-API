// Fill out your copyright notice in the Description page of Project Settings.


#include "gameInstanceforretrievingdata.h"
#include "Misc/CommandLine.h" //to access the command line
#include "Misc/Parse.h" //to allow the reading of json?
#include "Kismet/GameplayStatics.h" //allows this to find the player
#include "Logging/LogMacros.h"



void UgameInstanceforretrievingdata::Init() //runs at the start of the game
{
    Super::Init();
    UE_LOG(LogTemp, Warning, TEXT("FIND ME! initialised"));
    parseSpotifyCode(); //calls the function
}

void UgameInstanceforretrievingdata::parseSpotifyCode()
{
    UE_LOG(LogTemp, Warning, TEXT("FIND ME! called function"));
    FString cmdline = FCommandLine::Get(); //gets the command line and stores as a string
    FString authCode;

    if (FParse::Value(*cmdline, TEXT("code="), authCode)) //if it can find the authanticator code
    {
        UE_LOG(LogTemp, Warning, TEXT("FIND ME! found code"));
        UE_LOG(LogTemp, Warning, TEXT("Spotify Auth Code: %s"), *authCode); 

        //set the spotify web app details
        FString redirectURI = "myunrealapp://callback";
        FString clientID = "2f80b6cc7fea4977bef8869e2d1e2709";
        FString clientSecret = "e96186a6b74f4b2e8773cefc429626cc";

        FHttpModule* http = &FHttpModule::Get(); //gets the global http module
        FHttpRequestPtr request = http->CreateRequest(); //creates an http request object

        //gets the access token
        request->SetURL("https://accounts.spotify.com/api/token"); //setting the url to request an access token
        request->SetVerb("POST"); //setting the verd to send some data
        request->SetHeader("Content-Type", "application/x-www-form-urlencoded"); //defines the contents and the encoding rules

        FString body = FString::Printf( //setting the body with all required data
            TEXT("grant_type=authorization_code&code=%s&redirect_uri=%s&client_id=%s&client_secret=%s"),
            *authCode,
            *redirectURI,
            *clientID,
            *clientSecret
        );

        request->SetContentAsString(body); //setting the body
        request->OnProcessRequestComplete().BindUObject(this, &UgameInstanceforretrievingdata::onTokenResponse); //sets the function to respond
        request->ProcessRequest(); //sends the request
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("unable to find the code"));
    }
}

void UgameInstanceforretrievingdata::onTokenResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful)
{
    UE_LOG(LogTemp, Warning, TEXT("FIND ME! called token function"));
    if(!successful || !response.IsValid()) return; //if the response is unsucceeful or empty, exit

    UE_LOG(LogTemp, Warning, TEXT("FIND ME! looking for access token..."));
    TSharedPtr<FJsonObject> jsonobj;
    TSharedRef<TJsonReader<>> jsonreader = TJsonReaderFactory<>::Create(response->GetContentAsString()); //gets the json response text

    if (FJsonSerializer::Deserialize(jsonreader, jsonobj)) //checks for the json file and finds all the variables
    {
        UE_LOG(LogTemp, Warning, TEXT("FIND ME! found access token!"));
        accessToken = jsonobj->GetStringField(TEXT("access_token"));
        FString refreshToken = jsonobj->GetStringField(TEXT("refresh_token"));
        expires = jsonobj->GetIntegerField(TEXT("expires_in"));
        getTopArtists();
    }
}

void UgameInstanceforretrievingdata::getTopArtists()
{
    UE_LOG(LogTemp, Warning, TEXT("FIND ME! called artist function"));
    FHttpModule* http = &FHttpModule::Get(); //gets the global http module
    FHttpRequestPtr request = http->CreateRequest(); //creates an http request object

    //gets the top artists
    request->SetURL("https://api.spotify.com/v1/me/top/artists?limit=5&time_range=medium_term&country=GB"); //sets the url to request the top artists for the current user
    request->SetVerb("GET"); //this sets the method (in this case retreiving the data)
    request->SetHeader("Authorization", "Bearer " + accessToken); //uses the access token
    request->SetHeader("Content-Type", "application/json"); //sets the response type

    request->OnProcessRequestComplete().BindUObject(this, &UgameInstanceforretrievingdata::onTopArtistsResponse); //sets the function to respond
    request->ProcessRequest(); //sends the request
}

void UgameInstanceforretrievingdata::onTopArtistsResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful)
{
    if(!successful || !response.IsValid()) return; //if the response is unsucceeful or empty, exit

    UE_LOG(LogTemp, Warning, TEXT("FIND ME! looking for artists..."));
    TSharedPtr<FJsonObject> jsonobj;
    TSharedRef<TJsonReader<>> jsonreader = TJsonReaderFactory<>::Create(response->GetContentAsString()); //gets the json response text

    if (FJsonSerializer::Deserialize(jsonreader, jsonobj)) //checks for the json file and finds all the variables
    {
        topArtists.Empty(); //empties the array

        TSharedPtr<FJsonObject> errorObject = jsonobj->GetObjectField(TEXT("error"));
        if (errorObject.IsValid())
        {
            int32 status = errorObject->GetIntegerField(TEXT("status"));
            FString message = errorObject->GetStringField(TEXT("message"));

            UE_LOG(LogTemp, Warning, TEXT("Status: %d, Message: %s"), status, *message);
        }


        const TArray<TSharedPtr<FJsonValue>> artists = jsonobj->GetArrayField(TEXT("items"));

        for (const TSharedPtr<FJsonValue>& artistValue : artists)
        {
            TSharedPtr<FJsonObject> artistobj = artistValue->AsObject(); //sets the json file as an accessable object
            FspotifyArtist artist; //creates an artist struct

            artist.name = artistobj->GetStringField(TEXT("name")); //gets the name of the artist
            artist.id = artistobj->GetStringField(TEXT("id"));

            UE_LOG(LogTemp, Warning, TEXT("FIND ME! found: %s"), *artist.name);

            const TArray<TSharedPtr<FJsonValue>> images = artistobj->GetArrayField(TEXT("images")); //gets the images array (of the varying sizes)
            if (images.Num() > 0)
            {
                artist.imageURL = images[0]->AsObject()->GetStringField(TEXT("url"));
            }

            UE_LOG(LogTemp, Warning, TEXT("FIND ME! initial top song name: %s"), *artist.topSong.name);
            topArtists.Add(artist);
        }
    }

    for (FspotifyArtist& artist : topArtists)
    {
        getTopSong(artist);
    }
}

void UgameInstanceforretrievingdata::getTopSong(FspotifyArtist artist)
{
    UE_LOG(LogTemp, Warning, TEXT("FIND ME! called song function"));
    FHttpModule* http = &FHttpModule::Get(); //gets the global http module
    FHttpRequestPtr request = http->CreateRequest(); //creates an http request object

    //gets the top artists
    request->SetURL("https://api.spotify.com/v1/artists/" + artist.id + "/top-tracks?market=GB"); //sets the url to request the top songs for the current artist
    request->SetVerb("GET"); //this sets the method (in this case retreiving the data)
    request->SetHeader("Authorization", "Bearer " + accessToken); //uses the access token
    request->SetHeader("Content-Type", "application/json"); //sets the response type

    //request->OnProcessRequestComplete().BindUObject(this, &UgameInstanceforretrievingdata::onTopSongResponse); //sets the function to respond
    request->OnProcessRequestComplete().BindLambda(
        [this, artist](FHttpRequestPtr request, FHttpResponsePtr response, bool successful)
        {
            this->onTopSongResponse(request, response, successful, artist);
        }
    );
    request->ProcessRequest(); //sends the request
}

void UgameInstanceforretrievingdata::onTopSongResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful, FspotifyArtist artist)
{
    if(!successful || !response.IsValid()) return; //if the response is unsucceeful or empty, exit

    UE_LOG(LogTemp, Warning, TEXT("FIND ME! looking for songs..."));
    UE_LOG(LogTemp, Warning, TEXT("FIND ME! artist is: %s"), *artist.name);
    TSharedPtr<FJsonObject> jsonobj;
    TSharedRef<TJsonReader<>> jsonreader = TJsonReaderFactory<>::Create(response->GetContentAsString()); //gets the json response text

    if (FJsonSerializer::Deserialize(jsonreader, jsonobj)) //checks for the json file and finds all the variables
    {
        TSharedPtr<FJsonObject> errorObject = jsonobj->GetObjectField(TEXT("error")); //checks for any errors
        if (errorObject.IsValid())
        {
            int32 status = errorObject->GetIntegerField(TEXT("status"));
            FString message = errorObject->GetStringField(TEXT("message"));

            UE_LOG(LogTemp, Warning, TEXT("Status: %d, Message: %s"), status, *message);
        }


        const TArray<TSharedPtr<FJsonValue>> songs = jsonobj->GetArrayField(TEXT("tracks")); //gets the tracks array

        TSharedPtr<FJsonObject> topSongObj = songs[0]->AsObject(); //sets the json file as an accessable object

        artist.topSong.name = topSongObj->GetStringField(TEXT("name")); //gets the name of the artist
        artist.topSong.id = topSongObj->GetStringField(TEXT("id"));

        UE_LOG(LogTemp, Warning, TEXT("FIND ME! found: %s"), *artist.topSong.name);
    }

    for (int i=0;i<5;i++)
    {
        if (topArtists[i].id == artist.id)
        {
            topArtists[i] = artist;
            break;
        }
    }
}
