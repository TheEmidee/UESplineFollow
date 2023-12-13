#pragma once

#include <Components/SplineComponent.h>
#include <CoreMinimal.h>

#include "SFSplineComponent.generated.h"

UCLASS( ClassGroup = ( "Custom" ), meta = ( BlueprintSpawnableComponent ) )
class SPLINEFOLLOW_API USFSplineComponent final : public USplineComponent
{
    GENERATED_BODY()

public:
#if WITH_EDITOR
    friend class FSWSplineMarkerComponentVisualizer;
#endif

    const TArray< FSWSplineMarkerProxy > & GetSplineMarkerProxies() const;

    void OnRegister() override;

    void UpdateSpline() override;

#if WITH_EDITOR
    void CheckForErrors() override;
#endif

private:
    UPROPERTY( EditAnywhere )
    TArray< FSWSplineMarker_Static > StaticActionMarkers;

    UPROPERTY( EditAnywhere )
    TArray< FSWSplineMarker_LevelActor > LevelActorActionMarkers;

    TArray< FSWSplineMarkerProxy > SplineMarkerProxies;
};

FORCEINLINE const TArray< FSWSplineMarkerProxy > & USFSplineComponent::GetSplineMarkerProxies() const
{
    return SplineMarkerProxies;
}