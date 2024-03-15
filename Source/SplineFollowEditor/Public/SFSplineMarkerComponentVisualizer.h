#pragma once

#include "Components/SFSplineMarkers.h"

#include <CoreMinimal.h>
#include <SplineComponentVisualizer.h>

class USFSplineComponent;
class UTexture2D;
class FPrimitiveDrawInterface;
class FSceneView;
class UActorComponent;

class SPLINEFOLLOWEDITOR_API FSFSplineMarkerComponentVisualizer final : public FSplineComponentVisualizer
{
public:
    FSFSplineMarkerComponentVisualizer();

    void DrawVisualization( const UActorComponent * component, const FSceneView * view, FPrimitiveDrawInterface * pdi ) override;

private:
    template < typename _MARKER_TYPE_ >
    void DrawMarkers( const TArray< _MARKER_TYPE_ > & markers, const USFSplineComponent & spline_component, FPrimitiveDrawInterface * pdi ) const
    {
        for ( const auto & marker : markers )
        {
            switch ( marker.Infos.Type )
            {
                case ESFSplineMarkerType::Single:
                {
                    FVector world_location;
                    DrawMarker( world_location, marker.Infos.SingleActionNormalizedSplineDistance, spline_component, marker, pdi );
                }
                break;
                case ESFSplineMarkerType::Window:
                {
                    FVector start_world_location, end_world_location;

                    const auto is_valid = marker.Infos.WindowStartNormalizedSplineDistance < marker.Infos.WindowEndNormalizedSplineDistance;

                    DrawMarker( start_world_location, marker.Infos.WindowStartNormalizedSplineDistance, spline_component, marker, pdi, is_valid ? FLinearColor::Green : FLinearColor::Red );
                    DrawMarker( end_world_location, marker.Infos.WindowEndNormalizedSplineDistance, spline_component, marker, pdi, is_valid ? FLinearColor::Blue : FLinearColor::Red );

                    DrawDashedLine( pdi, start_world_location, end_world_location, FLinearColor::Yellow, 16.0f, SDPG_World );
                }
                break;
                default:
                {
                    checkNoEntry();
                }
                break;
            }
        }
    }

    void DrawMarker( FVector & world_location, float spline_length_percentage, const USFSplineComponent & spline_component, const FSFSplineMarker & marker, FPrimitiveDrawInterface * pdi, const FLinearColor & texture_color = FLinearColor::White ) const;

    UTexture2D * BadTexture;
};
