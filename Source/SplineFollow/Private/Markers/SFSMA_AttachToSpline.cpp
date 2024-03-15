#include "Markers/SFSMA_AttachToSpline.h"

#include "Components/SFSplineFollowingMovementComponent.h"
#include "Misc/DataValidation.h"

USFSMA_AttachToSpline::USFSMA_AttachToSpline()
{
    NormalizedDistanceOnSpline = 0.0f;
    bEnableMovement = true;
    bLoops = false;
    bAttachToSpline = true;
}

void USFSMA_AttachToSpline::ExecuteAction_Implementation( AActor * actor, const FSFSplineMarkerInfos & marker_infos ) const
{
    FollowNewSplineInternal( actor, marker_infos );
}

void USFSMA_AttachToSpline::ExecuteStartWindowAction_Implementation( AActor * actor, const FSFSplineMarkerInfos & marker_infos ) const
{
    FollowNewSplineInternal( actor, marker_infos );
}

#if WITH_EDITOR
EDataValidationResult USFSMA_AttachToSpline::IsDataValid( FDataValidationContext & context ) const
{
    Super::IsDataValid( context );

    if ( Actor == nullptr )
    {
        context.AddError( FText::FromString( TEXT( "The actor is null" ) ) );
    }

    return context.GetNumErrors() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
#endif

void USFSMA_AttachToSpline::FollowNewSplineInternal( AActor * actor, const FSFSplineMarkerInfos & marker_infos ) const
{
    auto * spline_following_component = actor->FindComponentByClass< USFSplineFollowingMovementComponent >();

    checkf( spline_following_component != nullptr, TEXT( "Spline markers should be executed on actors which follow a spline" ) );

    if ( !ensureAlwaysMsgf( Actor != nullptr, TEXT( "No Spline assigned" ) ) )
    {
        return;
    }

    float rotation_speed;
    bool it_overrides_rotation_speed;

    switch ( RotationSpeedType )
    {
        case ESWAttachToSplineRotationSpeedType::KeepSameRotationSpeed:
        {
            it_overrides_rotation_speed = false;
            rotation_speed = 0.0f;
        }
        break;
        case ESWAttachToSplineRotationSpeedType::ApplyFixedRotationSpeed:
        {
            it_overrides_rotation_speed = true;
            rotation_speed = FixedRotationSpeed;
        }
        break;
        default:
        {
            checkNoEntry();
            it_overrides_rotation_speed = true;
            rotation_speed = 0.0f;
        };
    }

    spline_following_component->FollowSpline(
        FSFFollowSplineInfos {
            Actor.Get(),
            NormalizedDistanceOnSpline,
            static_cast< bool >( bEnableMovement ),
            static_cast< bool >( bLoops ),
            SpeedProviderClassOverride,
            static_cast< bool >( bAttachToSpline ),
            it_overrides_rotation_speed,
            rotation_speed } );
}
