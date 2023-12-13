#include "Components/SFSplineMarkers.h"

#include <Misc/DataValidation.h>

bool USFSplineMarkerActorFilter::CanSelectActor_Implementation( AActor * /* actor */ )
{
    return false;
}

UWorld * USFSplineMarkerActorFilter::GetWorld() const
{
    if ( IsTemplate() )
    {
        return nullptr;
    }

    if ( const auto * outer = GetOuter() )
    {
        return outer->GetWorld();
    }

    return nullptr;
}

bool USFSplineMarkerActorFilter_SelectAll::CanSelectActor_Implementation( AActor * /*actor*/ )
{
    return true;
}

bool USFSplineMarkerActorFilter_SelectByClass::CanSelectActor_Implementation( AActor * actor )
{
    if ( ClassToImplement == nullptr )
    {
        return true;
    }

    return actor->IsA( ClassToImplement );
}

#if WITH_EDITOR
EDataValidationResult USFSplineMarkerActorFilter_SelectByClass::IsDataValid( FDataValidationContext & context ) const
{
    Super::IsDataValid( context );

    if ( ClassToImplement == nullptr )
    {
        context.AddError( FText::FromString( TEXT( "ClassToImplement is null" ) ) );
    }

    return context.GetNumErrors() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
#endif

void USFSplineMarkerAction::ProcessAction( AActor * actor, const ESWSplineMarkerProcessActionType action_type, const FSWSplineMarkerInfos & marker_infos ) const
{
    for ( const auto & actor_filter_class : ActorFilters )
    {
        if ( actor_filter_class == nullptr )
        {
            continue;
        }

        if ( !actor_filter_class->GetDefaultObject< USFSplineMarkerActorFilter >()->CanSelectActor( actor ) )
        {
            return;
        }
    }

    switch ( action_type )
    {
        case ESWSplineMarkerProcessActionType::Single:
        {
            ExecuteAction( actor, marker_infos );
        }
        break;
        case ESWSplineMarkerProcessActionType::WindowStart:
        {
            ExecuteStartWindowAction( actor, marker_infos );
        }
        break;
        case ESWSplineMarkerProcessActionType::WindowEnd:
        {
            ExecuteEndWindowAction( actor, marker_infos );
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }
}

UWorld * USFSplineMarkerAction::GetWorld() const
{
    if ( IsTemplate() )
    {
        return nullptr;
    }

    if ( const auto * outer = GetOuter() )
    {
        return outer->GetWorld();
    }

    return nullptr;
}

void USFSplineMarkerAction::ExecuteAction_Implementation( AActor * /*actor*/, const FSWSplineMarkerInfos & /*marker_infos*/ ) const
{
}

void USFSplineMarkerAction::ExecuteStartWindowAction_Implementation( AActor * /*actor*/, const FSWSplineMarkerInfos & /*marker_infos*/ ) const
{
}

void USFSplineMarkerAction::ExecuteEndWindowAction_Implementation( AActor * /*actor*/, const FSWSplineMarkerInfos & /*marker_infos*/ ) const
{
}

void ASWSplineMarkerLevelActor::ProcessAction( AActor * actor, const ESWSplineMarkerProcessActionType action_type, const FSWSplineMarkerInfos & marker_infos ) const
{
    for ( const auto & actor_filter_class : ActorFilters )
    {
        if ( actor_filter_class == nullptr )
        {
            continue;
        }

        if ( !actor_filter_class->GetDefaultObject< USFSplineMarkerActorFilter >()->CanSelectActor( actor ) )
        {
            return;
        }
    }

    switch ( action_type )
    {
        case ESWSplineMarkerProcessActionType::Single:
        {
            ExecuteAction( actor, marker_infos );
        }
        break;
        case ESWSplineMarkerProcessActionType::WindowStart:
        {
            ExecuteStartWindowAction( actor, marker_infos );
        }
        break;
        case ESWSplineMarkerProcessActionType::WindowEnd:
        {
            ExecuteEndWindowAction( actor, marker_infos );
        }
        break;
        default:
        {
            checkNoEntry();
            break;
        }
    }
}

void ASWSplineMarkerLevelActor::ExecuteAction_Implementation( AActor * /*actor*/, const FSWSplineMarkerInfos & /*marker_infos*/ ) const
{
}

void ASWSplineMarkerLevelActor::ExecuteStartWindowAction_Implementation( AActor * /*actor*/, const FSWSplineMarkerInfos & /*marker_infos*/ ) const
{
}

void ASWSplineMarkerLevelActor::ExecuteEndWindowAction_Implementation( AActor * /*actor*/, const FSWSplineMarkerInfos & /*marker_infos*/ ) const
{
}

bool FSWSplineMarker::IsValid() const
{
    return ItIsEnabled;
}

void FSWSplineMarker::AddSplineMarkerProxies( TArray< FSWSplineMarkerProxy > & /*proxies*/ ) const
{
}

UTexture2D * FSWSplineMarker::GetSprite() const
{
    return nullptr;
}

bool FSWSplineMarker_Static::IsValid() const
{
    return Super::IsValid() && ActionClass != nullptr;
}

void FSWSplineMarker_Static::AddSplineMarkerProxies( TArray< FSWSplineMarkerProxy > & proxies ) const
{
    const auto * marker_action = ActionClass->GetDefaultObject< USFSplineMarkerAction >();

    switch ( Infos.Type )
    {
        case ESWSplineMarkerType::Single:
        {
            proxies.Emplace( FSWSplineMarkerProxy( Infos.SingleActionNormalizedSplineDistance, [ marker_action, infos = Infos ]( AActor * actor ) {
                marker_action->ProcessAction( actor, ESWSplineMarkerProcessActionType::Single, infos );
            } ) );
        }
        break;
        case ESWSplineMarkerType::Window:
        {
            proxies.Emplace( FSWSplineMarkerProxy( Infos.WindowStartNormalizedSplineDistance, [ marker_action, infos = Infos ]( AActor * actor ) {
                marker_action->ProcessAction( actor, ESWSplineMarkerProcessActionType::WindowStart, infos );
            } ) );

            proxies.Emplace( FSWSplineMarkerProxy( Infos.WindowEndNormalizedSplineDistance, [ marker_action, infos = Infos ]( AActor * actor ) {
                marker_action->ProcessAction( actor, ESWSplineMarkerProcessActionType::WindowEnd, infos );
            } ) );
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }
}

UTexture2D * FSWSplineMarker_Static::GetSprite() const
{
    if ( ActionClass != nullptr )
    {
        if ( const auto * marker_action = ActionClass->GetDefaultObject< USFSplineMarkerAction >() )
        {
            return marker_action->GetSprite();
        }
    }

    return Super::GetSprite();
}

FSWSplineMarker_LevelActor::FSWSplineMarker_LevelActor()
{
    LevelActor = nullptr;
}

bool FSWSplineMarker_LevelActor::IsValid() const
{
    return Super::IsValid() && LevelActor != nullptr;
}

void FSWSplineMarker_LevelActor::AddSplineMarkerProxies( TArray< FSWSplineMarkerProxy > & proxies ) const
{
    switch ( Infos.Type )
    {
        case ESWSplineMarkerType::Single:
        {
            proxies.Emplace( FSWSplineMarkerProxy( Infos.SingleActionNormalizedSplineDistance, [ level_actor = LevelActor, infos = Infos ]( AActor * actor ) {
                level_actor->ProcessAction( actor, ESWSplineMarkerProcessActionType::Single, infos );
            } ) );
        }
        break;
        case ESWSplineMarkerType::Window:
        {
            proxies.Emplace( FSWSplineMarkerProxy( Infos.WindowStartNormalizedSplineDistance, [ level_actor = LevelActor, infos = Infos ]( AActor * actor ) {
                level_actor->ProcessAction( actor, ESWSplineMarkerProcessActionType::WindowStart, infos );
            } ) );

            proxies.Emplace( FSWSplineMarkerProxy( Infos.WindowEndNormalizedSplineDistance, [ level_actor = LevelActor, infos = Infos ]( AActor * actor ) {
                level_actor->ProcessAction( actor, ESWSplineMarkerProcessActionType::WindowEnd, infos );
            } ) );
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }
}

UTexture2D * FSWSplineMarker_LevelActor::GetSprite() const
{
    if ( LevelActor != nullptr )
    {
        return LevelActor->GetSprite();
    }

    return Super::GetSprite();
}
