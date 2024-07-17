#include "Components/SFSplineComponent.h"

#include "Components/SFSplineMarkers.h"

#include <Logging/MessageLog.h>

#if WITH_EDITOR
#include <Misc/UObjectToken.h>
#endif

TArray< FSFSplineMarker > USFSplineComponent::GetMarkersByObjectType( TSubclassOf< USFSplineMarkerObject > object_type ) const
{
    TArray< FSFSplineMarker > markers;

    for ( const auto & marker : SplineMarkers )
    {
        if ( marker.Object.IsA( object_type ) )
        {
            markers.Add( marker );
        }
    }

    return markers;
}

TArray< FSFSplineMarker > USFSplineComponent::GetActionMarkers() const
{
    return GetMarkersByObjectType( USFSplineMarkerObject_Action::StaticClass() );
}

TArray< FSFSplineMarker > USFSplineComponent::GetLevelActorMarkers() const
{
    return GetMarkersByObjectType( USFSplineMarkerObject_LevelActor::StaticClass() );
}

TArray< FSFSplineMarker > USFSplineComponent::GetDataMarkers() const
{
    return GetMarkersByObjectType( USFSplineMarkerObject_Data::StaticClass() );
}

void USFSplineComponent::OnRegister()
{
    Super::OnRegister();

    for ( auto & marker : SplineMarkers )
    {
        marker.Infos.UpdateDistances( GetSplineLength() );
    }

    const auto fill_proxies = []( TArray< FSFSplineMarkerProxy > & proxies, const auto & markers ) {
        for ( const auto & marker : markers )
        {
            if ( marker.IsValid() )
            {
                marker.AddSplineMarkerProxies( proxies );
            }
        }
    };

    fill_proxies( SplineMarkerProxies, SplineMarkers );

    SplineMarkerProxies.Sort( []( const auto & left, const auto & right ) {
        return left.SplineNormalizedDistance < right.SplineNormalizedDistance;
    } );
}

void USFSplineComponent::UpdateSpline()
{
    const auto old_length = GetSplineLength();

    Super::UpdateSpline();

    const auto new_length = GetSplineLength();

    const auto recompute_normalized_distance = [ old_length, new_length ]( const auto current_distance ) {
        return FMath::Clamp( current_distance * old_length / new_length, 0.0f, 1.0f );
    };

    const auto set_new_distances = [ recompute_normalized_distance, new_length ]( auto & infos ) {
        infos.SingleActionNormalizedSplineDistance = recompute_normalized_distance( infos.SingleActionNormalizedSplineDistance );
        infos.WindowEndNormalizedSplineDistance = recompute_normalized_distance( infos.WindowEndNormalizedSplineDistance );
        infos.WindowStartNormalizedSplineDistance = recompute_normalized_distance( infos.WindowStartNormalizedSplineDistance );
        infos.UpdateDistances( new_length );
    };

    for ( auto & marker : SplineMarkers )
    {
        set_new_distances( marker.Infos );
    }

    for ( auto & static_action_marker : StaticActionMarkers )
    {
        set_new_distances( static_action_marker.Infos );
    }

    for ( auto & level_actor_action_marker : LevelActorActionMarkers )
    {
        set_new_distances( level_actor_action_marker.Infos );
    }
}

void USFSplineComponent::Serialize( FArchive & archive )
{
    if ( archive.IsSaving() )
    {
        const auto create_marker = [ & ]< typename T, typename U >( T legacy_marker, U object ) {
            return FSFSplineMarker( legacy_marker.Name, legacy_marker.ItIsEnabled, legacy_marker.Infos, object );
        };

        for ( const auto & action_marker : StaticActionMarkers )
        {
            auto new_marker = create_marker( action_marker, NewObject< USFSplineMarkerObject_Action >( this ) );
            Cast< USFSplineMarkerObject_Action >( new_marker.Object )->ActionClass = action_marker.ActionClass->GetOwnerClass();
            SplineMarkers.Emplace( new_marker );
        }

        StaticActionMarkers.Empty();

        for ( const auto & level_actor_marker : LevelActorActionMarkers )
        {
            auto new_marker = create_marker( level_actor_marker, NewObject< USFSplineMarkerObject_LevelActor >( this ) );
            Cast< USFSplineMarkerObject_LevelActor >( new_marker.Object )->LevelActor = level_actor_marker.LevelActor;
            SplineMarkers.Emplace( new_marker );
        }

        LevelActorActionMarkers.Empty();

        for ( const auto & data_marker : DataMarkers )
        {
            auto new_marker = create_marker( data_marker, NewObject< USFSplineMarkerObject_Data >( this ) );
            Cast< USFSplineMarkerObject_Data >( new_marker.Object )->Data = data_marker.Data;
            SplineMarkers.Emplace( new_marker );
        }

        DataMarkers.Empty();
    }

    Super::Serialize( archive );
}

