// Fill out your copyright notice in the Description page of Project Settings.


#include "gameInstanceforretrievingdata.h"

//image stuff
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "Rendering/Texture2DResource.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "ImageUtils.h"
#include "Engine/Texture2D.h"  
#include "ImageCore.h"
#include "Misc/Paths.h" 
#include "Async/Async.h"  

#include "Misc/CommandLine.h" //to access the command line
#include "Misc/Parse.h" //to allow the reading of json?
#include "Kismet/GameplayStatics.h" //allows this to find the player
#include "Logging/LogMacros.h"



void UgameInstanceforretrievingdata::Init() //runs at the start of the game
{
    Super::Init();
    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! initialised"));
    parseSpotifyCode(); //calls the function
    completionCount = 0;
}

void UgameInstanceforretrievingdata::parseSpotifyCode()
{
    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! called function"));
    FString cmdline = FCommandLine::Get(); //gets the command line and stores as a string
    FString authCode;

    if (FParse::Value(*cmdline, TEXT("code="), authCode)) //if it can find the authanticator code
    {
        //UE_LOG(LogTemp, Warning, TEXT("FIND ME! found code"));
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
    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! called token function"));
    if(!successful || !response.IsValid()) return; //if the response is unsucceeful or empty, exit

    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! looking for access token..."));
    TSharedPtr<FJsonObject> jsonobj;
    TSharedRef<TJsonReader<>> jsonreader = TJsonReaderFactory<>::Create(response->GetContentAsString()); //gets the json response text and adds it to the JSON object

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
    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! called artist function"));
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

    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! looking for artists..."));
    TSharedPtr<FJsonObject> jsonobj;
    TSharedRef<TJsonReader<>> jsonreader = TJsonReaderFactory<>::Create(response->GetContentAsString()); //gets the json response text and adds it to the JSON object

    if (FJsonSerializer::Deserialize(jsonreader, jsonobj)) //checks for the json file and finds all the variables
    {
        topArtists.Empty(); //empties the array

        TSharedPtr<FJsonObject> errorObject = jsonobj->GetObjectField(TEXT("error")); //this checks for an error message
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
                TSharedPtr<FJsonObject> imageobj = images[0]->AsObject(); //gets the standard image (first in the array)
                if (imageobj.IsValid() && imageobj->HasField(TEXT("url")))
                {
                    artist.imageURL = images[0]->AsObject()->GetStringField(TEXT("url"));
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! found image url: %s"), *artist.imageURL);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("FIND ME! no url"));
                }
            }

            //UE_LOG(LogTemp, Warning, TEXT("FIND ME! initial top song name: %s"), *artist.topSong.name);
            topArtists.Add(artist);
        }
    }

    for (FspotifyArtist& a : topArtists) //gets the top song and (should) get the image for each artist
    {
        getTopSong(a); //a is a reference to the artist
        getArtistImage(a);
    }
}

void UgameInstanceforretrievingdata::getTopSong(FspotifyArtist& artist)
{
    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! called song function"));
    FHttpModule* http = &FHttpModule::Get(); //gets the global http module
    FHttpRequestPtr request = http->CreateRequest(); //creates an http request object

    //gets the top artists
    request->SetURL("https://api.spotify.com/v1/artists/" + artist.id + "/top-tracks?market=GB"); //sets the url to request the top songs for the current artist
    request->SetVerb("GET"); //this sets the method (in this case retreiving the data)
    request->SetHeader("Authorization", "Bearer " + accessToken); //uses the access token
    request->SetHeader("Content-Type", "application/json"); //sets the response type

    request->OnProcessRequestComplete().BindLambda( //this passes the referenced artist to the response function
        [this, &artist](FHttpRequestPtr request, FHttpResponsePtr response, bool successful)
        {
            this->onTopSongResponse(request, response, successful, artist);
        }
    );
    request->ProcessRequest(); //sends the request
}

