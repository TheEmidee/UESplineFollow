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
    const TArray< FSFSplineMarker_Data > & GetSplineDataMarker() const;

    void OnRegister() override;

    void UpdateSpline() override;

#if WITH_EDITOR
    void CheckForErrors() override;
#endif

private:
    UPROPERTY( EditAnywhere )
    TArray< FSFSplineMarker_Static > StaticActionMarkers;

    UPROPERTY( EditAnywhere )
    TArray< FSFSplineMarker_LevelActor > LevelActorActionMarkers;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TArray< FSFSplineMarker_Data > DataMarkers;

    TArray< FSFSplineMarkerProxy > SplineMarkerProxies;
};

FORCEINLINE const TArray< FSFSplineMarkerProxy > & USFSplineComponent::GetSplineMarkerProxies() const
{
    return SplineMarkerProxies;
}

FORCEINLINE const TArray< FSFSplineMarker_Data > & USFSplineComponent::GetSplineDataMarker() const
{
    return DataMarkers;
}