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
    static void GetFloat( const FString & section, const FString & key, UPARAM( ref ) float & value );

    UFUNCTION( BlueprintCallable )
    static void SaveFloat( const FString & section, const FString & key, UPARAM( ref ) float & value );
};