void UgameInstanceforretrievingdata::onTopSongResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful, FspotifyArtist& artist)
{
    if(!successful || !response.IsValid()) return; //if the response is unsucceeful or empty, exit

    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! looking for songs..."));
    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! artist is: %s"), *artist.name);
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

        //this is to know when the broadcast can go out (when everything is retreived)
        completionCount++;
        UE_LOG(LogTemp, Warning, TEXT("FIND ME! completionCount: %d"), completionCount);

        if (completionCount == 10)
        {
            completionCount++; //this is to make sure only one broadcast goes out
            UE_LOG(LogTemp, Warning, TEXT("FIND ME! completionCount: %d"), completionCount);
            UE_LOG(LogTemp, Warning, TEXT("FIND ME! broadcast!"));
            onFinishedRetrieve.Broadcast();
        }
    }
}

void UgameInstanceforretrievingdata::getArtistImage(FspotifyArtist& artist)
{
    //HTTP module and object
    FHttpModule* http = &FHttpModule::Get();
    FHttpRequestPtr request = http->CreateRequest();

    request->SetURL(artist.imageURL); //setting the correct url endpoint
    request->SetVerb("GET");
    request->OnProcessRequestComplete().BindLambda( //this passes the referenced artist to the response function
        [this, &artist](FHttpRequestPtr request, FHttpResponsePtr response, bool successful)
        {
            this->onArtistImageResponse(request, response, successful, artist);
        }
    );
    request->ProcessRequest();
}


void UgameInstanceforretrievingdata::onArtistImageResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful, FspotifyArtist& artist) //works perfectly fine
{
    if (!successful || !response.IsValid()) return;

    AsyncTask(ENamedThreads::GameThread, [=, this, &artist]() mutable
    {
        //this dowloads the image from a url

        const TArray<uint8>& ImageData = response->GetContent();

        //saving the image
        FString savePath = FPaths::ProjectSavedDir() / (artist.name + TEXT("_image.png"));
        FFileHelper::SaveArrayToFile(ImageData, *savePath);
        
        //this loads the image
        FImage loadedImage;
        if (!FImageUtils::LoadImage(*savePath, loadedImage))
        {
            UE_LOG(LogTemp, Error, TEXT("FIND ME! failed to load image, %s"), *savePath);
        }

        //checking the format (which is correct)
        if (loadedImage.Format != ERawImageFormat::BGRA8)
        {
            UE_LOG(LogTemp, Error, TEXT("FIND ME! not correct format"));
            return;
        }

        //creating a Texture2D
        UTexture2D* Texture = UTexture2D::CreateTransient(loadedImage.SizeX, loadedImage.SizeY, PF_R8G8B8A8);
        if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
        {
            UE_LOG(LogTemp, Error, TEXT("FIND ME! Texture creation failed"));
            return;
        }

        //modifying low-level data (annoyingly)
        /*
        Texture->SRGB = true;
        Texture->NeverStream = true;
        Texture->AddToRoot();
        */

        // Copy pixel data
        void* TextureMemory = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
        FMemory::Memcpy(TextureMemory, loadedImage.RawData.GetData(), loadedImage.RawData.Num());
        Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
        Texture->UpdateResource();

        artist.image = Texture; //set the Texture2D reference
        
        //this is to know when the broadcast can go out (when everything is retrieved)
        completionCount++;
        UE_LOG(LogTemp, Warning, TEXT("FIND ME! completionCount: %d"), completionCount);

        if (completionCount == 10)
        {
            completionCount++;
            UE_LOG(LogTemp, Warning, TEXT("FIND ME! completionCount: %d"), completionCount);
            UE_LOG(LogTemp, Warning, TEXT("FIND ME! broadcast!"));
            onFinishedRetrieve.Broadcast();
        }
    });
}

