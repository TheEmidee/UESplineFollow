#include "SFSplineMarkerComponentVisualizer.h"

#include "Components/SFSplineComponent.h"
#include "Components/SFSplineMarkers.h"

#include <Components/SplineComponent.h>

FSFSplineMarkerComponentVisualizer::FSFSplineMarkerComponentVisualizer()
{
    BadTexture = LoadObject< UTexture2D >( nullptr, TEXT( "/Engine/EditorResources/Bad" ), nullptr, LOAD_None, nullptr );
}

void FSFSplineMarkerComponentVisualizer::DrawVisualization( const UActorComponent * component, const FSceneView * view, FPrimitiveDrawInterface * pdi )
{
    FSplineComponentVisualizer::DrawVisualization( component, view, pdi );

    if ( const auto * spline_component = Cast< const USFSplineComponent >( component ) )
    {
        DrawMarkers( spline_component->LevelActorActionMarkers, *spline_component, pdi );
        DrawMarkers( spline_component->StaticActionMarkers, *spline_component, pdi );
        DrawMarkers( spline_component->DataMarkers, *spline_component, pdi );
    }
}

void FSFSplineMarkerComponentVisualizer::DrawMarker( FVector & world_location, const float spline_length_percentage, const USFSplineComponent & spline_component, const FSFSplineMarker & marker, FPrimitiveDrawInterface * pdi, const FLinearColor & texture_color ) const
{
    const auto distance = spline_length_percentage * spline_component.GetSplineLength();
    world_location = spline_component.GetWorldLocationAtDistanceAlongSpline( distance );

    const auto * sprite = marker.GetSprite();

    const auto texture_resource = sprite != nullptr
                                      ? sprite->GetResource()
                                      : BadTexture->GetResource();

    const auto color = ( marker.IsValid() && marker.ItIsEnabled )
                           ? texture_color
                           : FLinearColor::Red;

    pdi->DrawSprite( world_location, 64.0f, 64.0f, texture_resource, color, SDPG_World, 0.0f, 0.0f, 0.0f, 0.0f );
}
