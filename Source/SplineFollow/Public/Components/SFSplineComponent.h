#pragma once

#include "SFSplineMarkers.h"

#include <Components/SplineComponent.h>
#include <CoreMinimal.h>

#include "SFSplineComponent.generated.h"

UCLASS( ClassGroup = ( "Custom" ), meta = ( BlueprintSpawnableComponent ) )
class SPLINEFOLLOW_API USFSplineComponent final : public USplineComponent
{
    GENERATED_BODY()

public:
#if WITH_EDITOR
    friend class FSFSplineMarkerComponentVisualizer;
#endif

    const TArray< FSFSplineMarkerProxy > & GetSplineMarkerProxies() const;

    UFUNCTION( BlueprintCallable, BlueprintPure )
    TArray< FSFSplineMarker > GetMarkersByObjectType( TSubclassOf< USFSplineMarkerObject > object_type ) const;

    UFUNCTION( BlueprintCallable, BlueprintPure )
    TArray< FSFSplineMarker > GetActionMarkers() const;

    UFUNCTION( BlueprintCallable, BlueprintPure )
    TArray< FSFSplineMarker > GetLevelActorMarkers() const;

    UFUNCTION( BlueprintCallable, BlueprintPure )
    TArray< FSFSplineMarker > GetDataMarkers() const;

    void OnRegister() override;

    void UpdateSpline() override;

#if WITH_EDITOR
    UFUNCTION( BlueprintCallable )
    void CreateMarkerFromDefault( const FSFSplineMarker & default_marker, FSFSplineMarker & new_marker );
    UFUNCTION( BlueprintCallable )
    void SaveSplineMarkers( const TArray< FSFSplineMarker > & markers_to_save );

    void CheckForErrors() override;
#endif

private:
    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    TArray< FSFSplineMarker > SplineMarkers;

    UPROPERTY()
    TArray< FSFSplineMarker_Static > StaticActionMarkers;

    UPROPERTY()
    TArray< FSFSplineMarker_LevelActor > LevelActorActionMarkers;

    UPROPERTY()
    TArray< FSFSplineMarker_Data > DataMarkers;

    TArray< FSFSplineMarkerProxy > SplineMarkerProxies;
};

FORCEINLINE const TArray< FSFSplineMarkerProxy > & USFSplineComponent::GetSplineMarkerProxies() const
{
    return SplineMarkerProxies;
}