void UgameInstanceforretrievingdata::getCurrentSong(int32 num)
{
    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! called artist function"));
    FHttpModule* http = &FHttpModule::Get(); //gets the global http module
    FHttpRequestPtr request = http->CreateRequest(); //creates an http request object

    //gets the top artists
    request->SetURL("https://api.spotify.com/v1/me/player"); //sets the url to request the top artists for the current user
    request->SetVerb("GET"); //this sets the method (in this case retreiving the data)
    request->SetHeader("Authorization", "Bearer " + accessToken); //uses the access token
    request->SetHeader("Content-Type", "application/json"); //sets the response type

    request->OnProcessRequestComplete().BindLambda( //this passes the num to the response function
        [this, num](FHttpRequestPtr request, FHttpResponsePtr response, bool successful)
        {
            this->onCurrentSongResponse(request, response, successful, num);
        }
    );
    request->ProcessRequest(); //sends the request
}

void UgameInstanceforretrievingdata::onCurrentSongResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful, int32 num)
{
    if(!successful || !response.IsValid()) return; //if the response is unsucceeful or empty, exit

    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! looking for artists..."));
    TSharedPtr<FJsonObject> jsonobj;
    TSharedRef<TJsonReader<>> jsonreader = TJsonReaderFactory<>::Create(response->GetContentAsString()); //gets the json response text and adds it to the JSON object

    if (FJsonSerializer::Deserialize(jsonreader, jsonobj)) //checks for the json file and finds all the variables
    {
        TSharedPtr<FJsonObject> errorObject = jsonobj->GetObjectField(TEXT("error")); //this checks for an error message
        if (errorObject.IsValid())
        {
            int32 status = errorObject->GetIntegerField(TEXT("status"));
            FString message = errorObject->GetStringField(TEXT("message"));

            UE_LOG(LogTemp, Warning, TEXT("Status: %d, Message: %s"), status, *message);
        }

        const TArray<TSharedPtr<FJsonValue>> details = jsonobj->GetArrayField(TEXT("item"));
        currentsongid = jsonobj->GetStringField(TEXT("id"));
        gottenCurrentPlayback.Broadcast(num);
    }
}

void UgameInstanceforretrievingdata::queueSpotifyTrack(FString uri) //adds the specified track to the queue
{
    FHttpRequestRef request = FHttpModule::Get().CreateRequest();
    request->SetURL("https://api.spotify.com/v1/me/player/queue?uri=" + uri);
    request->SetVerb("POST");
    request->SetHeader("Authorization", "Bearer " + accessToken);
    request->ProcessRequest();
}

void UgameInstanceforretrievingdata::skipTrack() //skips the current track
{
    FHttpRequestRef request = FHttpModule::Get().CreateRequest();
    request->SetURL("https://api.spotify.com/v1/me/player/next");
    request->SetVerb("POST");
    request->SetHeader("Authorization", "Bearer " + accessToken);
    request->ProcessRequest();
}

void UgameInstanceforretrievingdata::play() //plays the current track
{
    FHttpRequestRef request = FHttpModule::Get().CreateRequest();
    request->SetURL("https://api.spotify.com/v1/me/player/play");
    request->SetVerb("POST");
    request->SetHeader("Authorization", "Bearer " + accessToken);
    request->ProcessRequest();
}

//the graveyard :(