#if WITH_EDITOR
void USFSplineComponent::SaveSplineMarkers( TArray< FSFSplineMarker > markers_to_save )
{
    SplineMarkers.Reset();

    const auto create_marker = [ & ]< typename U >( const auto & marker, U * object ) {
        object->Sprite = marker.Object->Sprite;
        object->Color = marker.Object->Color;
        return FSFSplineMarker( marker.Name, marker.ItIsEnabled, marker.Infos, object );
    };

    for ( const auto & marker : markers_to_save )
    {
        if ( const auto * action_marker = Cast< USFSplineMarkerObject_Action >( marker.Object ) )
        {
            auto new_marker = create_marker( marker, NewObject< USFSplineMarkerObject_Action >( this, marker.Object->GetClass() ) );
            Cast< USFSplineMarkerObject_Action >( new_marker.Object )->ActionClass = action_marker->ActionClass->GetOwnerClass();
            SplineMarkers.Emplace( new_marker );
        }

        if ( const auto * level_actor_marker = Cast< USFSplineMarkerObject_LevelActor >( marker.Object ) )
        {
            auto new_marker = create_marker( marker, NewObject< USFSplineMarkerObject_LevelActor >( this, marker.Object->GetClass() ) );
            Cast< USFSplineMarkerObject_LevelActor >( new_marker.Object )->LevelActor = level_actor_marker->LevelActor;
            SplineMarkers.Emplace( new_marker );
        }

        if ( const auto * data_marker = Cast< USFSplineMarkerObject_Data >( marker.Object ) )
        {
            auto new_marker = create_marker( marker, NewObject< USFSplineMarkerObject_Data >( this, marker.Object->GetClass() ) );
            Cast< USFSplineMarkerObject_Data >( new_marker.Object )->Data = data_marker->Data;
            SplineMarkers.Emplace( new_marker );
        }
    }
}

void USFSplineComponent::CheckForErrors()
{
    Super::CheckForErrors();

    if ( IsTemplate() )
    {
        return;
    }

    const auto check_markers_are_valid = [ spline_component = this ]( const auto & array, const FString & property_name ) {
        for ( const auto & marker : array )
        {
            if ( marker.ActionClass == nullptr )
            {
                FMessageLog( "MapCheck" )
                    .Error()
                    ->AddToken( FUObjectToken::Create( spline_component ) )
                    ->AddToken( FTextToken::Create( FText::FromString( FString::Printf( TEXT( "has an invalid action class in %s" ), *property_name ) ) ) );

                break;
            }
        }
    };

    check_markers_are_valid( StaticActionMarkers, "StaticActionMarkers" );

    for ( const auto & marker : StaticActionMarkers )
    {
        if ( marker.Infos.Type == ESFSplineMarkerType::Single )
        {
            continue;
        }

        if ( marker.Infos.WindowEndNormalizedSplineDistance <= marker.Infos.WindowStartNormalizedSplineDistance )
        {
            FMessageLog( "MapCheck" )
                .Error()
                ->AddToken( FUObjectToken::Create( this ) )
                ->AddToken( FTextToken::Create( FText::FromString( TEXT( "StaticActionMarkers contains an item which has an End marker before the Start marker" ) ) ) );
            break;
        }
    }

    for ( const auto & marker : LevelActorActionMarkers )
    {
        if ( marker.LevelActor == nullptr )
        {
            FMessageLog( "MapCheck" )
                .Error()
                ->AddToken( FUObjectToken::Create( this ) )
                ->AddToken( FTextToken::Create( FText::FromString( TEXT( "LevelActorActionMarkers contains a null item" ) ) ) );
            break;
        }

        if ( marker.Infos.Type == ESFSplineMarkerType::Single )
        {
            continue;
        }

        if ( marker.Infos.WindowEndNormalizedSplineDistance <= marker.Infos.WindowStartNormalizedSplineDistance )
        {
            FMessageLog( "MapCheck" )
                .Error()
                ->AddToken( FUObjectToken::Create( this ) )
                ->AddToken( FTextToken::Create( FText::FromString( TEXT( "LevelActorActionMarkers contains an item which has an End marker before the Start marker" ) ) ) );
            break;
        }
    }
}
#endif
