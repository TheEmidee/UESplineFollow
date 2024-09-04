#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "SFSavePropertiesHelperLibrary.generated.h"

UCLASS()
class SPLINEFOLLOWEDITOR_API USFSavePropertiesHelperLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    static void GetArray( const FString & section, const FString & key, UPARAM( ref ) TArray< FString > & value );

    UFUNCTION( BlueprintCallable )
    static void SaveArray( const FString & section, const FString & key, UPARAM( ref ) TArray< FString > & value );

    UFUNCTION( BlueprintCallable )
    static void GetBool( const FString & section, const FString & key, UPARAM( ref ) bool & value );

    UFUNCTION( BlueprintCallable )
    static void SaveBool( const FString & section, const FString & key, UPARAM( ref ) bool & value );

    UFUNCTION( BlueprintCallable )
    static void GetColor( const FString & section, const FString & key, UPARAM( ref ) FColor & value );

    UFUNCTION( BlueprintCallable )
    static void SaveColor( const FString & section, const FString & key, UPARAM( ref ) FColor & value );

    UFUNCTION( BlueprintCallable )
    static void GetFloat( const FString & section, const FString & key, UPARAM( ref ) float & value );

    UFUNCTION( BlueprintCallable )
    static void SaveFloat( const FString & section, const FString & key, UPARAM( ref ) float & value );

    UFUNCTION( BlueprintCallable )
    static void GetString( const FString & section, const FString & key, UPARAM( ref ) FString & value );

    UFUNCTION( BlueprintCallable )
    static void SaveString( const FString & section, const FString & key, UPARAM( ref ) FString & value );
};