/*ATTEMPT 3
void UgameInstanceforretrievingdata::onArtistImageResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful, FspotifyArtist& artist)
{
    if (!successful || !response.IsValid()) return;

    AsyncTask(ENamedThreads::GameThread, [=, this, &artist]() mutable
    {
        const TArray<uint8>& ImageData = response->GetContent();

        IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
        EImageFormat Format = ImageWrapperModule.DetectImageFormat(ImageData.GetData(), ImageData.Num());
        TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);

        if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(ImageData.GetData(), ImageData.Num()))
        {
            TArray<uint8> RawData;
            if (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, RawData))
            {
                int32 expectedsize = ImageWrapper->GetWidth() * ImageWrapper->GetHeight() * 4;
                if (RawData.Num() != expectedsize)
                {
                    UE_LOG(LogTemp, Error, TEXT("FIDE ME! rawdata size not correct!"));
                }
                UTexture2D* Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_R8G8B8A8);
                if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
                {
                    UE_LOG(LogTemp, Error, TEXT("FIDE ME! Texture creation failed or Mip data missing!"));
                }
                Texture->SRGB = false;
                Texture->NeverStream = true;
                Texture->AddToRoot();

                void* TextureMemory = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
                FMemory::Memcpy(TextureMemory, RawData.GetData(), RawData.Num());
                Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
                Texture->UpdateResource();

                /*
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! ImageData size: %d"), ImageData.Num());
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! Detected format: %d"), (int32)Format);
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! ImageWrapper valid: %d"), ImageWrapper.IsValid());
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! SetCompressed success: %d"), ImageWrapper->SetCompressed(ImageData.GetData(), ImageData.Num()));
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! RawData size: %d"), RawData.Num());
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! Image dimensions: %d x %d"), ImageWrapper->GetWidth(), ImageWrapper->GetHeight());
                

                UE_LOG(LogTemp, Warning, TEXT("FIND ME! texture pointer %p"), Texture);
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! Texture resource: %p"), (void*)Texture->GetResource());
                artist.image = Texture;
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! artist texture pointer %p"), artist.image);

                completionCount++;
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! completionCount: %d"), completionCount);

                if (completionCount == 10)
                {
                    completionCount++;
                    UE_LOG(LogTemp, Warning, TEXT("FIND ME! completionCount: %d"), completionCount);
                    UE_LOG(LogTemp, Warning, TEXT("FIND ME! broadcast!"));
                    onFinishedRetrieve.Broadcast();
                }
            }
        }
    });  
}
*/

/* ATTEMPT 2
void UgameInstanceforretrievingdata::onArtistImageResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful, FspotifyArtist& artist)
{
    if (!successful || !response.IsValid()) return;

    const TArray<uint8>& ImageData = response->GetContent();

    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    EImageFormat Format = ImageWrapperModule.DetectImageFormat(ImageData.GetData(), ImageData.Num());
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);

    if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(ImageData.GetData(), ImageData.Num()))
    {
         TArray<uint8> RawData;
        if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
        {
            FImageView View;
            View.RawData = RawData.GetData();
            View.Width = ImageWrapper->GetWidth();
            View.Height = ImageWrapper->GetHeight();
            View.Format = ERawImageFormat::BGRA8;

            UTexture2D* Texture = FImageUtils::CreateTexture2DFromImage(View);

            if (Texture)
            {
                artist.image = Texture;

                completionCount++;
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! completionCount: %d"), completionCount);

                if (completionCount == 10)
                {
                    completionCount++;
                    UE_LOG(LogTemp, Warning, TEXT("FIND ME! completionCount: %d"), completionCount);
                    UE_LOG(LogTemp, Warning, TEXT("FIND ME! broadcast!"));
                    onFinishedRetrieve.Broadcast();
                }
            }
        }
    }

    
}*/

/* ATTEMPT 1
void UgameInstanceforretrievingdata::onArtistImageResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool successful, FspotifyArtist& artist)
{
    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! start image return"));
    if(!successful || !response.IsValid()) return; //if the response is unsucceeful or empty, exit

    //UE_LOG(LogTemp, Warning, TEXT("FIND ME! found image request!"));

    UTexture2D* tempimage;

    const TArray<uint8>& imageData = response->GetContent();
    
    IImageWrapperModule& iwm = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    EImageFormat format = iwm.DetectImageFormat(imageData.GetData(), imageData.Num());
    TSharedPtr<IImageWrapper> iw = iwm.CreateImageWrapper(format);

    if (iw.IsValid() && iw->SetCompressed(imageData.GetData(), imageData.Num()))
    {
        //UE_LOG(LogTemp, Warning, TEXT("FIND ME! found iw"));
        TArray<uint8> rawData;
        /*
        rawData.SetNum(iw->GetWidth() * iw->GetHeight() * 4);
        for (int32 i = 0; i < rawData.Num(); i += 4)
        {
            rawData[i] = 255;     // B
            rawData[i+1] = 0;     // G
            rawData[i+2] = 0;     // R
            rawData[i+3] = 255;   // A
        }
        if (iw->GetRaw(ERGBFormat::BGRA, 8, rawData))
        {
            tempimage = UTexture2D::CreateTransient(
                iw->GetWidth(),
                iw->GetHeight(),
                PF_B8G8R8A8
            );

            tempimage->SRGB = true;
            tempimage->NeverStream = true;
            tempimage->AddToRoot();
            tempimage->CompressionSettings = TC_Default;
            tempimage->Filter = TF_Default;   
            
            FTexture2DMipMap& mip = tempimage->GetPlatformData()->Mips[0];
            mip.SizeX = iw->GetWidth();
            mip.SizeY = iw->GetHeight();
            void* textureData = mip.BulkData.Lock(LOCK_READ_WRITE);
            //FMemory::Memcpy(textureData, rawData.GetData(), rawData.Num());
            mip.BulkData.Unlock();
            tempimage->UpdateResource();

            UE_LOG(LogTemp, Warning, TEXT("FIND ME! Image dimensions: %d x %d"), iw->GetWidth(), iw->GetHeight());
            UE_LOG(LogTemp, Warning, TEXT("FIND ME! Raw data size: %d"), rawData.Num());

            if (tempimage)
            {
                //UE_LOG(LogTemp, Warning, TEXT("FIND ME! alomst have the image"));
                void* td = tempimage->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE); //gets the mipmap level of pixels
                FMemory::Memcpy(td, rawData.GetData(), rawData.Num()); //compies the pixels into the tecture's memory
                tempimage->GetPlatformData()->Mips[0].BulkData.Unlock(); //makes it ready for use

                tempimage->UpdateResource();

                UMaterialInterface* BaseMaterial = Cast<UMaterialInterface>(
                    StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Script/Engine.Material'/Game/artistPfp.artistPfp'"))
                );
                UMaterialInstanceDynamic* dyMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                if (dyMat)
                {
                    dyMat->SetTextureParameterValue("textureParam", tempimage);

                    UTextureRenderTarget2D* renderTarget = NewObject<UTextureRenderTarget2D>();
                    renderTarget->InitAutoFormat(iw->GetWidth(), iw->GetHeight());
                    renderTarget->UpdateResource(); // Make sure it's ready
                    renderTarget->UpdateResourceImmediate(true);
                    UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, renderTarget, dyMat);
                    // Or use DrawMaterialToRenderTarget if needed
                    // Save to disk
                    FString savePath = FPaths::ProjectSavedDir() + artist.name + TEXT("_debug_texture.png");
                    TUniquePtr<FArchive> fileWriter(IFileManager::Get().CreateFileWriter(*savePath));
                    if (fileWriter)
                    {
                        FImageUtils::ExportRenderTarget2DAsPNG(renderTarget, *fileWriter);
                        fileWriter->Close(); // Optional but good practice
                    }
                }

                artist.image = tempimage;
                UE_LOG(LogTemp, Warning, TEXT("FIND ME! GOT THE IMAGE"));
            }
        }
    }

    completionCount++;
    UE_LOG(LogTemp, Warning, TEXT("FIND ME! completionCount: %d"), completionCount);

    if (completionCount == 10)
    {
        completionCount++;
        UE_LOG(LogTemp, Warning, TEXT("FIND ME! completionCount: %d"), completionCount);
        UE_LOG(LogTemp, Warning, TEXT("FIND ME! broadcast!"));
        onFinishedRetrieve.Broadcast();
    }
}
*/